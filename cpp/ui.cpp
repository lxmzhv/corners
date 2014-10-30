/* Game corners */

#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <assert.h>

#include <ui.h>
#include <game.h>
#include <display.h>
#include <strings.h>

const char* GETTEXT_PACKAGE = "corners";
const char* LOCALEDIR       = "locale";

// Declarations

static gboolean Timer_cb( gpointer data );
static void AITurn();

// Variables

GameOptions Options;
P<Game>     game;
Field       PossibleTurns;

DisplOption DisplOpt = doDEFAULT;
GameModes GameMode = gmHUMAN_AI;

bool Freeze = true;
bool Over = false;
bool Interactive = true;
bool ShowPossibleTurns = false;
Player Winner = pNO;

Point2i MouseDown, MouseMove;
Point2i SelectedCell[2];

GtkWidget    *Window = NULL;
GtkWidget    *DrawingArea = NULL;
GtkWidget    *RightBox = NULL;
GdkGC        *GC = NULL;
GtkStatusbar *StatusBar = NULL;
guint         StatusBarContextID = 0;
GtkWidget    *Menu = NULL;
GtkUIManager *UI = NULL;
GtkRadioButton *P1Human_rb = NULL;
GtkRadioButton *P1AI_rb    = NULL;
GtkRadioButton *P2Human_rb = NULL;
GtkRadioButton *P2AI_rb    = NULL;
const int     InfoCnt = 7;
GtkWidget    *InfoLabels[InfoCnt];
GtkWidget    *ExpLabels[2] = { NULL, NULL };
GtkWidget    *Interactive_tb = NULL;
GtkWidget    *ShowPossibleTurns_tb = NULL;

static Array<GtkActionEntry>       MenuEntries;
static Array<GtkRadioActionEntry>  ColorEntries;
static Array<GtkRadioActionEntry>  ShapeEntries;
static Array<GtkRadioActionEntry>  ModeEntries;
static Array<GtkToggleActionEntry> InvFldEntries;

// Functions
static PlayerType GetPlayer( int player_num )
{
  assert( player_num == 1 || player_num == 2 );
  return (PlayerType)( (GameMode >> (player_num-1)) & 1 );
}

static int GetActivePlayerNum()
{
  return game->GetActive() == pWHITE ? 1 : 2;
}

static PlayerType GetActivePlayer()
{
  return GetPlayer( GetActivePlayerNum() );
}

static Point2i& GetSelCell()
{
  return SelectedCell[GetActivePlayerNum()-1];
}

static void SetStatus( const char* msg )
{
  gtk_statusbar_push( StatusBar, StatusBarContextID, msg );
}

static void UpdateMenus()
{
  GtkWidget *item = NULL;

  item = gtk_ui_manager_get_widget( UI, "/ToolBar/Auto" );
  gtk_widget_set_sensitive( item, Freeze );

  item = gtk_ui_manager_get_widget( UI, "/MenuBar/ActionMenu/Auto" );
  gtk_widget_set_sensitive( item, Freeze );

  item = gtk_ui_manager_get_widget( UI, "/ToolBar/Pause" );
  gtk_widget_set_sensitive( item, !Freeze );

  item = gtk_ui_manager_get_widget( UI, "/MenuBar/ActionMenu/Pause" );
  gtk_widget_set_sensitive( item, !Freeze );

  bool can_undo = game->GetLastTurn( NULL );
  item = gtk_ui_manager_get_widget( UI, "/MenuBar/ActionMenu/Back" );
  gtk_widget_set_sensitive( item, can_undo );

  item = gtk_ui_manager_get_widget( UI, "/ToolBar/Back" );
  gtk_widget_set_sensitive( item, can_undo );

  item = gtk_ui_manager_get_widget( UI, "/MenuBar/ActionMenu/Next" );
  gtk_widget_set_sensitive( item, !Over );

  item = gtk_ui_manager_get_widget( UI, "/ToolBar/Next" );
  gtk_widget_set_sensitive( item, !Over );
}

static void UpdateStatusBar()
{
  const int sz = 0x200;
  char buf[sz] = "";

  if( Over )
  {
    bool enclosed = game->IsEnclosed( pWHITE ) || game->IsEnclosed( pBLACK );
    int  win_gap  = game->GetWinGap();
    if( enclosed )
      snprintf( buf, sz-1, _("Player %d was surrounded"), NextPlayer(Winner) );
    else if( win_gap == 0 )
      strncpy( buf, _("Draw"), sz-1 );
    else
      snprintf( buf, sz-1, _("Player %d finished %d turns later"), NextPlayer(Winner), win_gap );
  }
  else if( Winner != pNO )
    snprintf( buf, sz-1, _("Player %d won"), Winner );
  else
    snprintf( buf, sz-1, _("Player %d turns"), GetActivePlayerNum() );

  buf[sz-1] = 0;
  SetStatus( buf );
}

static void UpdateGameInfo()
{
  const GameStat& stat = game->GetGameStat();
  static String info[InfoCnt];

  const GameStat& stat1 = game->GetPlayer1().GetStat();
  const GameStat& stat2 = game->GetPlayer2().GetStat();
  const Experience& exp1 = game->GetPlayer1().GetExp();
  const Experience& exp2 = game->GetPlayer2().GetExp();

  for( int i = 0; i < InfoCnt; ++i )
    info[i].Clear();

  info[0] = stat.GetCount() + 1;
  info[1] = game->GetCurTurn();
  info[2] << game->GetWins1() << " / " << game->GetWins2();
  info[3] = game->GetDraws();
  info[4].Resize(0x100);
  snprintf( info[4], 0x100, "%.2lf", stat.GetAverage() );

  if( DisplOpt & doSHOW_EXPERIENCE )
  {
    if( exp1.GetFieldNum() > 0 )
      info[5] << exp1.GetFieldNum() << " " << _("fields") << " / "
              << exp1.GetTurnNum() << " " << _("turns");
    if( exp2.GetFieldNum() > 0 )
      info[6] << exp2.GetFieldNum() << " " << _("fields") << " / "
              << exp2.GetTurnNum() << " " << _("turns");

    gtk_widget_show( ExpLabels[0] );
    gtk_widget_show( ExpLabels[1] );
    gtk_widget_show( InfoLabels[5] );
    gtk_widget_show( InfoLabels[6] );
  }
  else
  {
    gtk_widget_hide( ExpLabels[0] );
    gtk_widget_hide( ExpLabels[1] );
    gtk_widget_hide( InfoLabels[5] );
    gtk_widget_hide( InfoLabels[6] );
  }

  for( int i = 0; i < InfoCnt; ++i )
    gtk_label_set_text( GTK_LABEL(InfoLabels[i]), info[i] );
}

static void UpdateWindow()
{
  UpdateGameInfo();
  UpdateStatusBar();
  UpdateMenus();

  gdk_window_invalidate_rect( Window->window, &Window->allocation, TRUE );
}

static void UpdateGameMode()
{
  if( GetPlayer(1) == pHUMAN )
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(P1Human_rb), TRUE );
  else
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(P1AI_rb), TRUE );

  if( GetPlayer(2) == pHUMAN )
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(P2Human_rb), TRUE );
  else
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(P2AI_rb), TRUE );
}

static void Run()
{
  if( !Freeze )
    return;

  Freeze = false;
  GameMode = gmAI_AI;
  SelectedCell[0].Init(-1,-1);
  SelectedCell[1].Init(-1,-1);
  PossibleTurns.InitNull();

  g_idle_add( Timer_cb, NULL );

  UpdateGameMode();
  UpdateWindow();
}

static void Stop()
{
  if( Freeze )
    return;

  Freeze = true;

  if( GameMode == gmAI_AI )
    GameMode = (game->GetActive() == pWHITE ? gmHUMAN_AI : gmAI_HUMAN);

  UpdateGameMode();
  UpdateWindow();
}

static void ToggleFreeze()
{
  Freeze ? Run() : Stop();
}

static void InvertX_cb()
{
  DisplOpt ^= doINVERT_X;
  UpdateWindow();
}

static void InvertY_cb()
{
  DisplOpt ^= doINVERT_Y;
  UpdateWindow();
}

static void DefaultPosition_cb()
{
  DisplOpt &= !(doINVERT_X|doINVERT_Y);
  UpdateWindow();
}

static void Init()
{
  if( Interactive )
    Stop();

  Winner = pNO;
  Over = false;
  game->Init();

  MouseDown.Init( -1, -1 );
  MouseMove.Init( -1, -1 );
  SelectedCell[0].Init( -1, -1 );
  SelectedCell[1].Init( -1, -1 );
  PossibleTurns.Init( game->GetField().Size() );

  UpdateGameMode();
  UpdateWindow();

  if( GetPlayer(1) == pAI )
    AITurn();
}

static void AdjustWindowSize()
{
  GdkScreen* screen = gdk_screen_get_default();

  /* Get screen size, subtract panel size & windows decoration */
  gint width  = gdk_screen_get_width( screen )*95/100;
  gint height = gdk_screen_get_height( screen )*80/100;

  /* Subtract info area size */
  POINT rb_size = { 0, 0 };
  gtk_widget_get_size_request( RightBox, &rb_size.x, &rb_size.y );
  width -= rb_size.x;

  /* Compute cell size */
  gint cell_width  = width  / Options.field_size.x;
  gint cell_height = height / Options.field_size.y;
  gint cell_size   = Min( cell_width, cell_height );

  /* Change field size */
  gtk_window_set_position( GTK_WINDOW(Window), GTK_WIN_POS_CENTER );
  gtk_widget_set_size_request( DrawingArea, cell_size * Options.field_size.x,
                                            cell_size * Options.field_size.y );
}

void NewGame()
{
  POINT sz = Options.field_size;

  AdjustWindowSize();
  game = new Game( sz.x, sz.y, Options.chip_area_size.x, Options.chip_area_size.y,
                   Options.difficulty, Options.ai_opt, Options.ai_deep );
  Init();
}

static void MessageDialog( const char* message )
{
  GtkWidget* dialog = NULL;
  
  dialog = gtk_message_dialog_new(GTK_WINDOW(Window),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_INFO,
                                  GTK_BUTTONS_CLOSE,
                                  message);

  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

static void UpdatePossibleTurns()
{
  const Field& field = game->GetField();
  const Point2i& cell = GetSelCell();

  if( ShowPossibleTurns && field.DoesContain(cell) && field(cell)==game->GetActive() )
    FindPossibleTurns( field, GetSelCell(), PossibleTurns );
  else
    PossibleTurns.InitNull();
}

static void PostTurn()
{
  UpdatePossibleTurns();
  UpdateWindow();

  if( Winner == pNO && game->GetWinner() != pNO )
    Winner = game->GetWinner();

  int win_turn = game->GetWinTurn();

  if( !Over && game->IsOver() )
  {
    Over = true;

    if( Interactive )
    {
      Stop();

      bool enclosed = game->IsEnclosed( pWHITE ) || game->IsEnclosed( pBLACK );
      int  win_gap  = game->GetWinGap();
      Player opponent = NextPlayer(Winner);
      char msg[0x100];

      if( enclosed )
        snprintf( msg, sizeof(msg), _("Player %d has surrounded player %d (%d turns)."), Winner, opponent, win_turn );
      else if( win_gap == 0 )
        snprintf( msg, sizeof(msg), _("Draw (%d turns)."), win_turn );
      else
        snprintf( msg, sizeof(msg), _("Player %d has finished %d turns later."), opponent, win_gap );
      MessageDialog( msg );
    }
    else
      if( !Freeze )
        Init();
  }

  if( Interactive && !Over )
    if( Winner == pWHITE && game->GetCurTurn() == win_turn + 1 && game->GetActive() == pWHITE ||
        Winner == pBLACK && game->GetFinishedTurns() == win_turn && game->GetActive() == pWHITE )
    {
      Stop();

      char msg[0x100];
      snprintf( msg, sizeof(msg), _("Player %d has won (%d turns)."), Winner, win_turn );
      MessageDialog( msg );
    }
}

static void AITurn()
{
  bool notify_skipped = !game->IsFinished( game->GetActive() );

  bool res = game->AITurn();
  PostTurn();

  if( Interactive && notify_skipped && !res )
  {
    Stop();

    /*MessageBox( widget, "Игрок пропустил свой ход.", "Сообщение", MB_OK );*/
  }
}

static gboolean Timer_cb( gpointer data )
{
  if( Freeze )
    return FALSE;

  AITurn();
  return TRUE;
}

static bool TryManualTurn( const Point2i& from, const Point2i& to )
{
  if( from == to )
    return false;

  if( !game->ManualTurn( from, to, DisplOpt & doEVALUATE_TURNS ) )
    return false;

  PostTurn();

  if( GetActivePlayer() == pAI ) 
    AITurn();
  else if( game->IsFinished(game->GetActive()) )
  {
    game->SkipTurn();
    PostTurn();
  }

  return true;
}

void GetCellCoor( const Point2i& p, Point2i& cell_pnt )
{
  Point2i   sz( DrawingArea->allocation.width,
                DrawingArea->allocation.height );
  DisplCoor dspl( sz, DisplOpt );

  const Field& field = game->GetField();
  int       max_x = field.Size().x, max_y = field.Size().y;
  double    dx = double(sz.x)/max_x, dy = double(sz.y)/max_y;

  cell_pnt.Init( int(dspl.X(p.x)/dx), int(dspl.Y(p.y)/dy) );
}

static void SelectCell( Point2i p, Player player )
{
  Point2i cell;
  GetCellCoor( p, cell );

  const Field& field = game->GetField();
  if( GameMode != gmAI_AI && field.DoesContain(cell) && field(cell) == player )
  {
    SelectedCell[player==pWHITE?0:1] = cell;
    UpdatePossibleTurns();
    UpdateWindow();
  }
}

static gboolean DrawField_cb( GtkWidget *widget, GdkEventExpose *event, gpointer )
{
  Point2i* cur = NULL;

  if( !Over && MouseMove.x >= 0 && MouseMove.y >= 0 &&
      game->GetField()(GetSelCell()) == game->GetActive() )
    cur = &MouseMove;

  DrawField( widget, GC, DisplOpt, GetSelCell(), cur, PossibleTurns );

  return TRUE;
}

static gboolean MouseDown_cb( GtkWidget *widget,
                              GdkEventButton *event,
                              gpointer )
{
  if( event->type != GDK_BUTTON_PRESS )
    return FALSE;

  switch( event->button )
  {
    case 1:
    {
      MouseDown.x = (int)event->x;
      MouseDown.y = (int)event->y;

      Point2i from, to;
      from = GetSelCell();
      GetCellCoor( MouseDown, to );

      Player player = game->GetActive();
      TryManualTurn( from, to );
      SelectCell( MouseDown, player );

      return TRUE;
    }
    case 3:
    {
      GetSelCell().Init( -1, -1 );
      PossibleTurns.InitNull();
      UpdateWindow();

      GtkWidget *menu = gtk_ui_manager_get_widget( UI, "/LocalMenu" );
      gtk_menu_popup( GTK_MENU(menu), NULL, NULL, NULL, NULL,
                      event->button, event->time );
      return TRUE;
    }
  }

  return FALSE;
}

static gboolean MouseMove_cb( GtkWidget *widget,
                              GdkEventMotion *event,
                              gpointer )
{
  if( event->state & GDK_BUTTON1_MASK )
  {
    MouseMove.Init( (int)event->x, (int)event->y );
    UpdateWindow();
  }

  return FALSE;
}

static gboolean MouseUp_cb( GtkWidget *widget,
                            GdkEventButton *event,
                            gpointer)
{
  if( event->type != GDK_BUTTON_RELEASE )
    return FALSE;

  Point2i up, from, to;

  up.x = (int)event->x;
  up.y = (int)event->y;

  GetCellCoor( MouseDown, from );
  GetCellCoor( up, to );

  Player player = game->GetActive();
  if( TryManualTurn( from, to ) )
    SelectCell( up, player );

  MouseDown.Init( -1, -1 ); 
  MouseMove.Init( -1, -1 ); 

  UpdateWindow();

  return FALSE;
}

static void RollBack()
{
  game->Rollback( GameMode == gmHUMAN_HUMAN ? 1 : 2 );

  if( game->GetWinner() == pNO )
    Winner = pNO;
  if( !game->IsOver() )
    Over = false;

  Stop();
  UpdateWindow();
}

static void ChangeColors()
{
  ThemeIdx = (ColorThemeIndex)((ThemeIdx + 1)%COLOR_THEME_NUMBER);
  UpdateWindow();
}

static void ChangeShape()
{
  Shape = (Shape == SHAPE_CIRCLE) ? SHAPE_SQUARE : SHAPE_CIRCLE;
  UpdateWindow();
}

static void AutoTurn()
{
  Stop();

  PlayerType first = GetActivePlayer();

  AITurn();

  if( first == pHUMAN && GetActivePlayer() == pAI )
    AITurn();
  else if( game->IsFinished(game->GetActive()) )
  {
    game->SkipTurn();
    PostTurn();
  }
}

static gboolean KeyDown_cb( GtkWidget   *widget,
                            GdkEventKey *event,
                            gpointer )
{
  bool control = (event->state & GDK_CONTROL_MASK) != 0;

  switch( event->keyval )
  {
    case GDK_Up:
    case GDK_Down:
      if( !control )
        return FALSE;
      DisplOpt ^= doINVERT_Y;
      break;
    case GDK_Left:
    case GDK_Right:
      if( !control )
        return FALSE;
      DisplOpt ^= doINVERT_X;
      break;
    case GDK_c:
      ChangeColors();
      return TRUE;
    case GDK_e:
      DisplOpt ^= doEVALUATE_TURNS;
      break;
    case GDK_i:
      Interactive = !Interactive;
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Interactive_tb), Interactive );
      break;
    case GDK_s:
      ChangeShape();
      return TRUE;
    case GDK_t:
      ShowPossibleTurns = !ShowPossibleTurns;
      gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(ShowPossibleTurns_tb), ShowPossibleTurns );
      break;
    case GDK_x:
      DisplOpt ^= doSHOW_EXPERIENCE;
      break;
    default:
      return FALSE;
  }

  UpdateWindow();
  return TRUE;
}

static gboolean Quit_cb()
{
  gtk_main_quit();
  return FALSE;
}

static void MenuQuit_cb( GtkAction *action )
{
  gtk_main_quit();
}

static void Colors_cb( GtkAction *action, GtkRadioAction *current )
{
  ThemeIdx = (ColorThemeIndex)gtk_radio_action_get_current_value( current );

  UpdateWindow();
}

static void Shape_cb( GtkAction *action, GtkRadioAction *current )
{
  Shape = (ChipShape)gtk_radio_action_get_current_value( current );

  UpdateWindow();
}

static void ChangePlayer( GtkToggleButton *tb, int player_num )
{
  gboolean active  = gtk_toggle_button_get_active( tb );
  int      bit_num = player_num - 1;

  GameMode = (GameModes)( GameMode & ~(1 << bit_num) );
  GameMode = (GameModes)( GameMode | ((active ? pHUMAN : pAI) << bit_num) );

  // If user has changed a player to be a computer then do AI turn
  if( player_num == GetActivePlayerNum() && GetActivePlayer() == pAI )
    AITurn();

  UpdateWindow();
}

static void ChangePlayer1_cb( GtkToggleButton *tb, gpointer )
{
  ChangePlayer( tb, 1 );
}

static void ChangePlayer2_cb( GtkToggleButton *tb, gpointer )
{
  ChangePlayer( tb, 2 );
}

static void Interactive_cb( GtkToggleButton *tb, gpointer )
{
  Interactive = gtk_toggle_button_get_active( tb );
}

static void ShowPossibleTurns_cb( GtkToggleButton *tb, gpointer )
{
  ShowPossibleTurns = gtk_toggle_button_get_active( tb );
  UpdatePossibleTurns();
  UpdateWindow();
}

static void ActionInit( GtkActionEntry *action,
                         const gchar *name,
                         const gchar *stock_id,
                         const gchar *label,
                         const gchar *accelerator,
                         const gchar *tooltip,
                         GCallback callback )
{
  action->name        = name;
  action->stock_id    = stock_id;
  action->label       = label;
  action->accelerator = accelerator;
  action->tooltip     = tooltip;
  action->callback    = callback;
}

static void RadioActionInit( GtkRadioActionEntry *action,
                             const gchar *name,
                             const gchar *stock_id,
                             const gchar *label,
                             const gchar *accelerator,
                             const gchar *tooltip,
                             gint value )
{
  action->name        = name;
  action->stock_id    = stock_id;
  action->label       = label;
  action->accelerator = accelerator;
  action->tooltip     = tooltip;
  action->value       = value;
}

static void ToggleActionInit( GtkToggleActionEntry *action,
                              const gchar *name,
                              const gchar *stock_id,
                              const gchar *label,
                              const gchar *accelerator,
                              const gchar *tooltip,
                              GCallback callback,
                              gboolean is_active )
{
  action->name        = name;
  action->stock_id    = stock_id;
  action->label       = label;
  action->accelerator = accelerator;
  action->tooltip     = tooltip;
  action->callback    = callback;
  action->is_active   = is_active;
}

static void CreateMenuEntries()
{
  /* name, stock id, label, accelerator, tooltip, callback */
  ActionInit( MenuEntries.Append(1), "GameMenu",        0, _("_Game"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "ActionMenu",      0, _("_Action"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "DisplayMenu",     0, _("_Display"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "HelpMenu",        0, _("_Help"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "ColorMenu", GTK_STOCK_SELECT_COLOR, _("_Color"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "ShapeMenu",       0, _("_Chips"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "ModeMenu",        0, _("_Mode"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "PositionMenu",    0, _("_Position"), 0, 0, 0 );
  ActionInit( MenuEntries.Append(1), "New",   GTK_STOCK_NEW,  _("_New"),  "<control>N", _("New game"), G_CALLBACK(Init) );
  ActionInit( MenuEntries.Append(1), "Quit",  GTK_STOCK_QUIT, _("_Quit"), "<control>Q", _("Quit"), G_CALLBACK(MenuQuit_cb) );
  ActionInit( MenuEntries.Append(1), "About", GTK_STOCK_ABOUT,_("_About"), NULL,        _("About"), G_CALLBACK(About) );
  ActionInit( MenuEntries.Append(1), "Rules", GTK_STOCK_HELP, _("_Rules"), NULL,        _("Rules"), G_CALLBACK(Rules) );
  ActionInit( MenuEntries.Append(1), "Next",  GTK_STOCK_REDO, _("_Next"), "N",          _("Let AI turn"), G_CALLBACK(AutoTurn) );
  ActionInit( MenuEntries.Append(1), "Back",  GTK_STOCK_UNDO, _("_Back"), "B",          _("Undo turn"), G_CALLBACK(RollBack) );
  ActionInit( MenuEntries.Append(1), "Auto",  GTK_STOCK_MEDIA_PLAY,  _("_Auto"), "A",   _("Run AI vs AI game"), G_CALLBACK(ToggleFreeze) );
  ActionInit( MenuEntries.Append(1), "Pause", GTK_STOCK_MEDIA_PAUSE, _("_Pause"), "P",  _("Stop AI vs AI game"), G_CALLBACK(Stop) );
  ActionInit( MenuEntries.Append(1), "ChangeColors", GTK_STOCK_SELECT_COLOR, _("_Change colors"), "C",  _("Change color scheme"), G_CALLBACK(ChangeColors) );
  ActionInit( MenuEntries.Append(1), "Preferences", GTK_STOCK_PREFERENCES, _("_Preferences"), "<control>P",  _("Preferences"), G_CALLBACK(Preferences) );
  ActionInit( MenuEntries.Append(1), "DefaultPosition", NULL, _("_Default"), "<control>Home", _("Default field position"), G_CALLBACK(DefaultPosition_cb) );
}

static void CreateColorEntries()
{
  /* name, stock id, label, accelerator, tooltip, value */ 
  RadioActionInit( ColorEntries.Append(1), "Classic", NULL, _("_Classic"), NULL, _("Classic theme"), COLOR_THEME_CLASSIC );
  RadioActionInit( ColorEntries.Append(1), "Dark",  NULL, _("_Dark"),  NULL, _("Dark theme"), COLOR_THEME_DARK );
  RadioActionInit( ColorEntries.Append(1), "Light", NULL, _("_Light"), NULL, _("Light theme"), COLOR_THEME_LIGHT );
}

static void CreateShapeEntries()
{
  /* name, stock id, label, accelerator, tooltip, value */ 
  RadioActionInit( ShapeEntries.Append(1), "Square", NULL, _("_Square"), NULL, _("Square"), SHAPE_SQUARE );
  RadioActionInit( ShapeEntries.Append(1), "Circle", NULL, _("_Circle"), NULL, _("Circle"), SHAPE_CIRCLE );
}

static void CreateInvertFieldEntries()
{
  /* name, stock_id, label, accelerator, tooltip, GCallback callback, is_active */
  ToggleActionInit( InvFldEntries.Append(1), "InvertX", 0, _("Invert _X"), 0, _("Rotate field horizontally"), G_CALLBACK(InvertX_cb), FALSE );
  ToggleActionInit( InvFldEntries.Append(1), "InvertY", 0, _("Invert _Y"), 0, _("Rotate field vertically"),   G_CALLBACK(InvertY_cb), FALSE );
}

static const char *UIInfo = "\
<ui>\
  <menubar name='MenuBar'>\
    <menu action='GameMenu'>\
      <menuitem action='New'/>\
      <menuitem action='Preferences'/>\
      <separator/>\
      <menuitem action='Quit'/>\
    </menu>\
    <menu action='ActionMenu'>\
      <menuitem action='Next'/>\
      <menuitem action='Back'/>\
      <separator/>\
      <menuitem action='Auto'/>\
      <menuitem action='Pause'/>\
    </menu>\
    <menu action='DisplayMenu'>\
      <menu action='ShapeMenu'>\
        <menuitem action='Square'/>\
        <menuitem action='Circle'/>\
      </menu>\
      <menu action='ColorMenu'>\
        <menuitem action='Classic'/>\
        <menuitem action='Dark'/>\
        <menuitem action='Light'/>\
      </menu>\
      <menu action='PositionMenu'>\
        <menuitem action='InvertX'/>\
        <menuitem action='InvertY'/>\
        <menuitem action='DefaultPosition'/>\
      </menu>\
    </menu>\
    <menu action='HelpMenu'>\
      <menuitem action='Rules'/>\
      <menuitem action='About'/>\
    </menu>\
  </menubar>\
  <popup name='LocalMenu'>\
    <menuitem action='Next'/>\
    <menuitem action='Back'/>\
    <separator/>\
    <menuitem action='Auto'/>\
    <menuitem action='Pause'/>\
  </popup>\
  <toolbar  name='ToolBar'>\
    <toolitem action='New'/>\
    <separator/>\
    <toolitem action='Back'/>\
    <toolitem action='Next'/>\
    <separator/>\
    <toolitem action='Auto'/>\
    <toolitem action='Pause'/>\
    <separator/>\
    <toolitem action='ChangeColors'/>\
    <toolitem action='Preferences'/>\
  </toolbar>\
</ui>";

static void CreateMenu()
{
  if( UI != NULL )
    return;

  CreateMenuEntries();
  CreateColorEntries();
  CreateShapeEntries();
  CreateInvertFieldEntries();

  GtkActionGroup *actions = gtk_action_group_new( "Actions" );
  gtk_action_group_add_actions( actions, MenuEntries, MenuEntries.Size(), Window );
  gtk_action_group_add_radio_actions( actions, ColorEntries, ColorEntries.Size(),
                                      COLOR_THEME_DEFAULT, G_CALLBACK(Colors_cb), NULL);
  gtk_action_group_add_radio_actions( actions, ShapeEntries, ShapeEntries.Size(),
                                      SHAPE_DEFAULT, G_CALLBACK(Shape_cb), NULL);
  gtk_action_group_add_toggle_actions( actions, InvFldEntries,
                                       InvFldEntries.Size(), NULL );

  UI = gtk_ui_manager_new();
  gtk_ui_manager_insert_action_group( UI, actions, 0 );

  GError *error = NULL;
  if( gtk_ui_manager_add_ui_from_string( UI, UIInfo, -1, &error ) )
    return;

  g_message( "Building menus failed: %s", error->message );
  g_error_free( error );
}

static GtkWidget* CreateInfoTable()
{
  GtkWidget *hbox, *vbox1, *vbox2, *label;

  hbox  = gtk_hbox_new( FALSE, 0 );
  vbox1 = gtk_vbox_new( FALSE, 0 );
  vbox2 = gtk_vbox_new( FALSE, 0 );

  gtk_box_pack_start( GTK_BOX(hbox), vbox1, FALSE, FALSE, 10 );
  gtk_box_pack_start( GTK_BOX(hbox), vbox2, FALSE, FALSE, 10 );

  const char* fields[InfoCnt] = { _("game:"), _("turn:"), _("wins:"),
    _("draws:"), _("score:"), _("experience 1:"), _("experience 2:") };

  for( int i = 0; i < InfoCnt; ++i )
  {
    label         = gtk_label_new( fields[i] );
    InfoLabels[i] = gtk_label_new( "" );

    gtk_misc_set_alignment( GTK_MISC(label), 1, 0 );
    gtk_misc_set_alignment( GTK_MISC(InfoLabels[i]), 0, 0 );

    gtk_box_pack_start( GTK_BOX(vbox1), label,         TRUE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(vbox2), InfoLabels[i], TRUE, FALSE, 0 );

    if( i == 5 )
      ExpLabels[0] = label;
    else if( i == 6 )
      ExpLabels[1] = label;
  }

  return hbox;
}

static GtkWidget* CreatePlayerOption()
{
  GtkWidget *p1_label = NULL,
            *p2_label = NULL,
            *p1h_rb = NULL,
            *p1c_rb = NULL,
            *p2h_rb = NULL,
            *p2c_rb = NULL;
  GtkBox    *hbox3 = NULL,
            *vbox_p1 = NULL,
            *vbox_p2 = NULL;

  // Create widgets

  p1_label = gtk_label_new( _("Player 1") );
  p1h_rb = gtk_radio_button_new_with_label( NULL, _("Human") );
  P1Human_rb = GTK_RADIO_BUTTON( p1h_rb );
  p1c_rb = gtk_radio_button_new_with_label_from_widget( P1Human_rb, _("Computer") );
  P1AI_rb = GTK_RADIO_BUTTON( p1c_rb );

  p2_label = gtk_label_new( _("Player 2") );
  p2h_rb = gtk_radio_button_new_with_label( NULL, _("Human") );
  P2Human_rb = GTK_RADIO_BUTTON( p2h_rb );
  p2c_rb = gtk_radio_button_new_with_label_from_widget( P2Human_rb, _("Computer") );
  P2AI_rb = GTK_RADIO_BUTTON( p2c_rb );

  hbox3   = GTK_BOX(gtk_hbox_new( FALSE, 0 ));
  vbox_p1 = GTK_BOX(gtk_vbox_new( FALSE, 0 ));
  vbox_p2 = GTK_BOX(gtk_vbox_new( FALSE, 0 ));


  // Lay the widgets on-screen
  
  gtk_box_pack_start( hbox3, GTK_WIDGET(vbox_p1), FALSE, FALSE, 0 );

  gtk_box_pack_start( vbox_p1, p1_label, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox_p1, p1h_rb, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox_p1, p1c_rb, FALSE, FALSE, 0 );

  gtk_box_pack_start( hbox3,   GTK_WIDGET(vbox_p2), FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox_p2, p2_label, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox_p2, p2h_rb, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox_p2, p2c_rb, FALSE, FALSE, 0 );

  // Connect handlers

  g_signal_connect( G_OBJECT( p1h_rb ), "toggled",
                    G_CALLBACK( ChangePlayer1_cb ), NULL );

  g_signal_connect( G_OBJECT( p2h_rb ), "toggled",
                    G_CALLBACK( ChangePlayer2_cb ), NULL );

  return GTK_WIDGET(hbox3);
}

static void CreateDialog()
{
  GtkWidget *label = NULL,
            *statusbar = NULL,
            *vbox0 = NULL,
            *hbox1 = NULL,
            *menu_bar = NULL,
            *tool_bar = NULL;
  
  CreateMenu();

  /* Create widgets used by the application */

  Window      = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  DrawingArea = gtk_drawing_area_new();
  statusbar   = gtk_statusbar_new();
  StatusBar   = GTK_STATUSBAR(statusbar);
  StatusBarContextID = gtk_statusbar_get_context_id( StatusBar, "" );
  vbox0       = gtk_vbox_new( FALSE, 0 );
  hbox1       = gtk_hbox_new( FALSE, 0 );
  RightBox    = gtk_vbox_new( FALSE, 0 );
  Interactive_tb = gtk_check_button_new_with_label( _("Interactive") );
  ShowPossibleTurns_tb = gtk_check_button_new_with_label( _("Show possible turns") );

  menu_bar = gtk_ui_manager_get_widget(UI, "/MenuBar");
  tool_bar = gtk_ui_manager_get_widget(UI, "/ToolBar");

  /* Lay the widgets on-screen */

  gtk_container_add( GTK_CONTAINER(Window), vbox0 );

  gtk_box_pack_start( GTK_BOX(vbox0), menu_bar, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(vbox0), tool_bar, FALSE, FALSE, 0 );
  gtk_container_add( GTK_CONTAINER(vbox0), hbox1 );
  gtk_box_pack_end( GTK_BOX(vbox0), statusbar, FALSE, FALSE, 0 );

  gtk_container_add( GTK_CONTAINER( hbox1 ), DrawingArea );
  gtk_box_pack_end( GTK_BOX(hbox1), RightBox, FALSE, FALSE, 0 );

  gtk_box_pack_start( GTK_BOX(RightBox), CreatePlayerOption(), FALSE, FALSE, 10 );
  gtk_box_pack_start( GTK_BOX(RightBox), Interactive_tb,       FALSE, FALSE, 5 );
  gtk_box_pack_start( GTK_BOX(RightBox), ShowPossibleTurns_tb, FALSE, FALSE, 5 );
  gtk_box_pack_end( GTK_BOX(RightBox), CreateInfoTable(), TRUE, FALSE, 0 );

  /* Set properties */

  gtk_window_add_accel_group( GTK_WINDOW(Window), gtk_ui_manager_get_accel_group(UI) );
  gtk_window_set_title( GTK_WINDOW(Window), _("Corners") );
  gtk_widget_set_double_buffered( DrawingArea, TRUE );
  gtk_widget_set_events( DrawingArea, gtk_widget_get_events(DrawingArea)
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_POINTER_MOTION_MASK );
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Interactive_tb), Interactive );
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(ShowPossibleTurns_tb), ShowPossibleTurns );
  AdjustWindowSize();

  /* Connect handlers */

  g_signal_connect( G_OBJECT( Window ),  "delete-event",
                    G_CALLBACK( Quit_cb ), NULL );

  g_signal_connect( G_OBJECT( DrawingArea ), "expose-event",
                    G_CALLBACK( DrawField_cb ), NULL );

  g_signal_connect( G_OBJECT( DrawingArea ), "button-press-event",
                    G_CALLBACK( MouseDown_cb ), NULL );

  g_signal_connect( G_OBJECT( DrawingArea ), "motion-notify-event",
                    G_CALLBACK( MouseMove_cb ), NULL );

  g_signal_connect( G_OBJECT( DrawingArea ), "button-release-event",
                    G_CALLBACK( MouseUp_cb ), NULL );

  g_signal_connect( G_OBJECT( Window ), "key-press-event",
                    G_CALLBACK( KeyDown_cb ), NULL );

  g_signal_connect( G_OBJECT( Interactive_tb ), "toggled",
                    G_CALLBACK( Interactive_cb ), NULL );

  g_signal_connect( G_OBJECT( ShowPossibleTurns_tb ), "toggled",
                    G_CALLBACK( ShowPossibleTurns_cb ), NULL );
}

static void InitDrawing()
{
  if( GC == NULL )
    GC = gdk_gc_new( GDK_DRAWABLE( DrawingArea->window ) );
}

static void CleanupDrawing()
{
  g_object_unref( GC );
}

int main( int argc, char *argv[] )
{
  /* Set up i18n parameters */
  bindtextdomain( GETTEXT_PACKAGE, LOCALEDIR );
  bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
  textdomain( GETTEXT_PACKAGE );

  gtk_init( &argc, &argv );

  CreateDialog();

  gtk_widget_show_all( Window );
  gtk_window_set_focus( GTK_WINDOW(Window), NULL );

  /* Create new drawing context */
  InitDrawing();

  /* Init game */
  NewGame();

  /* Run */
  gtk_main();

  /* Cleanup */
  CleanupDrawing();

  return 0;
}

#include <ui.h>
#include <display.h>
#include <stdio.h>

GdkColor RGB( double red, double green, double blue )
{
  GdkColor col;

  col.pixel = 0;
  col.red   = int(red*0xFFFF);
  col.green = int(green*0xFFFF);
  col.blue  = int(blue*0xFFFF);

  return col;
}

ColorTheme LightTheme =
{
  RGB( 1, 1, 1 ),      // field
  RGB( 0, 0.4, 0 ),    // player 1
  RGB( 0, 0.4, 0.4 ),  // player 2
  RGB( 0, 0, 0 ),      // grid
  RGB( 1, 1, 0.63 ),   // target area
  RGB( 1, 0, 0 ),      // selected cell border
  RGB( 1, 0, 0 ),      // arrows
  RGB( 0, 0, 0.5 ),    // experience arrows
  RGB( 0.7, 0.7, 0.7 ) // possible turn 
};

ColorTheme DarkTheme =
{
  RGB( 0, 0, 0 ),          // field
  RGB( 0.63, 0.63, 0 ),    // player 1
  RGB( 0, 0.63, 0 ),       // player 2
  RGB( 0.19, 0.19, 0.19 ), // grid
  RGB( 0.19, 0, 0 ),       // target area
  RGB( 0.5, 0, 0 ),        // selected cell border
  RGB( 0.5, 0, 0 ),        // arrows
  RGB( 0.7, 0.7, 0.7 ),    // experience arrows
  RGB( 0.1, 0.1, 0.1 )  // possible turn 
};

ColorTheme ClassicTheme =
{
  RGB( 0.34, 0.20, 0 ),    // field
  RGB( 1, 1, 1 ),          // player 1
  RGB( 0, 0, 0 ),          // player 2
  RGB( 0.08, 0.04, 0 ),    // grid
  RGB( 0.40, 0.20, 0 ),    // target area
  RGB( 0.7, 0, 0 ),        // selected cell border
  RGB( 0.7, 0, 0 ),        // arrows
  RGB( 0.7, 0.7, 0.7 ),    // experience arrows
  RGB( 0.5, 0.3, 0 )       // possible turn 
};

/* The order of themes is synchronous with the enum ColorThemeIndex */
ColorTheme* ColorThemes[COLOR_THEME_NUMBER] = { &ClassicTheme, &DarkTheme, &LightTheme };
ColorThemeIndex ThemeIdx = COLOR_THEME_DEFAULT;
ChipShape Shape = SHAPE_DEFAULT;

GdkColor GetFieldColor( const Point2i& p, Point2i p1[2], Point2i p2[2] )
{
  for( int i = 0; i < 2; ++i )
    if( p.x >= p1[i].x && p.x <= p2[i].x && p.y >= p1[i].y && p.y <= p2[i].y )
      return ColorThemes[ThemeIdx]->target_area;

  return ColorThemes[ThemeIdx]->field;
}

static void SetLineWidth( GdkGC *gc, int width )
{
  gdk_gc_set_line_attributes( gc, width, GDK_LINE_SOLID,
                              GDK_CAP_NOT_LAST, GDK_JOIN_MITER );
}

static void DrawArrow( GdkDrawable* drawable, GdkGC* gc, const DisplCoor& dspl,
                       const Point2i& from, const Point2i& to, GdkColor color )
{
  const Field&  field = game->GetField();
  POINT sz = dspl.Size();

  if( !field.DoesContain( from ) || !field.DoesContain( to ) || from == to )
    return;

  int max_x = field.Size().x, max_y = field.Size().y;
  double dx = double(sz.x)/max_x, dy = double(sz.y)/max_y;

  gdk_gc_set_rgb_fg_color( gc, &color );
  SetLineWidth( gc, 1 );

  Point2i p_from( int((from.x+0.5)*dx), int((from.y+0.5)*dy) );
  Point2i p_to( int((to.x+0.5)*dx), int((to.y+0.5)*dy) );

  gdk_draw_line( drawable, gc, dspl.X(p_from.x), dspl.Y(p_from.y),
                               dspl.X(p_to.x), dspl.Y(p_to.y) );

  double l = Max( Min(dx,dy)/5, 2.0 );
  Point2i v, p1, p2, p3;

  v = p_from - p_to;
  double len = Length_2d(v);
  if( fabs(len) < NULL_DOUBLE )
    return;
  v.Init( int(v.x*l/len), int(v.y*l/len) );
  p1 = p_to + v;

  GdkPoint arrow[3];
  const int divider = 4;
  arrow[0].x = dspl.X(p_to.x);
  arrow[0].y = dspl.Y(p_to.y);

  arrow[1].x = dspl.X(p1.x - v.y/divider);
  arrow[1].y = dspl.Y(p1.y + v.x/divider);

  arrow[2].x = dspl.X(p1.x + v.y/divider);
  arrow[2].y = dspl.Y(p1.y - v.x/divider);

  gdk_draw_polygon( drawable, gc, TRUE, arrow, 3 );
}

static void DrawArrowNum( GtkWidget* widget, GdkDrawable* drawable, GdkGC* gc,
                          const DisplCoor& dspl, const Point2i& from, const Point2i& to,
                          double value )
{
  static char buf[0x100];
  POINT sz = dspl.Size();

  const Field& field = game->GetField();
  int max_x = field.Size().x, max_y = field.Size().y;
  double dx = double(sz.x)/max_x, dy = double(sz.y)/max_y;

  Point2i c1( dspl.X(dx*from.x+dx/2), dspl.Y(dy*from.y+dy/2) );
  Point2i c2( dspl.X(dx*to.x+dx/2), dspl.Y(dy*to.y+dy/2) );
  sprintf( buf, "%.2f", value );

  PangoLayout *layout = gtk_widget_create_pango_layout( widget, buf );
  gdk_draw_layout( drawable, gc, (c1.x+c2.x)/2, (c1.y+c2.y)/2, layout );
}

static void DrawLastTurn( GtkWidget* widget, GdkDrawable* drawable, GdkGC* gc,
                          const DisplCoor& dspl, DisplOption opt )
{
   GameTurn turn;
   for( int i = 0; i < 2; ++i )
   {
     game->GetLastTurn( &turn, i );
     if( turn.from != turn.to )
     {
       DrawArrow( drawable, gc, dspl, turn.from, turn.to, ColorThemes[ThemeIdx]->arrow );
       if( opt & doEVALUATE_TURNS && turn.diff != NO_DIFF )
         DrawArrowNum( widget, drawable, gc, dspl, turn.from, turn.to, turn.diff );
     }
   }
}

static void DrawExpTurns( GtkWidget* widget, GdkDrawable* drawable, GdkGC* gc, const DisplCoor& dspl )
{
   const Experience& exp = game->GetActive() == pWHITE ?
                           game->GetPlayer1().GetExp() :
                           game->GetPlayer2().GetExp();
   const FieldStateExp* fs_exp = exp.Find( game->GetField() );

   if( !fs_exp )
      return;

   const Point2i *from = 0, *to = 0;
   double average = 0.0;
   int cnt = fs_exp->GetTurnNum();
   const TurnExpPtr* turn_exp = fs_exp->GetTurnExp();

   for( int i = cnt-1; i >= 0; --i )
   {
      from = &turn_exp[i]->GetFrom();
      to = &turn_exp[i]->GetTo();
      average = turn_exp[i]->GetStat().GetAverage();

      DrawArrow( drawable, gc, dspl, *from, *to, ColorThemes[ThemeIdx]->exp_arrow );
      DrawArrowNum( widget, drawable, gc, dspl, *from, *to, average );
   }
}

static void DrawChip( GdkDrawable* drawable, GdkGC* gc, Player player, int x, int y, int dx, int dy, DrawChipOptions opt )
{
  GdkColor color = (player == pWHITE ? ColorThemes[ThemeIdx]->player1 :
                                       ColorThemes[ThemeIdx]->player2);
  gdk_gc_set_rgb_fg_color( gc, &color );

  switch( Shape )
  {
    case SHAPE_SQUARE:
      gdk_draw_rectangle( drawable, gc, TRUE, x+1, y+1, int(dx-2), int(dy-2) );
      break;
    case SHAPE_CIRCLE:
      gdk_draw_arc( drawable, gc, TRUE, x+1, y+1, int(dx-2), int(dy-2), 0, 64*360 );
      break;
  }

  if( (opt&dcoSELECTED)==0 )
    return;

  /* Draw border */

  SetLineWidth( gc, 3 );
  gdk_gc_set_rgb_fg_color( gc, &ColorThemes[ThemeIdx]->selected );

  switch( Shape )
  {
    case SHAPE_SQUARE:
      gdk_draw_rectangle( drawable, gc, FALSE, x, y, int(dx), int(dy) );
      break;
    case SHAPE_CIRCLE:
      gdk_draw_arc( drawable, gc, FALSE, x, y, int(dx), int(dy), 0, 64*360 );
      break;
  }
}

void DrawField( GtkWidget* widget, GdkGC* gc, DisplOption opt, POINT selected,
                const Point2i* moving, const Field& possible_turns )
{
  GdkDrawable *drawable = GDK_DRAWABLE( widget->window );
  POINT     sz = { widget->allocation.width, widget->allocation.height };
  Point2i   p, p1[2], p2[2];
  const Field& field = game->GetField();
  int       max_x = field.Size().x, max_y = field.Size().y;
  int       x, y;
  double    dx = double(sz.x)/max_x, dy = double(sz.y)/max_y;
  Player    cell, active = game->GetActive();
  GdkColor  color;
  DisplCoor dspl( sz, opt );

  GetTargetArea( field, game->CountX(), game->CountY(), pWHITE, p1[0], p2[0] );
  GetTargetArea( field, game->CountX(), game->CountY(), pBLACK, p1[1], p2[1] );

  // Fill the field with grid color
  gdk_gc_set_rgb_fg_color( gc, &ColorThemes[ThemeIdx]->grid );
  gdk_draw_rectangle( drawable, gc, TRUE, 0, 0, sz.x, sz.y );

  // Draw all the cells
  for( p.x = 0; p.x < max_x; ++p.x )
    for( p.y = 0; p.y < max_y; ++p.y )
    {
      cell = (p == selected ? pNO : field(p));

      x = Min( dspl.X(dx*p.x), dspl.X(dx*(p.x+1)) );
      y = Min( dspl.Y(dy*p.y), dspl.Y(dy*(p.y+1)) );

      // Draw the cell
      if( cell == pNO && possible_turns(p) == pWHITE )
        color = ColorThemes[ThemeIdx]->possible_turn;
      else
        color = GetFieldColor( p, p1, p2 );
      gdk_gc_set_rgb_fg_color( gc, &color );
      gdk_draw_rectangle( drawable, gc, TRUE, x+1, y+1, int(dx-2), int(dy-2) );

      // Draw the player chip
      if( cell == pWHITE || cell == pBLACK )
        DrawChip( drawable, gc, cell, x, y, (int)dx, (int)dy, dcoNONE );
    }

  // Draw border of the target areas
  SetLineWidth( gc, 3 );
  gdk_gc_set_rgb_fg_color( gc, &ColorThemes[ThemeIdx]->grid );

  GdkPoint v1, v2;
  for( int i = 0; i < 2; ++i )
  {
    v1.x = dspl.X(dx*p1[i].x);
    v1.y = dspl.Y(dy*p1[i].y);

    v2.x = dspl.X(dx*(p2[i].x+1));
    v2.y = dspl.Y(dy*(p2[i].y+1));

    x = Min( v1.x, v2.x );
    y = Min( v1.y, v2.y );

    gdk_draw_rectangle( drawable, gc, FALSE, x, y,
                        abs(v2.x-v1.x), abs(v2.y-v1.y) );
  }

  // Draw selected cell
  p = selected;
  Player player = field.DoesContain(p) ? field(p) : pNO;
  if( player == pBLACK || player == pWHITE )
  {
    if( moving )
    {
      Point2i to;
      GetCellCoor( *moving, to );
      if( field.DoesContain(to) )
        p = to;
    }

    x = Min( dspl.X(dx*p.x), dspl.X(dx*(p.x+1)) );
    y = Min( dspl.Y(dy*p.y), dspl.Y(dy*(p.y+1)) );

    /* draw cell */
    DrawChip( drawable, gc, player, x, y, (int)dx, (int)dy, dcoSELECTED );
  }

  // Draw experience
  if( opt & doSHOW_EXPERIENCE )
    DrawExpTurns( widget, drawable, gc, dspl );

  // Draw last turn arrow
  DrawLastTurn( widget, drawable, gc, dspl, opt );
}

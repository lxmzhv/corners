#include <ui.h>
#include <stdio.h>

class PrefDialog
{
  public:
    PrefDialog()
    {
      dialog   = NULL;
      x_entry  = NULL;
      y_entry  = NULL;
      n_entry  = NULL;
      m_entry  = NULL;
      ai_level = NULL;
      ai_deep  = NULL;
      friendly_ai = NULL;
    }

    GtkWidget *dialog;
    GtkWidget *x_entry;
    GtkWidget *y_entry;
    GtkWidget *n_entry;
    GtkWidget *m_entry;
    GtkWidget *ai_level;
    GtkWidget *ai_deep;
    GtkWidget *friendly_ai;
};

static void gtk_entry_set_int( GtkWidget* entry, int value )
{
  gchar buf[0x100];

  sprintf( buf, "%d", value );
  gtk_entry_set_text( GTK_ENTRY(entry), buf );
}

static int gtk_entry_get_int( GtkWidget* entry )
{
  const gchar* text = gtk_entry_get_text( GTK_ENTRY(entry) );
  return atoi( text );
}

static GtkWidget* gtk_entry_num_create()
{
  GtkWidget *entry = gtk_entry_new();

  gtk_entry_set_max_length( GTK_ENTRY(entry), 8 );
  gtk_entry_set_width_chars( GTK_ENTRY(entry), 8 );

  return entry;
}

static void pref_create_ai_level_frame( PrefDialog& dlg )
{
  GtkWidget *frame = NULL,
            *consider_label = NULL;
  GtkBox    *vbox = NULL;

  /* Create widgets */

  frame = gtk_frame_new( _("Difficulty") );
  vbox = GTK_BOX(gtk_vbox_new( FALSE, 0 ));
  gtk_container_set_border_width( GTK_CONTAINER(vbox), 5 );

  dlg.ai_level = gtk_combo_box_new_text();
  gtk_combo_box_append_text( GTK_COMBO_BOX(dlg.ai_level), _("Normal (AI 1)") );
  gtk_combo_box_append_text( GTK_COMBO_BOX(dlg.ai_level), _("Hard (AI 2)") );

  consider_label = gtk_label_new( _("Number of considered turns") );
  gtk_misc_set_alignment( GTK_MISC(consider_label), 0, 0 );

  dlg.ai_deep = gtk_entry_num_create();
  dlg.friendly_ai = gtk_check_button_new_with_label( _("Consider opponent turns") );

  /* Pack widgets */

  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dlg.dialog)->vbox), frame, FALSE, FALSE, 0 );
  gtk_container_add( GTK_CONTAINER(frame), GTK_WIDGET(vbox) );
  gtk_box_pack_start( vbox, dlg.ai_level, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox, consider_label, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox, dlg.ai_deep, FALSE, FALSE, 0 );
  gtk_box_pack_start( vbox, dlg.friendly_ai, FALSE, FALSE, 10 );
}

static GtkWidget* pref_create_xy_frame( const gchar* frame_label,
                                        const gchar* label1,
                                        const gchar* label2,
                                        GtkWidget**  p_entry1,
                                        GtkWidget**  p_entry2 )
{
  GtkWidget *frame = NULL,
            *xy_hbox  = NULL,
            *x_label = NULL,
            *x_entry = NULL,
            *y_label = NULL,
            *y_entry = NULL;

  frame = gtk_frame_new( frame_label );

  xy_hbox = gtk_hbox_new( FALSE, 0 );
  gtk_container_set_border_width( GTK_CONTAINER(xy_hbox), 5 );

  x_label = gtk_label_new( label1 );
  x_entry = gtk_entry_num_create();

  y_label = gtk_label_new( label2 );
  y_entry = gtk_entry_num_create();

  gtk_container_add( GTK_CONTAINER(frame), xy_hbox );

  gtk_box_pack_start( GTK_BOX(xy_hbox), x_label, FALSE, FALSE, 0 );
  gtk_box_pack_start( GTK_BOX(xy_hbox), x_entry, TRUE,  TRUE,  7 );
  gtk_box_pack_start( GTK_BOX(xy_hbox), y_label, FALSE, FALSE, 7 );
  gtk_box_pack_start( GTK_BOX(xy_hbox), y_entry, TRUE,  TRUE,  0 );

  *p_entry1 = x_entry;
  *p_entry2 = y_entry;

  return frame;
}

static void pref_set_values( PrefDialog *dlg, GameOptions* opt )
{
  gtk_entry_set_int( dlg->x_entry, opt->field_size.x );
  gtk_entry_set_int( dlg->y_entry, opt->field_size.y );

  gtk_entry_set_int( dlg->n_entry, opt->chip_area_size.x );
  gtk_entry_set_int( dlg->m_entry, opt->chip_area_size.y );

  gtk_combo_box_set_active( GTK_COMBO_BOX(dlg->ai_level), opt->difficulty );

  gtk_entry_set_int( dlg->ai_deep, opt->ai_deep + 1 );

  bool hinder_turns = (opt->ai_opt & aioFRIENDLY) == 0;
  gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(dlg->friendly_ai), hinder_turns );
}

static bool pref_get_values( PrefDialog *dlg, GameOptions* opt )
{
  POINT size, chip_area_size;
  int   ai_deep;

  size.x = gtk_entry_get_int( dlg->x_entry );
  size.y = gtk_entry_get_int( dlg->y_entry );

  chip_area_size.x = gtk_entry_get_int( dlg->n_entry );
  chip_area_size.y = gtk_entry_get_int( dlg->m_entry );

  ai_deep = gtk_entry_get_int( dlg->ai_deep ) - 1;

  if( size.x < 2 || size.y < 2 || size.x > 100 || size.y > 100 )
    return false;

  if( chip_area_size.x < 1 || chip_area_size.y < 1 ||
      chip_area_size.x > size.x/2 || chip_area_size.y > size.y/2 )
    return false;

  if( ai_deep < 0 )
    return false;

  if( !opt )
    return true;

  opt->field_size = size;
  opt->chip_area_size = chip_area_size;
  opt->difficulty = (AIDifficulty) gtk_combo_box_get_active( GTK_COMBO_BOX(dlg->ai_level) );
  opt->ai_deep = ai_deep;

  if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(dlg->friendly_ai) ) )
    opt->ai_opt = AIOption(opt->ai_opt & ~aioFRIENDLY);
  else
    opt->ai_opt = AIOption(opt->ai_opt | aioFRIENDLY);

  return true;
}

static bool pref_changed( PrefDialog *dlg )
{
  bool valid = pref_get_values( dlg, NULL );
  gtk_dialog_set_response_sensitive( GTK_DIALOG(dlg->dialog), GTK_RESPONSE_OK, valid );
  return valid;
}

static gboolean pref_changed_cb( GtkWidget *widget, GdkEventFocus *event, PrefDialog* dlg )
{
  pref_changed( dlg );
  return FALSE;
}

static void pref_apply_cb( GtkEntry *entry, PrefDialog* dlg )
{
  if( pref_changed(dlg) )
    gtk_dialog_response( GTK_DIALOG(dlg->dialog), GTK_RESPONSE_OK );
}

void Preferences( GtkAction *action, GtkWidget* window )
{
  PrefDialog dlg;
  GtkWidget* widget = NULL;
  GtkBox*    box = NULL;

  /* Construct dialog */

  dlg.dialog = gtk_dialog_new_with_buttons( _("Preferences"), GTK_WINDOW(window),
                  (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                  NULL );
  gtk_dialog_set_default_response( GTK_DIALOG(dlg.dialog), GTK_RESPONSE_OK );

  box = GTK_BOX(GTK_DIALOG(dlg.dialog)->vbox);

  widget = pref_create_xy_frame( _("Field size"), "x", "y", &dlg.x_entry, &dlg.y_entry );
  gtk_box_pack_start( box, widget, FALSE, FALSE, 10 );

  widget = pref_create_xy_frame( _("Chip area size"), "x", "y", &dlg.n_entry, &dlg.m_entry );
  gtk_box_pack_start( box, widget, FALSE, FALSE, 10 );

  pref_create_ai_level_frame( dlg );

  /* Set callbacks */

  GtkWidget* validating_widgets[] = { dlg.x_entry, dlg.y_entry, dlg.n_entry, dlg.m_entry, dlg.ai_deep, NULL };
  for( GtkWidget** p_entry = validating_widgets; *p_entry; ++p_entry )
  {
    g_signal_connect( G_OBJECT(*p_entry), "activate", G_CALLBACK(pref_apply_cb), &dlg );
    g_signal_connect( G_OBJECT(*p_entry), "focus-out-event", G_CALLBACK(pref_changed_cb), &dlg );
  }

  /* Set values */

  pref_set_values( &dlg, &Options );

  /* Run */

  gtk_widget_show_all( dlg.dialog );
  gint response = gtk_dialog_run( GTK_DIALOG(dlg.dialog) );

  /* Get values */

  if( response == GTK_RESPONSE_OK &&
      pref_get_values( &dlg, &Options ) )
    NewGame();

  /* Destroy the dialog */

  gtk_widget_destroy( dlg.dialog );
}

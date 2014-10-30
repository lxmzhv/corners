#include <ui.h>
#include <string.h>

#if GTK_MAJOR_VERSION >= 2 && GTK_MINOR_VERSION > 4

void About( GtkAction* action, GtkWidget* window )
{
  const gchar* authors[] = { _("Alexey Mozhaev"), NULL };
  const gchar* comments = _("A logical game as known as Halma.\nDedicated to River Tam.");

  gtk_show_about_dialog( GTK_WINDOW(window),
                         "name", _("Corners"),
                         "program-name", _("Corners"),
                         "version", "1.4",
                         "authors", authors,
                         "translator-credits", authors[0],
                         "comments", comments,
                         "license", _("License"),
                         "wrap-license", TRUE,
                         NULL );
}

#else

void About( GtkAction*, GtkWidget* ) {}

#endif

void Rules( GtkAction* action, GtkWidget* window )
{
  const gchar* rules = _("Rules text");

  GtkWidget *dialog = NULL,
            *text_view = NULL;
  GtkTextBuffer *buffer = NULL;

  dialog = gtk_dialog_new_with_buttons( _("Rules"), GTK_WINDOW(window),
                  (GtkDialogFlags)(GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT),
                  GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                  NULL );

  text_view = gtk_text_view_new();

  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
  gtk_text_buffer_set_text( buffer, rules, strlen(rules) );

  gtk_text_view_set_editable( GTK_TEXT_VIEW(text_view), FALSE );
  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD );

  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->vbox), text_view, TRUE, TRUE, 10 );

  gtk_window_set_position( GTK_WINDOW(dialog), GTK_WIN_POS_CENTER );
  gtk_widget_set_size_request( text_view, 500, 550 );

  gtk_widget_show_all( dialog );
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}

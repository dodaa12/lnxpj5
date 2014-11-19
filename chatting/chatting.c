#include <gtk/gtk.h>


int main( int argc, char *argv[])
{

  GtkWidget *window;
  GtkWidget *fixed;
  GtkWidget *input;
  GtkWidget *entry;
  GtkWidget *chat_list,*id_list;
 

  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_widget_set_size_request (window, 830, 470);
  gtk_window_set_title(GTK_WINDOW(window), "채팅창");
  gtk_window_set_resizable(GTK_WINDOW(window),TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(window), 15);

  fixed = gtk_fixed_new();
  gtk_container_add(GTK_CONTAINER(window), fixed);
  
  
  entry = gtk_entry_new();
  gtk_container_add(GTK_CONTAINER(window), entry);
  gtk_widget_set_size_request(entry, 510, 30);
  gtk_fixed_put(GTK_FIXED(fixed), entry, 5, 400);
  
  input = gtk_button_new_with_label("입력");
  gtk_container_add(GTK_CONTAINER(window), input);
  gtk_widget_set_size_request(input, 70, 30);
  gtk_fixed_put(GTK_FIXED(fixed), input, 520, 400);

  chat_list = gtk_text_view_new ();
  gtk_container_add(GTK_CONTAINER(window), chat_list);
  gtk_widget_set_size_request(chat_list, 500, 300);
  gtk_fixed_put(GTK_FIXED(fixed), chat_list, 10, 50);

  id_list = gtk_text_view_new ();
  gtk_container_add(GTK_CONTAINER(window), id_list);
  gtk_widget_set_size_request(id_list, 250, 250);
  gtk_fixed_put(GTK_FIXED(fixed), id_list, 540, 70);

  g_signal_connect_swapped(G_OBJECT(window), "destroy",
        G_CALLBACK(gtk_main_quit), G_OBJECT(window));

  gtk_widget_show_all(window);
  gtk_main();

  return 0;
}

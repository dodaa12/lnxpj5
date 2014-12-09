#include <gtk/gtk.h>
typedef struct _Data Data;
struct _Data{
   GtkWidget *window;
   GtkWidget *button;
   GtkWidget *label;
};

G_MODULE_EXPORT void
quit (GtkWidget *window, gpointer data){
   gtk_main_quit();
}

int main(int argc, char argv[]){
   GtkBuilder *builder;
   GError *error;
   Data *data;
   gtk_init(&argc,&argv);
   /*빌더 생성 및 UI 파일 열기*/
   builder = gtk_builder_new();
   if(!gtk_builder_add_from_file(builder,"chat2.ui",&error)){
      g_print("UI파일을 읽을 때 오류 발생!\n");
      g_print("메시지 : %s\n",error->message);
      g_free(error);

      return(1);
   }
   data = g_slice_new(Data);
   data->window = GTK_WIDGET(gtk_builder_get_object(builder,"window1"));
   data->button = GTK_WIDGET(gtk_builder_get_object(builder,"text_input"));
   data->label = GTK_WIDGET(gtk_builder_get_object(builder,"text_view_name"));
   gtk_builder_connect_signals (builder,data);
   g_object_unref(G_OBJECT(builder));
   gtk_widget_show_all(data->window);
   gtk_main();
   g_slice_free(Data,data);
   return(0);
}

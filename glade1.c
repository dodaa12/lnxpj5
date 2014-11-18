#include <gtk/gtk.h> 
typedef struct _Data Data; 
struct _Data 
{ 
GtkWidget *window; 
GtkWidget *label1;
GtkWidget *label2; 
GtkWidget *label3; 
GtkWidget *label4;
GtkWidget *textview1;  
GtkWidget *textview2;
GtkWidget *textview3;
GtkWidget *textview4;
GtkWidget *statusbar1;
}; 
G_MODULE_EXPORT 
int main (int argc, char *argv[]) 
{ 
GtkBuilder *builder; 
GError *error; 
Data *data;
gtk_init (&argc, &argv); 
/* 빌더 생성 및 UI 파일 열기 */ 
builder = gtk_builder_new (); 
if ( !gtk_builder_add_from_file (builder, "test.ui", &error)) 
{
 g_print ("UI 파일을 읽을 때 오류 발생!\n");
 g_print ("메시지: %s\n", error->message); 
g_free (error); 
return (1); 
}
data = g_slice_new (Data); 
data->window = GTK_WIDGET (gtk_builder_get_object (builder, "window1")); 
data->label1 = GTK_WIDGET (gtk_builder_get_object (builder, "label1"));
data->label2 = GTK_WIDGET (gtk_builder_get_object (builder, "label2"));
data->label3 = GTK_WIDGET (gtk_builder_get_object (builder, "label3"));
data->label4 = GTK_WIDGET (gtk_builder_get_object (builder, "label4"));
data->textview1 = GTK_WIDGET (gtk_builder_get_object (builder, "textview1"));
data->textview2 = GTK_WIDGET (gtk_builder_get_object (builder, "textview2"));
data->textview3 = GTK_WIDGET (gtk_builder_get_object (builder, "textview3"));
data->textview4 = GTK_WIDGET (gtk_builder_get_object (builder, "textview4"));
data->statusbar1 = GTK_WIDGET (gtk_builder_get_object (builder, "statusbar1"));
g_object_unref (G_OBJECT (builder)); 
gtk_widget_show_all (data->window); 
gtk_main (); 
g_slice_free (Data, data); 
return (0);
}

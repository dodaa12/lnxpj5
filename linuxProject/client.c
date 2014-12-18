#include <gtk/gtk.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXCLIENT 10

char username[20];
// 접속 화면 위젯 데이터
typedef struct _Data Data; 
struct _Data { 
	GtkWidget *window; 
	GtkWidget *btn_connect; 
	GtkWidget *btn_exit; 
	GtkWidget *label;
	GtkWidget *entry;
}; 

//GtkWidget *window;
// 리스트 뷰
GtkWidget *window;
GtkWidget *view;
GtkListStore *store;
GtkTreeIter iter, search;
GtkCellRenderer *renderer;
GtkTreePath *path;
GtkTreeViewColumn *col;
GtkTreeSelection *selection;
///////////////////////////

// 채팅 화면 위젯 데이터
typedef struct _cData cData; 
struct _cData { 
	GtkWidget *window; 
	GtkWidget *btn_enter; 
	GtkWidget *entry;
	GtkWidget *mtextview;
	GtkWidget *ptextview;
}; 
cData *cdata;

/* 아래 enum 은 열을 구분하는 데 유용 */
enum
{
	COLUMN_1,
	COLUMN_2,
	COLUMN_3,
	N_COLUMNS
};
// 종료 시그널 처리
void quit ( GtkWidget *window, gpointer data)
{
	gtk_main_quit ();
}
// 접속 버튼 시그널 처리
G_MODULE_EXPORT void on_connect_clicked (GtkButton *button, Data *data) { 
	const gchar *name; // 대화명
	// 대화명 받아오기
	name = gtk_entry_get_text (GTK_ENTRY(data->entry));
	strcpy(username, name);
	int connect_id=0;
	// 서버에 접속
	if((connect_id = connect_server()) == 1){		
		gtk_widget_destroy (data->window);
		waitroom();
	}
}
// exit 버튼 시그널 처리
G_MODULE_EXPORT void on_exit_clicked (GtkButton *button, Data *data) { 
	gtk_main_quit ();
}
// 최초 화면
int main (int argc, char *argv[]) { 
	GtkBuilder *builder; GError *error; Data *data;
	gtk_init (&argc, &argv); 
	/* 빌더 생성 및 UI 파일 열기 */ 
	builder = gtk_builder_new (); 
	if ( !gtk_builder_add_from_file (builder, "frame1.ui", &error)) { 
		g_print ("UI 파일을 읽을 때 오류 발생!\n"); 
		g_print ("메시지: %s\n", error->message); 
		g_free (error); 
		return (1); 
	} 
	data = g_slice_new (Data); 
	data->window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
	data->btn_connect = GTK_WIDGET (gtk_builder_get_object (builder, "connect")); 
	data->btn_exit = GTK_WIDGET (gtk_builder_get_object (builder, "exit")); 
	data->label = GTK_WIDGET (gtk_builder_get_object (builder, "title"));
	data->entry = GTK_WIDGET (gtk_builder_get_object (builder, "getname"));
	gtk_builder_connect_signals (builder, data);	
	g_object_unref (G_OBJECT (builder)); 
	gtk_widget_show_all (data->window); 
	gtk_main (); g_slice_free (Data, data); 
	return (0); 
}
int iDs; // 디스크립터 저장
int iaClient[MAXCLIENT]; // 접속 가능한 인원 수 - 소켓 번호를 가지고 있다.
fd_set fsStatus; // select()에 사용되는 소켓 식별자 셋
int iCounter; // 반복문을 위한 변수
int iDSize; 
int iRet;
unsigned char my_room_id[4]; // 내 대화방 식별자
unsigned char create_room_id[4]; // 대화방 생성시 식별자
unsigned char recv_room_id[4]; // 서버로 부터 받은 메세지의 대화방 식별자
unsigned char sendBuf[256], recvBuf[256];
struct sockaddr_in stAddr; // 서버 주소 구조체
int connect_server()
{   
   iDSize = sizeof(struct sockaddr_in);
   bzero(&stAddr, iDSize); // 0으로 구조체 초기화  
   // TCP를 사용하여 스트림 소켓을 연다.(반환형은 int)
   // 첫번째 인자는 IP protocol family - 2층 선택
  // 두번째 인자는 TCP설정, 세번째 인자는 Transmission Control Protocol - 3층
  iDs = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   if(iDs<0) // 예외 처리 - 소켓 생성에 실패했을 때
    {
     perror("socket() failed");
     close(iDs); // 열었던 소켓을 닫는다.  
         return -10;
     }
   stAddr.sin_family = AF_INET; // socket()의 첫번째 인자와 같다.
   // 주소 변환
  iRet = inet_pton(AF_INET, "127.0.0.1", &stAddr.sin_addr.s_addr);
   if(iRet == 0)
   {
     printf("inet_pton() failed... invalid address string");
     close(iDs); // 열었던 소켓을 닫는다.  
     return -100;
   }
   else if(iRet < 0)
   {
     perror("inet_pton() failded");  
     close(iDs); // 열었던 소켓을 닫는다.  
     return -100;
   }
   // 서버 포트(포트 문을 열어준다.)
   stAddr.sin_port = htons(8000); // 포트 8000번 연다.
           // 여기까지가 기본 세팅이며 client와 동일하다.
   
   if(connect(iDs, (struct sockaddr *)&stAddr, iDSize))
   {
     perror("connect() failed");
     close(iDs);
     return -10;
   }
   return 1;  
 }
int unconnect_server(){
	close(iDs);
	return 1;
}
void set_room(){
	//stycpy(my_room_id, "0001");
}
// 대화방 생성 버튼 시그널 처리
void btn_create(GtkWidget *button, gpointer data){
	// 다이얼로그 생성
	GtkWidget *dialog;
	GtkWidget *d_roomname; // 방 제목 텍스트
	GtkWidget *d_create; // 생성 버튼
	GtkWidget *d_cancel; // 취소 버튼
	GtkWidget *label;
	gint result;
	dialog = gtk_dialog_new_with_buttons ("대화방 생성", GTK_WINDOW (window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_YES, GTK_RESPONSE_YES, GTK_STOCK_NO, GTK_RESPONSE_NO, NULL);
	label = gtk_label_new ("대화방을 생성 하시겠습니까?");
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, TRUE, TRUE, 30);
	gtk_widget_show_all (dialog);
	result = gtk_dialog_run (GTK_DIALOG (dialog));
	// selected row iter 식별
	int *i;
	GtkTreeModel *tm ;
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
	if(gtk_tree_selection_get_selected (selection, &tm, &iter)){
		path = gtk_tree_model_get_path (tm, &iter);
		i = gtk_tree_path_get_indices(path);
	}
	gtk_tree_path_free (path);	

	switch (result) {
	case GTK_RESPONSE_YES:		
		gtk_list_store_set (store, &iter, COLUMN_2, 1, COLUMN_3, username, -1);
		break;
	case GTK_RESPONSE_NO:
		break;
	default:
		break;
	}
	gtk_widget_destroy (dialog);	
	gtk_widget_show_all(window);	
}
int btn_enter(GtkWidget *button, gpointer data){
	// selected row iter 식별	
	int *i;
	GtkTreeModel *tm ;
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(view));
	if(gtk_tree_selection_get_selected (selection, &tm, &iter)){
		path = gtk_tree_model_get_path (tm, &iter);
		i = gtk_tree_path_get_indices(path);
	}	
	switch(i[0]){
	case 0 :
		strcpy(my_room_id, "0001");
		break;
	case 1 :
		strcpy(my_room_id, "0002");
		break;
	case 2 :
		strcpy(my_room_id, "0003");
		break;
	case 3 :
		strcpy(my_room_id, "0004");
		break;
	case 4 :
		strcpy(my_room_id, "0005");
		break;
	case 5 :
		strcpy(my_room_id, "0006");
		break;
	case 6 :
		strcpy(my_room_id, "0007");
		break;
	case 7 :
		strcpy(my_room_id, "0008");
		break;
	case 8 :
		strcpy(my_room_id, "0009");
		break;
	case 9 :
		strcpy(my_room_id, "0010");
		break;
	}
	gtk_tree_path_free (path);
	gtk_widget_destroy (window);
	chattingroom();
	return -1;
}
int waitroom ()
{	
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *enter;  // 방 입장 버튼
	GtkWidget *create; // 방 생성 버튼	

	int i;
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "대기실");
	gtk_window_set_default_size ( GTK_WINDOW(window), 500, 300);
	g_signal_connect ( GTK_OBJECT (window), "destroy", GTK_SIGNAL_FUNC (quit), NULL);
	// 리스트 뷰 생성
	store = gtk_list_store_new (N_COLUMNS, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
	vbox = gtk_vbox_new (FALSE, 0);
	hbox = gtk_hbox_new (FALSE, 0);
	view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), COLUMN_1, "방번호", renderer, "text", COLUMN_1, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), COLUMN_2, "인원", renderer, "text", COLUMN_2, NULL);
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW(view), COLUMN_3, "방장", renderer, "text", COLUMN_3, NULL);
	
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0001", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0002", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0003", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0004", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0005", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0006", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0007", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0008", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0009", COLUMN_2, 0, COLUMN_3, "", -1);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set (store, &iter, COLUMN_1, "0010", COLUMN_2, 0, COLUMN_3, "", -1);

	// 방 입장 버튼 생성
	enter = gtk_button_new_with_label("대화방 입장");
	// 방 생성 버튼 생성
	create = gtk_button_new_with_label("대화방 생성");
	// v_box에 붙이기
	gtk_box_pack_start(GTK_BOX(vbox), view, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
	// h_box에 붙이기
	gtk_box_pack_start(GTK_BOX(hbox), create, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), enter, TRUE, TRUE, 0);	
	// 방 입장 버튼 시그널 연결
	g_signal_connect(GTK_OBJECT(enter), "clicked", GTK_SIGNAL_FUNC(btn_enter), NULL);
	// 방 생성 버튼 시그널 연결
	g_signal_connect(GTK_OBJECT(create), "clicked", GTK_SIGNAL_FUNC(btn_create), NULL);	
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_container_add(GTK_CONTAINER(window), hbox);	
	gtk_widget_show_all(window);
	gtk_main ();
	return 0;
}
// 문자열 수신 쓰레드
void *recv_thread (void *arg)
{
	while(1){
		// 소켓 식별자 벡터를 '0'으로 초기화 하고 서버 소켓이 사용하도록 설정
		FD_ZERO(&fsStatus); // select()가 호출된후 매번 '0'으로 초기화
		// 랑데뷰 소켓(3번 셋트, 1로 체크해 준다.)
		// 그룹은 총 랑데뷰, 키보드, 커뮤니케이션(아래쪽)
		FD_SET(iDs, &fsStatus);  // 디스크립터, 소켓식별자 벡터
		FD_SET(0, &fsStatus); // select가 0번(서버의 키보드 입력)까지 감시한다.
		// 파일디스크립터, 입력, 출력, 에러, 타임아웃(감시하는 시간-NULL은 무한대기)
		// 해당하는 소켓에 입력이 없으면 BLOCK 된다.
		if(0 > select(iDs+1, &fsStatus, NULL, NULL, NULL))
		{
			perror("select() error... ");
			close(iDs);
			return arg;
		}
		// server가 보낸 입력을 받을 때
		if(FD_ISSET(iDs, &fsStatus))
		{
			fflush(stdin);
			memset(recvBuf, 0, 256);
			// 저수준으로 읽는다. recvBuf 길이를 알기 위해 iRet로 반환값을 저장한다.
			iRet = read(iDs, recvBuf, sizeof(recvBuf)) - 1;
			recvBuf[iRet] = '\0';
			strncpy(recv_room_id, recvBuf, 4);
			recv_room_id[4] = '\0';
			if(!strcmp(my_room_id, recv_room_id)){
				int i=0, j=0;
				for(i = 4, j = 0 ; i<iRet ; i++, j++){
					recvBuf[j] = recvBuf[i];
				}
				recvBuf[iRet-4] = '\0';
				// 채팅 내용 삽입
				GtkTextBuffer *buffer;
				GtkTextIter iter;
				buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cdata->mtextview)) ;
				gtk_text_buffer_get_iter_at_offset(buffer, &iter, -1);
				gtk_text_buffer_insert(buffer, &iter, recvBuf, -1);
				gtk_text_buffer_insert(buffer, &iter, "\n", -1);
			}
		}		
	}  
	return arg;
}
// 전송 버튼 시그널 처리 (문자열 송신)
G_MODULE_EXPORT int on_enter_clicked (GtkButton *button, cData *data) {	
	const gchar *gmsg; // entry 입력 메세지	
	char sendmsg[260];

	// 입력 메세지 받아오기(entry)
	gmsg = gtk_entry_get_text (GTK_ENTRY(cdata->entry));
	
	FD_ZERO(&fsStatus);
	fflush(stdin);
	memset(sendBuf, 0, 256);
	strcpy(sendmsg, my_room_id);
	strcat(sendmsg, gmsg);
	strcpy(sendBuf, sendmsg);
	iRet = strlen(sendBuf);
	write(iDs, sendBuf, iRet+1);
	gtk_entry_set_text (GTK_ENTRY(cdata->entry), "");		
	return 0;
}
int chattingroom(){
	pthread_t tid;
	int status;		
	/* 쓰레드 생성 */
	status = pthread_create (&tid, NULL, recv_thread, NULL);
	if (status != 0)
		perror ("Create thread");	
	GtkBuilder *builder; GError *error;
	/* 빌더 생성 및 UI 파일 열기 */ 
	builder = gtk_builder_new (); 
	if ( !gtk_builder_add_from_file (builder, "chattingroom.ui", &error)) { 
		g_print ("UI 파일을 읽을 때 오류 발생!\n"); 
		g_print ("메시지: %s\n", error->message); 
		g_free (error); 
		return (1); 
	} 
	cdata = g_slice_new (cData); 
	cdata->window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
	cdata->btn_enter = GTK_WIDGET (gtk_builder_get_object (builder, "enter")); 
	cdata->mtextview = GTK_WIDGET (gtk_builder_get_object (builder, "textview1")); 
	cdata->ptextview = GTK_WIDGET (gtk_builder_get_object (builder, "textview2"));
	cdata->entry = GTK_WIDGET (gtk_builder_get_object (builder, "entry1"));
	gtk_builder_connect_signals (builder, cdata);	
	// 채팅방 대화명 삽입
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(cdata->ptextview)) ;
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, -1);
	gtk_text_buffer_insert(buffer, &iter, username, -1);
	gtk_text_buffer_insert(buffer, &iter, "\n", -1);

	g_object_unref (G_OBJECT (builder)); 
	gtk_widget_show_all (cdata->window); 
	gtk_main (); 
	g_slice_free (cData, cdata); 	
	return (0); 	
}

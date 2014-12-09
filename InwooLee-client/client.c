// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXCLIENT 2

int main()
 {  
   int iDs; // 디스크립터 저장
   int iaClient[MAXCLIENT]; // 접속 가능한 인원 수 - 소켓 번호를 가지고 있다.
   
   fd_set fsStatus; // select()에 사용되는 소켓 식별자 셋
   int iCounter; // 반복문을 위한 변수
   int iDSize; 
   int iRet;
   unsigned char my_room_id[4]; // 대화방 식별자
   unsigned char recv_room_id[4]; // 서버로 부터 받은 메세지의 대화방 식별자
   unsigned char ucBuf[256];
   struct sockaddr_in stAddr; // 서버 주소 구조체
  
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
     printf("inet_pton() failed", "invalid address string");
     close(iDs); // 열었던 소켓을 닫는다.  
     return -100;
   }
   else if(iRet < 0)
   {
     perror("inet_pton() failded");  
     close(iDs); // 열었던 소켓을 닫는다.  
     return -100;
   }
   // ip 출력  
   printf("IP : %s\n", inet_ntoa(stAddr.sin_addr));
   // 서버 포트(포트 문을 열어준다.)
   stAddr.sin_port = htons(8000); // 포트 8000번 연다.
           // 여기까지가 기본 세팅이며 client와 동일하다.
   
   if(connect(iDs, (struct sockaddr *)&stAddr, iDSize))
   {
     perror("connect() failed");
     close(iDs);
     return -10;
   }

  while(1)
   {
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
       return -100;
     }
     strcpy(my_room_id, "0001");
     // client가 server로 입력을 보낼 때
    if(0 != FD_ISSET(0, &fsStatus)) // 서버의 키보드 입력 확인
    {
       iRet = read(0, ucBuf, sizeof(ucBuf));
       ucBuf[iRet] = 0;
       printf("%s\n", ucBuf);
       write(iDs, my_room_id, 4);       
       write(iDs, ucBuf, iRet);
       continue;
     }
     // server가 보낸 입력을 받을 때
    if(0!=FD_ISSET(iDs, &fsStatus))
     {
       // 저수준으로 읽는다. ucBuf의 길이를 알기 위해 iRet로 반환값을 저장한다.
       iRet = read(iDs, ucBuf, sizeof(ucBuf));
       ucBuf[iRet] = 0;
       strncpy(recv_room_id, ucBuf, 4);
       recv_room_id[4] = 0;
       printf("%s\n", recv_room_id);
       printf("%s\n", ucBuf);
     }
   }
   close(iDs); // 열었던 소켓을 닫는다.  
   return 0;
 }

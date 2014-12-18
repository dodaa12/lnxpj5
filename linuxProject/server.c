// server.c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define MAXCLIENT 10

int main()
 {  
   int iDs; // 디스크립터 저장
  int maxDs; // 파일디스크립터 설정을 위한 변수
  int iAccept; // accept 함수의 반환값 저장
  int iaClient[MAXCLIENT]; // 접속 가능한 인원 수 - 소켓 번호를 가지고 있다.
   
   fd_set fsStatus; // select()에 사용되는 소켓 식별자 셋
  int iCNum = 0; // 접속자 수(client number)
   int iCounter; // 반복문을 위한 변수
  int iAddSize; 
   int iRet;
   unsigned char ucBuf[256];
   struct sockaddr_in stAddr; // 서버 주소 구조체
  struct sockaddr_in stAccept;
   
   iAddSize = sizeof(struct sockaddr_in);
   bzero(&stAddr, iAddSize); // 0으로 구조체 초기화  
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
   
   // 여기부터 client와 달라진다.
   // 지역 주소에 바인드(bind-소켓에 이름을 묶는다.)
   // 소켓 세팅(윈도우 레지스트 함수랑 유사)
   if(0>bind(iDs, (struct sockaddr *)&stAddr, iAddSize))
   {
     perror("bind() failed");
     close(iDs); // 열었던 소켓을 닫는다.  
     return -100;
   }
   // 소켓이 들어오는 요청을 처리할 수 있도록 설정(listen)  
   if(0>listen(iDs, 5)) 
   {
     perror("listen() failed");
     close(iDs);
     return -100;
   }

   maxDs = iDs+1;
   while(1)
   {    
     // 소켓 식별자 벡터를 '0'으로 초기화 하고 서버 소켓이 사용하도록 설정
    FD_ZERO(&fsStatus); // select()가 호출된후 매번 '0'으로 초기화
    // 랑데뷰 소켓(3번 셋트, 1로 체크해 준다.)
     // 그룹은 총 랑데뷰, 키보드, 커뮤니케이션(아래쪽)
     FD_SET(iDs, &fsStatus);  // 디스크립터, 소켓식별자 벡터
    FD_SET(0, &fsStatus); // select가 0번(서버의 키보드 입력)까지 감시한다.
     // 접속 인원 체크
    for(iCounter=0;iCNum>iCounter;++iCounter)
     {
       FD_SET(iaClient[iCounter], &fsStatus);
     }
     // 파일디스크립터, 입력, 출력, 에러, 타임아웃(감시하는 시간-NULL은 무한대기)
     // 해당하는 소켓에 입력이 없으면 BLOCK 된다.
     // accept는 select가 통과시켜 주어야 호출된다.
     if(0 > select(maxDs, &fsStatus, NULL, NULL, NULL))
     {
       perror("select() error... ");
       close(iDs);
       return -100;
     }
     // 클라이언트의 연결을 기다림(accept-소켓에 연결을 받아들인다.)
     // accept는 소캣을 새로 만들어 소켓이 두개가 된다. 그리고 소켓에 번호(파일디스크립터)가 할당된다.
     // 위에서 만든 소켓은 랑데뷰 소켓(분류만 담당한다. 실제 일은 하지 않는다.)
     // 실제 통신이 시작된다. - 접속자가 늘어날때마다 소켓이 계속 생성된다.
     // 랑데뷰 소켓은 3번, 좀더 정확하게는 iDs)
     // 접속되지 않았던 유저가 접속하려하면 if문 안으로 들어간다.(accept 수행)
     if(1 == FD_ISSET(iDs, &fsStatus)) // return 값이 1이면 참
    {
       iAccept = accept(iDs, (struct sockaddr *)&stAccept, &iAddSize);
       if(0>iAccept)  
       {
         perror("accept() error");
         close(iDs); // 랑데뷰 함수를 닫는다.
         // 들어온 접속자 만큼 닫는다.- 접속자가 없으면 돌지 않는다.
         for(iCounter=0;iCNum>iCounter;++iCounter)
         {
           close(iaClient[iCounter]);
         }
         return -100;
       }
       // 접속 제한 인원보다 많이 들어 왔는가 확인
      if(MAXCLIENT <= iCNum)
       {
         write(iAccept, "Server is full connection\n", sizeof("Server is full connection\n"));
         close(iAccept);
         continue ;
       }
       // 중간에 빈 파일디스크립터로 생성되었을 경우에는 더하면 안되므로
      if(iAccept == maxDs)
       {
         maxDs = iAccept + 1;   
         iaClient[iCNum] = iAccept;
         ++iCNum;
       }       
       continue ;
     }
     // 들어온 쪽을 찾기 위해 열려진 부분을 모두 찾아야 한다.
     for(iCounter=0;iCNum>iCounter;++iCounter)// 전체 접속 인원 검색
    {
       // 해당하는 Client에 1이 세팅(입력이 들어왔는가)
       // 사용자 소켓 번호 체크
      if(1==FD_ISSET(iaClient[iCounter], &fsStatus))
       {
         // 저수준으로 읽는다. ucBuf의 길이를 알기 위해 iRet로 반환값을 저장한다.
	 fflush(stdin);
	 memset(ucBuf, 0, 256);	 
         iRet = read(iaClient[iCounter], ucBuf, sizeof(ucBuf));
         ucBuf[iRet-1] = '\0';
         for(iCounter=0;iCNum>iCounter;++iCounter)// 전체 접속 인원 검색
         {
           write(iaClient[iCounter], ucBuf, iRet);
         }
       }
     }    
   }
   close(iDs); // 열었던 소켓을 닫는다.  
   close(iAccept);
   return 0;
 }

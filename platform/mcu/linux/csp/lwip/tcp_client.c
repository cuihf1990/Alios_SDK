#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include<sys/wait.h>
#include <string.h>

int main(void *arg)
{  //int sock;
    struct sockaddr_in echoserver;

    struct hostent *host;
    int sock, bytes_received;
    struct sockaddr_in server_addr;

    //udp_client();
    //return;

    char url[256] = "http://cmnsguider.yunos.com";


            /* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
    //host = lwip_gethostbyname(url);
    printf("------------------------test_lwipi gethost----------------------%p\n",host);
    /* Create the TCP socket */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
       //Die("Failed to create socket");
       printf("error\n");
    }
    printf("------------------------3 %d\n", sock);

    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    //echoserver.sin_len = sizeof(echoserver);
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr("30.5.125.103");  /* IP address */
    //echoserver.sin_addr.s_addr = inet_addr("10.101.170.46");  /* IP address */
    echoserver.sin_port = htons(8800);       /* server port */
    /* Establish connection */
    if (connect(sock,
                (struct sockaddr *) &echoserver,
                sizeof(echoserver)) < 0) {
      printf("Failed to connect with server\n");
      return 0;
    }

    printf("connect ok!!!! GOOD !!!!!!!\n");

    char buf[256] = {0};
    while(1)
    {
        int n = write(sock, "HELLO HELLO HELLO HELLO", 24);
        printf("write ok: %d\n", n);

        int r = read(sock, buf, 256);

        printf("read GOT: %d %s\n", r, buf);

        sleep(1);
    }
}

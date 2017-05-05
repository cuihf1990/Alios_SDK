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

/*********************************************************************
*filename: tcpserver.c
*purpose:tcp服务端程序
********************************************************************/
int main(int argc, char ** argv)
{
    int sockfd,new_fd;

    struct sockaddr_in my_addr; /* 本机地址信息 */
    struct sockaddr_in their_addr; /* 客户地址信息 */
    unsigned int sin_size, myport, lisnum;

    if(argv[1])  myport = atoi(argv[1]);
    else myport = 8800;

    if(argv[2])  lisnum = atoi(argv[2]);
    else lisnum = 2;

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    printf("socket %d ok \n",myport);

    my_addr.sin_family=PF_INET;
    my_addr.sin_port=htons(myport);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(my_addr.sin_zero), 0);
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
     printf("bind ok \n");

    if (listen(sockfd, lisnum) == -1) {
        perror("listen");
        exit(1);
    }
 printf("listen ok \n");

 /*
    while(1) {
        sin_size = sizeof(struct sockaddr_in);
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }

        printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));
        if (!fork()) { //子进程代码段
            if (send(new_fd, "Hello, world!\n", 14, 0) == -1) {
                perror("send");
                close(new_fd);
                exit(0);
            }
        }
        close(new_fd); //父进程不再需要该socket
        waitpid(-1,NULL,WNOHANG);//等待子进程结束，清除子进程所占用资源
    }
 */

    sin_size = sizeof(struct sockaddr_in);
    if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
        perror("accept");
  exit(0);
    }
    printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));

 int   step = 0;
 while(1) {
  char  szSnd[63] = {0};
  sprintf(szSnd,"i am server [%d]\n",step);
  step++;
        if (send(new_fd, szSnd, strlen(szSnd), 0) == -1) {
            perror("send");
            close(new_fd);
            break;
        }

  printf("send msg: %s \n",szSnd);

  sleep(1);
 }

 exit(0);
  }


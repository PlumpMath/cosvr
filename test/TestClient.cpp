#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
	if(argc != 2)
	{
		printf("\n Usage: %s <ip of server> \n", argv[0]);
		return 1;
	}

    int sockfd = 0, n = 0;
    struct sockaddr_in serv_addr;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(20003);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    }



		char recvBuff[1024];
		char sendBuff[1024] = "hello!";
		printf("write start.\n");
		write(sockfd, sendBuff, strlen(sendBuff)+1);
		printf("write end.\n");
		read(sockfd, recvBuff, strlen(sendBuff)+1);
		printf("%s\n", recvBuff);
		sleep(1);

    return 0;
}

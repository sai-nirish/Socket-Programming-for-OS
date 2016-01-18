#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portnum, n;

    struct sockaddr_in server_addr;
   // struct hostent *server;

    char inBuffer[513];
    // char outBuffer[512];
    if (argc < 3) {
       fprintf(stderr,"usage %s serverIP port\n", argv[0]);
       exit(0);
    }

    /* create socket, get sockfd handle */

    portnum = atoi(argv[2]);

    /* Create client socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
      {
    fprintf(stderr, "ERROR opening socket\n");
    exit(1);
      }

    /* Fill in server address */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton(argv[1], &server_addr.sin_addr))
      {
    fprintf(stderr, "ERROR invalid server IP address\n");
    exit(1);
      }
    server_addr.sin_port = htons(portnum);


    /* connect to server */

    if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) 
        error("ERROR connecting");

    /* ask user for input */

    //xprintf("Enter the File: ");
    bzero(inBuffer,513);
    fgets(inBuffer,512,stdin);

    /* send user message to server */

    n = write(sockfd,inBuffer,strlen(inBuffer));
    printf("%d\n",n);
    if (n < 0) 
         error("ERROR writing to socket");
    

    /* read reply from server */
    int sum = 0;
    while(1){
        bzero(inBuffer,513);
        n = read(sockfd,inBuffer,512);
        sum = sum + n;
        if (n < 0) 
            error("ERROR reading from socket");
        if(n == 0) break;
        // printf("%d\n",n);
        //printf("%s\n",inBuffer);
    }
    printf("Total Bytes received: %d\n",sum);
    return 0;
}

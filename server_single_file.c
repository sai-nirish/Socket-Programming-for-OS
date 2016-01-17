/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno, clilen;
     char inBuffer[256],file[100],outBuffer[100];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     /* create socket */

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);

     /* bind socket to this port number on this machine */

     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     
     /* listen for incoming connection requests */

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     /* accept a new request, create a newsockfd */

     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     /* read message from client */

     bzero(inBuffer,256);
     bzero(file,50);
     n = read(newsockfd,inBuffer,255);
     if (n < 0) error("ERROR reading from socket");
     printf("Here is the message: %s, %d\n",inBuffer,n);
     if(inBuffer[0] == 'g' && inBuffer[1] == 'e' && inBuffer[2] == 't'){
        int i = 0;
        for(i = 0;i < n-5;i++){
            file[i] = inBuffer[i+4];
        }
        file[n-5] = '\0';
     }
     printf("file: %s\n",file);
     FILE* fp;
     fp = fopen((const char*)file,"r");
     bzero(outBuffer,100);
     int bytesRead = 0;
     if(fp != NULL){
        while((bytesRead = fread(outBuffer,1,sizeof(outBuffer),fp))>0){
            printf("bytesRead: %d\n", bytesRead );
            n = write(newsockfd,outBuffer,bytesRead);
            if (n < 0) error("ERROR writing to socket");
            printf("bytesWritten: %d\n", n);
            bzero(outBuffer,100);
        }
        if (feof(fp)) {
        printf("End-of-File reached.\n");
        }
        else printf("End-of-File was not reached. \n");
        fclose (fp);
       }
    
     
     //printf("outBuffer: %s\n",outBuffer);

     /* send reply to client */

    
     return 0; 
}

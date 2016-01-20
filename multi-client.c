#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

struct thread_data{
    struct sockaddr_in serv_addr;
    int experimentTime,sleepTime,mode,id;
    int requests;
    double responseTime;
};

void* clientThread(void* threadarg){

    struct thread_data *my_data;
    my_data = (struct thread_data *) threadarg;
    char inBuffer[30];
    char outBuffer[512];

    int sockfd, n, x;
    int mode;
    int experimentTime,sleepTime,threadId;
    struct sockaddr_in s_addr;

    s_addr = my_data->serv_addr;
    mode = my_data->mode;
    experimentTime = my_data->experimentTime;
    sleepTime = my_data->sleepTime;
    threadId = my_data->id;

    printf("threadno = %d\n", threadId);

    time_t startTime, endTime;
    float elapsedTime = 0.0;
    startTime = time(NULL);

    struct timeval rs1,rs2;
    double totalResponseTime = 0.0;

    printf("experimentTime %d\n", experimentTime);
    while(elapsedTime < experimentTime){
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) 
          {
        fprintf(stderr, "ERROR opening socket\n");
        exit(1);
          }
        
        if (connect(sockfd,(struct sockaddr *)&s_addr,sizeof(s_addr)) < 0) 
        error("ERROR connecting");
        bzero(inBuffer,30);
        if(mode){
            x = rand() % 1000;
           sprintf(inBuffer,"get files/foo%d.txt\0",x);
        }
        else{
            x = 0;
           sprintf(inBuffer,"get files/foo%d.txt\0",x);
        }

        gettimeofday(&rs1,NULL);
        n = write(sockfd,inBuffer,strlen(inBuffer));
        printf("%s\n",inBuffer);
        if (n < 0) 
             error("ERROR writing to socket");
        
        int sum = 0;
        bzero(outBuffer,512);
        while(1){            
            n = read(sockfd,outBuffer,512);
            sum = sum + n;
            if (n < 0) 
                error("ERROR reading from socket");
            if(n == 0) break;
            // printf("%d\n",n);
            //printf("%s\n",inBuffer);
        }
        gettimeofday(&rs2,NULL);
        if(n == 0){
            my_data->requests++;
            totalResponseTime = totalResponseTime + (1000*(rs2.tv_sec - rs1.tv_sec) + (rs2.tv_usec-rs1.tv_usec)/1000);
        }
        // printf("Total Bytes received: %d\n",sum);
        printf("thread %d,file %s recieved ,Total Bytes received: %d\n",threadId,inBuffer,sum);
        close(sockfd);
        sleep(sleepTime);        
        endTime = time(NULL);
        elapsedTime = endTime-startTime;
        printf("elapsedTime %f\n", elapsedTime);
    }
    my_data->responseTime = totalResponseTime/(my_data->requests);
    printf("threadId %d,requests %d\n",threadId,my_data->requests);
    printf("threadId %d,responseTime %f\n",threadId,my_data->responseTime);
    return;
}

int main(int argc, char *argv[])
{
    
    if (argc != 7) {
       fprintf(stderr,"usage %s serverIP port usercount totalTime sleepTime mode\n", argv[0]);
       exit(0);
    }

    int portnum,numThreads,bMode;
    int totalTime,sleepTime;
    struct sockaddr_in server_addr;
    void* status;

    //char mode[10] = argv[6];
    if(!(strcmp(argv[6],"random"))){
        bMode = 1;
    }
    else if(!(strcmp(argv[6],"fixed"))){
        bMode = 0;
    }
    else{
        fprintf(stderr,"No such mode exists \n");
        exit(0);
    }
    portnum = atoi(argv[2]);
    numThreads = atoi(argv[3]);
    totalTime = atoi(argv[4]);
    sleepTime = atoi(argv[5]);


    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(!inet_aton(argv[1], &server_addr.sin_addr))
      {
        fprintf(stderr, "ERROR invalid server IP address\n");
        exit(1);
      }
    server_addr.sin_port = htons(portnum);
    
    pthread_t threads[numThreads];
    pthread_attr_t attr;
    struct thread_data td[numThreads];
    int i,rc;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    

    for( i=0; i < numThreads; i++ ){
        printf("main() : creating thread, %d\n", i);
        td[i].serv_addr = server_addr;
        td[i].experimentTime = totalTime;
        td[i].sleepTime = sleepTime;
        td[i].mode = bMode;
        td[i].id = i;
        td[i].requests = 0;
        td[i].responseTime = 0.0;
        rc = pthread_create(&threads[i], NULL, clientThread, (void *)&td[i]);
        if (rc){
        printf("Error:unable to create thread, %d\n", rc);
        exit(-1);
        }
    }

    // free attribute and wait for the other threads
   pthread_attr_destroy(&attr);
   for( i=0; i < numThreads; i++ ){
      rc = pthread_join(threads[i], &status);
      if (rc){
         printf("Error:unable to join, %d\n", rc);
         exit(-1);
      }
      printf("Main: completed thread id: %d\n", i);
      // printf("  exiting with status :  %s\n",status );
    } 

    double throughput = 0.0,responseTime = 0.0,totalResponseTime = 0.0;
    int totalRequests = 0;
    for ( i = 0; i < numThreads; i++){
        totalRequests = td[i].requests + totalRequests;
        totalResponseTime = td[i].responseTime + totalResponseTime;
    }

    throughput = (double)totalRequests/totalTime;
    responseTime = totalResponseTime/(numThreads*1000);

    printf("throughput = %f reqs/s \n", throughput);
    printf("average response time = %f sec\n", responseTime);

    pthread_exit(NULL);
    return 0;
}

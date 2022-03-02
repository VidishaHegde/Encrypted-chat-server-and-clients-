#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <sys/socket.h>





int main(int argc , char *argv[]){
    int sockt,listn1,listn2;
    char messageR1[1024];
    char messageR2[1024];
    int rsize=0;
    char done[]="exit";
    struct sockaddr_in server;
    char *ip="127.0.0.1";

    //printf("%d\n",argc);
    if(argc<2){
        printf("No Port number!\n");
        return 0;
    }
    char pno[20];
    strcpy(pno,argv[1]);
    int p=atoi(pno);

    sockt=socket(AF_INET,SOCK_STREAM,0);
    if (sockt<0) { 
        printf("Socket creation failed!\n"); 
        exit(0); 
    } 
    server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_port = htons(p);
	
	if(bind(sockt,(struct sockaddr *)&server , sizeof(server)) < 0){
		printf("bind failed. Error\n");
		return 1;
	}

    while(1){

        printf("Server is Active.\n");
        listen(sockt,1);
        listn1=accept(sockt,(struct sockaddr*)NULL, NULL); 
        if(listn1<0){
            printf("Connection not accepted!\n"); 
        }
        else{
            printf("Connected to Client 1.\nReady for another client.\n");
            listen(sockt,1);
            listn2=accept(sockt,(struct sockaddr*)NULL, NULL); 
            if(listn2<0){
                printf("Connection not accepted!\n"); 
            }
            else{
                printf("Connected to Client 2.\n");
            }
        }
        //printf("%d %d\n",listn1,listn2);
        int pid=fork();
        if(pid==0){
            //listen to client 1, send to client 2
            while(1){
                
                if(recv(listn1,messageR1,sizeof(messageR1),0)<0){
                    printf("Recieve faliled\n");
                    break;
                }
                int exit=0;
                for(int i=0;i<4;i++){
                    if(done[i]==messageR1[i]) exit++;
                    
                }
                if(exit==4){
                    printf("Client 1 Left\n");
                    
                    break;  
                }
                if(strlen(messageR1)>0) {
                    printf("Message from client 1: %s\n",messageR1);
                    printf("Message len: %ld\n",strlen(messageR1));
                }
                if(send(listn2,messageR1,256,0) < 0){
                    printf("Send failed\n");
                    break;
                } 
                if(strlen(messageR1)>0) printf("Server is Listening.\n");
                memset(messageR1,0,sizeof(messageR1));
                
            }
        }
        while(1){
            //listen to client 2, send to client 1            
            
            if(recv(listn2,messageR2,sizeof(messageR2),0)<0){
                printf("Recieve faliled\n");
                break;
            }
            int exit=0;
            for(int i=0;i<4;i++){
                if(done[i]==messageR1[i]) exit++;
                
            }
            if(exit==4){
                printf("Client 2 Left\n");
                
                break;  
            }
            if(strlen(messageR2)>0){
                printf("Message from client 1: %s\n",messageR2);
                printf("Message len: %ld\n",strlen(messageR2));
            }
            if(send(listn1,messageR2,256,0) < 0){
                printf("Send failed\n");
                break;
            } 
            if(strlen(messageR2)>0) printf("Server is Listening.\n");
            memset(messageR2,0,sizeof(messageR2));
        }
        
        close(listn1);
        close(listn2);
        fflush(stdout);
    }
    
    close(sockt);
    fflush(stdin);
    return 0;
}   
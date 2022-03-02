#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h> 
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>



int padding = RSA_PKCS1_PADDING;

RSA * createRSAWithFilename(char *filename, int public){
    FILE * fp = fopen(filename,"rb");
    if(fp == NULL){
        printf("Unable to open file %s \n",filename);
        return NULL;    
    }
    RSA *rsa= RSA_new() ;
 
    if(public){
        rsa = PEM_read_RSA_PUBKEY(fp, &rsa,NULL, NULL);
    }
    else{
        rsa = PEM_read_RSAPrivateKey(fp, &rsa,NULL, NULL);
    }
 
    return rsa;
}

int public_encrypt(unsigned char * data,int data_len,unsigned char * keyfile, unsigned char *encrypted){
    RSA * rsa = createRSAWithFilename(keyfile,1);
    
    int result = RSA_public_encrypt(data_len,data,encrypted,rsa,padding);
    //printf("result: %d\n",result);
    return result;
}

int private_decrypt(unsigned char * enc_data,int data_len,unsigned char * keyfile, unsigned char *decrypted){
    RSA * rsa = createRSAWithFilename(keyfile,0);
    int dl = 256;
    int  result = RSA_private_decrypt(dl,enc_data,decrypted,rsa,padding);
    //printf("result: %d\n",result);
    return result;
}
 
void printLastError(char *msg)
{
    char * err = malloc(130);;
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    printf("%s ERROR: %s\n",msg, err);
    free(err);
}


int main(int argc , char *argv[]){
    int sockt,connct;
    int exitnow=0;
    char messageR[1024];
    char messageS[1024];
    int messagep=0;
    char done[]="exit";
    struct sockaddr_in client;
    char messageRecieved[2048];
    char encryptedMessage[2048];

    if(argc<5){
        printf("Enter right number of arguments!\n");
        return 0;
    }
    //argv[1] - ip address at which it will find the server
    //argv[2] - the port number of the server at that IP address
    //argv[3] - the filename containing its private key
    //argv[4] - the filename containing the other client's public key
    
    char ip[20];
    strcpy(ip,argv[1]);
    char pno[20];
    strcpy(pno,argv[2]);
    int p=atoi(pno);


    sockt=socket(AF_INET,SOCK_STREAM,0);

    if (sockt == -1) { 
        printf("Socket creation failed!\n"); 
        exit(0); 
    } 

    client.sin_family = AF_INET;
    client.sin_addr.s_addr = inet_addr(ip);
    client.sin_port = htons(p);
    connct=connect(sockt,(struct sockaddr*)&client, sizeof(client));

    if(connct<0){
        printf("Could not connect to server!\n");
        return 0;
    }

    int pid=fork();
    if(pid==0){
        //listen
        while(1){
            if(exitnow==4) exit(0);
            if(recv(sockt,messageR,sizeof(messageR),0)<0){
                printf("Recieve failed\n");
            }
            //decrypt
            int decryptedLength = private_decrypt(messageR,strlen(messageR),argv[3],messageRecieved);
            if(strlen(messageR)>0){
                printf("Message recieved: %s\n",messageR);
                //printf("Message len: %ld\n",strlen(messageR));
            }
            if(decryptedLength == -1){
                printLastError("Decrypt failed ");
                //exit(0);
            }
            if(strlen(messageRecieved)>0) printf("Decrypted message: %s\n",messageRecieved);
            memset(messageR,0,sizeof(messageR));
            memset(messageRecieved,0,sizeof(messageRecieved));
        }
    }
    while(1){
        //send
        //printf("Enter message to Server: ");
        fgets(messageS,sizeof(messageS),stdin);
        //printf("message to server: %s\n",messageS);
        
        for(int i=0;i<4;i++){
            if(done[i]==messageS[i]) exitnow++;
            
        }
        if(exitnow==4){
            send(sockt,messageS,strlen(messageS),0); 
            
            fflush(stdin);
            fflush(stdout);
            close(sockt);
            exit(0);
        }
        messageS[strlen(messageS)]='\0';
        //encrypt the message
        int encryptedLength = public_encrypt(messageS,strlen(messageS)+1,argv[4],encryptedMessage);
        //printf("encryptedLength: %d\n",encryptedLength);
        if(send(sockt,encryptedMessage,encryptedLength,0) < 0){
            printf("Send failed\n");
        } 
        
        //printf("message sent: %s\n",encryptedMessage);
        memset(messageS,0,sizeof(messageS));
        memset(encryptedMessage,0,sizeof(encryptedMessage));
    }

    return 0;
}
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*
 *
 *     Developed by Manuele Graziani 5H
 *
 */



#define SERVER_PORT 5001
#define THREADNUMBER 100

// parametri passati ai thread
typedef  struct {
    struct sockaddr_in LocalAddress, serverAddress;
}Parameters;


// Ha il compito di creare una socket e connettersi al server
void *ThreadFunction(void* parameters) {


    Parameters * parameters1 = (Parameters*)parameters;
    int socketFileDescriptor;

    socketFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFileDescriptor == -1) {

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        return (EXIT_FAILURE);
    }


    if (bind(socketFileDescriptor, (struct sockaddr *) &parameters1->LocalAddress, sizeof(parameters1->LocalAddress)) != 0) {

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        return (EXIT_FAILURE);
    }

    if (connect(socketFileDescriptor, (struct sockaddr *) &parameters1->serverAddress, sizeof(parameters1->serverAddress)) != 0){

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        return (EXIT_FAILURE);
    } else
    {
        write(socketFileDescriptor,"Ciao sono il client",20);
        char Buffer[50];
        recv(socketFileDescriptor,&Buffer,50,0);
        fprintf(stderr, "%s\n",Buffer);
        close(socketFileDescriptor);

    }



    pthread_exit(NULL);
}


int main() {

    struct sockaddr_in LocalAddress, ServerAddress;
    pthread_t threadID[THREADNUMBER];


    bzero(&ServerAddress, sizeof(ServerAddress));

    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    ServerAddress.sin_port = htons(SERVER_PORT);


    bzero(&LocalAddress, sizeof(LocalAddress));

    LocalAddress.sin_family = AF_INET;
    LocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    LocalAddress.sin_port = htons(0);

    Parameters *parameters= malloc(sizeof(Parameters));
    if (parameters == NULL)
        exit(EXIT_FAILURE);

    else {
        parameters->LocalAddress = LocalAddress;
        parameters->serverAddress = ServerAddress;
    }


    for(int i = 0; i < THREADNUMBER; i++)
    {
        pthread_create(&threadID[i], NULL, ThreadFunction, parameters);
    }

    for(int i = 0; i < THREADNUMBER; i++)
    {
        pthread_join(threadID[i],NULL);
    }


    exit(EXIT_SUCCESS);
}



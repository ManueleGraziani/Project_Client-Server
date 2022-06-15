#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "List.h"

/*
 *
 *     Developed by Manuele Graziani 5H
 *
 */


#define SERVER_PORT 5001
#define LISTEN_MAXIMUM_LENGTH_QUEUE 10
#define DEBUG // Utilizzato per la stampa per il debug


// Questa struct conterrà le informazioni relative al thread che verranno inserite nella lista myListOfThreadInfo
typedef struct {
    pthread_t threadID;
    bool isComplete;
    int connectionFileDescriptor;
} ThreadInfo;


// Questa struct conterrà le informazioni relative al thread che ha il compito di effettuare le join
typedef struct {

    List* threadList;
    pthread_cond_t pthreadCond;
    pthread_mutex_t pthreadMutex;

    bool terminationCondition;
    bool imSleeping;

} ThreadTerminatorParameters;


// Implementazione delle funzioni ______________________________________________________________________________________
struct sockaddr_in SocketSetting(struct sockaddr_in serverAddress) {
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(SERVER_PORT);

    return serverAddress;
}

// Inizializza il socket
int SocketInitialization(int listenFileDescriptor) {

    if ((listenFileDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        return (EXIT_FAILURE);
    }


    return listenFileDescriptor;

}

// effettua la bind
void AssigningNameToSocket(int listenFileDescriptor, struct sockaddr_in serverAddress) {
    if (bind(listenFileDescriptor, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) != 0) {

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


}

// Imposta il socket in un stato passivo in ascolto
void SetSocketIntoListenMode(int listenFileDescriptor) {
    if (listen(listenFileDescriptor, LISTEN_MAXIMUM_LENGTH_QUEUE) != 0) {

        fprintf(stderr, "[ERROR] %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

// Funzione che verrà eseguita dai thread
void *ThreadFunction(void *input) {

    ThreadInfo *threadInfo = (ThreadInfo *) input;

    char *buffer = calloc(sizeof(char), 50);

    recv(threadInfo->connectionFileDescriptor, buffer, 50, 0);
    fprintf(stderr, " %s\n", buffer);
    write(threadInfo->connectionFileDescriptor, "Ciao sono il server", 20);

    free(buffer);
    threadInfo->isComplete = true;

    close(threadInfo->connectionFileDescriptor);


    pthread_exit(NULL);
}

// Ha il compito di effettuare la join hai thread che hanno terminato (capito tramite variabile booleana isComplete) liberando il relativo nodo nella lista
void joinTerminatedThreads(List *list) {

    ThreadInfo *currentThreadInfo;
    ListNode *currentNode;
    ListNode *previousNode = NULL;
    ListNode *nextNode = NULL;

    for (currentNode = list->head; currentNode != NULL; currentNode = nextNode) {

        nextNode = currentNode->next;
        currentThreadInfo = currentNode->data;

        if (currentThreadInfo->isComplete) {

            if (list->size == 1) { // caso in cui la lista è di solo un nodo

                list->tail = NULL;
                list->head = NULL;
                list->size = 0;

            } else {

                if (list->head == currentNode)
                    list->head = currentNode->next;
                else {

                    previousNode->next = nextNode;
                    if (nextNode == NULL)
                        list->tail = previousNode;
                }

                list->size--;
            }

            pthread_join(currentThreadInfo->threadID, NULL);

            #ifdef DEBUG
            fprintf(stderr, "join effettuata \n");
            #endif

            free(currentThreadInfo);
            free(currentNode);

        } else
            previousNode = currentNode;
    }
}




void *ThreadTerminatorRoutine(void *input) {

    ThreadTerminatorParameters *parameters = (ThreadTerminatorParameters *) input;

    while(parameters->terminationCondition) {

        // se la grandezza della lista è uguale a zero, il thread "andrà a dormire"
        pthread_mutex_lock(&parameters->pthreadMutex);
        if (parameters->threadList->size == 0) {

            parameters->imSleeping = true;
            pthread_cond_wait(&parameters->pthreadCond, &parameters->pthreadMutex);
        }

        parameters->imSleeping = false;
        joinTerminatedThreads(parameters->threadList);

        pthread_mutex_unlock(&parameters->pthreadMutex);
    }

    return NULL;
}


int main() {

    ThreadTerminatorParameters parameters;
    pthread_t terminatorThreadID;
    int listenFileDescriptor;
    struct sockaddr_in clientAddress, serverAddress;
    socklen_t clientAddressLen;

    parameters.threadList = allocateList();
    parameters.terminationCondition = true;

    if (pthread_mutex_init(&(parameters.pthreadMutex), NULL) != 0)
        exit(EXIT_FAILURE);

    if (pthread_cond_init(&(parameters.pthreadCond), NULL) != 0)
        exit(EXIT_FAILURE);

    if(pthread_create(&terminatorThreadID, NULL, ThreadTerminatorRoutine, &parameters) != 0) // inizializzo il thread
        exit(EXIT_FAILURE);


    // chiamata alle funzioni

    clientAddressLen = sizeof(clientAddress);

    serverAddress = SocketSetting(serverAddress);

    listenFileDescriptor = SocketInitialization(listenFileDescriptor);

    AssigningNameToSocket(listenFileDescriptor, serverAddress);

    SetSocketIntoListenMode(listenFileDescriptor);

    for (;;) {

        // alloca a tempo di run time threadInfo, se l'esecuzione va a buon fine procede instaurazione della connessione con gli eventuali client, altrimenti termina l'esecuzione del programma
        ThreadInfo* threadInfo = malloc(sizeof(ThreadInfo));
        if (threadInfo == NULL)
            exit(EXIT_FAILURE);
        else {

            threadInfo->isComplete = false;

            #ifdef DEBUG
            fprintf(stderr, "Accepting... \n");
            #endif

            threadInfo->connectionFileDescriptor = accept(listenFileDescriptor, (struct sockaddr *) &clientAddress, &clientAddressLen); // Effettua il collegamento con il client tramite 3 way handshake
            pthread_create(&threadInfo->threadID, NULL, ThreadFunction, threadInfo); // crea il thread

            pthread_mutex_lock(&parameters.pthreadMutex);
            insert(parameters.threadList, threadInfo); // inserisce i dati relativi al nuovo thread nella lista

            if (parameters.imSleeping == true)
                pthread_cond_signal(&parameters.pthreadCond);

            pthread_mutex_unlock(&parameters.pthreadMutex);
        }
    }
}

#include <stdio.h>     
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>   
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>


// there is no queue in c(((
#include "../queue.h"


static volatile int keepRunning = 1;


#define MAXPENDING 10
#define CLIENT_WAIT 1
#define CLIENT_FINISH 2

// Pointer to queue;
node_t *queue = NULL;
struct sockaddr_in cutter_addr;
struct sockaddr_in client_addr;
int cutter_connected = 0;
int client_connected = 0;

typedef struct thread_args {
    int socket;
} thread_args;

void intHandler(int dummy) {
    keepRunning = 0;
}


void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void* handleClient(void* args) {
    int server_socket;
    int client_length;
    pthread_detach(pthread_self());
    server_socket = ((thread_args*)args)->socket;
    free(args);
    client_length = sizeof(client_addr);
    int client_data[3];
    while(keepRunning) {
        recvfrom(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&client_addr, &client_length);
        printf("[SYSTEM] Client connected!\n");
        while(keepRunning) {
            recvfrom(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&client_addr, &client_length);
            if (client_data[1] == -1) {
                break;
            }
            printf("[Server] New client in the queue #%d\n", client_data[0]);
            enqueue(&queue, client_data[0]);
        }
        printf("[SYSTEM] Client disconected!\n");
    }
    printf("SHUTDOWN1\n");
    client_data[0] = -1;
    sendto(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
}


void* handleCutter(void *args) {
    int server_socket;
    int client_length;
    pthread_detach(pthread_self());
    server_socket = ((thread_args*)args)->socket;
    free(args);
    client_length = sizeof(cutter_addr);
    int client_data[3];
    while (keepRunning) {
        recvfrom(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&cutter_addr, &client_length);
        printf("[SYSTEM] Cutter connected!\n");
        client_data[0] = 0;
        while(keepRunning) {
            recvfrom(server_socket, client_data, sizeof(client_data), MSG_DONTWAIT, (struct sockaddr*)&cutter_addr, &client_length);
            if (client_data[0] == -1) {
                printf("[SYSTEM] Cutter disconected!\n");
                break;
            }
            if (client_data[1] == 1) {
                printf("[Server] Client #%d finished\n", client_data[0]);
                client_data[1] = 0;
            }
            if (queue == NULL) {
                sleep(1);
                continue;
            }
            client_data[0] = dequeue(&queue);
            sendto(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&cutter_addr, sizeof(cutter_addr));
            printf("[Server] Sent client #%d to cutter\n", client_data[0]);
            sleep(2 + rand() % 2);
            client_data[0] = 0;
        }
    }
    printf("SHUTDOWN2\n");
    client_data[0] = -1;
    sendto(server_socket, client_data, sizeof(client_data), 0, (struct sockaddr*)&cutter_addr, sizeof(cutter_addr));
}


int createServiceOnPort(char* name, unsigned short server_port) {
    pthread_t serviceThreadId;
    int server_socket;
    int client_socket;
    struct sockaddr_in server_addr;

    if ((server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) DieWithError("socket() failed");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;              
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    server_addr.sin_port = htons(server_port);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) DieWithError("bind() failed");
    printf("[SYSTEM] Service '%s' is running on %s:%d\n", name, inet_ntoa(server_addr.sin_addr), server_port);
    return server_socket;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, intHandler);
    unsigned short client_port;
    unsigned short cutter_port;
    int client_socket;
    int cutter_socket;
    pthread_t threadId;

    if (argc != 3)
    {
        fprintf(stderr, "Usage:  %s <Port for cutter> <Port for client>\n", argv[0]);
        exit(1);
    }

    cutter_port = atoi(argv[1]);
    client_port = atoi(argv[2]);

    cutter_socket = createServiceOnPort("Cutter", cutter_port);
    client_socket = createServiceOnPort("Clients", client_port);

    thread_args *args = (thread_args*) malloc(sizeof(thread_args));
    args->socket = client_socket;
    if (pthread_create(&threadId, NULL, handleClient, (void*) args) != 0) DieWithError("pthread_create() failed");

    thread_args *args2 = (thread_args*) malloc(sizeof(thread_args));
    args2->socket = cutter_socket;
    if (pthread_create(&threadId, NULL, handleCutter, (void*) args2) != 0) DieWithError("pthread_create() failed");

    while(keepRunning) {
        sleep(1);
    }
    sleep(3);
    return 0;
}

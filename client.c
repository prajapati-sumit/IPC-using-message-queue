
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define SERVER_TO_CLIENT_ID 1
#define CLIENT_TO_SERVER_ID 2

int MIN, MAX;
int key;
int msqid;
long int msg_type;  // type of message received
struct Message {
    long int type;  // type of message sent
    int num;
} msg;

int send(int data) {
    msg.type = CLIENT_TO_SERVER_ID;
    msg.num = data;

    return msgsnd(msqid, (void *)&msg, sizeof(msg.num), 0);
}
int receive() {
    msg_type = SERVER_TO_CLIENT_ID;
    msg.type = CLIENT_TO_SERVER_ID;
    int status = msgrcv(msqid, (void *)&msg, sizeof(msg.num), msg_type, 0);
    return (status == -1 ? -1 : msg.num);
}
int getRandomNumber(int l, int r) { return (rand() % (r - l + 1)) + l; }
void init() {
    // this will give different seeds to different clients
    time_t t;
    srand((int)time(&t) % getpid());

    // using client's process id as the key of the message queue
    key = getpid();

    // making connection with the message queue
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        fprintf(stderr, "error accessing message queue: %s\n", strerror(errno));
        exit(1);
    }
}

void handleSignal() { exit(0); }
int main() {
    // handling userdefined signal
    signal(SIGUSR1, handleSignal);
    init();

    int guess;
    while (1) {
        if ((MIN = receive()) == -1) {
            fprintf(stderr, "error receiving the data from server: %s\n",
                    strerror(errno));
            exit(1);
        }
        if (MIN <= 0) {
            // the game has ended
            // the result is announced. Refer README for details
            // the client sleeps until server sends exit signal
            while (1) {
            }
        }
        if ((MAX = receive()) == -1) {
            fprintf(stderr, "error receiving the data from server: %s\n",
                    strerror(errno));
            exit(1);
        }
        // guessing a random number
        guess = getRandomNumber(MIN, MAX);

        // sending guessed number to server
        if (send(guess) == -1) {
            fprintf(stderr, "error sending the data to server\n");
            exit(1);
        }
    }
    return 0;
}

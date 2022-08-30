
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

int MIN, MAX;
int key;
int msg_type;
int msqid;
struct Message {
    long int type;
    int num;
} msg;

int send(int data) {
    fprintf(stdout, "client sent: %d to queue %d\n", data, msqid);
    msg.num = data;
    return msgsnd(msqid, (void *)&msg, sizeof(msg.num), 0);
}
int receive() {
    int status = msgrcv(msqid, (void *)&msg, sizeof(msg.num), msg_type, 0);
    fprintf(stdout, "client received: %d from queue %d\n", msg.num, msqid);
    return (status == -1 ? -1 : msg.num);
}
int getRandomNumber(int l, int r) { return (rand() % (r - l + 1)) + l; }
void init() {
    srand(time(NULL));

    msg_type = 1;
    msg.type = 1;

    // using server process id as the key of the message queue
    key = getpid();

    // making connection with the message queue
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        fprintf(stderr, "error accessing message queue: %s\n", strerror(errno));
        exit(1);
    }
}

void handleSignal() { exit(0); }
int main() {
    fprintf(stderr, "inside client\n");
    signal(SIGUSR1, handleSignal);
    init();

    int guess;
    while (1) {
        MIN = 5;
        MAX = 20;
        if ((MIN = receive()) == -1) {
            fprintf(stderr, "error receiving the data from server: %s\n",
                    strerror(errno));
            exit(1);
        }
        if ((MAX = receive()) == -1) {
            fprintf(stderr, "error receiving the data from server: %s\n",
                    strerror(errno));
            exit(1);
        }
        guess = getRandomNumber(MIN, MAX);
        if (send(guess) == -1) {
            fprintf(stderr, "error sending the data to server\n");
            exit(1);
        }
    }
    return 0;
}

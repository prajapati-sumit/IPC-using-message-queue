// author: Sumit Kumar Prajapati (B20CS074)
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// message type ids to distinguish between sender and receiver
// of a particular message (Refer README)
#define SERVER_TO_CLIENT_ID 1
#define CLIENT_TO_SERVER_ID 2

int MIN, MAX;

key_t* keys;  // array of keys of message queue for k different clients
pid_t* pids;  // array of keys of process ids for k different clients
int* msqids;  // array of msqids of message queue for k different clients

int* clientScore;          // score of the clients after each round
int* clientGuess;          // value received from clients for each round
int serverGuess;           // value guessed by server (G) for each round
int numClients;            // number of clients playing the game
int roundNumber;           // current round number of the game
int winnerOfCurrentRound;  // id of the client who won the current round
const int points = 5;      // points received for winning each round
const int threshold = 50;  // points required to win the game

long int msg_type;  // type of message received
struct Message {
    long int type;  // type of message sent
    int num;
} msg;

// function to send a message to message queue from server to client with id
// 'clientIdx'
int send(int data, int clientIdx) {
    msg.num = data;
    msg.type = SERVER_TO_CLIENT_ID;
    return msgsnd(msqids[clientIdx], (void*)&msg, sizeof(msg.num), 0);
}

// function to server a message from message queue from client with id
// 'clientIdx' to server
int receive(int clientIdx) {
    msg_type = CLIENT_TO_SERVER_ID;
    msg.type = SERVER_TO_CLIENT_ID;
    int status =
        msgrcv(msqids[clientIdx], (void*)&msg, sizeof(msg.num), msg_type, 0);
    return (status == -1 ? -1 : msg.num);
}
int getRandomNumber(int l, int r) { return (rand() % (r - l + 1)) + l; }

void initSetup() {
    time_t t;
    srand((int)time(&t) % getpid());  // giving random seed

    roundNumber = 1;

    fprintf(stdout, "Enter the number of clients(k):");
    fscanf(stdin, "%d", &numClients);
    if (numClients < 0) {
        fprintf(stderr, "Invalid k value (Refer to README). Exiting...\n");
        exit(1);
    }
}
void initGame() {
    fprintf(stdout, "\n-------------Game Starts-----------\n");
    clientScore = (int*)malloc(sizeof(int) * numClients);
    clientGuess = (int*)malloc(sizeof(int) * numClients);
    for (int i = 0; i < numClients; i++) {
        clientScore[i] = 0;
        clientGuess[i] = 0;
    }
}
int printRequiredStats() {
    // omitting the denominator term since it is same for all clients.
    winnerOfCurrentRound = 0;

    // ties are broken by the client id, minimum id is given priority
    for (int i = 0; i < numClients; i++) {
        if (abs(clientGuess[i] - serverGuess) <
            abs(clientGuess[winnerOfCurrentRound] - serverGuess)) {
            winnerOfCurrentRound = i;
        }
    }
    // printing the required statistics
    clientScore[winnerOfCurrentRound] += points;
    fprintf(stdout, "\nRound #%d ,Range = [%d,%d]\n", roundNumber, MIN, MAX);
    fprintf(stdout, "Received tokens from the clients:\n");
    fprintf(stdout, "c_ids:\t\t");
    for (int i = 0; i < numClients; i++) {
        fprintf(stdout, "%d\t\t", i);
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "tokens:\t\t");
    for (int i = 0; i < numClients; i++) {
        fprintf(stdout, "%d\t\t", clientGuess[i]);
    }
    fprintf(stdout, "\n");
    fprintf(stdout, "scores:\t\t");
    for (int i = 0; i < numClients; i++) {
        fprintf(stdout, "%d\t\t", clientScore[i]);
    }
    fprintf(stdout, "\nServer's Guess: %d", serverGuess);
    fprintf(stdout, "\n\n");
    fprintf(stdout, "Winner of round #%d is Client #%d\n", roundNumber,
            winnerOfCurrentRound);

    fprintf(stdout, "\n\n");
    roundNumber++;

    // one of the clients reached the threshold and hence is the winner
    if (clientScore[winnerOfCurrentRound] >= threshold) {
        fprintf(stdout, "Client #%d won the game\n ", winnerOfCurrentRound);
        fprintf(stdout, "\n-------------Game Ends-----------\n\n");

        // declaring result to all the clients
        for (int i = 0; i < numClients; i++)
            send(-pids[winnerOfCurrentRound], i);
        return 1;
    }
    return 0;
}

// function to terminate the server and clients process if some failure occurs
void die() {
    for (int i = 0; i < numClients; i++) {
        kill(pids[i], SIGKILL);
    }
    exit(1);
}
int main() {
    initSetup();

    keys = (int*)malloc(sizeof(int) * numClients);
    pids = (pid_t*)malloc(sizeof(pid_t) * numClients);

    for (int i = 0; i < numClients; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            // Ith client
            if (execl("./client", "./client", NULL) == -1) {
                fprintf(stderr, "error forking child : %s\n", strerror(errno));
                die();
            }
        } else {
            keys[i] = pids[i];
        }
    }
    // only server process will come to this line

    msqids = (int*)malloc(sizeof(int) * numClients);
    if (msqids == NULL) {
        fprintf(stderr, "error allocating space for message queues: %s\n",
                strerror(errno));
        die();
    }

    // creating message queues for each clients
    for (int i = -1; i < numClients; i++) {
        if ((msqids[i] = msgget(keys[i], 0665 | IPC_CREAT)) == -1) {
            fprintf(stderr, "error accessing message queue: %s\n",
                    strerror(errno));
            die();
        }
    }

    // make required setups/intialization for the game
    initGame();

    // start the game
    while (1) {
        // reading MIN and MAX from the user for each round

        fprintf(stdout, "Round #%d\n", roundNumber);
        fprintf(stdout, "MIN MAX = ");
        fscanf(stdin, "%d %d", &MIN, &MAX);

        if (MIN <= 0 || MIN > MAX) {
            fprintf(stderr,
                    "Invalid MIN or MAX (Refer to README). Enter again\n");
            continue;
        }

        // sending MIN and MAX to the clients
        for (int i = 0; i < numClients; i++) {
            if (send(MIN, i) == -1) {
                fprintf(stderr, "error sending the data to client: %s\n",
                        strerror(errno));
                exit(1);
            }
            if (send(MAX, i) == -1) {
                fprintf(stderr, "error sending the data to client: %s\n",
                        strerror(errno));
                exit(1);
            }
        }
        // server guesses G
        serverGuess = getRandomNumber(MIN, MAX);

        for (int i = 0; i < numClients; i++) {
            // server receives ith token from ith client
            if ((clientGuess[i] = receive(i)) == -1) {
                fprintf(stderr, "error receiving the data from client: %s\n",
                        strerror(errno));
                exit(1);
            }
        }
        int gameEnded = printRequiredStats();
        if (gameEnded) break;
    }
    for (int i = 0; i < numClients; i++) {
        // sending user defined signals to terminate the client programs
        kill(pids[i], SIGUSR1);
    }
    // closing all message queues
    for (int i = 0; i < numClients; i++) msgctl(msqids[i], IPC_RMID, NULL);
    return 0;
}

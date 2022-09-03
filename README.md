# IPC using message queue: A server-client game. [Github Link](https://github.com/prajapati-sumit/IPC-using-message-queue)

This project implements IPC using message queue to simulate a server-client game.

![overall_setup](https://raw.githubusercontent.com/prajapati-sumit/images/main/IPC_using_message_queues.png)

## Game Description

1.  We have a set of `k clients` _(k taken as input from the user)_ and one single server.
2.  The server is responsible to create and maintain the message queues.
3.  The game consists of several rounds of server-client communications using message queues.
4.  At the beginning of each round, the server accepts two integers `MIN` and `MAX` from the user, which it sends to all the clients using the message queue.
5.  Each client `j` randomly generates a guessed token (an integer) within the range `[MIN:MAX]` and sends that to the server, denoted by $R_j$ .
6.  Now, the server also generates a token, say `G` (which is also an integer) in the range `[MIN:MAX]`, independently (which is not shared with the clients).
7.  On receiving $R_j$ , from all the clients, the server computes:
    $$δ = \frac{|R_j−G|}{∑(R_j−G)} , ∀j ∈ [0,1,2,...,k-1]$$

8.  Among all the clients, the client scoring the smallest `δ` wins the round and gets a score of `5`.
9.  The first client crossing the total score 50 over multiple rounds is declared as the champion, which in turn ends the game.

## Requirements

1. `gcc` compiler to run C program.
2. `unix` based OS/Environment

## How to Run?

1. `cd` to this directory and just run the `run.sh` bash script using `./run.sh`
2. Enter the number of clients, `k`, that should play this game.
3. For each subsequent round give space separated input of `MIN MAX`, the range of values to be guessed for the round.

## Game Details

1.  For each round, the clients send their guessed token to server. The server also guesses a number in the same range.
2.  The winner of the round is calculated as per the above given rules.
3.  After each round, the stats of the game is printed which consists of token sent by each client for this round and total score of each client after this round.
4.  The game ends if anyone of the clients touches `50` points mark.

## Implementation Details

1. `k message queues` are created for the communication of `k server-client pairs`.
2. Specific message type convention is followed for each queue, otherwise it may be possible that server sends a message to the queue and then receives the same message from the queue under the assumption that the message was sent by the client.
3. To overcome above situation, by convention, the server always sends messages with `msg_type = 1` to the queue and always receives messages with `msg_type = 2` from the queue. Similarily, a client always sends messages with `msg_type = 2` and receives messages with `msg_type = 1`.
4. The value of k entered by the user must be positive i.e. `k>0`.
5. The value of `MIN MAX` entered for each round must satistfy: `0 < MIN < MAX`.
6. When the game ends, each client is sent an unusual `MIN` value to the client at the start of next round. This passes the message to the clients that the game has ended. If, `MIN = - x, where x >= 0` is sent by the server, it signifies that client with `process_id = x` has won the game.

## References

1. [DexTutor YT Playlist](https://www.youtube.com/playlist?list=PLlr7wO747mNp5nn0hteJFnt1rpdx6GG-_)
2. StackOverflow - for helping in fixing every error I got.
3. [Linux Man Page](https://man7.org/linux/man-pages/man1/man.1.html)

## Screenshots

1. The number of clients, taken as input in `k`

    ![start](https://raw.githubusercontent.com/prajapati-sumit/images/main/k.png)

2. The input of `MIN MAX` for each round

    ![start](https://raw.githubusercontent.com/prajapati-sumit/images/main/minmax.png)

3. Error handling if, invalid range is given

    ![start](https://raw.githubusercontent.com/prajapati-sumit/images/main/errorminmax.png)

4. The stats is printed after each round

    ![start](https://raw.githubusercontent.com/prajapati-sumit/images/main/stats.png)

5. The game ends

    ![start](https://raw.githubusercontent.com/prajapati-sumit/images/main/gameend.png)

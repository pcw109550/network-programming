#ifndef __SERVER_H_
#define __SERVER_H_

typedef struct Node {
    char *packet;
    struct Node *next;
} Node;

Node *init_node(char *packet);
void  fini_node(Node *head);

void  send_server_hello(int sockfd, unsigned short *seq);
void  store_file(int filefd, Node *head);

void  sigchld_handler(int sig);

#endif
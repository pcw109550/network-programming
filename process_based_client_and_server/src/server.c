#include <stdio.h>
#include "csapp.h"
#include "common.h"
#include "server.h"

#define DEBUG

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    unsigned short client_port;
    
    unsigned short initial_seq = 0;
    unsigned short *seq = &initial_seq;
    char *packet;
    char *filename;
    // head node does not have packet content
    Node *head, *node;
    int filefd;

    port = atoi(argv[1]); /* the server listens on a port passed 
                             on the command line */
    Signal(SIGCHLD, sigchld_handler);
    listenfd = open_listenfd(port); 
    clientlen = sizeof(clientaddr);

    while (1) {
        connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);

        if (Fork() == 0) {
            Close(listenfd);
            hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                            sizeof(clientaddr.sin_addr.s_addr), AF_INET);
            haddrp = inet_ntoa(clientaddr.sin_addr);
            client_port = ntohs(clientaddr.sin_port);

            fprintf(stdout, "server connected to %s (%s), port %u\n",
                    hp->h_name, haddrp, client_port);

            head = init_node(NULL);
            node = head;

            /* handshake */
            packet = recv_packet(connfd);
            initial_seq = GET_SEQ(packet) + 1;

            // CLIENT_HELLO
            if (GET_CMD(packet) != CL_HELLO) {
                // Server does not need to send the error command
                // send_error(connfd, seq);
                Close(connfd);
                fini_packet(packet);
                continue;
            }
            fini_packet(packet);

            // SERVER_HELLO
            send_server_hello(connfd, seq);

            // DATA_DELIVERY && DATA_STORE
            while (1) {
                packet = recv_packet(connfd);
                if (GET_CMD(packet) == DATA_STORE) {
                    // DATA_STORE
                    filename = PACKET_CONTENT(packet);
                    break;
                } else
                    assert(GET_CMD(packet) == DATA_DELIVERY);
                // DATA_DELIVERY
                node->next = init_node(packet);
                node = node->next;
            }

            filefd = Open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0664);
            fini_packet(packet);
            store_file(filefd, head);
            fini_node(head);
            Close(filefd);
            Close(connfd);
            exit(EXIT_SUCCESS);
        }
        Close(connfd);
    }
}

void send_server_hello(int sockfd, unsigned short *seq) {
#ifdef DEBUG
    fprintf(stdout, "> SEQ: %5hu: [SERVER_HELLO]\n", *seq);
#endif
    char *packet = init_packet(*seq, PACKET_HEADER_SIZE, SV_HELLO, NULL);
    send(sockfd, packet, GET_SIZE(packet), 0);
    fini_packet(packet);
    *seq = *seq + 1;
}

Node *init_node(char *packet) {
    Node *node = (Node *)calloc(1, sizeof(Node));
    node->packet = packet;
    return node;
}

void  fini_node(Node *head) {
    if (head == NULL)
        return;
    Node *node = head;
    Node *next = head->next;
    if (next == NULL) {
        free(node);
        return;
    }
    do {
        fini_packet(node->packet);
        free(node);
        node = next;
        next = node->next;
    } while (next != NULL);
    fini_packet(node->packet);
    free(node);
}

void  store_file(int filefd, Node *head) {
    assert(head != NULL);
    Node *node = head->next;
    if (node == NULL)
        return;
    char *packet;
    do {
        packet = node->packet;
        Write(filefd, PACKET_CONTENT(packet), GET_CONTENT_SIZE(packet));
        node = node->next;
    } while (node != NULL);
}

void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
        fprintf(stderr, "server child exit %d\n", pid);
    }
}
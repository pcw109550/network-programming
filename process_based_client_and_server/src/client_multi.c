#include <stdio.h>
#include "csapp.h"
#include "client_multi.h"
#include "common.h"

#define DEBUG

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [server_ip] [server_port] [number_of_requests]\n", argv[0]);
        return EXIT_FAILURE;
    }

    unsigned short initial_seq;
    unsigned short *seq;

    Signal(SIGCHLD, sigchld_handler);
    int clientfd; 
    char *host = argv[1];
    int port = atoi(argv[2]);
    int num_req = atoi(argv[3]);
    int pid, i = 0;
    char *packet;

    for (i = 0; i < num_req; i++) {
        msleep(SLEEP_DURATION);

        if (Fork() == 0) {
            init_rand();
            initial_seq = gen_sequence();
            seq = &initial_seq;

            clientfd = Open_clientfd(host, port);

            pid = getpid();
            /* handshake */

            // CLIENT_HELLO
            send_client_hello(clientfd, seq);

            // SERVER_HELLO
            packet = recv_packet(clientfd);
            if (GET_CMD(packet) != SV_HELLO) {
                send_error(clientfd, seq);
                return EXIT_FAILURE;
            }
            fini_packet(packet);

            // DATA_DELIVERY
            char *content = get_content(pid);
            send_data_delivery(clientfd, seq, content, strlen(content));
            free_content(content);

            // DATA_STORE
            char *filename = get_filename(pid);
            send_data_store(clientfd, seq, filename);
            free_filename(filename);

            Close(clientfd);
            exit(EXIT_SUCCESS);
        }
    }
    return EXIT_SUCCESS;
}

void send_client_hello(int sockfd, unsigned short *seq) {
#ifdef DEBUG
    fprintf(stdout, "> SEQ: %5hu: [CLIENT_HELLO]\n", *seq);
#endif
    char *packet = init_packet(*seq, PACKET_HEADER_SIZE, CL_HELLO, NULL);
    send(sockfd, packet, GET_SIZE(packet), 0);
    fini_packet(packet);
    *seq = *seq + 1;
}

void send_data_delivery(int sockfd, unsigned short *seq, char *content, unsigned short content_size) {
#ifdef DEBUG
    fprintf(stdout, "> SEQ: %5hu: [DATA_DELIVERY]: CONTENT SIZE: %5hu\n", *seq, content_size);
#endif
    char *packet = init_packet(*seq, CALC_PACKET_SIZE(content_size), DATA_DELIVERY, content);
    send(sockfd, packet, GET_SIZE(packet), 0);
    fini_packet(packet);
    *seq = *seq + 1;
}

void send_data_store(int sockfd, unsigned short *seq, char *filename) {
#ifdef DEBUG
    fprintf(stdout, "> SEQ: %5hu: [DATA_STORE]: FILENAME %s\n", *seq, filename);
#endif
    unsigned short filename_len = (unsigned short)strlen(filename);
    char *packet = init_packet(*seq, PACKET_HEADER_SIZE + filename_len, DATA_STORE, filename);
    send(sockfd, packet, GET_SIZE(packet), 0);
    fini_packet(packet);
    *seq = *seq + 1;
}

void sigchld_handler(int sig) {
    pid_t pid;
    while ((pid = waitpid(-1, 0, WNOHANG)) > 0) {
        fprintf(stderr, "client_multi child exit %d\n", pid);
    }
}

char *get_content(int pid) {
    char *content = (char *)calloc(MAX_CONTENT_SIZE_CLIENTMULTI, sizeof(char));
    sprintf(content, "I am a process %d", pid);
    return content;
}

void free_content(char *content) {
    free(content);
}

char *get_filename(int pid) {
    char *filename = (char *)calloc(MAX_CONTENT_SIZE_CLIENTMULTI, sizeof(char));
    sprintf(filename, "%d.txt", pid);
    return filename;
}

void free_filename(char *filename) {
    free(filename);
}

void msleep(int msec) {
    usleep(1000 * msec);
}

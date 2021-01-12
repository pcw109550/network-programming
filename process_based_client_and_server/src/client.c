#include <stdio.h>
#include "csapp.h"
#include "client.h"
#include "common.h"

#define DEBUG

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s [server_ip] [server_port] [filename]\n", argv[0]);
        return EXIT_FAILURE;
    }

    init_rand();
    unsigned short initial_seq = gen_sequence();
    unsigned short *seq = &initial_seq;

    int clientfd; 
    char *host = argv[1];
    int  port = atoi(argv[2]);
    char *filename = argv[3];
    int filesize = get_filesize(filename);
    int filefd = Open(filename, O_RDONLY, 0);
    char *packet;
    
    clientfd = Open_clientfd(host, port);

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
    send_file(clientfd, seq, filefd, filesize);
    close(filefd);

    // DATA_STORE
    send_data_store(clientfd, seq, filename);

    Close(clientfd); 

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

void send_file(int sockfd, unsigned short *seq, int filefd, int filesize) {
    int total_chunk_num, chunk_num = 0;
    unsigned short chunk_size = 0;
    int total_size_sent = 0, size_remaining = filesize;
    char *chunk;
    total_chunk_num = filesize / MAX_CONTENT_SIZE + (filesize % MAX_CONTENT_SIZE != 0);
    for (chunk_num = 0; chunk_num < total_chunk_num; chunk_num++) {
        chunk_size = size_remaining >= MAX_CONTENT_SIZE ? MAX_CONTENT_SIZE : size_remaining % MAX_CONTENT_SIZE;
        chunk = (char *)calloc(chunk_size, sizeof(char));
        Read(filefd, chunk, chunk_size);
        send_data_delivery(sockfd, seq, chunk, chunk_size);
        free(chunk);
        size_remaining -= chunk_size;
        total_size_sent += chunk_size;
    }
    assert(size_remaining == 0 && total_size_sent == filesize);
}
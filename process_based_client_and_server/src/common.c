#include <stdio.h>
#include "common.h"
#include "csapp.h"

#define DEBUG
#define SAFERAND

#ifdef DEBUG
char *msg[] = {"", "CLIENT_HELLO", "SERVER_HELLO", "DATA_DELIVERY", "DATA_STORE", "ERROR"};
#endif

void fill_packet_header(char *packet, unsigned short seq, unsigned short size, Command cmd) {
    assert(size >= 8);
    SET_VERSION(packet);
    SET_USERID(packet);
    SET_SEQ(packet, seq);
    SET_SIZE(packet, size);
    SET_CMD(packet, cmd);
}

void fill_packet_content(char *packet, char *content) {
    char *packet_content = PACKET_CONTENT(packet);
    unsigned short content_size = GET_CONTENT_SIZE(packet);
    assert(0 <= content_size && content_size <= MAX_CONTENT_SIZE);
    if (content != NULL)
        memcpy(packet_content, content, content_size);
}
    

char *init_packet(unsigned short seq, unsigned short size, Command cmd, char *content) {
    assert(size <= MAX_PACKET_SIZE);
    char *packet = (char *)calloc(size, sizeof(char));
    fill_packet_header(packet, seq, size, cmd);
    fill_packet_content(packet, content);
    return packet;
}

void fini_packet(char *packet) {
    free(packet);
}

void init_rand(void) {
#ifdef SAFERAND
    int fd = Open("/dev/urandom", O_RDONLY, 0);
    unsigned int seed = 0xdeadbeef;
    read(fd, (char *)&seed, 4);
    Close(fd);
    srand(seed);
#else
    time_t t;
    srand((unsigned)time(&t));
#endif
}

unsigned short gen_sequence(void) {
    return (unsigned short)(rand() & 0xFFFF);
}

void send_error(int sockfd, unsigned short *seq) {
#ifdef DEBUG
    fprintf(stdout, "> SEQ: %5hu: [ERROR]\n", *seq);
#endif
    char *packet = init_packet(*seq, PACKET_HEADER_SIZE, ERROR, NULL);
    send(sockfd, packet, GET_SIZE(packet), 0);
    fini_packet(packet);
    *seq = *seq + 1;
}

void packet_info(unsigned short seq, unsigned short size, Command cmd) {
    assert(1 <= cmd && cmd <= 5);
    fprintf(stdout, "< SEQ: %5hu: [%s]: CONTENT SIZE: %5hu\n", seq, msg[cmd], size);
}

char *recv_packet(int sockfd) {
    char *packet_header = init_packet(0, PACKET_HEADER_SIZE, 0, NULL);
    recv_packet_header(sockfd, packet_header);
    unsigned short seq = GET_SEQ(packet_header);
    unsigned short size = GET_SIZE(packet_header);
    Command cmd = GET_CMD(packet_header);
    char *packet = init_packet(seq, size, cmd, NULL);
    recv_packet_content(sockfd, packet);
#ifdef DEBUG
    packet_info(seq, GET_CONTENT_SIZE(packet), cmd);
#endif
    return packet;
}

void recv_packet_header(int sockfd, char *packet) {
    unsigned short total_size = PACKET_HEADER_SIZE, size_recved = 0, total_recved = 0;
    while (total_recved < total_size) {
        if ((size_recved = recv(sockfd, packet, PACKET_HEADER_SIZE - total_recved, 0)) < 0)
            break;
        else {
            total_recved += size_recved;
            packet += size_recved;
        }
    }
    assert(total_recved == total_size);
}

void recv_packet_content(int sockfd, char *packet) {
    char *packet_content = PACKET_CONTENT(packet);
    unsigned short content_size = GET_CONTENT_SIZE(packet);
    assert(0 <= content_size && content_size <= MAX_CONTENT_SIZE);
    unsigned short total_size = content_size, size_recved = 0, total_recved = 0;
    while (total_recved < total_size) {
        if ((size_recved = recv(sockfd, packet_content, total_size - total_recved, 0)) < 0)
            break;
        else {
            total_recved += size_recved;
            packet_content += size_recved;
        }
    }
    assert(total_recved == total_size);
}

int get_filesize(char *filename) {
    struct stat buffer;
    int status, filesize = 0;
    status = stat(filename, &buffer);
    if (status == 0 && S_ISREG(buffer.st_mode)) {
        filesize = buffer.st_size;
    } else {
        fprintf(stderr, "%s is not a proper txtfile\n", filename);
        exit(EXIT_FAILURE);
    }
    return filesize;
}
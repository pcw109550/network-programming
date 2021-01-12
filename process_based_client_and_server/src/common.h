#ifndef __COMMON_H_
#define __COMMON_H_
#include <assert.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>

#define VERSION 4
#define USERID  8
typedef enum Command { CL_HELLO = 1, SV_HELLO, DATA_DELIVERY, DATA_STORE, ERROR} Command;

#define SET_VERSION(packet)       packet[0] = VERSION;
#define SET_USERID(packet)        packet[1] = USERID;
#define SET_SEQ(packet, seq)      *(unsigned short *)&packet[2] = htons(seq)
#define SET_SIZE(packet, size)    *(unsigned short *)&packet[4] = htons(size)
#define SET_CMD(packet, cmd)      *(unsigned short *)&packet[6] = htons(cmd)
#define GET_SEQ(packet)           ntohs(*(unsigned short *)&packet[2])
#define GET_SIZE(packet)          ntohs(*(unsigned short *)&packet[4])
#define GET_CONTENT_SIZE(packet)  (unsigned short)(GET_SIZE(packet) - 8)
#define GET_CMD(packet)           ntohs(*(unsigned short *)&packet[6])

#define PACKET_CONTENT(packet)    &packet[8]
#define PACKET_HEADER_SIZE        (unsigned short)8
#define CALC_PACKET_SIZE(content_size)    (unsigned short)(content_size + 8)
#define MAX_CONTENT_SIZE          (unsigned short)0xFFF7    // 0xFFFF - 8
#define MAX_PACKET_SIZE           (unsigned short)0xFFFF

void  fill_packet_header(char *packet, unsigned short seq, unsigned short size, Command cmd);
void  fill_packet_content(char *packet, char *content);
char *init_packet(unsigned short seq, unsigned short size, Command cmd, char *content);
void  fini_packet(char *packet);

void  init_rand(void);
unsigned short gen_sequence(void);

void  send_error(int sockfd, unsigned short *seq);

char *recv_packet(int sockfd);
void  recv_packet_header(int sockfd, char *packet);
void  recv_packet_content(int sockfd, char *packet);
void  packet_info(unsigned short seq, unsigned short size, Command cmd);

int   get_filesize(char *filename);

#endif
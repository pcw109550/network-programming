#ifndef __CLIENT_H_
#define __CLIENT_H_
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void send_client_hello(int sockfd, unsigned short *seq);
void send_data_delivery(int sockfd, unsigned short *seq, char *content, unsigned short content_size);
void send_data_store(int sockfd, unsigned short *seq, char *filename);

void send_file(int sockfd, unsigned short *seq, int filefd, int filesize);

#endif
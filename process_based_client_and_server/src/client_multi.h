#ifndef __CLIENT_MULTI_H_
#define __CLIENT_MULTI_H_

#define MAX_CONTENT_SIZE_CLIENTMULTI 24 // longer than "I am a process 65535" or "65535.txt"
#define SLEEP_DURATION 100
void send_client_hello(int sockfd, unsigned short *seq);
void send_data_delivery(int sockfd, unsigned short *seq, char *content, unsigned short content_size);
void send_data_store(int sockfd, unsigned short *seq, char *filename);

void  sigchld_handler(int sig);

char *get_content(int pid);
void  free_content(char *content);
char *get_filename(int pid);
void  free_filename(char *filename);

void msleep(int msec);

#endif
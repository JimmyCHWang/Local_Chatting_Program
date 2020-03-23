#ifndef _CSE156_H_JW_
#define _CSE156_H_JW_

#include<sys/socket.h>
#include<sys/select.h>
#include<sys/types.h>
#include<sys/time.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstdint>
#include<cstdio>
#include<bits/stdc++.h>
#include<sys/stat.h>
#include<string>
#include<regex>
#include<cstdlib>
#include<cstring>
#include<pthread.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<signal.h>

#define slen sizeof(struct sockaddr_in)
#define MAX_PCK_LEN 8192

typedef struct sockaddr_in SA;

typedef struct so_ad_pair_t {
	int sockfd;
	SA addr;
} sapair;

typedef struct pck_n_t {
	char text[MAX_PCK_LEN+1];
	bool eof;
} pck_n;

// Called when facing an error, exit(1).
void error(const char str[]);

FILE* Fopen(const char* path, const char* mode);

void mk_sapair(sapair* p, int sockfd, const SA addr);

// Build a TCP socket
int Create_TCP();

// Build a UDP socket
int Create_UDP();

// Set socket address
SA Set_address(const in_addr_t addr, const int port);

// Server> Bind Socket
int Bind(int sockfd, const struct sockaddr* addr);

// Client> TCP Connection
int Connect(int sockfd, const struct sockaddr* addr);

// Server> Listen Socket
int Listen(int sockfd, const int max_conn);

// Server> Accept Connection
int Accept(int sockfd, struct sockaddr* addr, socklen_t* servlen);

// UDP: Send to some port
int Sendto(int sockfd, const char* buf, int buf_size, int flags, const SA* servaddr, socklen_t servlen);

// UDP: Receive from some port
int Recvfrom(int sockfd, char* buf, int buf_size, int flags, SA* servaddr, socklen_t* servlen);

void Set_Timeval(struct timeval* t, long tv_sec, long tv_usec);

// Is it a valid IP
bool isIP(const char ip[]);

// Is it a positive number
bool isPNumber(const char number[]);

// Book Provided Library
ssize_t	readn(int fd, void *vptr, size_t n);
ssize_t	writen(int fd, const void *vptr, size_t n);

void print_addr(SA addr);

#endif



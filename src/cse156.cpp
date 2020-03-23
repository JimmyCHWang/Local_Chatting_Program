#include "cse156.h"

using namespace std;

void error(const char str[]) {
	fprintf(stderr, "%s\n", str);
	exit(1);
	return;
}

FILE* Fopen(const char* path, const char* mode) {
	FILE* f;	
	if ((f = fopen(path, mode)) == NULL) {
		string err_mes = string("ERROR: cannot open server info text file: ") + string(path);
		error(err_mes.c_str());
	}
	return f;
}

void mk_sapair(sapair* p, int sockfd, const SA addr) {
	p->sockfd = sockfd;
	p->addr = addr;
	return;
}

int Create_TCP() {
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) 
		error("ERROR: Failed when creating TCP socket.");
	return sockfd;
}

int Create_UDP() {
	int sockfd;
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
		error("ERROR: Failed when creating UDP socket.");
	return sockfd;
}

SA Set_address(const in_addr_t addr, const int port) {
	SA sa;
	bzero(&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = addr;
	return sa;
}

int Bind(int sockfd, const struct sockaddr* addr) {
	if ( bind(sockfd, addr, sizeof(*addr)) < 0 )
		return -1;
		//error("ERROR: Failed when Binding the port.");
	return 0;
}

int Connect(int sockfd, const struct sockaddr* addr) {
	int ret_val = -1;
	if ( (ret_val = connect(sockfd, addr, sizeof(*addr))) < 0 )
		return -1;
		//error("ERROR: Failed when Connect to the port.");
	return ret_val;
}

int Listen(int sockfd, const int max_conn) {
	if ( listen(sockfd, max_conn) < 0 )
		//return -1;
		error("Server> Cannot Listen to the port.");
	return 0;
}

int Accept(int sockfd, struct sockaddr* addr, socklen_t* servlen) {
	int ret_val;
	if ( (ret_val = accept(sockfd, addr, servlen)) < 0 )
		return -1;
		//error("Server> Failed when Accepting the connection");
	return ret_val;
}


int Sendto(int sockfd, const char* buf, int buf_size, int flags, const SA* servaddr, socklen_t servlen) {
	int len;
	if ( (len = sendto(sockfd, buf, buf_size, flags, (struct sockaddr*) servaddr, servlen)) < 0 )
		return -1;
	return len;
}

int Recvfrom(int sockfd, char* buf, int buf_size, int flags, SA* servaddr, socklen_t* servlen) {
	int len;
	if ( (len = recvfrom(sockfd, buf, buf_size, flags,(struct sockaddr*) servaddr, servlen)) < 0 )
		return -1;
	return len;
}

void Set_Timeval(struct timeval* t, long tv_sec, long tv_usec) {
	t->tv_sec = tv_sec;
	t->tv_usec = tv_usec;
	return;
}

bool isIP(const char ip[]) {
	regex ip_format = regex("^([0-9]{1,3}.){3}[0-9]{1,3}(:[0-9]{1,5}){0,1}$"); // Match IP with IP pattern in regular expressions
	if (!std::regex_match(std::string(ip), ip_format)) {			// If doesn't match, that's an error
		return false;
	}
	return true;
}

bool isPNumber(const char number[]) {
	int n = atoi(number);
	if (n <= 0) return false;
	return true;
}

ssize_t	readn(int fd, void *vptr, size_t n) {
	size_t	nleft;
	ssize_t	nread;
	char	*ptr;

	ptr = (char*) vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;		/* and call read() again */
			else
				return(-1);
		} else if (nread == 0)
			break;				/* EOF */

		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}

ssize_t	writen(int fd, const void *vptr, size_t n) {
	size_t		nleft;
	ssize_t		nwritten;
	const char	*ptr;

	ptr = (char*) vptr;
	nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
			if (nwritten < 0 && errno == EINTR)
				nwritten = 0;		/* and call write() again */
			else
				return(-1);			/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

void print_addr(SA addr) {
	char buffer[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(addr.sin_addr), buffer, INET_ADDRSTRLEN);
	
	printf("IP address = %s, Port = %d\n", buffer, (int) addr.sin_port);
	return;
}
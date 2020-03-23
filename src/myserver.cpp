
#include "final_prot.h"

#define MAX_CLIENTS 16

using namespace std;

typedef struct ClientInfo_t {
	bool valid;
	int id;
	char client_id[MAX_ID_LENGTH];
	int sockfd;
	states state;
	SA addr; // Address only for Chat
} Clinfo;

Clinfo clinfo[MAX_CLIENTS];

pthread_t threads[MAX_CLIENTS];

int server_socket;

// Function Prototypes
void arg_checks(int argc, char* argv[]);
void generate_waitlist (char* buffer);
void* thread_module(void* argv);

// For Debug: Print Client Info, use Ctrl + Z
void sig_handler(int signo) {
	if (signo == SIGINT) {
		int cnt = 0;
		fprintf(stderr, "\n********DEBUG MODE:********\n\n");
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clinfo[i].valid) {
				cnt++;
				fprintf(stderr, "ID = %d; Name = %s; state = %d;\n", clinfo[i].id, clinfo[i].client_id, clinfo[i].state);
			}
		}
		fprintf(stderr, "Total Valid Users: %d\n", cnt);
		fprintf(stderr, "********END DEBUG MODE********\n\n");
	}
	if (signo == SIGTSTP) {
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clinfo[i].valid) {
				close(clinfo[i].sockfd);
			}
		}
		close(server_socket);
		fprintf(stderr, "Ctrl+Z, terminated.\n");
		exit(1);
	}
	return;
}

int main(int argc, char* argv[]) {
	
	arg_checks(argc, argv);

	server_socket = Create_TCP();
	printf("Server> Socket Created\n");
	int server_port = atoi(argv[1]);

	SA server_addr = Set_address(INADDR_ANY, server_port);
	
	if (Bind(server_socket, (struct sockaddr*) &server_addr) < 0) {
		error("Server> ERROR: Failed when Binding the port.");
	}

	Listen(server_socket, MAX_CLIENTS);
	
	printf("Server> Port binded %d. Server Up.\n", server_port);

	packet client_packet;
	bzero(&clinfo, sizeof(clinfo));

	bzero(&client_packet, sizeof(packet));
	
	struct sigaction act;
	bzero(&act, sizeof(struct sigaction));
	act.sa_handler = sig_handler;
	if (sigaction(SIGTSTP, &act, NULL) < 0) {
		error("Client ERROR: Failed when catching the signal SIGTSTP");
	}
	if (sigaction(SIGINT, &act, NULL) < 0) {
		error("Client ERROR: Failed when catching the signal SIGINT");
	}

	while (true) {
		SA cli_addr = Set_address(INADDR_ANY, server_port);
		socklen_t socklen = slen;
		int cli_socket = Accept(server_socket, (struct sockaddr*) &cli_addr, &socklen);
		if (cli_socket < 0) {
			fprintf(stderr, "Server> WARNING: Accept() return -1");
			continue;
		}

		int vacant = 0;
		while ((vacant < MAX_CLIENTS) && (clinfo[vacant].valid == true))
			vacant++;
		clinfo[vacant].valid = true;
		clinfo[vacant].id = vacant+1;
		clinfo[vacant].sockfd = cli_socket;
		clinfo[vacant].state = INFO;

		printf("Server> Incoming new user, assigned ID %d.\n", vacant);

		pthread_create(threads+vacant, NULL, thread_module, &clinfo[vacant]);
	}
	
	close(server_socket);

	return 0;
}

void arg_checks(int argc, char* argv[]) {
	// Argument Check
	if (argc != 2) 
		error("Server> ERROR: argument number not matched. See USER_GUIDE_SERVER.");
		
	if (!isPNumber(argv[1]))
		error("Server> ERROR: please input a valid connection number.");

	return;
}

void generate_waitlist (char* buffer) {
	vector<string> cont;
	cont.clear();
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clinfo[i].valid && clinfo[i].state == WAIT) 
			cont.push_back(string(clinfo[i].client_id));
	}
	sort(cont.begin(), cont.end());

	string reply = "";
	char name_buf[MAX_ID_LENGTH];
	for (int i = 0; i < (int)cont.size(); i++) {
		sprintf(name_buf, "%d) %s\n", i+1, cont[i].c_str());
		reply = reply + string(name_buf);
		bzero(name_buf, sizeof(name_buf));
	}
	
	strcpy(buffer, reply.c_str());
	
	return;	
}

Clinfo* client_arp_id (uint8_t cli_id) {
	int i = 0;
	while (i < MAX_CLIENTS) {
		if (clinfo[i].valid) {
			 if (clinfo[i].id == (int)cli_id)
				break;
		}
		i++;
	}
	if (i < MAX_CLIENTS) {
		return clinfo+i;
	}
	return NULL;
}

Clinfo* client_arp_clid (char* username) {
	int i = 0;
	while (i < MAX_CLIENTS) {
		if (clinfo[i].valid) {
			 if (strcmp(clinfo[i].client_id, username) == 0)
				 break;
		}			
		i++;
	}
	if (i < MAX_CLIENTS) {
		return clinfo+i;
	}
	return NULL;
}

void* thread_module (void* argv) {
	Clinfo* cinfo = ((Clinfo*)argv);
	packet cli_packet;
	cinfo->state = INFO;

	bool shutdown = false;

	char buffer[MAX_TEXT_LEN];

	while (!shutdown) {
		Server_RecvPck (cinfo->sockfd, &cli_packet);
		bzero(buffer, sizeof(buffer));
		
		printf("Thread %d> Opcode Received: 0x%02X\n", cinfo->id-1, cli_packet.opcode);

		switch (cli_packet.opcode) {
			case (0x01):{
				bool flag = true;
				for (int i = 0; i < MAX_CLIENTS; i++)
					if (clinfo[i].valid && (strcmp(clinfo[i].client_id, cli_packet.text) == 0)) flag = false;
				if (flag) {
					Server_LoginAcc (cinfo->sockfd, cinfo->id);
					printf("Thread %d> LoginAccepted: ID = %d\n", cinfo->id-1, cinfo->id);
					strcpy(cinfo->client_id, cli_packet.text);
				} else {
					Server_LoginRej (cinfo->sockfd);
					printf("Thread %d> LoginRejected\n", cinfo->id-1);
					shutdown = true;
				}	
				break;}
			case (0x02):{
				generate_waitlist(buffer);
				Server_SendList(cinfo->sockfd, cinfo->id, buffer);
				printf("Thread %d> Sent Waitlist\n", cinfo->id-1);
				break;}
			case (0x03):{
				Clinfo* reply = NULL;
				if (cli_packet.to_id == 255) {
					reply = client_arp_clid(cli_packet.text);
				} else {
					reply = client_arp_id(cli_packet.to_id);
				}
				if (reply == NULL) {
					printf("Thread %d> Requested User Not Found\n", cinfo->id-1);
					Server_IDARP(cinfo->sockfd, cinfo->id, 255, NULL, buffer);
				} else {
					printf("Thread %d> Sent ARP: %s, %d\n", cinfo->id-1, reply->client_id, (int)(reply->id));
					Server_IDARP(cinfo->sockfd, cinfo->id, reply->id, &(reply->addr), reply->client_id);
				}

				break;}
			case (0x04):{
				fprintf(stderr, "Thread %d> ERROR: Opcode 0x04 Unimplemented\n", cinfo->id-1);
				break;}
			case (0x05):{
				cinfo->state = WAIT;
				printf("Thread %d> Changed State: WAIT\n", cinfo->id-1);
				cinfo->addr = cli_packet.addr;
				printf("Thread %d> Received Address: ", cinfo->id-1);
				print_addr(cli_packet.addr);
				break;}
			case (0x06):{
				cinfo->state = CHAT;
				printf("Thread %d> Changed State: CHAT\n", cinfo->id-1);
				break;}
			case (0x07):{
				cinfo->state = INFO;
				printf("Thread %d> Changed State: INFO\n", cinfo->id-1);
				break;}
			case (0x0F):{
				shutdown = true;
				break;}
			default:{
				fprintf(stderr, "Thread %d> ERROR: Unrecognized Opcode: 0x%02X\n", cinfo->id-1, cli_packet.opcode);
				shutdown = true;
			}
		}
	}
	close(cinfo->sockfd);
	fprintf(stdout, "Thread %d> Thread Terminated.\n", cinfo->id - 1);
	bzero(cinfo, sizeof(Clinfo));
	cinfo->valid = false;
	pthread_exit(NULL);
}

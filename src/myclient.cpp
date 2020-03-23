
#include "final_prot.h"

using namespace std;

char myname[MAX_ID_LENGTH];
char prompt[MAX_ID_LENGTH+1];
states mystate;
bool state_update;
bool comm_end;
bool termination;
uint8_t myid;

char mytext[MAX_TEXT_LEN];
char buffer[MAX_TEXT_LEN];

void arg_checks(int argc, char* argv[]);
void* wait_thread(void* argv);
void info_handler(int serv_fd);
void wait_handler(int serv_fd);
void chat_handler(int serv_fd);

int server_sock;

int cli_sock;
uint8_t cli_id;
SA cli_sock_addr;
char cli_name[MAX_ID_LENGTH];

pthread_t wait_th;

void fresh_client() {
	cli_sock = Create_TCP();
//	SA sockaddr = Set_address(inet_addr("127.0.0.1"), 0);
//	Bind(cli_sock, (struct sockaddr*) &sockaddr);
//	socklen_t slent = slen;
	bzero(&cli_sock_addr, sizeof(SA));
//	getsockname(cli_sock, (struct sockaddr*) &cli_sock_addr, &slent);
	return;
}

void sig_handler(int signo) {
	if (signo == SIGINT) {
		printf("^C\n");
		state_update = true;
	}
	if (signo == SIGTSTP) {
		printf("^Z\n");
		if (cli_sock != 0) close(cli_sock);
		close(server_sock);
		rl_callback_handler_remove();
		exit(1);
	}
	return;
}


void my_rlhandler(char* line) {
	if (line == NULL) {
		termination = true;
	} else if (line != NULL) {
		if (*line != 0) {
			add_history(line);
		}
		if (strlen(line) < MAX_TEXT_LEN) {
			strcpy(mytext, line);
			comm_end = true;
		} else {
			fprintf(stderr, "WARNING: TEXT MESSAGE TOO LONG\n");
		}
		free(line);
	}
	
	if (comm_end) {
		if (strlen(mytext) == 0) {
			comm_end = false;
			return;
		}
		switch (mystate) {
			case (INFO):
				info_handler(server_sock);
				break;
			case (WAIT):
				wait_handler(server_sock);
				break;
			case (CHAT):
				chat_handler(server_sock);
				break;
			default:
				fprintf(stderr, "WARNING: Unrecognized state\n");
				break;
		}
		bzero(mytext, sizeof(mytext));
		comm_end = false;
	}
		
	return;
}
		

int main(int argc, char* argv[]) {
	
	arg_checks(argc, argv);
	
	mystate = INFO;
	
	bzero(&myname, sizeof(myname));
	strcpy(myname, argv[3]);
	
	strcpy(prompt, argv[3]);
	strcat(prompt, "> ");

	// Server Socket to connect server;
	server_sock = Create_TCP();
	
	// Client Socket waiting for access;
	cli_sock = 0;
	cli_id = 255;
	
	// Connect to Server First
	SA server = Set_address(inet_addr(argv[1]), atoi(argv[2]));
	if (Connect(server_sock, (struct sockaddr*) &server) < 0)
		error("Client ERROR: Cannot reach the server");
	
	printf("Client Connected to the server.\n");
	Client_Login(server_sock, myname);
	
	packet serv_pck;
	packet cli_pck;
		
	bzero(&serv_pck, sizeof(packet));
	Server_RecvPck(server_sock, &serv_pck);
	
		
	if (serv_pck.opcode != 0x11) {
		close(server_sock);
		close(cli_sock);
		error("Client ID is already taken.");
	}
	
	myid = serv_pck.to_id;
	
	printf("Server: Welcome, %s!\n", myname);
	
	// Signal Handle
	struct sigaction act;
	bzero(&act, sizeof(struct sigaction));
	act.sa_handler = sig_handler;
	if (sigaction(SIGINT, &act, NULL) < 0) {
		close(server_sock);
		error("Client ERROR: Failed when catching the signal SIG_INT.");
	}
	if (sigaction(SIGTSTP, &act, NULL) < 0) {
		close(server_sock);
		error("Client ERROR: Failed when catching the signal SIG_TSTP.");
	}
	
	// Readline Handle
	rl_callback_handler_install(prompt, (rl_vcpfunc_t*) &my_rlhandler);
	
	// fdset
	fd_set rset;
	
	// Global Var Init
	state_update = false;
	comm_end = false;
	termination = false;
		
	while (!termination) {
		
		struct timeval tmout;
		Set_Timeval(&tmout, 0, 10000);

		FD_ZERO(&rset);
		FD_SET(STDIN_FILENO, &rset);
		FD_SET(server_sock, &rset);
		if (cli_sock != 0 && mystate != INFO) FD_SET(cli_sock, &rset);
		
		//printf("\a");
	
		select(FD_SETSIZE, &rset, NULL, NULL, &tmout);
		
		if (FD_ISSET(STDIN_FILENO, &rset) && !state_update) {
			rl_callback_read_char();
		}
		if (state_update) {
			state_update = false;
			if (mystate == WAIT) {
				pthread_cancel(wait_th);
				close(cli_sock);
				printf("Stopped waiting.\n");
			}
			if (mystate == CHAT) {
				Client_Terminate(cli_sock, myid, cli_id);
				close(cli_sock);
				printf("Left conversation with %s.\n", cli_name);
			}
			mystate = INFO;
			Client_Info(server_sock, myid);
			bzero(cli_name, sizeof(cli_name));
			cli_sock = 0;
			rl_forced_update_display();
			continue;
		}
		
		if (FD_ISSET(server_sock, &rset)) {
			bzero(&serv_pck, sizeof(serv_pck));
			Server_RecvPck(server_sock, &serv_pck);
			switch (serv_pck.opcode) {
				case (0x12): {
					for (uint32_t i=0; i<serv_pck.text_len; i++)
						printf("%c", serv_pck.text[i]);
					rl_forced_update_display();
					break;
				}
				default:
					fprintf(stderr, "Unexpected packet from server, opcode 0x%02X\n", serv_pck.opcode);
					termination = true;
			}
		}
		if (mystate == CHAT && FD_ISSET(cli_sock, &rset)) {
			bzero(&cli_pck, sizeof(packet));
			Server_RecvPck(cli_sock, &cli_pck);
			switch (cli_pck.opcode) {
				case (0x23): {
					printf("\n");
					printf("%s: ", cli_name);
					for (uint32_t i=0; i<cli_pck.text_len; i++)
						printf("%c", cli_pck.text[i]);
					printf("\n");
					rl_forced_update_display();
					break;
				}
				case (0x24): {
					printf("\n%s terminates the conversation.\n", cli_name);
					mystate = INFO;
					Client_Info(server_sock, myid);
					cli_sock = 0;
					rl_forced_update_display();
					break;
				}
				default:
					fprintf(stderr, "\nUnexpected packet from client, opcode 0x%02X\n", cli_pck.opcode);
					termination = true;
			}
		}
	}
	
	if (cli_sock != 0) close(cli_sock);
	rl_callback_handler_remove();
	close(server_sock);
	
	printf("\nBye!\n");

	return 0;
}

bool isDigitOrNum(char* id) {
	if (strlen(id) > MAX_ID_LENGTH) return false;
	for (int i = 0; i < (int)strlen(id); i++) {
		if (!((id[i] >= 'A' && id[i] <= 'Z') || (id[i] >= 'a' && id[i] <= 'z') || (id[i] >= '0' && id[i] <= '9')))
			return false;
	}
	return true;
}

void arg_checks(int argc, char* argv[]) {
	// Argument Check
	if (argc != 4) 
		error("Client> ERROR: argument number not matched. See USER_GUIDE_CLIENT.");
		
	if (!isIP(argv[1]))
		error("Client> ERROR: server info text file not exist.");

	if (!isPNumber(argv[2]))
		error("Client> ERROR: please input a valid connection number.");

	if (!isDigitOrNum(argv[3]))
		error("Client> ERROR: Invalid username. See USER_GUIDE_CLIENT");

	return;
}

void info_handler(int serv_fd) {
	comm_end = 0;
	bzero(buffer, sizeof(buffer));
	if (mytext[0] != '/') {
		printf("ERROR, command unrecognized.\n");
		return;
	}
	strcpy(buffer, mytext);
	buffer[strlen(mytext)] = ' ';
	buffer[strlen(mytext)+1] = '\0';
	char* ptr = strtok(buffer, " ");
	if (strcmp(ptr, "/quit") == 0) {
		Client_Quit(serv_fd, myid);
		termination = true;
	} else if (strcmp(ptr, "/list") == 0) {
		Client_ReqList(serv_fd, myid);
		bzero(buffer, sizeof(buffer));
		Client_ReqList_handler(serv_fd, buffer);
		printf("%s", buffer);
	} else if (strcmp(ptr, "/wait") == 0) {
		fresh_client();
		printf("Waiting for connection.\n");
		pthread_create(&wait_th, NULL, &wait_thread, NULL);
		mystate = WAIT;
	} else if (strcmp(ptr, "/connect") == 0) {
		ptr = strtok(NULL, " ");
		string his_name = string(ptr);
		if (strcmp(ptr, myname) == 0) {
			printf("ERROR: You cannot connect to yourself.\n");
			return;
		}
		Client_IDARP(serv_fd, myid, ptr, 255);
		SA target_addr;
		bzero(buffer, sizeof(buffer));
		if (Client_IDARP_handler(serv_fd, &cli_id, &target_addr, buffer) < 0) {
			printf("ERROR: User %s does not exist.\n", his_name.c_str());
			return;
		}
		
		strcpy(cli_name, his_name.c_str());
		
		int temp_fd;
		fresh_client();
		//printf("Ready to connect %s. Number ID = %d. Received addr: ", his_name.c_str(), cli_id);
		//print_addr(target_addr);
		if ( (temp_fd = Connect(cli_sock, (struct sockaddr*) &target_addr)) < 0) {
			printf("ERROR: Failed to connect user %s.\n", his_name.c_str());
			return;
		}
		Client_Connect(cli_sock, myid, cli_id, myname);
		if (Client_Connect_handler(cli_sock) < 0) {
			printf("ERROR: Didn't receive accept message.\n");
			return;
		}
		
		Client_Chat(serv_fd, myid);
		printf("Connected to %s.\n", cli_name);
		mystate = CHAT;
	} else if (strcmp(ptr, "/state") == 0) {
		print_state(mystate);
	} else {
		printf("Command %s not recognized.\n", ptr);
	}
	return;
}

void wait_handler(int serv_fd) {
	comm_end = 0;
	bzero(buffer, sizeof(buffer));
	if (mytext[0] != '/') {
		printf("ERROR, command unrecognized.\n");
		return;
	}
	strcpy(buffer, mytext);
	buffer[strlen(mytext)] = ' ';
	buffer[strlen(mytext)+1] = '\0';
	char* ptr = strtok(buffer, " ");
	if (strcmp(ptr, "/quit") == 0) {
		Client_Quit(serv_fd, myid);
		pthread_cancel(wait_th);
		termination = true;
	} else if (strcmp(ptr, "/list") == 0) {
		Client_ReqList(serv_fd, myid);
		bzero(buffer, sizeof(buffer));
		Client_ReqList_handler(serv_fd, buffer);
		printf("%s", buffer);
	} else if (strcmp(ptr, "/wait") == 0) {
		printf("ERROR, You are already waiting.\n");
	} else if (strcmp(ptr, "/connect") == 0) {
		printf("ERROR, Cannot connect in wait mode.\n");
	} else if (strcmp(ptr, "/state") == 0) {
		print_state(mystate);
	} else {
		printf("Command %s not recognized.\n", ptr);
	}
	return;
}
	
void chat_handler(int serv_fd) {
	comm_end = 0;
	bzero(buffer, sizeof(buffer));
	if (mytext[0] != '/') {
		Client_SendText(cli_sock, myid, cli_id, mytext);
		return;
	}
	strcpy(buffer, mytext);
	buffer[strlen(mytext)] = ' ';
	buffer[strlen(mytext)+1] = '\0';
	char* ptr = strtok(buffer, " ");
	if (strcmp(ptr, "/quit") == 0) {
		Client_Quit(serv_fd, myid);
		Client_Terminate(cli_sock, myid, cli_id);
		close(cli_sock);
		termination = true;
	} else if (strcmp(ptr, "/list") == 0) {
		printf("ERROR, Cannot request list in chat mode.\n");
	} else if (strcmp(ptr, "/wait") == 0) {
		printf("ERROR, Cannot wait in chat mode.\n");
	} else if (strcmp(ptr, "/connect") == 0) {
		printf("ERROR, Cannot connect in chat mode.\n");
	} else if (strcmp(ptr, "/state") == 0) {
		print_state(mystate);
	} else {
		printf("Command %s not recognized.\n", ptr);
	}
	return;
}
	
void* wait_thread(void* argv) {
	int cli_fd = cli_sock;
	SA cli_addr;
	socklen_t cli_len = slen;
	
	cli_addr = cli_sock_addr;
	
	Listen(cli_sock, 1);
	int temp_fd;
	socklen_t slent = slen;
	bzero(&cli_sock_addr, sizeof(SA));
	getsockname(cli_sock, (struct sockaddr*) &cli_sock_addr, &slent);
	Client_Wait(server_sock, myid, cli_sock_addr);
	temp_fd = Accept(cli_fd, (struct sockaddr*) &cli_addr, &cli_len);
	if (cli_fd < 0) {
		printf("\nFailed to connect.\n");
		rl_forced_update_display();
		pthread_exit(NULL);
	} else {
		close(cli_fd);
		cli_sock = temp_fd;
		cli_sock_addr = cli_addr;
	}
	
	Client_Accept_handler(cli_sock, myid, &cli_id, cli_name);
	printf("\nConnection from %s.\n", cli_name);
	mystate = CHAT;
	Client_Accept(cli_sock, myid, cli_id, myname);
	Client_Chat(server_sock, myid);
	rl_forced_update_display();
	
	pthread_exit(NULL);
}


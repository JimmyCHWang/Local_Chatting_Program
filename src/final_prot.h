#ifndef _FINAL_PROT_H_
#define _FINAL_PROT_H_

#include "cse156.h"
#define MAX_TEXT_LEN 2048
#define MAX_ID_LENGTH 256

typedef enum states_t {
	INFO, WAIT, CHAT
} states;

typedef struct packet_t {
	uint8_t opcode;
	uint8_t from_id;
	uint8_t to_id;
	uint8_t flag;
	SA addr; 
	uint32_t text_len;
	char text[MAX_TEXT_LEN];
} packet;

#define pcklen (sizeof(packet))

void Client_Login (const int sockfd, const char* username);
uint8_t Client_Login_handler (const int sockfd);

void Client_ReqList (const int sockfd, const uint8_t from_id);
int Client_ReqList_handler (const int sockfd, char buffer[]);

void Client_IDARP (const int sockfd, const uint8_t from_id, const char* username, const uint8_t to_id);
int Client_IDARP_handler (const int sockfd, uint8_t* cli_id, SA* addr, char name_buffer[]);

void Client_Wait (const int sockfd, const uint8_t from_id, const SA listen_addr);

void Client_Chat (const int sockfd, const uint8_t from_id);

void Client_Info (const int sockfd, const uint8_t from_id);

void Client_Quit (const int sockfd, const uint8_t from_id);

void Server_RecvPck (const int sockfd, packet* buffer);

void Server_LoginAcc (const int sockfd, const uint8_t assign_id);

void Server_SendList (const int sockfd, const uint8_t to_id, const char* list);

void Server_IDARP (const int sockfd, const uint8_t to_id, const uint8_t probe_id, const SA* addr, const char* username);

void Server_LoginRej (const int sockfd);

void Client_Connect (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char* username);
int Client_Connect_handler (const int sockfd);

void Client_Accept (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char* username);
int Client_Accept_handler (const int sockfd, const uint8_t from_id, uint8_t* to_id, char name_buffer[]);

void Client_SendText (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char text[]);

void Client_Terminate (const int sockfd, const int from_id, const uint8_t to_id);

// Return the opcode of received packet.
uint8_t Client_RecvText_handler (const int sockfd, char buffer[]);

void print_state(states ms);

#endif


#include "final_prot.h"

void Client_Login (const int sockfd, const char* username) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x01;
	pck.from_id = 255;
	pck.to_id = 0;
	pck.text_len = strlen(username);
	strcpy(pck.text, username);
	writen(sockfd, &pck, pcklen);
	return;
}
uint8_t Client_Login_handler (const int sockfd) {
	packet pck;
	bzero(&pck, sizeof(packet));
	readn(sockfd, &pck, pcklen);

	if (pck.opcode == 0x11) {
		return pck.to_id;
	}

	return 255; // Indicating Rejected
}	


void Client_ReqList (const int sockfd, const uint8_t from_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x02;
	pck.from_id = from_id;
	pck.to_id = 0;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;
}

int Client_ReqList_handler (const int sockfd, char buffer[]) {
	packet pck;
	bzero(&pck, sizeof(packet));
	readn(sockfd, &pck, pcklen);

	if (pck.opcode == 0x12) {
		for (uint32_t i = 0; i < pck.text_len; i++) {
			buffer[i] = pck.text[i];
		}
		buffer[pck.text_len] = '\0';
		return pck.text_len;
	}

	return -1; // Indicating an error occurred.
}	

void Client_IDARP (const int sockfd, const uint8_t from_id, const char* username, const uint8_t to_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x03;
	pck.from_id = from_id;
	
	if (to_id == 255) {
		// User Name Mode
		pck.to_id = 255;
		pck.text_len = strlen(username);
		strcpy(pck.text, username);
	} else {
		// ID Mode
		pck.to_id = to_id;
		pck.text_len = 0;
	}

	writen(sockfd, &pck, pcklen);
	return;
}

int Client_IDARP_handler (const int sockfd, uint8_t* cli_id, SA* addr, char name_buffer[]) {
	packet pck;
	bzero(&pck, sizeof(packet));
	readn(sockfd, &pck, pcklen);

	if (pck.opcode == 0x13 && pck.from_id != 255) {
		*cli_id = pck.from_id;
		*addr = pck.addr;
		for (uint32_t i = 0; i < pck.text_len; i++) {
			name_buffer[i] = pck.text[i];
		}
		name_buffer[pck.text_len] = '\0';
		return 0;
	}

	return -1;
}

void Client_Wait (const int sockfd, const uint8_t from_id, const SA listen_addr) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x05;
	pck.from_id = from_id;
	pck.to_id = 0;
	pck.addr = listen_addr;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;
}

/*void _Client_Opcode_ (const int sockfd, const uint8_t from_id, const uint8_t opcode);*/

void Client_Chat (const int sockfd, const uint8_t from_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x06;
	pck.from_id = from_id;
	pck.to_id = 0;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}

void Client_Info (const int sockfd, const uint8_t from_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x07;
	pck.from_id = from_id;
	pck.to_id = 0;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}

void Client_Quit (const int sockfd, const uint8_t from_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x0F;
	pck.from_id = from_id;
	pck.to_id = 0;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}


void Server_RecvPck (const int sockfd, packet* buffer) {
	bzero(buffer, sizeof(packet));
	readn(sockfd, buffer, pcklen);	
	return;
}

void Server_LoginAcc (const int sockfd, const uint8_t assign_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x11;
	pck.from_id = 0;
	pck.to_id = assign_id;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}

void Server_SendList (const int sockfd, const uint8_t to_id, const char* list) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x12;
	pck.from_id = 0;
	pck.to_id = to_id;
	pck.text_len = strlen(list);
	strcpy(pck.text, list);
	writen(sockfd, &pck, pcklen);
	return;	
}

void Server_IDARP (const int sockfd, const uint8_t to_id, const uint8_t probe_id, const SA* addr, const char* username) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x13;
	pck.from_id = probe_id;
	pck.to_id = to_id;
	if (addr != NULL) {
		pck.addr = *addr;
	}
	pck.text_len = strlen(username);
	strcpy(pck.text, username);
	writen(sockfd, &pck, pcklen);
	return;	
}

void Server_LoginRej (const int sockfd) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x15;
	pck.from_id = 0;
	pck.to_id = 255;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}

void Client_Connect (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char* username) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x21;
	pck.from_id = from_id;
	pck.to_id = to_id;
	pck.text_len = strlen(username);
	strcpy(pck.text, username);
	writen(sockfd, &pck, pcklen);
	return;	
}
int Client_Connect_handler (const int sockfd) {
	packet buffer; 
	bzero(&buffer, sizeof(packet));
	int rlen = readn(sockfd, &buffer, pcklen);
	if (rlen <= 0) return -1;
	if (buffer.opcode == 0x22)
		return 1;

	return -1;
}

void Client_Accept (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char* username) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x22;
	pck.from_id = from_id;
	pck.to_id = to_id;
	pck.text_len = strlen(username);
	strcpy(pck.text, username);
	writen(sockfd, &pck, pcklen);
	return;	
}

int Client_Accept_handler (const int sockfd, const uint8_t from_id, uint8_t* to_id, char name_buffer[]) {
	packet pck;
	bzero(&pck, sizeof(packet));
	readn(sockfd, &pck, pcklen);
	if (pck.opcode == 0x21 && pck.to_id == from_id) {
		*to_id = pck.from_id;
		for (uint32_t i = 0; i < pck.text_len; i++) {
			name_buffer[i] = pck.text[i];
		}
		name_buffer[pck.text_len] = '\0';
	} else {
		return -1;
	}
	return 0;
}

void Client_SendText (const int sockfd, const uint8_t from_id, const uint8_t to_id, const char text[]) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x23;
	pck.from_id = from_id;
	pck.to_id = to_id;
	pck.text_len = strlen(text);
	strcpy(pck.text, text);
	writen(sockfd, &pck, pcklen);
	return;	
}

void Client_Terminate (const int sockfd, const int from_id, const uint8_t to_id) {
	packet pck;
	bzero(&pck, sizeof(packet));
	pck.opcode = 0x24;
	pck.from_id = from_id;
	pck.to_id = to_id;
	pck.text_len = 0;
	writen(sockfd, &pck, pcklen);
	return;	
}

// Return the opcode of received packet.
uint8_t Client_RecvText_handler (const int sockfd, char buffer[]) {
	packet pck;
	bzero(&pck, sizeof(packet));
	readn(sockfd, &pck, pcklen);
	for (uint32_t i = 0; i < pck.text_len; i++) {
		buffer[i] = pck.text[i];
	}
	buffer[pck.text_len] = '\0';
	return pck.opcode;
}

void print_state(states ms) {
	if (ms == WAIT) printf("State: WAIT\n");
	if (ms == INFO) printf("State: INFO\n");
	if (ms == CHAT) printf("State: CHAT\n");
}
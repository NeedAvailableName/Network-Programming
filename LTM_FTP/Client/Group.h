#pragma once
#include "Resources.h"
#include "DataIO.h"


void handleGroupResponse(int msgOpcode) {
	switch (msgOpcode) {
	
		case CREATE_GROUP_SUCCESS:
			cout << " => SYSTEM : Create group successfully" << endl;
			break;
		case GROUP_ALREADY_EXIST:
			cout << " => SYSTEM : Group has already exist" << endl;
			break;
		case ENTER_SUCCESS:
			cout << " => SYSTEM : Welcome" << endl;
			break;
		case ENTER_FAILED:
			cout << " => SYSTEM : Enter group failed" << endl;
			break;
		case LEAVE_SUCCESS:
			cout << " => SYSTEM : Leave group succesfully" << endl;
			break;
		case LEAVE_FAILED:
			cout << " => SYSTEM : Leave group failed" << endl;
			break;
		case GROUP_NOT_FOUND:
			cout << " => SYSTEM : Group is not found" << endl;
			break;
		case REQUEST_SUCCESS:
			cout << " => SYSTEM : Request to group is sent successfully" << endl;
			break;
		case ACCEPT_SUCCESS:
			cout << " => SYSTEM : Request is accepted" << endl;
			break;
		case ALREADY_IN_GROUP:
			cout << " => SYSTEM : User is already in group" << endl;
			break;
	}
}

void showGroup(SOCKET sock, char* payload, int option, char* result = NULL) {
	Message msg;
	sendMessage(sock, payload, option);
	recvMessage(sock, msg);

	char* nameGroup = (char*)malloc(sizeof(char) * BUFF_SIZE);
	nameGroup = strtok_s(msg.payload, DELIMETER, &payload);
	
	while (nameGroup != NULL) {
		if(result != NULL){
			strcat(result, nameGroup);
			strcat(result, " ");
		}
		cout << nameGroup << endl;
		nameGroup = strtok_s(payload, DELIMETER, &payload);
	}
	free(nameGroup);
}

int createGroup(SOCKET sock, char* idMember, char* nameGroup) {
	int ret;
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", idMember, nameGroup);
	ret = sendMessage(sock, message, CREATE_GROUP);
	free(message);
	if (ret != SOCKET_ERROR) {
		recvMessage(sock, msg);
		handleGroupResponse(msg.opcode);
		return msg.opcode;
	} else {
		cout << "Something wrong" << endl;
		return -1;
	}
}

int enterGroup(SOCKET sock, Account& acc, Group& gr, char* nameGroup) {
	int ret;
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", acc.id, nameGroup);
	ret = sendMessage(sock, message, ENTER_GROUP);
	if (ret != SOCKET_ERROR) {
		recvMessage(sock, msg);

		if (msg.opcode == ENTER_SUCCESS) {
			handleGroupResponse(msg.opcode);
			acc.role = atoi(msg.payload);
			strcpy_s(gr.nameGroup, nameGroup);
			free(message);
			return msg.opcode;
		} else {
			handleGroupResponse(msg.opcode);
			free(message);
			return msg.opcode;
		}
	} else {
		cout << "Something wrong" << endl;
		return -1;
	}
}

int joinGroup(SOCKET sock, char* idMember, char* nameGroup) {
	int ret;
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", idMember, nameGroup);
	ret = sendMessage(sock, message, JOIN_GROUP);
	free(message);
	if (ret != SOCKET_ERROR) {
		recvMessage(sock, msg);
		handleGroupResponse(msg.opcode);
		return msg.opcode;
	} else {
		cout << "Something wrong" << endl;
		return -1;
	}
}

int leaveGroup(SOCKET sock, char* idMember, char* nameGroup) {
	int ret;
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", idMember, nameGroup);
	ret = sendMessage(sock, message, LEAVE_GROUP);
	if (ret != SOCKET_ERROR) {
		recvMessage(sock, msg);
		handleGroupResponse(msg.opcode);
		return msg.opcode;
	} else {
		cout << "Something wrong" << endl;
		return -1;
	}
	free(message);
}

void logout(SOCKET sock, char* idMember) {
	int ret = sendMessage(sock, idMember, LOGOUT);
	if (ret == SOCKET_ERROR) {
		cout << "Something wrong" << endl;
	}
}

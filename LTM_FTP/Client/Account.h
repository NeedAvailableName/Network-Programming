#pragma once

#include "Resources.h"
#include "DataIO.h"


void handleAccountResponse(int msgOpcode) {
	switch (msgOpcode) {
		case LOGIN_SUCCESS:
			cout << " => SYSTEM : Login succesfully" << endl;
			break;
		case ALREADY_LOGIN:
			cout << " => SYSTEM : You've already login" << endl;
			break;
		case CANT_LOGIN:
			cout << " => SYSTEM : Can't login cus some one is using" << endl;
			break;
		case BLOCKED:
			cout << " => SYSTEM : Your account is blocked" << endl;
			break;
		case ID_NOT_FOUND:
			cout << " => SYSTEM : Account is not found" << endl;
			break;
		case WRONG_PASSWORD:
			cout << " => SYSTEM : Wrong password" << endl;
			break;
		case REGISTER_SUCCESS:
			cout << " => SYSTEM : Register successfully. You can login now" << endl;
			break;
		case ACCOUNT_EXIST:
			cout << " => SYSTEM : Username has already exist" << endl;
			break;
		case LOGOUT_SUCCESS:
			cout << " => SYSTEM : Logout sucessfully" << endl;
			break;
	}
}

int login(SOCKET sock, Account& acc, char* id, char* pass) {
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", id, pass);
	sendMessage(sock, message, LOGIN);
	recvMessage(sock, msg);
	//handleResponse(msg);
	free(message);
	if (msg.opcode == LOGIN_SUCCESS) {
		strcpy(acc.id, msg.payload);
		strcpy(acc.user, id);
		strcpy(acc.pass, pass);
		handleAccountResponse(msg.opcode);
		return SECOND_FRAME ;
	} else if (msg.opcode == CANT_LOGIN || msg.opcode == WRONG_PASSWORD || msg.opcode == ID_NOT_FOUND) {
		handleAccountResponse(msg.opcode);
		return msg.opcode ;
	}else{
		return -1;
	}
}

int registerAcc(SOCKET client, char* id, char* pass) {
	Message msg;
	char* message = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(message, "%s %s", id, pass);

	sendMessage(client, message, REGISTER);
	recvMessage(client, msg);
	if (msg.opcode == REGISTER_SUCCESS || msg.opcode == ACCOUNT_EXIST) {
		handleAccountResponse(msg.opcode);
		return msg.opcode;
	} else {
		cout << "Some thing wrong" << endl;
		return -1;
	}
}
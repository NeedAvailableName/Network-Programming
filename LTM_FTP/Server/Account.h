#pragma once
#include "Resources.h"
#include "DataIO.h"


Account getUserAndPass(Message msg) {
	Account acc;

	//Copy msg.payload to buff to get id and pass
	char* buff = (char*)malloc(sizeof(char) * msg.length);
	char* user = (char*)malloc(sizeof(char) * 20);
	char* pass = (char*)malloc(sizeof(char) * 20);

	strncpy(buff, msg.payload, msg.length);
	buff[msg.length - 1] = 0;

	//Split buff to id and pass
	user = strtok_s(buff, DELIMETER, &buff);
	strcpy(pass, buff);
	strncpy(acc.user, user, strlen(user) + 1);
	strncpy(acc.pass, pass, strlen(pass) + 1);
	return acc;
}


int checkAccount(SOCKET connSock, vector<Account>& list, Account& acc, int type) {

	//Check account status when client want to log in
	if (type == LOGIN) {
		for (int i = 0; i < list.size(); i++) {
			if (list[i].sock == connSock) {
				return ALREADY_LOGIN;
			}
		}
		for (int i = 0; i < list.size(); i++) {

			if (strcmp(list[i].user, acc.user) == 0) {
				if (strcmp(list[i].pass, acc.pass) == 0) {
					if (list[i].status == 1) {
						if (list[i].sock == 0) {
							strcpy_s(acc.id, list[i].id);
							list[i].sock = connSock;
							acc.sock = connSock;
							acc.status = 1;
							acc.wrongPass = 0;

							return LOGIN_SUCCESS;

						} else {

							return CANT_LOGIN;
						}
					} else {
						return BLOCKED;
					}
				} else {

					list[i].wrongPass++;
					if (list[i].wrongPass > 3) {
						list[i].status = 0;
						return BLOCKED;
					} else {
						return WRONG_PASSWORD;
					}
				}
			}
		}
		return ID_NOT_FOUND;

		//Check account status when client want to log out
	} else if (type == LOGOUT) {
		for (int i = 0; i < list.size(); i++) {
			if (list[i].sock == connSock) {
				list[i].sock = 0;
				return LOGOUT_SUCCESS;
			}
		}
		return HAVENT_LOGIN;

	} else if (type == REGISTER) {
		for (int i = 0; i < list.size(); i++) {
			if (strcmp(list[i].user, acc.user) == 0) {
				return ACCOUNT_EXIST;
			}
		}
		acc.sock = 0;
		acc.status = 1;
		acc.wrongPass = 0;
		sprintf(acc.id, "MEM%02d", list.size() + 1);
		list.push_back(acc);
		
		//update account to text file
		updateAccountTxt(listAccount);
		return REGISTER_SUCCESS;
	} else {
		return -1;
	}
}


void login(Message& msg, vector<Account>& listAcc, LPPER_HANDLE_DATA& pHD, LPPER_IO_OPERATION_DATA& pID, DWORD& transferredBytes) {
	Account acc;
	acc = getUserAndPass(msg);
	int status = checkAccount(pHD->socket, listAcc, acc, LOGIN);
	if (status == LOGIN_SUCCESS) {
		craftMessage(msg, status, 0, 6, acc.id);
	} else {
		craftMessage(msg, status, 0, 0, NULL);
	}
	memcpy(pID->buffer, &msg, MESSAGE_SIZE);
	sendMessage(pHD, pID, transferredBytes, ALL);
}

void logout(int nClients, vector<Account>& listAcc, LPPER_HANDLE_DATA& pHD, LPPER_IO_OPERATION_DATA& pID, DWORD& transferredBytes) {
	int i;
	for (i = 0; i < nClients; i++) {
		if (pHD->socket == listAccount[i].sock) {
			break;
		}
	}
	checkAccount(pHD->socket, listAccount, listAccount[i], LOGOUT);
	recvMessage(pHD, pID, transferredBytes, ALL);
}

void registerAcc(Message& msg, vector<Account>& listAcc, LPPER_HANDLE_DATA& pHD, LPPER_IO_OPERATION_DATA& pID, DWORD& transferredBytes) {
	Account acc;
	acc = getUserAndPass(msg);
	int status = checkAccount(pHD->socket, listAccount, acc, REGISTER);
	craftMessage(msg, status, 0, 0, NULL);
	memcpy(pID->buffer, &msg, MESSAGE_SIZE);
	sendMessage(pHD, pID, transferredBytes, ALL);
}

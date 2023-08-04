#pragma once
#include "Resources.h"


void printListAcc(vector<Account> list) {
	for (int i = 0; i < list.size(); i++) {
		cout << list[i].id << " " << list[i].user << " " << list[i].pass << " " << list[i].status << endl;
	}
}

void printListGr(vector<Group> list) {
	for (int i = 0; i < list.size(); i++) {
		cout << list[i].idGroup << " " << list[i].nameGroup << " " << list[i].nMember << " ";
		for (int j = 0; j < list[i].nMember; j++) {
			cout << list[i].member[j] << " ";
		}
		cout << endl;
	}
}

void loadAccountTxt(vector<Account>& list) {
	std::ifstream file_(FILE_ACCOUNT);
	int count = 0;
	if (!file_.fail()) {
		while (!file_.eof()) {
			Account acc = { NULL };
			file_ >> acc.id >> acc.user >> acc.pass >> acc.status;
			list.push_back(acc);
			count++;
		}
		std::cout << " => LOADED "<< count << " account(s)" << std::endl;
	} else {
		std::cout << " => ERROR : Cannot open file account.txt" << std::endl;
	}
	file_.close();
}

void loadGroupTxt(vector<Group>& list) {
	std::ifstream file_(FILE_GROUP);

	if (!file_.fail()) {
		while (!file_.eof()) {
			Group gr = { NULL };
			file_ >> gr.idGroup >> gr.nameGroup >> gr.nMember;
			for (int i = 0; i < gr.nMember; i++) {
				file_ >> gr.member[i];
			}
			list.push_back(gr);
		}
	} else {
		std::cout << " => ERROR : Cannot open file group.txt" << std::endl;
	}
	file_.close();
}

void updateGroupTxt(vector<Group> list) {
	std::ofstream file_("group.txt");

	if (!file_.fail()) {
		for (int i = 0; i < list.size(); i++) {
			if (i == list.size() - 1) {
				file_ << list[i].idGroup << " " << list[i].nameGroup << " " << list[i].nMember << " ";
				for (int j = 0; j < list[i].nMember; j++) {
					if (j == list[i].nMember - 1) {
						file_ << list[i].member[j];
					} else {
						file_ << list[i].member[j] << " ";
					}
				}
			} else {
				file_ << list[i].idGroup << " " << list[i].nameGroup << " " << list[i].nMember << " ";
				for (int j = 0; j < list[i].nMember; j++) {
					if (j == list[i].nMember - 1) {
						file_ << list[i].member[j];
					} else {
						file_ << list[i].member[j] << " ";
					}
				}
				file_ << endl;
			}
		}
	}
	file_.close();

}
void updateAccountTxt(vector<Account> list) {
	std::ofstream file_("account.txt");

	if (!file_.fail()) {
		for (int i = 0; i < list.size(); i++) {
			if (i == list.size() - 1) {
				file_ << list[i].id << " " << list[i].user << " " << list[i].pass << " " << list[i].status;
			} else {
				file_ << list[i].id << " " << list[i].user << " " << list[i].pass << " " << list[i].status << endl;
			}
		}
	}
	file_.close();
}


void craftMessage(Message& msg, int opcode, int offset, int length, char* payload) {
	msg.opcode = opcode;
	msg.offset = offset;
	msg.length = length;
	if (length != 0) {
		memcpy(msg.payload, payload, length);
	} else {
		memcpy(msg.payload, "", 1);
	}
}



void sendMessage(LPPER_HANDLE_DATA& pHD, LPPER_IO_OPERATION_DATA& pID, DWORD& transferredBytes, int status) {
	DWORD Flags;
	if (status != MISSING) {
		ZeroMemory(&(pID->overlapped), sizeof(OVERLAPPED));
		pID->sentBytes = 0;
		pID->dataBuff.buf = pID->buffer;
		pID->dataBuff.len = MESSAGE_SIZE;
		pID->operation = SEND;

		if (WSASend(pHD->socket,
					&(pID->dataBuff),
					1,
					&transferredBytes,
					0,
					&(pID->overlapped),
					NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				cout << "WSASend() failed with error " << WSAGetLastError() << endl;
				return;
			}
		}
	} else {
		ZeroMemory(&(pID->overlapped), sizeof(WSAOVERLAPPED));
		Flags = 0;
		pID->dataBuff.buf = pID->buffer + pID->sentBytes;
		pID->dataBuff.len = MESSAGE_SIZE - pID->sentBytes;
		pID->operation = SEND;
		if (WSASend(pHD->socket,
					&(pID->dataBuff),
					1,
					&transferredBytes,
					0,
					&(pID->overlapped),
					NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				cout << "WSASend() failed with error " << WSAGetLastError() << endl;
				return;
			}
		}
	}
}


void recvMessage(LPPER_HANDLE_DATA& pHD, LPPER_IO_OPERATION_DATA& pID, DWORD& transferredBytes, int status) {
	DWORD Flags;
	if (status != MISSING) {
		ZeroMemory(&(pID->overlapped), sizeof(OVERLAPPED));
		pID->recvBytes = 0;
		pID->operation = RECEIVE;
		Flags = 0;
		pID->dataBuff.len = MESSAGE_SIZE;
		pID->dataBuff.buf = pID->buffer;

		if (WSARecv(pHD->socket,
					&(pID->dataBuff),
					1,
					&transferredBytes,
					&Flags,
					&(pID->overlapped),
					NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				cout << "WSARecv() failed with error " << WSAGetLastError() << endl;
				return;
			}
		}
	} else {
		ZeroMemory(&(pID->overlapped), sizeof(OVERLAPPED));
		pID->dataBuff.buf = pID->buffer + pID->sentBytes;
		pID->dataBuff.len = MESSAGE_SIZE - pID->recvBytes;
		pID->operation = RECEIVE;

		if (WSARecv(pHD->socket,
					&(pID->dataBuff),
					1,
					&transferredBytes,
					0,
					&(pID->overlapped),
					NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				cout << "WSARecv() failed with error " << WSAGetLastError();
				return;
			}
		}
	}
}

Message getMessage(char* pID_buffer) {
	Message msg;
	memcpy((char*)&msg, pID_buffer, sizeof(Message));
	if (msg.length == 0) {
		msg.payload[0] = 0;
	} else if(msg.length < PAYLOAD_SIZE) {
		msg.payload[msg.length] = 0;
	} else {

	}
	return msg;
}

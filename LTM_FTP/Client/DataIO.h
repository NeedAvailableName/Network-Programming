#pragma once
#include "Resources.h"


int createFolderLocal(const char* nameGroup) {
	int check = _mkdir(nameGroup);
	return check;
}

int checkFile(char* filePath) {
	FILE* f;
	f = fopen(filePath, "r");
	if (f != NULL) {
		fclose(f);
		return 0;
	}
	return 1;
}

void craftMessage(Message& msg, int type, int offset, int length, char* message) {
	switch (type) {
		case DATA_UP:
			msg.opcode = DATA_UP;
			msg.offset = offset;
			msg.length = length;
			if (length != 0) {
				memcpy(msg.payload, message, length);
			}
			break;
		default:
			msg.opcode = type;
			msg.offset = offset;
			msg.length = length;
			strncpy(msg.payload, message, length);
			break;
	}
}


int sendMessage(SOCKET client, char* message, int type) {
	Message msg;
	int ret;

	craftMessage(msg, type, 0, strlen(message) + 1, message);
	ret = send(client, (char*)&msg, MESSAGE_SIZE, 0);
	if (ret == SOCKET_ERROR) {
		cout << " => ERROR : Cannot send message." << endl;
	}
	return ret;

}

int recvMessage(SOCKET client, Message& msg) {
	int ret = recv(client, (char*)&msg, MESSAGE_SIZE, 0);
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT)
			cout << "Time-out!" << endl;
		else {
			cout << " => ERROR : Cannot receive message." << endl;
		}
		return -1;
	}
	return ret;
}

int sendFile(SOCKET client, char* filePath) {
	FILE* file;
	Message msg;
	int ret, size, offset = 0;
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* buff = (char*)malloc(sizeof(char) * PAYLOAD_SIZE);
	sprintf(path, "%s/%s", CLIENT_FOLDER, filePath);
	cout<< path <<endl;
	file = fopen(path, "rb");
	if (file == NULL) {
		return -1;
	}
	//GET LENGTH OF FILE
	fseek(file, 0, SEEK_END);
	int nLeft = ftell(file);
	fseek(file, 0, SEEK_SET);

	while (nLeft > 0) {
		size = (nLeft > PAYLOAD_SIZE) ? PAYLOAD_SIZE : nLeft;
		fread(buff, 1, size, file);
		craftMessage(msg, DATA_UP, offset, size, buff);
		ret = send(client, (char*)&msg, MESSAGE_SIZE, 0);
		if (ret == SOCKET_ERROR || ret == 0) {
			free(buff);
			fclose(file);
			return -1;
		}
		offset++;
		nLeft = nLeft - size;
	}
	craftMessage(msg, DATA_UP, 0, 0, NULL);
	ret = send(client, (char*)&msg, MESSAGE_SIZE, 0);
	if (ret == SOCKET_ERROR || ret == 0) {
		fclose(file);
		return -1;
	}
	free(buff);
	fclose(file);
	return 1;
}

int recvFile(SOCKET client, char* fileName) {
	FILE* f;
	Message msg;
	int ret, offset;
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* buff = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(path, "%s/%s", CLIENT_FOLDER, fileName);

	f = fopen(path, "wb");
	if (f == NULL) {
		return -1;
	}

	do {
		ret = recv(client, (char*)&msg, MESSAGE_SIZE, 0);
		if (ret > 0) {
			if (msg.opcode == DATA_DOWN) {
				offset = msg.offset * PAYLOAD_SIZE;
				fseek(f, offset, SEEK_SET);
				fwrite(msg.payload, 1, msg.length, f);
			}
		} else if (ret == SOCKET_ERROR) {
			cout << " => ERROR : Cannot receive message. Code : " << WSAGetLastError() << endl;
			free(buff);
			free(path);
			fclose(f);
			return -1;
		}
	} while (msg.length != 0 && msg.opcode == DATA_DOWN);
	free(buff);
	free(path);
	fclose(f);
	return 0;
}

void getNameFromPath(char* filePath, char* fileName) {
	int length = strlen(filePath);
	int i, j;
	for (i = length - 1; i >= 0; i--) {
		if (filePath[i] - '/' == 0) {
			break;
		}
	}

	for (j = 0; i + 1 < length; i++, j++) {
		fileName[j] = filePath[i + 1];
	}
	fileName[j] = 0;
}

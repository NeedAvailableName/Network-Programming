#pragma once

//#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <direct.h>
#pragma warning(disable: 4996)
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BUFF_SIZE 2048
#define PAYLOAD_SIZE 1024
#define MESSAGE_SIZE 1036
#define DELIMETER " "
#define CLIENT_FOLDER "./ClientData"
#define SERVER_FOLDER "./ServerData"

typedef struct messsage {
	int opcode;
	int offset;
	int length;
	char payload[PAYLOAD_SIZE];
} Message;

typedef struct Account {
	char id[6];
	char user[25];
	char pass[25];
	int role;
}Account;

typedef struct Group {
	char idGroup[6];
	char nameGroup[20];
}Group;


enum ACCOUNT_REQUEST {
	LOGIN = 10,
	REGISTER = 11,
	LOGOUT = 12,
};

enum ACCOUNT_RESPONSE {
	LOGIN_SUCCESS = 100,
	LOGOUT_SUCCESS = 101,

	WRONG_INPUT = 102,
	ID_NOT_FOUND = 102,
	WRONG_PASSWORD = 104,
	ALREADY_LOGIN = 105,
	CANT_LOGIN = 106,
	HAVENT_LOGIN = 107,
	BLOCKED = 108,

	ACCOUNT_EXIST = 109,
	REGISTER_SUCCESS = 110,

};

enum GROUP_REQUEST {
	SHOW_MY_GROUP = 20,
	SHOW_OTHER_GROUP = 21,
	CREATE_GROUP = 22,
	ENTER_GROUP = 23,
	JOIN_GROUP = 24,
	LEAVE_GROUP = 25,
};

enum GROUP_RESPONSE {
	ENTER_SUCCESS = 120,
	ENTER_FAILED = 121,
	GROUP_NOT_FOUND = 122,

	CREATE_GROUP_SUCCESS = 123,
	GROUP_ALREADY_EXIST = 124,

	LEAVE_SUCCESS = 125,
	LEAVE_FAILED = 126,

	REQUEST_SUCCESS = 127,
	ACCEPT_SUCCESS = 128,
	ALREADY_IN_GROUP = 129,
};

enum FILE_REQUEST {
	UPLOAD = 30,
	DOWNLOAD = 31,
	DATA_UP = 32,
	DATA_DOWN = 33,
	LIST_FILE = 34,
	LIST_MEMBER = 35,
	CREATE_FOLDER = 36,
	DELETE_FOLDER = 37,
	DELETE_FILE = 38,
	CHANGE_DIRECTORY = 39,
	SHOW_REQUEST = 40,
	SHOW_LOG = 41,
	ACCEPT_REQUEST = 42,
};

enum FILE_RESPONSE {
	ACCEPT_UPLOAD = 130,
	UPLOAD_SUCCESS = 131,
	ACCEPT_DOWNLOAD = 132,
	DOWNLOAD_SUCCESS = 133,
	CREATE_FOLDER_SUCCESS = 134,
	DELETE_FOLDER_SUCCESS = 135,
	FOLDER_NOT_FOUND = 136,
	FOLDER_ALREADY_EXIST = 137,
	DELETE_FILE_SUCCESS = 138,
	FILE_NOT_FOUND = 139,
	CHANGE_SUCCESS = 140,
	FILE_LOG = 141,
};

enum FRAME {
	FIRST_FRAME = 1,
	SECOND_FRAME = 2,
	THIRD_FRAME = 3,
	EXIT = 4
};

enum ROLE {
	MEMBER = 0,
	ADMIN = 1,
};
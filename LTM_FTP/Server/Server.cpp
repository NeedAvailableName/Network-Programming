

#include "Resources.h"
#include "DataIO.h"
#include "Account.h"
#include "Group.h"
#include "File.h"

int nClients = 0;

HANDLE completionPort;
HANDLE emptyQueue, fullQueue, mutex;

CRITICAL_SECTION cs;
CRITICAL_SECTION perClient[MAX_MEMBER];

LPPER_HANDLE_DATA perHandleData[MAX_MEMBER];
LPPER_IO_OPERATION_DATA perIoData[MAX_MEMBER];

unsigned __stdcall serverWorkerThread(LPVOID CompletionPortID);

int main(int argc, char* argv[]) {

	SOCKADDR_IN serverAddr;
	SOCKET listenSock, acceptSock;
	SYSTEM_INFO systemInfo;
	DWORD transferredBytes;
	WSADATA wsaData;
	DWORD Flags = 0;

	if (WSAStartup((2, 2), &wsaData) != 0) {
		cout << "WSAStartup() failed with error " << GetLastError() << endl;
		return 1;
	}

	if ((completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)) == NULL) {
		cout << "CreateIoCompletionPort() failed with error " << GetLastError() << endl;
		return 1;
	}

	GetSystemInfo(&systemInfo);

	for (int i = 0; i < (int)systemInfo.dwNumberOfProcessors * 2; i++) {
		if (_beginthreadex(0, 0, serverWorkerThread, (void*)completionPort, 0, 0) == 0) {
			cout << "Create thread failed with error " << GetLastError() << endl;
			return 1;
		}
	}

	//Initialize some important thing in system
	InitializeCriticalSection(&cs);
	mutex = CreateMutex(NULL, NULL, NULL);
	fullQueue = CreateSemaphore(NULL, 0, BACKLOG, NULL);
	emptyQueue = CreateSemaphore(NULL, BACKLOG, BACKLOG, NULL);
	loadAccountTxt(listAccount);
	loadGroupTxt(listGroup);
	createFolder(SERVER_FOLDER);
	for (int i = 0; i < listGroup.size(); i++) {
		createSubFolder(SERVER_FOLDER, listGroup[i].nameGroup);
		createTempFolder(listGroup[i].nameGroup);
		createLogFile(listGroup[i].nameGroup);
		createRequestFile(listGroup[i].nameGroup);
	}


	if ((listenSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		cout << "WSASocket() failed with error " << WSAGetLastError() << endl;
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons((unsigned short)atoi((char*)argv[1]));

	if (bind(listenSock, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "bind() failed with error " << WSAGetLastError() << endl;
		return 1;
	}

	if (listen(listenSock, BACKLOG) == SOCKET_ERROR) {
		cout << "listen() failed with error " << WSAGetLastError() << endl;
		return 1;
	}

	cout << "Server started !" << endl;

	while (true) {

		if ((acceptSock = WSAAccept(listenSock, NULL, NULL, NULL, 0)) == SOCKET_ERROR) {
			cout << "WSAAccept() failed with error " << WSAGetLastError() << endl;
			return 1;
		}

		WaitForSingleObject(emptyQueue, INFINITE);
		WaitForSingleObject(mutex, INFINITE);

		if ((perHandleData[nClients] = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA))) == NULL) {
			cout << "GlobalAlloc() failed with error " << GetLastError() << endl;
			return 1;
		}
		perHandleData[nClients]->socket = acceptSock;
		cout << "Socket number " << perHandleData[nClients]->socket << " got connected..." << endl;

		if (CreateIoCompletionPort((HANDLE)perHandleData[nClients]->socket, completionPort, (ULONG_PTR)perHandleData[nClients], 0) == NULL) {
			cout << "CreateIoCompletionPort() failed with error " << GetLastError() << endl;
			return 1;
		}

		nClients++;
		cout<< nClients << " Clients" <<endl;
		ReleaseSemaphore(mutex, 1, NULL);
		ReleaseSemaphore(fullQueue, 1, NULL);

		if ((perIoData[nClients - 1] = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATION_DATA))) == NULL) {
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return 1;
		}

		ZeroMemory(&(perIoData[nClients - 1]->overlapped), sizeof(OVERLAPPED));
		perIoData[nClients - 1]->sentBytes = 0;
		perIoData[nClients - 1]->recvBytes = 0;
		perIoData[nClients - 1]->dataBuff.len = MESSAGE_SIZE;
		perIoData[nClients - 1]->dataBuff.buf = perIoData[nClients - 1]->buffer;
		perIoData[nClients - 1]->operation = RECEIVE;

		if (WSARecv(perHandleData[nClients - 1]->socket, &(perIoData[nClients - 1]->dataBuff), 1, &transferredBytes, &Flags, &(perIoData[nClients - 1]->overlapped), NULL) == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING) {
				cout << "WSARecv() failed with error " << WSAGetLastError() << endl;
				return 1;
			}
		}

	}
	updateGroupTxt(listGroup);
	updateAccountTxt(listAccount);

	DeleteCriticalSection(&cs);
	CloseHandle(mutex);
	CloseHandle(emptyQueue);
	CloseHandle(fullQueue);
	_getch();
	return 0;
}

unsigned __stdcall serverWorkerThread(LPVOID completionPortID) {

	HANDLE completionPort = (HANDLE)completionPortID;
	DWORD transferredBytes;
	LPPER_HANDLE_DATA pHD;
	LPPER_IO_OPERATION_DATA pID;
	DWORD Flags = 0;

	int status;
	int i;
	Message msg = { NULL };
	//Account acc = { NULL };
	char* curPath = (char*)malloc(sizeof(char) * 1024);
	char* fileName = (char*)malloc(sizeof(char) * 1024);
	char* buff = (char*)malloc(sizeof(char) * BUFF_SIZE);

	while (TRUE) {

		status = GetQueuedCompletionStatus(completionPort, &transferredBytes, (PULONG_PTR)&pHD, (LPOVERLAPPED*)&pID, INFINITE);
		// cout<<"Transfer " << transferredBytes << " Status " << status <<endl;
		if (transferredBytes == 0 || status == 0) {
			// cout << "Close" <<endl;
			WaitForSingleObject(fullQueue, INFINITE);
			WaitForSingleObject(mutex, INFINITE);

			logout(nClients, listAccount, pHD, pID, transferredBytes);

			cout << "Closing socket " << pHD->socket << endl;
			for (i = 0; i < nClients; i++) {
				if (perHandleData[i]->socket == pHD->socket) {
					break;
				}
			}

			if (closesocket(pHD->socket) == SOCKET_ERROR) {
				cout << "closesocket() failed with error " << WSAGetLastError() << endl;
			}
			int j;
			for (j = i; j < nClients - 1; j++) {
				perHandleData[j] = perHandleData[j + 1];
			}
			GlobalFree(pHD);
			GlobalFree(pID);

			nClients--;

			ReleaseSemaphore(mutex, 1, NULL);
			ReleaseSemaphore(emptyQueue, 1, NULL);
			continue;
		}

		if (pID->operation == RECEIVE) {
			pID->recvBytes += transferredBytes;
			pID->sentBytes = 0;

		} else if (pID->operation == SEND) {
			pID->sentBytes += transferredBytes;
			pID->recvBytes = 0;
		}


		if (pID->sentBytes < pID->recvBytes) {
			if (pID->recvBytes < MESSAGE_SIZE) {
				recvMessage(pHD, pID, transferredBytes, MISSING);
			} else {
				msg = getMessage(pID->buffer);

				switch (msg.opcode) {
					case LOGIN:
						login(msg, listAccount, pHD, pID, transferredBytes);
						break;
					case LOGOUT:
						EnterCriticalSection(&cs);
						logout(nClients, listAccount, pHD, pID, transferredBytes);
						LeaveCriticalSection(&cs);
						break;
					case REGISTER:
						registerAcc(msg, listAccount, pHD, pID, transferredBytes);
						break;

					case SHOW_MY_GROUP:
						showMyGroup(msg, listGroup);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case SHOW_OTHER_GROUP:
						showOtherGroup(msg, listGroup);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case CREATE_GROUP:
						createGroup(msg, listGroup);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case ENTER_GROUP:
						enterGroup(msg, listGroup, nClients, pHD->socket, perHandleData);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case JOIN_GROUP:
						joinGroup(msg, listGroup);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case LEAVE_GROUP:
						leaveGroup(msg, listGroup);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;


					case UPLOAD:
						for (i = 0; i < nClients; i++) {
							if (perHandleData[i]->socket == pHD->socket) {
								break;
							}
						}
						upload(msg, perHandleData[i], pHD->socket);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case DATA_UP:
						for (i = 0; i < nClients; i++) {
							if (perHandleData[i]->socket == pHD->socket) {
								break;
							}
						}
						//If file transfer not complete
						if (recvDataUpload(msg, perHandleData[i], listAccount) != 0) {
							recvMessage(pHD, pID, transferredBytes, ALL);
						}
						//File transfer completed 
						else {
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						}
						break;
					case DOWNLOAD:
						for (i = 0; i < nClients; i++) {
							if (perHandleData[i]->socket == pHD->socket) {
								break;
							}
						}
						download(msg, perHandleData[i]);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case LIST_FILE:
						listFileToString(listGroup, msg.payload);
						craftMessage(msg, LIST_FILE, 0, strlen(msg.payload) + 1, msg.payload);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case LIST_MEMBER:
						listMemberToString(listGroup, listAccount, msg.payload);
						craftMessage(msg, LIST_MEMBER, 0, strlen(msg.payload) + 1, msg.payload);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case CREATE_FOLDER:
						if (createFolder(msg.payload) != -1) {
							craftMessage(msg, CREATE_FOLDER_SUCCESS, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						} else {
							craftMessage(msg, FOLDER_ALREADY_EXIST, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						}
						break;
					case DELETE_FOLDER:
						if (removeFolder(msg.payload) != -1) {
							craftMessage(msg, DELETE_FOLDER_SUCCESS, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						} else {
							craftMessage(msg, FOLDER_NOT_FOUND, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						}
						break;
					case DELETE_FILE:
						if (deleteFile(msg.payload) != -1) {
							craftMessage(msg, DELETE_FILE_SUCCESS, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						} else {
							craftMessage(msg, FILE_NOT_FOUND, 0, 0, NULL);
							memcpy(pID->buffer, &msg, MESSAGE_SIZE);
							sendMessage(pHD, pID, transferredBytes, ALL);
						}
						break;
					case CHANGE_DIRECTORY:
						changeDirectory(msg);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case SHOW_REQUEST:
						requestToString(listGroup, msg.payload);
						craftMessage(msg, SHOW_REQUEST, 0, strlen(msg.payload) + 1, msg.payload);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case ACCEPT_REQUEST:
						acceptRequest(msg, listGroup, listAccount);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
					case SHOW_LOG:
						showLog(msg);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						break;
				}
			}
		} else {
			if (pID->sentBytes < MESSAGE_SIZE) {
				sendMessage(pHD, pID, transferredBytes, MISSING);
			} else {
				msg = getMessage(pID->buffer);
				EnterCriticalSection(&cs);
				if (msg.opcode == ACCEPT_DOWNLOAD) {
					int i;
					for (i = 0; i < nClients; i++) {
						if (perHandleData[i]->socket == pHD->socket) {
							break;
						}
					}
					int idx = perHandleData[i]->size - perHandleData[i]->nLeft;
					int offset = (int)(idx / PAYLOAD_SIZE);
					char* buff = (char*)malloc(sizeof(char) * BUFF_SIZE);

					fseek(perHandleData[i]->f, idx, SEEK_SET);
					int size = (perHandleData[i]->nLeft > PAYLOAD_SIZE) ? PAYLOAD_SIZE : perHandleData[i]->nLeft;
					fread(buff, size, 1, perHandleData[i]->f);
					craftMessage(msg, DATA_DOWN, offset, size, buff);
					memcpy(pID->buffer, &msg, MESSAGE_SIZE);
					sendMessage(pHD, pID, transferredBytes, ALL);
					perHandleData[i]->nLeft -= size;

				} else if (msg.opcode == DATA_DOWN) {
					int i;
					for (i = 0; i < nClients; i++) {
						if (perHandleData[i]->socket == pHD->socket) {
							break;
						}
					}
					if (msg.length != 0) {
						int idx = perHandleData[i]->size - perHandleData[i]->nLeft;
						int offset = (int)(idx / PAYLOAD_SIZE);
						char* buff = (char*)malloc(sizeof(char) * BUFF_SIZE);

						fseek(perHandleData[i]->f, idx, SEEK_SET);
						int size = (perHandleData[i]->nLeft > PAYLOAD_SIZE) ? PAYLOAD_SIZE : perHandleData[i]->nLeft;
						fread(buff, size, 1, perHandleData[i]->f);
						craftMessage(msg, DATA_DOWN, offset, size, buff);
						memcpy(pID->buffer, &msg, MESSAGE_SIZE);
						sendMessage(pHD, pID, transferredBytes, ALL);
						perHandleData[i]->nLeft -= size;
					} else {
						recvMessage(pHD, pID, transferredBytes, ALL);
					}
				} else {
					recvMessage(pHD, pID, transferredBytes, ALL);
				}
				LeaveCriticalSection(&cs);
			}
		}
	}
}

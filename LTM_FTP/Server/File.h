#pragma once
#include "Resources.h"
#include "DataIO.h"
#include "Group.h"

int getLength(char* file) {
	int length = 0;
	FILE* f;
	f = fopen(file, "r");
	if (f != NULL) {
		fseek(f, 0, SEEK_END);
		length = ftell(f);
		fseek(f, 0, SEEK_SET);
		fclose(f);
	}
	return length;
}


void createLogFile(char* idGroup) {
	char* path = (char*)malloc(sizeof(char) * 256);
	sprintf(path, "%s/%s/Temp/%s.txt", SERVER_FOLDER, idGroup, "Log");
	FILE* f;
	f = fopen(path, "r");
	if (f == NULL) {
		f = fopen(path, "w");
		fclose(f);
	}
	fclose(f);
	free(path);
}

void createRequestFile(char* idGroup) {
	char* path = (char*)malloc(sizeof(char) * 256);
	sprintf(path, "%s/%s/Temp/Request.txt", SERVER_FOLDER, idGroup);
	FILE* f;
	f = fopen(path, "r");
	if (f == NULL) {
		f = fopen(path, "w");
		fclose(f);
	}
	fclose(f);
	free(path);
}

// void loadRequestTxt(char* idGroup, vector<Group> groupList, FILE* f) {
// 	int i = 0;
// 	char* buff = (char*)malloc(sizeof(char) * 10);
// 	while(i < groupList.size()){
// 		if( strcmp(groupList[i++].idGroup, idGroup) == 0){
// 			break;
// 		}
// 	}
// 	int j = 0;
// 	if(i < groupList.size()){
// 		while(!feof(f)){
// 			 fscanf(f, " %s", buff);
// 			 groupList[i].request = buff;
// 		}
// 	}
// }


int createFolder(const char* nameGroup) {
	int ret = _mkdir(nameGroup);
	return ret;
}

void createSubFolder(const char* parent, char* children) {
	char* path = (char*)malloc(sizeof(char) * 256);
	sprintf(path, "%s/%s", parent, children);
	_mkdir(path);
	free(path);
}

void deleteSubFolder(const char* parent, char* children) {
	char* path = (char*)malloc(sizeof(char) * 256);
	sprintf(path, "%s/%s", parent, children);
	printf("Delete path%s\n", path);
	_rmdir(path);
	free(path);
}

void createTempFolder(char* idGroup) {
	char* path = (char*)malloc(sizeof(char) * 256);
	sprintf(path, "%s/%s/%s", SERVER_FOLDER, idGroup, "Temp");
	int check = _mkdir(path);
	free(path);
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

int checkFile(char* filePath) {
	FILE* f;
	f = fopen(filePath, "r");
	if (f != NULL) {
		fclose(f);
		return 0;
	}
	return 1;
}

int isFolder(char* path) {
	struct stat s;
	if (stat(path, &s) == 0) {
		if (s.st_mode & S_IFDIR) {
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}

int checkFolder(char* path, char* folder) {
	DIR* dr;
	struct dirent* en;
	char* folderPath = (char*)malloc(sizeof(char) * (strlen(path) + strlen(folder) + 2));
	memset(folderPath, 0, sizeof(folderPath));
	strcat(folderPath, path);
	strcat(folderPath, "/");
	strcat(folderPath, folder);
	// cout << "Folder path " << folderPath <<endl;
	dr = opendir(folderPath);
	if (dr != NULL) {
		while ((en = readdir(dr)) != NULL) {
			// cout << dr->ent.d_name <<endl;
			if (strcmp(en->d_name, ".") != 0) {
				closedir(dr);
				// cout << "Is folder" <<endl;
				return 0;
			}
		}
		closedir(dr);
	}
	return -1;
}

void upload(Message& msg, LPPER_HANDLE_DATA perHandleData, SOCKET pHdSocket) {
	sprintf(perHandleData->nameFile, "%s/%s", SERVER_FOLDER, msg.payload);
	cout << perHandleData->nameFile <<endl;
	if (checkFile(perHandleData->nameFile) == 1) {
		perHandleData->f = fopen(perHandleData->nameFile, "wb");
		perHandleData->size = getLength(perHandleData->nameFile);
		perHandleData->nLeft = perHandleData->size;
		craftMessage(msg, ACCEPT_UPLOAD, 0, 0, NULL);
	} else {
		craftMessage(msg, -1, 0, 0, NULL);
	}
}


void addToLog(char* nameGroup, char* nameFile, char* nameMember) {
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(path, "%s/%s/Temp/Log.txt", SERVER_FOLDER, nameGroup);
	char* content = (char*)malloc(sizeof(char) * 100);
	char* timeRecv = (char*)malloc(sizeof(char) * 100);
	time_t rawtime = time(NULL);
	tm* ptm = localtime(&rawtime);

	FILE* f;
	f = fopen(path, "a");
	if (f == NULL) {
		cout << "Ko the mo dc file";
		return;
	}

	strftime(timeRecv, 100, "%d/%m/%y\t%H:%M:%S", ptm);
	sprintf(content, "%s\t%s\t\t%s\n", timeRecv, nameFile, nameMember);

	fwrite(content, strlen(content), 1, f);
	fclose(f);
	free(path);
	free(content);
	free(timeRecv);

}


int recvDataUpload(Message& msg, LPPER_HANDLE_DATA& perHandleData, vector<Account> listAccount) {
	int j;
	if (msg.length != 0) {
		int offset = msg.offset * PAYLOAD_SIZE;
		fseek(perHandleData->f, offset, SEEK_SET);
		fwrite(msg.payload, 1, msg.length, perHandleData->f);
		return 1;
	} else {
		fclose(perHandleData->f);
		for (j = 0; j < listAccount.size(); j++) {
			if (perHandleData->socket == listAccount[j].sock) {
				break;
			}
		}
		addToLog(perHandleData->curGroup, perHandleData->nameFile, listAccount[j].user);
		craftMessage(msg, UPLOAD_SUCCESS, 0, 0, NULL);
		return 0;
	}
}
void download(Message& msg, LPPER_HANDLE_DATA& perHandleData) {
	sprintf(perHandleData->nameFile, "%s/%s", SERVER_FOLDER, msg.payload);
	cout<< perHandleData->nameFile <<endl;
	if (checkFile(perHandleData->nameFile) == 0) {
		perHandleData->f = fopen(perHandleData->nameFile, "rb");
		perHandleData->size = getLength(perHandleData->nameFile);
		perHandleData->nLeft = perHandleData->size;
		craftMessage(msg, ACCEPT_DOWNLOAD, 0, 0, NULL);
	} else {
		craftMessage(msg, FILE_NOT_FOUND, 0, 0, NULL);
	}
}

void listFileToString(vector<Group> gr, char* payload) {
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(path, "%s/%s/", SERVER_FOLDER, payload);
	sprintf(payload, "");
	DIR* dr;
	struct dirent* en;
	// cout <<path <<endl;
	dr = opendir(path);
	if (dr) {
		while ((en = readdir(dr)) != NULL) {
			if (strcmp(en->d_name, ".") != 0 && strcmp(en->d_name, "..") != 0) {
				strcat(payload, en->d_name);
				strcat(payload, DELIMETER);
			}
		}
		closedir(dr);
	}
}

void listMemberToString(vector<Group> gr, vector<Account> acc, char* payload) {
	int i;
	for (i = 0; i < gr.size(); i++) {
		if (strcmp(gr[i].nameGroup, payload) == 0) {
			break;
		}
	}
	sprintf(payload, "");
	for (int k = 0; k < acc.size(); k++) {
		for (int j = 0; j < gr[i].nMember; j++) {
			if (strcmp(gr[i].member[j], acc[k].id) == 0) {
				strcat(payload, acc[k].user);
				strcat(payload, DELIMETER);
			}
		}
	}

}

int removeFolder(char* path) {
	int ret = rmdir(path);
	return ret;
}

int deleteFile(char* path) {
	// cout << path <<endl;
	int ret = remove(path);
	return ret;
}

int isRoot(char* path) {
	for (int i = 0; i < strlen(path); i++) {
		if (path[i] == '/') {
			return -1;
		}
	}
	return 0;
}

void changeDirectory(Message& msg) {
	char* root = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* folder = (char*)malloc(sizeof(char) * BUFF_SIZE);
	root = strtok_s(msg.payload, FOLDER_DELIMETER, &folder);

	if (strcmp(folder, "..") == 0) {
		if (isRoot(root) == -1) {
			int i;
			for (i = msg.length; i >= 0; i--) {
				if (msg.payload[i + 1] == '/') {
					break;
				}
			}
			strncpy(folder, root, i);
			folder[i] = 0;
			cout << "Root path " << folder <<endl;
			sprintf(msg.payload, "%s", &folder[7]);
			craftMessage(msg, CHANGE_SUCCESS, 0, strlen(msg.payload) + 1, msg.payload);

		} else {
			craftMessage(msg, CHANGE_SUCCESS, 0, strlen(root) + 1, root);
		}
	} else {
		if (checkFolder(root, folder) == 0) {
			// for(int i = 0; i < 8; i ++){
			// 	cout << &root[i] <<endl;
			// }
			sprintf(msg.payload, "%s", root + strlen(SERVER_FOLDER) + 1);
			// sprintf(msg.payload, "%s", folder);
			cout << "Correct path " << msg.payload <<endl;
			craftMessage(msg, CHANGE_SUCCESS, 0, strlen(msg.payload), msg.payload);
		} else {
			cout << "Wrong path " << root <<"/" << folder <<endl;
			craftMessage(msg, FOLDER_NOT_FOUND, 0, 0, NULL);

		}

	}
}


void showLog(Message& msg) {
	char* buff = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(buff, "%s/%s/Temp/Log.txt", SERVER_FOLDER, msg.payload);
	FILE* f;
	f = fopen(buff, "r");
	int length = getLength(buff);
	fread(msg.payload, 1, length, f);
	msg.payload[length - 2] = 0;
	cout<< msg.payload <<endl;
	craftMessage(msg, FILE_LOG, 0, length - 2, msg.payload);
	free(buff);
}

void requestToString(vector<Group> list, char* payload) {
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(path, "%s/%s/Temp/Request.txt", SERVER_FOLDER, payload);
	int i, j = 0;
	for (i = 0; i < 10; i++) {
		if (strcmp(payload, list[i].nameGroup) == 0) {
			break;
		}
	}
	char* temp = (char*)malloc(sizeof(char) * 6);
	ifstream file(path);
	if (!file.fail()) {
		while (!file.eof()) {
			file >> temp;
			if (strcmp(temp, "") == 0) {
				break;
			}
			sprintf(list[i].request[j], "%s", temp);
		}
	}
	sprintf(payload, "");
	j = 0;
	while (strcmp(list[i].request[j], "") != 0) {
		strcat(payload, list[i].request[j++]);
	}
	file.close();
}

void updateRequest(Group gr) {
	int i = 0;
	char* path = (char*)malloc(sizeof(char) * BUFF_SIZE);
	sprintf(path, "%s/%s/Temp/Request.txt", SERVER_FOLDER, gr.nameGroup);

	ofstream file(path);
	if (!file.fail()) {
		while (strcmp(gr.request[i], "") != 0) {
			file << gr.request[i++] << " ";
		}
	}
	file.close();
}

void removeFromRequest(Group gr, char* userID){
	int i = 0;
	char* id = (char*)malloc(sizeof(char) * 6);
	int shift = -1;
	while(strcmp((id = gr.request[i++]), "") != 0){
		if(shift < 0){
			if(strcmp(id, userID) == 0){
				shift = i;
			}
		}
		if(shift >= 0){
			strcpy(id, gr.request[i]);
		}
	}
	updateRequest(gr);
}

void setPath(char* nameGroup, char* path) {
	sprintf(path, "%s/%s", SERVER_FOLDER, nameGroup);
}


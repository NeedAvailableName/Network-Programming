#pragma once
#include "Resources.h"
#include "DataIO.h"
#include "File.h"

int checkIdInRequest(vector<Group>& listGroup, char* group, char* id);

void addToRequest(Group& gr, char* idMember) {
	int i = 0;
	while (strcmp(gr.request[i], "") != 0) {
		i++;
	}
	sprintf(gr.request[i], "%s", idMember);
}

int checkMember(Group gr, char* idMember) {
	if (strcmp(idMember, gr.member[0]) == 0) {
		return 1;
	}
	for (int i = 1; i < gr.nMember; i++) {
		if (strcmp(idMember, gr.member[i]) == 0) {
			return 0;
		}
	}
	return -1;
}

int checkGroup(vector<Group>& list, char* nameGroup) {
	int i;
	for (i = 0; i < list.size(); i++) {
		if (strcmp(nameGroup, list[i].nameGroup) == 0) {
			return i;
		}
	}
	return -1;
}


void otherGroupToString(vector<Group> list, char* payload) {
	int flag = 1;
	vector<int> index;
	for (int i = 0; i < list.size(); i++) {
		for (int j = 0; j < list[i].nMember; j++) {
			if (strcmp(payload, list[i].member[j]) == 0) {
				flag = 0;
				break;
			}
			flag = 1;
		}
		if (flag == 0) {
			index.push_back(0);
		} else {
			index.push_back(1);
		}
	}
	sprintf(payload, "");
	for (int i = 0; i < index.size(); i++) {
		if (index[i] == 1) {
			strcat(payload, list[i].nameGroup);
			strcat(payload, DELIMETER);
		}
	}
}

void myGroupToString(vector<Group> list, char* payload) {
	int flag = 1;
	vector<int> index;
	for (int i = 0; i < list.size(); i++) {
		for (int j = 0; j < list[i].nMember; j++) {
			if (strcmp(payload, list[i].member[j]) == 0) {
				flag = 0;
				break;
			}
			flag = 1;
		}
		if (flag == 0) {
			index.push_back(0);
		} else {
			index.push_back(1);
		}
	}
	sprintf(payload, "");
	for (int i = 0; i < index.size(); i++) {
		if (index[i] == 0) {
			strcat(payload, list[i].nameGroup);
			strcat(payload, DELIMETER);
		}
	}
}

void showMyGroup(Message& msg, vector<Group> listGroup) {
	myGroupToString(listGroup, msg.payload);
	craftMessage(msg, SHOW_MY_GROUP, 0, strlen(msg.payload) + 1, msg.payload);
}
void showOtherGroup(Message& msg, vector<Group> listGroup) {
	otherGroupToString(listGroup, msg.payload);
	craftMessage(msg, SHOW_OTHER_GROUP, 0, strlen(msg.payload) + 1, msg.payload);
}

void createGroup(Message& msg, vector<Group>& listGroup) {
	int i, ret = 0;
	char* id = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* nameGroup = (char*)malloc(sizeof(char) * BUFF_SIZE);
	id = strtok_s(msg.payload, DELIMETER, &nameGroup);

	for (i = 0; i < listGroup.size(); i++) {
		if (strcmp(listGroup[i].nameGroup, nameGroup) == 0) {
			ret = 1;
		}
		if (strcmp(listGroup[i].idGroup, "") == 0) {
			break;
		}
	}
	Group gr;
	sprintf_s(gr.idGroup, "GR%02d", i + 1);
	sprintf_s(gr.nameGroup, "%s", nameGroup);
	gr.nMember = 1;
	sprintf_s(gr.member[0], "%s", id);

	createSubFolder(SERVER_FOLDER, gr.nameGroup);
	createTempFolder(gr.nameGroup);
	createLogFile(gr.nameGroup);
	createRequestFile(gr.nameGroup);
	listGroup.push_back(gr);
	if (ret == 0) {
		craftMessage(msg, CREATE_GROUP_SUCCESS, 0, 0, NULL);
		//update group to text file
		updateGroupTxt(listGroup);
	} else {
		craftMessage(msg, GROUP_ALREADY_EXIST, 0, 0, NULL);
	}
}

void enterGroup(Message& msg, vector<Group> listGroup, int nClients, SOCKET pHDSocket, LPPER_HANDLE_DATA perHandleData[]) {
	char* id = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* nameGroup = (char*)malloc(sizeof(char) * BUFF_SIZE);
	id = strtok_s(msg.payload, DELIMETER, &nameGroup);

	int j = checkGroup(listGroup, nameGroup);
	if (j != -1) {
		int role = checkMember(listGroup[j], id);
		if (role != -1) {
			sprintf(msg.payload, "%d", role);
			int i;
			for (i = 0; i < nClients; i++) {
				if (pHDSocket == perHandleData[i]->socket) {
					break;
				}
			}
			sprintf(perHandleData[i]->curGroup, "%s", listGroup[j].nameGroup);
			craftMessage(msg, ENTER_SUCCESS, 0, strlen(msg.payload) + 1, msg.payload);
		} else {
			craftMessage(msg, ENTER_FAILED, 0, 0, NULL);
		}
	} else {
		craftMessage(msg, ENTER_FAILED, 0, 0, NULL);
	}
}
void joinGroup(Message& msg, vector<Group>& listGroup) {
	char* id = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* nameGroup = (char*)malloc(sizeof(char) * BUFF_SIZE);
	id = strtok_s(msg.payload, DELIMETER, &nameGroup);

	int j;
	for (j = 0; j < listGroup.size(); j++) {
		if (strcmp(nameGroup, listGroup[j].nameGroup) == 0) {
			break;
		}
	}
	if (j != listGroup.size()) {
		if (checkMember(listGroup[j], id) == -1) {
			addToRequest(listGroup[j], id);
			updateRequest(listGroup[j]);
			craftMessage(msg, REQUEST_SUCCESS, 0, 0, NULL);
		} else {
			craftMessage(msg, ALREADY_IN_GROUP, 0, 0, NULL);

		}
	} else {
		craftMessage(msg, GROUP_NOT_FOUND, 0, 0, NULL);
	}
}

void deleteGroup(vector<Group>& list, char* idGroup) {
	vector<Group> temp;
	char* grName = (char*)malloc(sizeof(char) * 20);
	int i;
	for (i = 0; i < list.size(); i++) {
		if (strcmp(idGroup, list[i].idGroup) == 0) {
			break;
		}
	}
	strcpy(grName, list[i].nameGroup);
	for (int j = i + 1; j < list.size(); i++) {
		temp.push_back(list[j]);
	}
	for (; i < list.size(); i++) {
		list.pop_back();
	}
	for (int j = 0; i < list.size(); i++, j++) {
		list.push_back(temp[j]);
	}
	// updateGroupTxt(list);
	printf("Delete %s", grName);
	deleteSubFolder(SERVER_FOLDER, grName);
}


int deleteMember(vector<Group> list, Group& gr, char* idMember) {
	int i;
	for (i = 0; i < gr.nMember; i++) {
		if (strcmp(idMember, gr.member[i]) == 0) {
			break;
		}
	}
	if (i != MAX_MEMBER) {
		for (; i < gr.nMember - 1; i++) {
			strcpy_s(gr.member[i], gr.member[i + 1]);
		}
		strcpy_s(gr.member[i], "");
		gr.nMember--;
		if (gr.nMember == 0) {
			deleteGroup(listGroup, gr.idGroup);
		}
		return 0;
	} else {
		return 1;
	}
}

void leaveGroup(Message& msg, vector<Group>& listGroup) {
	char* id = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* nameGroup = (char*)malloc(sizeof(char) * BUFF_SIZE);
	id = strtok_s(msg.payload, DELIMETER, &nameGroup);
	int j = checkGroup(listGroup, nameGroup);
	if (j != -1) {
		if (checkMember(listGroup[j], id) != -1) {
			deleteMember(listGroup, listGroup[j], id);
			craftMessage(msg, LEAVE_SUCCESS, 0, 0, NULL);
			updateGroupTxt(listGroup);
		} else {
			craftMessage(msg, LEAVE_FAILED, 0, 0, NULL);
		}
	} else {
		craftMessage(msg, GROUP_NOT_FOUND, 0, 0, NULL);
	}
}


int createGroup(vector<Group>& list, char* id, char* nameGroup) {
	int i;
	for (i = 0; i < list.size(); i++) {
		if (strcmp(list[i].nameGroup, nameGroup) == 0) {
			return 1;
		}
		if (strcmp(list[i].idGroup, "") == 0) {
			break;
		}
	}
	Group gr;
	sprintf_s(gr.idGroup, "GR%02d", i + 1);
	sprintf_s(gr.nameGroup, "%s", nameGroup);
	gr.nMember = 1;
	sprintf_s(gr.member[0], "%s", id);

	list.push_back(gr);
	return 0;
}

int addMember(Group& gr, char* idMember) {
	int i;
	for (i = 0; i < gr.nMember; i++) {
		if (strcmp(gr.member[i], idMember) == 0) {
			return 1;
		}
		if (strcmp(gr.idGroup, "") == 0) {
			break;
		}
	}
	if (i != MAX_MEMBER) {
		strncpy_s(gr.member[i], idMember, 6);
		gr.nMember++;
		return 0;
	} else {
		return 1;
	}
}

void acceptRequest(Message& msg, vector<Group>& listGroup, vector<Account>& listAccount) {
	char* id = (char*)malloc(sizeof(char) * BUFF_SIZE);
	char* group = (char*)malloc(sizeof(char) * BUFF_SIZE);
	group = strtok_s(msg.payload, DELIMETER, &id);
	if(checkIdInRequest(listGroup, group, id) == 0){
		cout << "Id " << id << " not in request of " << group <<endl;
		craftMessage(msg, ID_NOT_FOUND, 0, 0, NULL);
		return;
	}
	int i;
	for (i = 0; i < listAccount.size(); i++) {
		if (strcmp(listAccount[i].id, id) == 0) {
			break;
		}
	}
	if (i != listAccount.size()) {
		int j;
		for (j = 0; j < listGroup.size(); j++) {
			if (strcmp(group, listGroup[j].nameGroup) == 0) {
				break;
			}
		}
		cout << "Add " << id << " to " << group <<endl;
		addMember(listGroup[j], id);
		craftMessage(msg, ACCEPT_SUCCESS, 0, 0, NULL);

		//update group to text file
		updateGroupTxt(listGroup);
		
		removeFromRequest(listGroup[j], id);
	}else{
		cout << "Not found  " << id <<endl;
		craftMessage(msg, ID_NOT_FOUND, 0, 0, NULL);
	}
	// free(id);
	// free(group);
}

int checkIdInRequest(vector<Group>& listGroup, char* group, char* id){
	int i, j;
	for (j = 0; j < listGroup.size(); j++) {
		if (strcmp(group, listGroup[j].nameGroup) == 0) {
			break;
		}
	}
	if(j < listGroup.size()){
		while (strcmp(listGroup[j].request[i], "") != 0) {
			if (strcmp(id, listGroup[j].request[i]) == 0) {
				return 1;
			}
			i++;
		}
		return 0;
	}
	return 0;
}
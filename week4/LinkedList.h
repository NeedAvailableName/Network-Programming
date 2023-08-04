#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFF_SIZE 1024

// define user account
typedef struct userAccount {
    char username[50];
    char password[50];
    int status;
    int countFalseLogin;
    int isSignedIn;
} userAccount;

// define account list
typedef struct accountList {
    userAccount userInfo;
    struct accountList *next;
} accountList;

// add new element to the last of list
void addUser(accountList **root, userAccount newUser) {
    accountList *curr = (accountList *)malloc(sizeof(accountList));

    strcpy(curr->userInfo.username, newUser.username);
    strcpy(curr->userInfo.password, newUser.password);
    curr->userInfo.status = newUser.status;
    curr->userInfo.countFalseLogin = 0;
    curr->userInfo.isSignedIn = 0;
    curr->next = NULL;

    accountList *temp = *root;
    if(temp == NULL) {
        *root = curr;
        curr->next = NULL;
    } 
    else {
        while(temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = curr;
        curr->next = NULL;
    }
}

// read data from file to account list
void readToList(accountList **root) {
    FILE *fptr = fopen("account.txt", "r");
    if(fptr == NULL) {
        printf("Cannot open file");
        return;
    }

    userAccount temp;
    while(!feof(fptr)) {
        fscanf(fptr, "%s %s %d", temp.username, temp.password, &temp.status);
        addUser(root, temp);
    }
    fclose(fptr);
}

// save data from list to file
void writeToFile(accountList *root) {
    FILE *fptr = fopen("account.txt", "w");
    if(fptr == NULL) {
        printf("Cannot open file");
        return;
    }

    accountList *temp = root;
    while(temp != NULL) {
        if(temp->next == NULL) {
            fprintf(fptr, "%s %s %d", temp->userInfo.username, temp->userInfo.password, temp->userInfo.status);
            break;
        }
        fprintf(fptr, "%s %s %d\n", temp->userInfo.username, temp->userInfo.password, temp->userInfo.status);
        temp = temp->next;
    }
    fclose(fptr);
}

// check account is in list or not
int checkExisted(accountList *root, char *username) {
    accountList *temp = root;
    while(temp != NULL) {
        if(strcmp(temp->userInfo.username, username) == 0) {
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

// user login
void signIn(accountList *root, char *username, char *password, char *msg) {
    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(strcmp(curr->userInfo.password, password) == 0) {
                if(curr->userInfo.status == 1) {
                    if(curr->userInfo.isSignedIn == 0) {
                        curr->userInfo.isSignedIn = 1;
                        strcpy(msg, "OK");
                        return;
                    }
                    else {
                        strcpy(msg, "User already logged in");
                        return;
                    }
                }
                else {
                    strcpy(msg, "Account not ready");
                    return;
                }
            }
            else {
                curr->userInfo.countFalseLogin++;
                if(curr->userInfo.countFalseLogin == 3) {
                    curr->userInfo.status = 0;
                    strcpy(msg, "Account is blocked");
                    return;
                }
                strcpy(msg, "Not OK");
                return;
            }
        }
        curr = curr->next;
    }
    strcpy(msg, "Cannot find account\n");
    return;
}

// user logout
void signOut(accountList *root, char *username, char *msg) {
    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(curr->userInfo.isSignedIn == 1) {
                curr->userInfo.isSignedIn = 0;
                strcpy(msg, "Goodbye ");
                strcat(msg, username);
                return;
            }
            else {
                strcpy(msg, "Account is not sign in\n");
                return;
            }
        }
        curr = curr->next;
    }
    strcpy(msg, "Cannot find account\n");
    return;
}

// change user password
void changePassword(accountList *root, char *username, char *password, char *msg) {
    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(curr->userInfo.isSignedIn == 1) {
                int i = 0, j = 0, k = 0;
                char number[50], character[50];
                memset(number, '\0', 50);
                memset(character, '\0', 50);
                number[0] = '@';
                character[0] = '@';
                
                for(i = 0; i < strlen(password); i++) {
                    if(isdigit(password[i]) != 0) {
                        number[j] = password[i];
                        j++;
                    }
                    else if(isalpha(password[i]) != 0) {
                        character[k] = password[i];
                        k++;
                    }
                    else {
                        printf("Error: Invalid password\n");
                        strcpy(msg, "Error");
                        return;
                    }
                }

                strcpy(msg, "OK");
                strcat(msg, "-");
                strcat(msg, number);
                strcat(msg, "-");
                strcat(msg, character);
                return;

                if(strcmp(msg, "Error") != 0) {
                    strcpy(curr->userInfo.password, password);
                }
                return;
            }
            else {
                strcpy(msg, "Account is not sign in\n");
                return;
            }
        }
        curr = curr->next;
    }
    strcpy(msg, "Cannot find account\n");
    return;
}

// delete list
void freeList(accountList *root) {
    accountList *curr = root;
    while(curr != NULL) {
        root = root->next;
        free(curr);
        curr = root;
    }
}

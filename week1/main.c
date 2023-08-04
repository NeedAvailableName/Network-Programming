#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

// do function 1: register
void registerAccount(accountList **root) {
    userAccount newUser;
    printf("Username: ");
    scanf("%s", newUser.username);

    if(checkExisted(*root, newUser.username) == 1) {
        printf("Account existed\n");
    } 
    else {
        printf("Password: ");
        scanf("%s", newUser.password);
        newUser.status = 1;
        addUser(root, newUser);
        printf("Successful registration\n");
        writeToFile(*root);
    }
}

// do function 2: sign in
void SignIn(accountList *root) {
    char username[50];
    char password[50];
    printf("Username: ");
    scanf("%s", username);

    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(curr->userInfo.status == 0) {
                printf("Account is blocked\n");
                return;
            }
            printf("Password: ");
            scanf("%s", password);

            if(strcmp(curr->userInfo.password, password) == 0) {
                printf("Hello %s\n", curr->userInfo.username);
                curr->userInfo.isSignedIn = 1;
                return;
            } 
            // wrong password
            else {
                curr->userInfo.countFalseLogin++;
                if(curr->userInfo.countFalseLogin == 3) {
                    curr->userInfo.status = 0;
                    printf("Password is incorrect. Account is blocked\n");
                    writeToFile(root);
                    return;
                }
                printf("Password is incorrect\n");
                return;
            }
        }
        curr = curr->next;
    }
    printf("Cannot find account\n");
}

// do function 3: search
void searchAccount(accountList *root) {
    char username[50];
    printf("Username: ");
    scanf("%s", username);

    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(curr->userInfo.status == 1) {
                printf("Account is active\n");
                return;
            } 
            else {
                printf("Account is blocked\n");
                return;
            }
        }
        curr = curr->next;
    }
    printf("Cannot find account\n");
}

// do function 4: sign out
void signOut(accountList *root) {
    char username[50];
    printf("Username: ");
    scanf("%s", username);

    accountList *curr = root;
    while(curr != NULL) {
        if(strcmp(curr->userInfo.username, username) == 0) {
            if(curr->userInfo.isSignedIn == 1) {
                printf("Goodbye %s\n", curr->userInfo.username);
                curr->userInfo.isSignedIn = 0;
                return;
            } 
            else {
                printf("Account is not sign in\n");
                return;
            }
            return;
        }
        curr = curr->next;
    }
    printf("Cannot find account\n");
}

int main() {
    accountList *root = NULL;
    readToList(&root);
    int choice;

    while(1) {
        printf("\nUSER MANAGEMENT PROGRAM\n");
        printf("---------------------------\n");
        printf("1. Register\n");
        printf("2. Sign in\n");
        printf("3. Search\n");
        printf("4. Sign out\n");
        printf("Your choice (1-4, other to quit)\n");
        scanf("%d", &choice);

        switch(choice) {
            case 1:
                registerAccount(&root);
                break;
            case 2:
                SignIn(root);
                break;
            case 3:
                searchAccount(root);
                break;
            case 4:
                signOut(root);
                break;
            default:
                return 0;
        }
    }
}
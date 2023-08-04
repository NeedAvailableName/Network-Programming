#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Check ip address or not, return 1 for ip address, 0 for not ip address
int checkIPAddress(char *address) {
    int i;
    int cnt = 0;
    for(i = 0; address[i] != '\0'; i++) {
        if(address[i] == '.') {
            cnt++;
        }
        else if(address[i] < '0' || address[i] > '9') {
            return 0;
        }
    }
    if(cnt != 3) {
        return 0;
    }
    return 1;
}

// Get host name from domain name
char *getHostNameFromDomainName(char *domainName) {
    struct hostent *hostName;
    hostName = gethostbyname(domainName);
    return hostName;
}

// Get host name from ip address
char *getHostNameFromIPAddress(char *ip) {
    struct hostent *hostName;
    struct in_addr address;
    inet_aton(ip, &address);
    hostName = gethostbyaddr(&address, sizeof(address), AF_INET);
    return hostName;
}


int main(int argc, char *argv[]) {
    struct hostent *hostName;
    if(argc != 2) {
        return 1;
    }

    if(checkIPAddress(argv[1])) {
        hostName = getHostNameFromIPAddress(argv[1]);
        if(hostName == NULL) {
            printf("Not found information\n");
            return 1;
        }
        printf("Official name: %s\n", hostName->h_name);
        printf("Alias name: \n");
        for(int i = 0; hostName->h_aliases[i] != NULL; i++) {
            printf("%s\n", hostName->h_aliases[i]);
        }
    }
    else {
        hostName = getHostNameFromDomainName(argv[1]);
        if(hostName == NULL) {
            printf("Not found information\n");
            return 1;
        }
        printf("Official IP: %s\n", inet_ntoa(*((struct in_addr *)hostName->h_addr)));
        printf("Alias IP: \n");
        for(int i = 1; hostName->h_addr_list[i] != NULL; i++) {
            printf("%s\n", inet_ntoa(*((struct in_addr *)hostName->h_addr_list[i])));
        }
    }
    return 0;
}
 
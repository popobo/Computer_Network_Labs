#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>

int main() {
    const char* hostname = "cs144.keithw.org";
    struct addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    struct addrinfo* result;
    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(status));
        return 1;
    }
    struct sockaddr_in* ipv4 = (struct sockaddr_in*)result->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(result->ai_family, &ipv4->sin_addr, ipstr, sizeof ipstr);
    printf("%s: %s\n", hostname, ipstr);
    freeaddrinfo(result);
    return 0;
}

#include <stdio.h>
#include <netinet/in.h> /* sockaddr ... */
#include <sys/socket.h> /* socket(), connect() */
#include <arpa/inet.h> /* inet_pton() */
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <errno.h> /* ECONNREFUSED */
#include "logger.h" /* FATALF() */

int main(int argc, char *argv[]) {
    // validate input args
    if (argc <= 1)
        FATALF("params should be", "<ip address>");

    // init destination address
    char *ipAddr = argv[1];
    in_port_t portNum;
    portNum = 1;
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(struct sockaddr_in));
    destAddr.sin_family = AF_INET;
    if ( !inet_pton(AF_INET, ipAddr, &destAddr.sin_addr) )
        FATALF("inet_pton() failed()", "invalid ip address");
    destAddr.sin_port = htons(portNum);

    // init a socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        ERRORF("socket() failed");

    // try to connect ports
    int conn;
    do {
        conn = connect(sock, (struct sockaddr *)&destAddr, sizeof(struct sockaddr_in));
        if (conn == 0)
            printf("open port: %d\n", portNum);
        else if (conn < 0 && errno != ECONNREFUSED)
            ERRORF("connect() failed");
        close(sock);
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        portNum++;
        destAddr.sin_port = htons(portNum);
    } while (portNum <= 65535);
    return 0;
}

#include <stdio.h>
#include <netinet/in.h> /* sockaddr ... */
#include <sys/socket.h> /* socket(), connect() */
#include <arpa/inet.h> /* inet_pton() */
#include <unistd.h> /* close(), getopt() */
#include <string.h> /* memset() */
#include <errno.h> /* ECONNREFUSED */
#include "logger.h" /* FATALF() */
enum {SYN_SCAN, CONNECT_SCAN} scan_mode = SYN_SCAN;

int main(int argc, char *argv[]) {
    // parse input args
    char *usage = "[-SC] <ip address>\n" \
                  "-S default, scan with SYN packets\n" \
                  "-C scan with connect(), i.e. the whole 3-way handshake procedure";
    int opt;
    while( (opt = getopt(argc, argv, "SCh")) != -1 ) {
        switch(opt) {
        case 'S': scan_mode = SYN_SCAN; break;
        case 'C': scan_mode = CONNECT_SCAN; break;
        case 'h': printf("Usage: %s\n", usage); return 0;
        default:
            FATALF("Usage", usage);
        }
    }
    if (scan_mode == SYN_SCAN) {
        printf("SYN scanner is in developing");
        return 0;
    }
    extern int optind;
    if (optind > argc)
        FATALF("Usage", "missing ip address");
    char *ipAddr = argv[optind];
    printf("Scanning %s with %s\n", ipAddr, scan_mode == SYN_SCAN ? "SYN packets" : "TCP connection");

    // init destination address
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

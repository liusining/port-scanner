#include <stdio.h>
#include <string.h> /* memset() */
#include <unistd.h> /* close(), getopt() */
#include <errno.h> /* ECONNREFUSED */
#if defined(__linux__) && !defined(__USE_BSD)
    #define __USE_BSD /* use BSD named ip struct */
#endif // __USE_BSD
#include <netinet/ip.h> /* ip struct */
#include <netinet/tcp.h> /* tcphdr */
#include <netinet/in.h> /* sockaddr ... */
#include <sys/socket.h> /* socket(), connect() */
#include <arpa/inet.h> /* inet_pton() */
#include "logger.h" /* FATALF(), ERRORF() */

enum {SYN_SCAN, CONNECT_SCAN} scan_mode = SYN_SCAN;

void scan_with_connect();
void scan_with_syn();

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
    extern int optind;
    if (optind > argc)
        FATALF("Usage", "missing ip address");
    char *ipAddr = argv[optind];
    printf("Scanning %s with %s\n", ipAddr, scan_mode == SYN_SCAN ? "SYN packets" : "TCP connection");

    // init destination address
    struct sockaddr_in destAddr;
    memset(&destAddr, 0, sizeof(struct sockaddr_in));
    destAddr.sin_family = AF_INET;
    if ( !inet_pton(AF_INET, ipAddr, &destAddr.sin_addr) )
        FATALF("inet_pton() failed()", "invalid ip address");

    // scan
    if (scan_mode == SYN_SCAN) { // scan with SYN packets
        scan_with_syn(&destAddr);
    } else { // scan with connect()
        scan_with_connect(&destAddr);
    }
    printf("Done!");
    return 0;
}

unsigned long csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

void scan_with_syn(struct sockaddr_in *destAddrPtr)
{
    // init a socket
    int sock = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0)
        ERRORF("socket() failed()");

    // init an ip header and a tcp header
    char datagram[4096];
    struct ip *iphead = (struct ip *)datagram;
    struct tcphdr *tcphead = (struct tcphdr *)datagram + sizeof(struct ip);
    memset(datagram, 0, 4096);
    iphead->ip_hl = 5;
    iphead->ip_v = 4;
    iphead->ip_tos = 0; // not very useful for ipv4
    iphead->ip_len = sizeof(struct ip) + sizeof(struct tcphdr);
    iphead->ip_id = 0;
    iphead->ip_off = 0; // what?
    iphead->ip_ttl = 225;
    iphead->ip_p = 6; // tcp
    iphead->ip_sum = 0;
    if (inet_pton(AF_INET, "0.0.0.0", &(iphead->ip_src)) == 0) // let kernel set the src address for you
        FATALF("inet_pton() failed", "invalid source address");
    iphead->ip_dst.s_addr = destAddrPtr->sin_addr.s_addr; // copy dest address
    tcphead->th_sport = 0; // source port, dest port will be handled later
    tcphead->th_seq = 1;
    tcphead->th_ack = 0;
    tcphead->th_x2 = 0;
    tcphead->th_off = 0;
    tcphead->th_flags = TH_SYN;
    tcphead->th_win = htons(65535); // an arbitrary window size
    tcphead->th_sum = 0;
    tcphead->th_urp = 0;
    in_port_t portNum;
    portNum = 1;
    while(portNum <= 65535) {
        destAddrPtr->sin_port = htons(portNum);
        tcphead->th_dport = destAddrPtr->sin_port;

        iphead->ip_sum = csum((unsigned short *)datagram, iphead->ip_len >> 1);
        { // notify kernel an ip header is already included
            const int optOn = 1;
            if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &optOn, sizeof(optOn)) < 0)
                ERRORF("setsockopt() failed");
        }

        if (sendto(sock, datagram, iphead->ip_len, 0, (struct sockaddr *)destAddrPtr, sizeof(struct sockaddr_in)) < 0)
            ERRORF("sendto() failed");
        portNum++;
    } 
}

void scan_with_connect(struct sockaddr_in *destAddrPtr)
{
    // init a destination port, starting from 1
    in_port_t portNum;
    portNum = 1;
    destAddrPtr->sin_port = htons(portNum);

    // init a socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        ERRORF("socket() failed");

    // try to connect ports
    int conn;
    do {
        conn = connect(sock, (struct sockaddr *)destAddrPtr, sizeof(struct sockaddr_in));
        if (conn == 0)
            printf("open port: %d\n", portNum);
        else if (conn < 0 && errno != ECONNREFUSED)
            ERRORF("connect() failed");
        close(sock);
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        portNum++;
        destAddrPtr->sin_port = htons(portNum);
    } while (portNum <= 65535);
}
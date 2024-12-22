#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/types.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>  
#define NETLINK_TESTFAMILY 25
#define MULTICAST_GROUP_ID 1  

void handle_message(struct nlmsghdr *nlh) {
    char *msg = (char *)NLMSG_DATA(nlh);
    char *token;
    token = strtok(msg,"\n");
    printf("msg is %s\n", msg);
    unsigned int  saddr;
    unsigned char protocol;
    token = strtok(NULL, "\n");
    
    // 複製協議和源地址
    if (token) {
        memcpy(&protocol, token, sizeof(protocol));
        memcpy(&saddr, token + sizeof(protocol), sizeof(saddr));
    }
    // token = strtok(NULL, "\n"); 
    // memcpy(&protocol, token, sizeof(protocol));
    // memcpy(&saddr, token + sizeof(protocol), sizeof(saddr));

    // printf("Received message: %s\n", msg);
    //printf("IP : %pI4 protocol %s\n",saddr, protocol);
    // struct in_addr ip_addr;
    // ip_addr.s_addr = saddr;
    char ip_str[INET_ADDRSTRLEN];  // 用於存儲 IP 地址字符串
    inet_ntop(AF_INET, &saddr, ip_str, INET_ADDRSTRLEN);

    // 打印源 IP 地址和協議
    //printf("IP : %s protocol : %u\n", ip_str, protocol);
    printf("IP : %s protocol : %u\n", ip_str, protocol);

}

int main() {
    struct sockaddr_nl sa;
    struct nlmsghdr *nlh;
    struct iovec iov;
    struct msghdr msg;
    int sock;
    int ret;

    sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_TESTFAMILY);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.nl_family = AF_NETLINK;
    sa.nl_pid = 0;  
    sa.nl_groups = MULTICAST_GROUP_ID; 

    ret = bind(sock, (struct sockaddr *)&sa, sizeof(sa));
    if (ret < 0) {
        perror("bind");
        close(sock);
        return -1;
    }

    while (1) {
        char buffer[4096];

        memset(&msg, 0, sizeof(msg));
        iov.iov_base = buffer;
        iov.iov_len = sizeof(buffer);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        ret = recvmsg(sock, &msg, 0);
        if (ret < 0) {
            perror("recvmsg");
            close(sock);
            return -1;
        }

        nlh = (struct nlmsghdr *)buffer;
        if (NLMSG_OK(nlh, ret)) {
            handle_message(nlh);
        }
    }

    close(sock);
    return 0;
}


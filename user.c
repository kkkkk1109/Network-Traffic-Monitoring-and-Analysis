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

#define NETLINK_TESTFAMILY 25
#define MULTICAST_GROUP_ID 1  

void handle_message(struct nlmsghdr *nlh) {
    char *msg = (char *)NLMSG_DATA(nlh);
    printf("Received message: %s\n", msg);
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


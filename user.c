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
#include <netdb.h>
#include "uthash/include/uthash.h"
#include <signal.h>
#define NETLINK_TESTFAMILY 25
#define MULTICAST_GROUP_ID 1  

typedef struct {
    char ip_adr[20];
    //[0] tcp packet numbers, [1] udp, [2] total packet lens btye
    unsigned long lens[3];
    UT_hash_handle hh;
}ip_stat;
// hash table
ip_stat *ht;
ip_stat *create(char *addr, int lens, int protocol){
    ip_stat *new = (ip_stat *)calloc(1, sizeof(ip_stat));
    strcpy(new->ip_adr, addr);
    if(protocol == 6){
        new->lens[0]++;
    }else if(protocol == 17){
        new->lens[1]++;
    }
    new->lens[2] += lens;
    return new;
}
void hash_check(char *addr, int lens, int protocol){
    ip_stat *find = NULL;
    HASH_FIND_STR(ht, addr, find);
    if(find == NULL){
        ip_stat *item = create(addr, lens, protocol);
        HASH_ADD_STR(ht, ip_adr, item);
    }else{
        if(protocol == 6){
            find->lens[0]++;
        }else if(protocol == 17){
            find->lens[1]++;
        }
        find->lens[2] += lens;
    }
}
// void print_all_ip(){
//     ip_stat *item, *tmp;
//     printf("ip:              |   tcp  |  udp |  lens(byte)  \n");
//     HASH_ITER(hh, ht, item, tmp){
//         printf("%s      | %lu  | %lu | %lu \n",item->ip_adr, item->lens[0], item->lens[1], item->lens[2]);
//     }
// }
void print_all_ip() {
    ip_stat *item, *tmp;
    // 表頭格式化輸出
    printf("%-15s | %5s | %5s | %10s\n", "IP", "TCP", "UDP", "Length(Bytes)");
    printf("------------------------------------------------------\n");

    // 輸出每個項目
    HASH_ITER(hh, ht, item, tmp) {
        printf("%-15s | %5lu | %5lu | %10lu\n", 
               item->ip_adr, item->lens[0], item->lens[1], item->lens[2]);
    }
}

void delete_ip(){
    ip_stat *item, *tmp;
    HASH_ITER(hh, ht, item, tmp){
        HASH_DEL(ht, item);
        free(item);
    }
}
void handle_message(struct nlmsghdr *nlh) {
    char *msg = (char *)NLMSG_DATA(nlh);
    char *token;
    //printf("msg is %s\n", msg);
    unsigned int protocol;
    unsigned int lens;
    char ipadr[20];
    token = strtok(msg,",");
    //printf("ip : %s", token);
    strcpy(ipadr, token);
    token = strtok(NULL, ",");
    //printf("protocol : %s", token);
    protocol = atoi(token);
    token = strtok(NULL, ",");
    //printf("lens : %s\n", token);
    lens = atoi(token);
    //printf("ip : %s protocol: %u, lens: %u\n", ipadr, protocol, lens);
    hash_check(ipadr, lens, protocol);
    print_all_ip();
}
void handle_sigint(int sig){
    printf("end program\n");
    delete_ip();
    printf("free ut hash table success!\n");
    exit(0);
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
    signal(SIGINT, handle_sigint);
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
    ht = NULL;
    //int count = 0;
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
        //count++;

    }
    delete_ip();
    close(sock);
    return 0;
}


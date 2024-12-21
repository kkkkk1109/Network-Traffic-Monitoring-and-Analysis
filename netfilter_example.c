#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/netlink.h>
// #include <net/netlink.h>
// #include <net/net_message.h>
// #include <net/netlink.h>
// #include <net/net_message.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
struct sock *socket;

#define NETLINK_TESTFAMILY 25
#define NETLINK_MYGORUP 2

// Hook structure
static struct nf_hook_ops nfho;

// hook function
static unsigned int hook_func(void *priv,struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header;
    if (!skb)
        return NF_ACCEPT;

    ip_header = ip_hdr(skb);
    //unsigned int protocol = ip_header->protocol;
    //message
    struct nlmsghdr *nlh;


    //int result;
    char message[128];
    size_t message_size;
    struct sk_buff *skb_out;
    if (ip_header) {
        // print in kernal
        pr_info("Netfilter Hook: Src IP = %pI4, Dest IP = %pI4",
                &ip_header->saddr, &ip_header->daddr);
        // generate message
        snprintf(message, sizeof(message), "Netfilter Hook: Src IP = %pI4, Dest IP = %pI4",
                &ip_header->saddr, &ip_header->daddr);
        message_size = strlen(message) + 1;
        skb_out = nlmsg_new(message_size, GFP_KERNEL);
        if (!skb_out) {
            pr_err("Failed to allocate a new skb\n");
            return NF_ACCEPT;
        }
        nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, message_size, 0);
        if(!nlh){
            pr_err("Failed to add nlmsghdr to skb\n");
            kfree_skb(skb_out);
            return NF_ACCEPT;
        }
        // dst_group = 0, unicast dst_group = 1, multicast
        NETLINK_CB(skb_out).dst_group = 1;
        strncpy(nlmsg_data(nlh), message, message_size);
        nlmsg_multicast(socket, skb_out, 0, 1, GFP_KERNEL);

		// if(protocol == IPPROTO_TCP){
		// 	pr_info("protocol: TCP\n");
		// }else if(protocol == IPPROTO_UDP){
		// 	pr_info("protocol : UDP\n");
		// }else if(protocol == IPPROTO_ICMP){
		// 	pr_info("protocol : ICMP");
		// }

	}

    return NF_ACCEPT;
}

static int __init netfilter_example_init(void) {
    pr_info("Netfilter example module loaded.\n");
    // dont have to process meesage from userspace
    struct netlink_kernel_cfg config = {
                .input = NULL
        };
    // create socket
    socket = netlink_kernel_create(&init_net, NETLINK_TESTFAMILY, &config);
    if(socket == NULL) return -1;
    // netfilter hook initializaion
    nfho.hook = hook_func;              
    nfho.hooknum = NF_INET_PRE_ROUTING;     
    nfho.pf = PF_INET;                      
    nfho.priority = NF_IP_PRI_FIRST;        

    if (nf_register_net_hook(&init_net, &nfho)) {
        pr_err("Netfilter: Failed to register net hook.\n");
        return -1;
    }

    return 0;
}

static void __exit netfilter_example_exit(void) {
    pr_info("Netfilter example module unloaded.\n");
    nf_unregister_net_hook(&init_net, &nfho);
}

module_init(netfilter_example_init);
module_exit(netfilter_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("kkkkk1109");
MODULE_DESCRIPTION("Netfilter ex");

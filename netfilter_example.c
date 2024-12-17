#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
// Hook structure
static struct nf_hook_ops nfho;

// hook function
static unsigned int hook_func(void *priv,
                              struct sk_buff *skb,
                              const struct nf_hook_state *state) {
    struct iphdr *ip_header;
    if (!skb)
        return NF_ACCEPT;

    ip_header = ip_hdr(skb);
    unsigned int protocal = ip_header->protocol;
	if (ip_header) {
        pr_info("Netfilter Hook: Src IP = %pI4, Dest IP = %pI4",
                &ip_header->saddr, &ip_header->daddr);
		if(protocal == IPPROTO_TCP){
			pr_info("protocal: TCP\n");
		}else if(protocal == IPPROTO_UDP){
			pr_info("protocal : UDP\n");
		}
	}

    return NF_ACCEPT;
}

static int __init netfilter_example_init(void) {
    pr_info("Netfilter example module loaded.\n");

    
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
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Simple Netfilter Example");

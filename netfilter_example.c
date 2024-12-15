#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

// Hook 結構
static struct nf_hook_ops nfho;

// 回調函數
static unsigned int hook_func(void *priv,
                              struct sk_buff *skb,
                              const struct nf_hook_state *state) {
    struct iphdr *ip_header;

    // 檢查是否為有效的 IPv4 封包
    if (!skb)
        return NF_ACCEPT;

    ip_header = ip_hdr(skb);
    if (ip_header) {
        pr_info("Netfilter Hook: Src IP = %pI4, Dest IP = %pI4\n",
                &ip_header->saddr, &ip_header->daddr);
    }

    // 繼續放行封包
    return NF_ACCEPT;
}

// 模組初始化
static int __init netfilter_example_init(void) {
    pr_info("Netfilter example module loaded.\n");

    // 設置 hook 操作
    nfho.hook = hook_func;                  // 設置回調函數
    nfho.hooknum = NF_INET_PRE_ROUTING;     // PRE_ROUTING 階段
    nfho.pf = PF_INET;                      // IPv4
    nfho.priority = NF_IP_PRI_FIRST;        // 優先級

    // 註冊 hook（使用新的 API）
    if (nf_register_net_hook(&init_net, &nfho)) {
        pr_err("Netfilter: Failed to register net hook.\n");
        return -1;
    }

    return 0;
}

// 模組卸載
static void __exit netfilter_example_exit(void) {
    pr_info("Netfilter example module unloaded.\n");

    // 移除 hook（使用新的 API）
    nf_unregister_net_hook(&init_net, &nfho);
}

module_init(netfilter_example_init);
module_exit(netfilter_example_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("YourName");
MODULE_DESCRIPTION("Simple Netfilter Example");


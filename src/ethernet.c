#include "ethernet.h"
#include "utils.h"
#include "driver.h"
#include "arp.h"
#include "ip.h"
#include <string.h>
#include <stdio.h>

/**
 * @brief 处理一个收到的数据包
 *        你需要判断以太网数据帧的协议类型，注意大小端转换
 *        如果是ARP协议数据包，则去掉以太网包头，发送到arp层处理arp_in()
 *        如果是IP协议数据包，则去掉以太网包头，发送到IP层处理ip_in()
 * 
 * @param buf 要处理的数据包
 */
void ethernet_in(buf_t *buf)
{
#define ETHERNET_HEAD_LEN 14
    int is_ARP = buf->data[12] == 8u && buf->data[13] == 6u;
    int is_IP = buf->data[12] == 8u && buf->data[13] == 0u;
    buf_remove_header(buf, ETHERNET_HEAD_LEN);
    if(is_ARP) arp_in(buf);
    else if(is_IP) ip_in(buf);
#undef ETHERNET_HEAD_LEN
}

/**
 * @brief 处理一个要发送的数据包
 *        你需添加以太网包头，填写目的MAC地址、源MAC地址、协议类型
 *        添加完成后将以太网数据帧发送到驱动层
 * 
 * @param buf 要处理的数据包
 * @param mac 目标ip地址
 * @param protocol 上层协议
 */
void ethernet_out(buf_t *buf, const uint8_t *mac, net_protocol_t protocol)
{
#define ETHERNET_HEAD_LEN 14
    buf_add_header(buf, ETHERNET_HEAD_LEN);
    uint8_t source_mac[6] = DRIVER_IF_MAC;
    uint8_t *p = buf->data;
    for(int i = 0; i < 6; i++) *p++ = mac[i];
    for(int i = 0; i < 6; i++) *p++ = source_mac[i];
    *p++ = protocol >> 8;
    *p++ = protocol & 255;
    driver_send(buf);
#undef ETHERNET_HEAD_LEN
}

/**
 * @brief 初始化以太网协议
 * 
 * @return int 成功为0，失败为-1
 */
int ethernet_init()
{
    buf_init(&rxbuf, ETHERNET_MTU + sizeof(ether_hdr_t));
    return driver_open();
}

/**
 * @brief 一次以太网轮询
 * 
 */
void ethernet_poll()
{
    if (driver_recv(&rxbuf) > 0)
        ethernet_in(&rxbuf);
}

#ifndef __ETH_IP_H__
#define __ETH_IP_H__

#include "ethernet.h"
#include "Arduino.h"
/**
 * Implement to handle incoming UDP data.
 */
uint16_t handleUDPData(uint16_t* port, uint8_t* data, uint16_t length);

uint16_t ethernet_ip_recieve(ip_packet* p, address* myaddress,
                             address* target);
uint16_t ethernet_ip_handleICMP(icmp_packet* p, size_t length, address* myaddress);
uint16_t ethernet_ip_handleUDP(udp_packet* p, address* myaddress);


/**
 * Send an ip packet.
 * 
 *
 * @param myaddress  ip & mac-address of sender.
 * @param destination  The mac and ip address of the destination. Use
 *          ethernet_arp_getMac() to look up a mac-address for an ip.
 * @param frame    Ethernet frame to store the UPD packet in. Minimum
 *                 size of frame->data is 20 + length.

 */
void ethernet_ip_send(ethernet_frame* frame, address* myaddress,
                      address* destination, uint16_t length);
/**
 * Creates a UDP packet in <code>buffer</code>.
 *
 * @param dest_port   Destination port.
 * @param src_port   port to send the packet from.
 * @param data  payload to send in the packet.
 * @param length  the size of the data in bytes.
 * @param frame    Ethernet frame to store the UPD packet in. Minimum
 *                 size of frame->data is 28 + length.
 */
void ethernet_ip_sendUDP(ethernet_frame* frame,
                         address* destination, uint16_t dest_port,
                         address* myaddress, uint16_t src_port,
                         uint8_t* data, uint8_t length);

#endif

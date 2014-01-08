#ifndef __ETH_ARP_H__
#define __ETH_ARP_H__

#include "ethernet.h"
#include "Arduino.h"

uint16_t ethernet_arp_recieve(arp_packet* p, address* myaddress,
                              address* target);

/**
 * Non-blocking function that finds the mac-address for an ip-address.
 */
boolean ethernet_arp_getMac(address* target, address* myaddress,
                         ethernet_frame* frame, uint16_t* timeout);

/**
 * @param ip      IP address to discover.
 * @param myaddress Address of sender.
 * @param frame   Ethernet frame to create the arp packet in. The length
 *                of frame->data has to be at least 28.
 */
boolean ethernet_arp_discover(uint8_t* ip, address* myaddress,
                              ethernet_frame* frame);


#endif

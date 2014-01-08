#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "Arduino.h"
#include "enc28j60.h" 
#include "netstruct.h"

typedef struct {
  uint8_t mac[6];
  uint8_t ip[4];
} address ;

// Broadcast mac address
static uint8_t ethernet_broadcast_mac[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/**
 * Implement to handle incoming arp packets
 */
uint16_t ethernet_arp_recieve(arp_packet* p, address* myaddress,
                              address* target);
/**
 * Implement to handle incoming ip packets
 */
uint16_t ethernet_ip_recieve(ip_packet* p, address* myaddress,
                             address* target);


void ethernet_setup(address* myaddress);
/**
 * Process an incoming ethernet frame.
 */
uint16_t ethernet_recieveFrame(address* myaddress, address* target,
                               uint8_t* buffer, uint16_t buffer_size);
uint16_t ethernet_checksum(uint8_t* data, uint8_t len);

void ethernet_send(ethernet_frame* frame, uint8_t* source_mac,
                   uint8_t* dest_mac, uint16_t length);

#endif

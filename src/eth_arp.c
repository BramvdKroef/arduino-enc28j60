#include "eth_arp.h"

uint16_t ethernet_arp_recieve(arp_packet* p, address* myaddress,
                              address* target) {
  arp_eth_ip_data* data;
  uint8_t i;
  
  // hardware type is ethernet, protocol is ipv4 & target ip is my
  // ip  
  if (p->hardware_type != ARP_HARDWARE_TYPE_ETH ||
      p->protocol_type != ARP_PROTOCOL_TYPE_IP4) {
    return 0;
  }
  data = (arp_eth_ip_data*)(p + 1);

  if (memcmp(data->target_ip, myaddress->ip, 4) != 0) 
    return 0;
  
  // Operation is a request
  if (p->operation == ARP_OPER_REQUEST) {
    // Create reply
    p->operation = ARP_OPER_REPLY;
    // Use sender info to set target 
    memcpy(data->target_mac, data->sender_mac, p->hardware_length);
    memcpy(data->target_ip, data->sender_ip, p->protocol_length);
    // Set sender to my address. 
    memcpy(data->sender_mac, myaddress->mac, 6);
    memcpy(data->sender_ip, myaddress->ip, 4);
    return sizeof(arp_packet) + (p->hardware_length + p->protocol_length) * 2;

  } else if (p->operation == ARP_OPER_REPLY) {
    // Link sender mac to sender ip
    if(memcmp(target->ip, data->sender_ip, 4) == 0) 
      memcpy(target->mac, data->sender_mac, 6);
  }
  return 0;
}

boolean ethernet_arp_discover(uint8_t* ip, address* myaddress,
                              ethernet_frame* frame) {
  frame->ethertype = ETHTYPE_ARP;

  arp_packet* p = (arp_packet*)(frame + 1);
  p->hardware_type = ARP_HARDWARE_TYPE_ETH;
  p->protocol_type = ARP_PROTOCOL_TYPE_IP4;
  p->operation = ARP_OPER_REQUEST;
  p->hardware_length = 6;
  p->protocol_length = 4;

  arp_eth_ip_data* data = (arp_eth_ip_data*)(p + 1);
  memcpy(data->target_mac, ethernet_broadcast_mac, 6);
  memcpy(data->target_ip, ip, 4);
  memcpy(data->sender_mac, myaddress->mac, 6);
  memcpy(data->sender_ip, myaddress->ip, 6);

  ethernet_send(frame, myaddress->mac, ethernet_broadcast_mac, 8 + 20);
}

boolean ethernet_arp_getMac(address* target, address* myaddress,
                         ethernet_frame* frame, uint16_t* timeout) {
  if (target->mac[0] != 0) 
    return true;
  
  if (*timeout == 0) { 
    ethernet_arp_discover(target->ip, myaddress, frame);
    *timeout = 1;
  } else {
    delay(10);
    if ((*timeout)++ == 200) {
      // Timeout resolving mac. Start over.
      *timeout = 0;
    }
  }
  return false;
}

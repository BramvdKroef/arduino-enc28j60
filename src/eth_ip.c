#include "eth_ip.h"

uint16_t ethernet_ip_recieve(ip_packet* p, address* myaddress,
                             address* target) {
  uint16_t length;
  
  if (memcmp(p->destination, myaddress->ip, 4) != 0)
    return 0;

  uint16_t replyLen = 0;
  length = reverse16(p->length);
  
  switch (p->protocol) {
  case IP_PROTO_ICMP:
    replyLen = ethernet_ip_handleICMP((icmp_packet*)(p + 1),
                                      length - sizeof(ip_packet), myaddress);
    break;
  case IP_PROTO_UDP:
    replyLen = ethernet_ip_handleUDP((udp_packet*)(p + 1), myaddress);
    break;
  case IP_PROTO_TCP:
    // handle tcp packets
    break;
  }
  
  if (replyLen > 0) {
    memcpy(p->destination, p->source, 4);
    memcpy(p->source, myaddress->ip, 4);
    length = 20 + replyLen;
    p->id = p->id + 1;
    p->length = reverse16(length);
    p->checksum = 0;
    p->checksum = ethernet_checksum((uint8_t*)p, 20);
    p->checksum = reverse16(p->checksum);

    return length;
  }
  return 0;
}

uint16_t ethernet_ip_handleICMP(icmp_packet* p, size_t length, address* myaddress) {
  if (p->type == ICMP_TYPE_ECHO_REQ) {
    // reply to ping requests
    p->type = ICMP_TYPE_ECHO_REPLY;
    p->checksum = 0;
    p->checksum = ethernet_checksum((uint8_t*)p, length);
    p->checksum = reverse16(p->checksum);

    return length;
  }
  return 0;
}

uint16_t ethernet_ip_handleUDP(udp_packet* p, address* myaddress) {
  uint16_t port = p->dest_port;
<<<<<<< HEAD
  p->dest_port = ((p->dest_port >> 8) & 0xFF) | ((p->dest_port << 8)
                                                 & 0xFF00);
  uint16_t length = ((p->length >> 8) & 0xFF) | ((p->length << 8)
                                                 & 0xFF00);
  uint16_t replyLen = 8 + handleUDPData(&p->dest_port, (uint8_t*)(p + 1),
                                        length - 8); 
=======
  p->dest_port = reverse16(p->dest_port);
                                         
  uint16_t length = reverse16(p->length);
  uint16_t replyLen = handleUDPData(&p->dest_port, (uint8_t*)(p + 1),
                                    length - 8); 
>>>>>>> 281ff16579314e41e7f558d1693f29effd3b6069
  
  if (replyLen > 0)  {
    p->dest_port = p->src_port;
    p->src_port = port;
    p->length = ((replyLen >> 8) & 0xFF) | ((replyLen << 8) & 0xFF00);
    p->checksum = 0; // checksum is optional
  }
  return replyLen;
}

void ethernet_ip_send(ethernet_frame* frame, address* myaddress,
                      address* destination, uint16_t length) {
  frame->ethertype = ETHTYPE_IP;
  
  ip_packet* ip = (ip_packet*)(frame + 1);
  ip->ihl_version = 4 + (0x5 << 4);
  ip->dscp_ecn = 0;
  memcpy(ip->source, myaddress->ip, 4);
  memcpy(ip->destination, destination->ip, 4);
  ip->ttl = 100;
  ip->offset = 0;
  ip->id = 1;

  ip->length = 20 + length;
  ip->length = reverse16(ip->length);
  ip->checksum = 0;
  ip->checksum = ethernet_checksum((uint8_t*)ip, 20);
  ip->checksum = reverse16(ip->checksum);
  ethernet_send(frame, myaddress->mac, destination->mac, 20 + length);
}


void ethernet_ip_sendUDP(ethernet_frame* frame,
                         address* destination, uint16_t dest_port,
                         address* myaddress, uint16_t src_port,
                         uint8_t* data, uint8_t length) {
  
  ip_packet* ip = (ip_packet*)(frame + 1);
  ip->protocol = IP_PROTO_UDP;
    
  udp_packet* udp = (udp_packet*)(ip + 1);
  memcpy((uint8_t*)(udp + 1), data, length);
  udp->src_port = reverse16(src_port);
  udp->dest_port = reverse16(dest_port);
  udp->length = length + 8;
  udp->length = reverse16(udp->length);
  
  ethernet_ip_send(frame, myaddress, destination, length + 8);
}

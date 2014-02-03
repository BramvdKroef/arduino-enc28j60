#include "ethernet.h"

void ethernet_setup(address* myaddress){

  /*initialize enc28j60*/
  enc28j60Init(myaddress->mac);
 
  // change clkout from 6.25MHz to 12.5MHz
  enc28j60clkout(2);

  delay(10);
  
  //* Magjack leds configuration, see enc28j60 datasheet, page 11 /
  // LEDA=green LEDB=yellow
  //
  // 0x880 is PHLCON LEDB=on, LEDA=on
  // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
  enc28j60PhyWrite(PHLCON,0x880);
  delay(500);
  
  //
  // 0x990 is PHLCON LEDB=off, LEDA=off
  // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
  enc28j60PhyWrite(PHLCON,0x990);
  delay(500);
 
  //
  // 0x880 is PHLCON LEDB=on, LEDA=on
  // enc28j60PhyWrite(PHLCON,0b0000 1000 1000 00 00);
  enc28j60PhyWrite(PHLCON,0x880);
  delay(500);
 
  //
  // 0x990 is PHLCON LEDB=off, LEDA=off
  // enc28j60PhyWrite(PHLCON,0b0000 1001 1001 00 00);
  enc28j60PhyWrite(PHLCON,0x990);
  delay(500);

  //
  // 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
  // enc28j60PhyWrite(PHLCON,0b0000 0100 0111 01 10);
  enc28j60PhyWrite(PHLCON,0x476);
  delay(100);

}

uint16_t ethernet_recieveFrame(address* myaddress, address* target,
                               uint8_t* buffer, uint16_t buffer_size) {
  uint16_t package_len = enc28j60PacketReceive(buffer_size, buffer);
  
  if (package_len == 0)
    return 0;
  
  ethernet_frame* frame = (ethernet_frame*)buffer;
  uint16_t replyLen = 0;
  uint8_t* payload = buffer + sizeof(ethernet_frame);
  
  if (frame->ethertype == ETHTYPE_ARP) {
    replyLen = ethernet_arp_recieve((arp_packet*)payload,
                                    myaddress, target);

  } else if (frame->ethertype == ETHTYPE_IP) {
    replyLen = ethernet_ip_recieve((ip_packet*)payload,
                                   myaddress, target);
  }

  if (replyLen > 0) {
    memcpy(frame->destination, frame->source, 6);
    memcpy(frame->source, myaddress->mac, 6);
    enc28j60PacketSend(14 + replyLen, buffer);
  }
  return replyLen;
}

void ethernet_send(ethernet_frame* frame, uint8_t* source_mac,
                   uint8_t* dest_mac, uint16_t length) {
  memcpy(frame->source, source_mac, 6);
  memcpy(frame->destination, dest_mac, 6);
  
  enc28j60PacketSend(14 + length, (uint8_t*)frame);
}


uint16_t ethernet_checksum(uint8_t* data, uint8_t len) {
  uint32_t checksum = 0;
  
  while (len > 1)  {
    checksum += ((*data << 8) & 0xFF00) | (*(data + 1) & 0xFF);
    data += 2;
    len -= 2;
  }

  /*  Add left-over byte, if any */
  if (len > 0)
    checksum += *data;

  /*  Fold 32-bit sum to 16 bits */
  while (checksum >> 16)
    checksum = (checksum & 0xffff) + (checksum >> 16);

  return (uint16_t) ~checksum;
}


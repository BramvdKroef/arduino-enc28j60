#ifndef __NETSTRUCT_H__
#define __NETSTRUCT_H__

typedef struct {
  uint8_t destination[6];
  uint8_t source[6];
  uint16_t ethertype;
  //uint32_t checksum;
} ethernet_frame;

#define ETHTYPE_IP	0x0008
#define ETHTYPE_ARP	0x0608

typedef struct {
  uint16_t hardware_type;
  uint16_t protocol_type;
  uint8_t hardware_length;
  uint8_t protocol_length;
  uint16_t operation;
} arp_packet;

#define ARP_HARDWARE_TYPE_ETH 0x0100
#define ARP_PROTOCOL_TYPE_IP4 0x0008
#define ARP_OPER_REQUEST      0x0100
#define ARP_OPER_REPLY        0x0200

typedef struct {
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
} arp_eth_ip_data;

typedef struct {
  uint8_t ihl_version;
  uint8_t dscp_ecn;
  uint16_t length;
  uint16_t id;
  uint16_t offset;
  uint8_t ttl;
  uint8_t protocol;
  uint16_t checksum;
  uint8_t source[4];
  uint8_t destination[4];
} ip_packet;

#define IP_PROTO_ICMP 0x01
#define IP_PROTO_TCP 0x06
#define IP_PROTO_UDP 0x11

typedef struct {
  uint16_t src_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
} udp_packet;

/*
 * UDP IPv4 pseudo header; used for checksum calculation.
 */
typedef struct {
  uint8_t source[4];
  uint8_t destination[4];
  uint8_t zeroes;
  uint8_t protocol;
  uint8_t udp_length;
} udp_ip4_pseudo_header;

typedef struct {
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint32_t header;
} icmp_packet;

#define ICMP_TYPE_ECHO_REPLY   0
#define ICMP_TYPE_UNREACHABLE  3
#define ICMP_TYPE_ECHO_REQ     8
#define ICMP_TYPE_TTL_EXCEEDED 11

#endif

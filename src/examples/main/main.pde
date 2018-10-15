
extern "C" {
#include "ethernet.h"
#include "eth_arp.h"
  uint16_t handleUDPData(uint16_t* port, uint8_t* data,
    uint16_t length);  
}

/**
 * Configure mac and ip address for the device.
 */
static address myaddress = {{0x54,0x55,0x58,0x10,0x00,0x24},
                            {192,168,1,12}};

/**
 * Set server mac and ip to the broadcast mac/ip.
 * 
 *  If you want to set the address to a specific ip you can set the
 *  mac address to all zeroes. The mac will be resolved using arp.
 */
static address server = {{0x0,0x0,0x0,0x0,0x0,0x0},
                         {192,168,1,1}};

#define BUFFER_SIZE 500 
static uint8_t buffer[BUFFER_SIZE + 1];
static uint16_t timeout;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting up.");
  ethernet_setup(&myaddress);
  Serial.println((int)enc28j60getrev());
  Serial.println("Ready.");
}

void loop() {
  ethernet_recieveFrame(&myaddress, &server, buffer,
                        BUFFER_SIZE);
  
  if (ethernet_arp_getMac(&server, &myaddress,
                          (ethernet_frame*) buffer, &timeout)) {
    // the mac address of the server is known
  }
  delay(100);
}

/**
 * Incoming udp packet. Write data to serial.
 */
uint16_t handleUDPData(uint16_t* port, uint8_t* data,
                       uint16_t length) {
  Serial.println(*port);
  Serial.write(data, length);
  return 0;
}

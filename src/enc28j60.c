/*********************************************
 * vim:sw=8:ts=8:si:et
 * To use the above modeline in vim you must have "set modeline" in your .vimrc
 * Author: Guido Socher 
 * Copyright: GPL V2
 * http://www.gnu.org/licenses/gpl.html
 *
 * Based on the enc28j60.c file from the AVRlib library by Pascal Stang.
 * For AVRlib See http://www.procyonengineering.com/
 * Used with explicit permission of Pascal Stang.
 *
 * Title: Microchip ENC28J60 Ethernet Interface Driver
 * Chip type           : ATMEGA88 with ENC28J60
 *********************************************/
#include <avr/io.h>
//#include "avr_compat.h"
#include "enc28j60.h"
#include "Arduino.h"
//#include "WConstants.h"  //all things wiring / arduino
//#include "timeout.h"
//
//#define F_CPU 10000000UL  // 12.5 MHz
/*
#ifndef ALIBC_OLD
#include <util/delay.h>
#else
#include <avr/delay.h>
#endif
*/

typedef struct enc28j60_connection {
  uint8_t cs_pin;
  uint8_t bank;
  uint16_t packet;
} enc28j60_connection;


uint8_t enc28j60_readOp(enc28j60_connection* c, uint8_t op, uint8_t address) {
  uint8_t ret;
  
  spi_cs_on(c->cs_pin);

  // issue read command
  spi_write(op | (address & ADDR_MASK));
  // read data
  ret = spi_read();
  // do dummy read if needed (for mac and mii, see datasheet page 29)
  if(address & 0x80) 
    ret = spi_read();
  
  // release CS
  spi_cs_off(c->cs_pin);
  
  return ret;
}

void enc28j60_writeOp(enc28j60_connection* c, uint8_t op, uint8_t address, uint8_t data){
  spi_cs_on(c->cs_pin);

  // issue write command
  spi_write(op | (address & ADDR_MASK));
  // write data
  spi_write(data);

  spi_cs_off(c->cs_pin);
}

void enc28j60_readBuffer(enc28j60_connection* c, uint16_t len, uint8_t* data) {
  spi_cs_on(c->cs_pin);

  // issue read command
  spi_write(ENC28J60_READ_BUF_MEM);
  spi_read_buffer(data, len);
  
  spi_cs_off(c->cs_pin);
}

void enc28j60_writeBuffer(enc28j60_connection* c, uint16_t len, uint8_t* data){
  spi_cs_on(c->cs_pin);
  
  // issue write command
  spi_write(ENC28J60_WRITE_BUF_MEM);
  spi_write_buffer(data, len);

  spi_cs_off(c->cs_pin);
}

void enc28j60_setBank(enc28j60_connection* c, uint8_t address) {
  // set the bank (if needed)
  if((address & BANK_MASK) != c->bank){
    // set the bank
    enc28j60_writeOp(c, ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
    enc28j60_writeOp(c, ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
    c->bank = (address & BANK_MASK);
  }
}

uint8_t enc28j60_read(enc28j60_connection* c, uint8_t address) {
  // set the bank
  enc28j60_setBank(c, address);
  // do the read
  return enc28j60_readOp(c, ENC28J60_READ_CTRL_REG, address);
}

void enc28j60_write(enc28j60_connection* c, uint8_t address, uint8_t data) {
  // set the bank
  enc28j60_setBank(c, address);
  // do the write
  enc28j60_writeOp(c, ENC28J60_WRITE_CTRL_REG, address, data);
}

void enc28j60_phyWrite(enc28j60_connection* c, uint8_t address, uint16_t data) {
  // set the PHY register address
  enc28j60_write(c, MIREGADR, address);
  // write the PHY data
  enc28j60_write(c, MIWRL, data);
  enc28j60_write(c, MIWRH, data>>8);
  // wait until the PHY write completes
  while(enc28j60_read(c, MISTAT) & MISTAT_BUSY){
    delayMicroseconds(15);
  }
}

void enc28j60_clkout(enc28j60_connection* c, uint8_t clk) {
  //setup clkout: 2 is 12.5MHz:
  enc28j60_write(c, ECOCON, clk & 0x7);
}

void enc28j60_reset(enc28j60_connection* c) {
  // perform system reset
  enc28j60_writeOp(c, ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
  delay(50);
  // check CLKRDY bit to see if reset is complete
  // The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
  //while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));

}

void enc28j60_setFilter(enc28j60_connection* c, uint16_t position,
                        uint16_t crc) {
  enc28j60_write(c, ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
  enc28j60_write(c, EPMM0, position & 0xFF);
  enc28j60_write(c, EPMM1, (position >> 8) & 0xFF);
  enc28j60_write(c, EPMCSL, crc & 0xFF);
  enc28j60_write(c, EPMCSH, (crc >> 8) & 0xFF);
}

void enc28j60_setMac(enc28j60_connection* c, uint8_t* macaddr) {
  // NOTE: MAC address in ENC28J60 is byte-backward
  enc28j60_write(c, MAADR5, macaddr[0]);
  enc28j60_write(c, MAADR4, macaddr[1]);
  enc28j60_write(c, MAADR3, macaddr[2]);
  enc28j60_write(c, MAADR2, macaddr[3]);
  enc28j60_write(c, MAADR1, macaddr[4]);
  enc28j60_write(c, MAADR0, macaddr[5]);
}

void enc28j60_init(enc28j60_connection* c, uint8_t cs_pin, uint8_t* macaddr) {
  // initialize I/O
  spi_init(cs_pin, 0x00, true);
  c->cs_pin = cs_pin;
  c->bank = 0;
  // set receive buffer start address
  c->packet = RXSTART_INIT;
  
  enc28j60_reset(c);

  // do bank 0 stuff
  // initialize receive buffer
  // 16-bit transfers, must write low byte first
  
  // Rx start
  enc28j60_write(c, ERXSTL, RXSTART_INIT&0xFF);
  enc28j60_write(c, ERXSTH, RXSTART_INIT>>8);

  // set receive pointer address
  enc28j60_write(c, ERXRDPTL, RXSTART_INIT&0xFF);
  enc28j60_write(c, ERXRDPTH, RXSTART_INIT>>8);
  // RX end
  enc28j60_write(c, ERXNDL, RXSTOP_INIT&0xFF);
  enc28j60_write(c, ERXNDH, RXSTOP_INIT>>8);
  // TX start
  enc28j60_write(c, ETXSTL, TXSTART_INIT&0xFF);
  enc28j60_write(c, ETXSTH, TXSTART_INIT>>8);
  // TX end
  enc28j60_write(c, ETXNDL, TXSTOP_INIT&0xFF);
  enc28j60_write(c, ETXNDH, TXSTOP_INIT>>8);

  // do bank 1 stuff, packet filter:
  // For broadcast packets we allow only ARP packtets
  // All other packets should be unicast only for our mac (MAADR)
  //
  // The pattern to match on is therefore
  // Type     ETH.DST
  // ARP      BROADCAST
  // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
  // in binary these poitions are:11 0000 0011 1111
  // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
  enc28j60_setFilter(c, 0x303F, 0xf7f9);

  // do bank 2 stuff
  // enable MAC receive
  enc28j60_write(c, MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
  // bring MAC out of reset
  enc28j60_write(c, MACON2, 0x00);
  // enable automatic padding to 60bytes and CRC operations
  enc28j60_writeOp(c, ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
  // set inter-frame gap (non-back-to-back)
  enc28j60_write(c, MAIPGL, 0x12);
  enc28j60_write(c, MAIPGH, 0x0C);
  // set inter-frame gap (back-to-back)
  enc28j60_write(c, MABBIPG, 0x12);
  // Set the maximum packet size which the controller will accept
  // Do not send packets longer than MAX_FRAMELEN:
  enc28j60_write(c, MAMXFLL, MAX_FRAMELEN&0xFF);	
  enc28j60_write(c, MAMXFLH, MAX_FRAMELEN>>8);

  // do bank 3 stuff
  // write MAC address
  enc28j60_setMac(c, macaddr);
  
  // no loopback of transmitted frames
  enc28j60_phyWrite(c, PHCON2, PHCON2_HDLDIS);
  // switch to bank 0
  enc28j60_setBank(ECON1);
  // enable interrutps
  enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);
  // enable packet reception
  enc28j60_writeOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

// read the revision of the chip:
uint8_t enc28j60_getrev(enc28j60_connection* c) {
  return (enc28j60_read(c, EREVID));
}

void enc28j60_packetSend(enc28j60_connection* c, uint16_t len, uint8_t* packet){
  // Set the write pointer to start of transmit buffer area
  enc28j60_write(c, EWRPTL, TXSTART_INIT&0xFF);
  enc28j60_write(c, EWRPTH, TXSTART_INIT>>8);
  // Set the TXND pointer to correspond to the packet size given
  enc28j60_write(c, ETXNDL, (TXSTART_INIT+len)&0xFF);
  enc28j60_write(c, ETXNDH, (TXSTART_INIT+len)>>8);
  // write per-packet control byte (0x00 means use macon3 settings)
  enc28j60_writeOp(c, ENC28J60_WRITE_BUF_MEM, 0, 0x00);
  // copy the packet into the transmit buffer
  enc28j60_writeBuffer(c, len, packet);
  // send the contents of the transmit buffer onto the network
  enc28j60_writeOp(c, ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
  // Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
  if( (enc28j60_read(c, EIR) & EIR_TXERIF) ){
    enc28j60_writeOp(c, ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
  }
}

// Gets a packet from the network receive buffer, if one is available.
// The packet will by headed by an ethernet header.
//      maxlen  The maximum acceptable length of a retrieved packet.
//      packet  Pointer where packet data should be stored.
// Returns: Packet length in bytes if a packet was retrieved, zero otherwise.
uint16_t enc28j60_packetReceive(enc28j60_connection* c, uint16_t maxlen, uint8_t* packet){
  uint16_t rxstat;
  uint16_t len;
  // check if a packet has been received and buffered
  //if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
  // The above does not work. See Rev. B4 Silicon Errata point 6.
  if( enc28j60_read(c, EPKTCNT) == 0){
    return(0);
  }
  
  // Set the read pointer to the start of the received packet
  enc28j60_write(c, ERDPTL, (c->packet));
  enc28j60_write(c, ERDPTH, (c->packet)>>8);
  // read the next packet pointer
  c->packet = enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0);
  c->packet |= enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0) << 8;

  // read the packet length (see datasheet page 43)
  len  = enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0);
  len |= enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0)<<8;
  len -= 4; //remove the CRC count
  // read the receive status (see datasheet page 43)
  rxstat  = enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0);
  rxstat |= enc28j60_readOp(c, ENC28J60_READ_BUF_MEM, 0)<<8;
  // limit retrieve length
  if (len > maxlen-1){
    len = maxlen-1;
  }
  // check CRC and symbol errors (see datasheet page 44, table 7-3):
  // The ERXFCON.CRCEN is set by default. Normally we should not
  // need to check this.
  if ((rxstat & 0x80)==0){
    // invalid
    len=0;
  }else{
    // copy the packet from the receive buffer
    enc28j60_readBuffer(c, len, packet);
  }
  // Move the RX read pointer to the start of the next received packet
  // This frees the memory we just read out
  enc28j60_write(c, ERXRDPTL, (c->packet));
  enc28j60_write(c, ERXRDPTH, (c->packet)>>8);
  // decrement the packet counter indicate we are done with this packet
  enc28j60_writeOp(c, ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
  return (len);
}


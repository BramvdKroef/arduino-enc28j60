
// low is active, high is passive
#define spi_cs_on(pin) digitalWrite(pin, LOW)
#define spi_cs_off(pin) digitalWrite(pin, HIGH) 


// Byte order is least significant bit first.
#define SPI_OPTIONS_LSB                       (1 << DORD)
// Clock signal is high when idle.
#define SPI_OPTIONS_CLOCK_POLARITY_HIGH_IDLE  (1 << CPOL)
// Sample data on trailing edge.
#define SPI_OPTIONS_CLOCK_PHASE_TRAILING_EDGE (1 << CPHA)

// SCK frequency (default is fosc/4)
#define SPI_OPTIONS_SPEED_FOSC16              0x01
#define SPI_OPTIONS_SPEED_FOSC64              0x02
#define SPI_OPTIONS_SPEED_FOSC128             0x03

void spi_init(uint8_t cs_pin, uint8_t options, bool double_speed) {
  pinMode(cs_pin, OUTPUT);
  spi_cs_off(cs_pin);

  pinMode(MOSI, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(MISO, INPUT);
	
  digitalWrite(MOSI, LOW);
  digitalWrite(SCK, LOW);

  SPCR =
    (1 << SPE)  |
    (1 << MSTR) |
    options;

  if (double_speed)
    SPSR |= (1 << SPI2X);
}

#define spi_read()      spi_transfer(0x00)
#define spi_write(data) spi_transfer(data)

void spi_read_buffer(uint8_t* buffer, size_t len) {
  while(len--) {
    *buffer = spi_read();
    buffer++;
  }
}

void spi_write_buffer(uint8_t* buffer, size_t len) {
  while(len--) {
    spi_write(*buffer);
    buffer++;
  }
}

uint8_t spi_transfer(uint8_t in) {
  // set the Data Register
  SPDR = in;

  // Wait for the Interupt Flag in the Status Register
  while (!(SPSR & (1 << SPIF)));

  // Return the Data Register 
  return (SPDR);
}

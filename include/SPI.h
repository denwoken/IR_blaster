
#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <Arduino.h>
//#include "Adafruit_ST77xx.h"

extern "C"
{
#include <ets_sys.h>
#include "user_interface.h"
}

#include "HardwareSerial.h"

#define SPI_TRANS_DONE_EN (BIT(9))
#define SPI_SLV_WR_STA_DONE_EN (BIT(8))
#define SPI_SLV_RD_STA_DONE_EN (BIT(7))
#define SPI_SLV_WR_BUF_DONE_EN (BIT(6))
#define SPI_SLV_RD_BUF_DONE_EN (BIT(5))
#define SPI_TRANS_DONE (BIT(4))
#define SPI_SLV_WR_STA_DONE (BIT(3))
#define SPI_SLV_RD_STA_DONE (BIT(2))
#define SPI_SLV_WR_BUF_DONE (BIT(1))
#define SPI_SLV_RD_BUF_DONE (BIT(0))

#define PERIPHS_IO_MUX 0x60000800

#define HSPI_ 1
#define SPI_ 0

#define REG_SPI_BASE(i) (0x60000200 - i * 0x100)
#define SPI_SLAVE(i) (REG_SPI_BASE(i) + 0x30)
#define SPI_CLOCK(i) (REG_SPI_BASE(i) + 0x18)
// Registers Operation {{
#define ETS_UNCACHED_ADDR(addr) (addr)
#define ETS_CACHED_ADDR(addr) (addr)

#define READ_REG(addr) (*((uint32_t *)ETS_UNCACHED_ADDR(addr)))
#define WRITE_REG(addr, val) (*((uint32_t *)ETS_UNCACHED_ADDR(addr))) = (uint32_t)(val)
#define CLEAR_REG_MASK(reg, mask) WRITE_PERI_REG((reg), (READ_PERI_REG(reg) & (~(mask))))
#define SET_REG_MASK(reg, mask) WRITE_PERI_REG((reg), (READ_PERI_REG(reg) | (mask)))
#define GET_REG_BITS(reg, hipos, lowpos) ((READ_PERI_REG(reg) >> (lowpos)) & ((1 << ((hipos) - (lowpos) + 1)) - 1))
#define SET_REG_BITS(reg, bit_map, value, shift) (WRITE_PERI_REG((reg), (READ_PERI_REG(reg) & (~((bit_map) << (shift)))) | ((value) << (shift))))

#define ADDR_8BIT_SPI1W0 (uint8_t *)&SPI1W0
#define ADDR_16BIT_SPI1W0 (volatile uint16_t *)&SPI1W0
#define ADDR_32BIT_SPI1W0 (uint32_t *)&SPI1W0

void begin_spi_intr(void (*func)(void));
void begin_spi(uint32_t freq);

void setBitOrder(uint8_t bitOrder);
void setByteOrder(uint8_t ByteOrder);

void setFrequency(uint32_t freq);

void setDataBits(uint16_t bits);
void write(uint8_t data);
void write16(uint16_t data);
void write32(uint32_t data);

void write2Bytes(uint16_t *dataPtr, uint16_t size);
void write2Bytes_(uint16_t *dataPtr, uint16_t size);
void write2Bytes(uint16_t data, uint16_t size);
void write2Bytes_(uint16_t data, uint16_t size);

#endif

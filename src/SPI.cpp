

#include "SPI.h"
//#include "HardwareSerial.h"
//#include "Adafruit_ST77xx.h"


static void setClockDivider(uint32_t clockDiv);





typedef union {
        uint32_t regValue;
        struct {
                unsigned regL :6;
                unsigned regH :6;
                unsigned regN :6;
                unsigned regPre :13;
                unsigned regEQU :1;
        };
} spiClk_t;

void begin_spi_intr(void(*func) (void)) {
    ETS_SPI_INTR_DISABLE();

    CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI_),
        SPI_TRANS_DONE_EN |
        SPI_SLV_WR_STA_DONE_EN |
        SPI_SLV_RD_STA_DONE_EN |
        SPI_SLV_WR_BUF_DONE_EN |
        SPI_SLV_RD_BUF_DONE_EN);
    //WRITE_PERI_REG(SPI_SLAVE(HSPI_), 0);
    SET_PERI_REG_MASK(SPI_SLAVE(HSPI_), SPI_TRANS_DONE_EN);

    CLEAR_PERI_REG_MASK(SPI_SLAVE(SPI_), 0x3ff);

    ETS_SPI_INTR_ATTACH(func, NULL);//Serial.println("attach interrupt sucess");
   // ETS_SPI_INTR_ENABLE();
}
void begin_spi(uint32_t freq) {

    pinMode(SCK, SPECIAL);  ///< GPIO14
    pinMode(MOSI, SPECIAL); ///< GPIO13

    SPI1C = 0;

    SPI1U = SPIUMOSI;// | SPIUDUPLEX | SPIUSSE;
    SPI1U1 = (7 << SPILMOSI);// | (7 << SPILMISO);
    SPI1C1 = 0;

    setFrequency(freq);
    setByteOrder(1);
    setBitOrder(0);
}




void setBitOrder(uint8_t bitOrder) {
    if(bitOrder == 0) {
        SPI1C &= ~(SPICWBO | SPICRBO);
    } else {
        SPI1C |= (SPICWBO | SPICRBO);
    }
}
void setByteOrder(uint8_t ByteOrder) {///////////////////////////////////////////////////////////////////////////////////////
    if (ByteOrder == 0) {
        SPI1U &= ~(SPIUWRBYO | SPIURDBYO);
    }
    else {
        SPI1U |= (SPIUWRBYO | SPIURDBYO);
    }

}

void setClockDivider(uint32_t clockDiv) {
    if(clockDiv == 0x80000000) {
        GPMUX |= (1 << 9); // Set bit 9 if sysclock required
    } else {
        GPMUX &= ~(1 << 9);
    }
    SPI1CLK = clockDiv;
}
static uint32_t ClkRegToFreq(spiClk_t * reg) {
    return (ESP8266_CLOCK / ((reg->regPre + 1) * (reg->regN + 1)));
}
void setFrequency(uint32_t freq) {
    while (SPI1CMD & SPIBUSY) {}

        static uint32_t lastSetFrequency = 0;
        static uint32_t lastSetRegister = 0;

        if (lastSetFrequency == freq && lastSetRegister == SPI1CLK) {
            // do nothing (speed optimization)
            return;
        }

        const spiClk_t minFreqReg = { 0x7FFFF020 };
        uint32_t minFreq = ClkRegToFreq((spiClk_t*)&minFreqReg);
        if (freq < minFreq) {
            // use minimum possible clock regardless
            setClockDivider(minFreqReg.regValue);
            lastSetRegister = SPI1CLK;
            lastSetFrequency = freq;
            return;
        }

        uint8_t calN = 1;

        spiClk_t bestReg = { 0 };
        int32_t bestFreq = 0;

        // aka 0x3F, aka 63, max for regN:6
        const uint8_t regNMax = (1 << 6) - 1;

        // aka 0x1fff, aka 8191, max for regPre:13
        const int32_t regPreMax = (1 << 13) - 1;

        // find the best match for the next 63 iterations
        while (calN <= regNMax) {

            spiClk_t reg = { 0 };
            int32_t calFreq;
            int32_t calPre;
            int8_t calPreVari = -2;

            reg.regN = calN;

            while (calPreVari++ <= 1) { // test different variants for Pre (we calculate in int so we miss the decimals, testing is the easyest and fastest way)
                calPre = (((ESP8266_CLOCK / (reg.regN + 1)) / freq) - 1) + calPreVari;
                if (calPre > regPreMax) {
                    reg.regPre = regPreMax;
                }
                else if (calPre <= 0) {
                    reg.regPre = 0;
                }
                else {
                    reg.regPre = calPre;
                }

                reg.regL = ((reg.regN + 1) / 2);
                // reg.regH = (reg.regN - reg.regL);

                // sn
                calFreq = ClkRegToFreq(&reg);
                //os_printf("-----[0x%08X][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d = %d\n", reg.regValue, freq, reg.regEQU, reg.regPre, reg.regN, reg.regH, reg.regL, calFreq);

                if (calFreq == static_cast<int32_t>(freq)) {
                    // accurate match use it!
                    memcpy(&bestReg, &reg, sizeof(bestReg));
                    break;
                }
                else if (calFreq < static_cast<int32_t>(freq)) {
                    // never go over the requested frequency
                    auto cal = std::abs(static_cast<int32_t>(freq) - calFreq);
                    auto best = std::abs(static_cast<int32_t>(freq) - bestFreq);
                    if (cal < best) {
                        bestFreq = calFreq;
                        memcpy(&bestReg, &reg, sizeof(bestReg));
                    }
                }
            }
            if (calFreq == static_cast<int32_t>(freq)) {
                // accurate match use it!
                break;
            }
            calN++;
        }

        // os_printf("[0x%08X][%d]\t EQU: %d\t Pre: %d\t N: %d\t H: %d\t L: %d\t - Real Frequency: %d\n", bestReg.regValue, freq, bestReg.regEQU, bestReg.regPre, bestReg.regN, bestReg.regH, bestReg.regL, ClkRegToFreq(&bestReg));

        setClockDivider(bestReg.regValue);
        lastSetRegister = SPI1CLK;
        lastSetFrequency = freq;




        if (freq == 80000000) {//  doesnt work
            WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105 | (1 << 9)); //Set bit 9 if 80MHz sysclock required
            WRITE_PERI_REG(SPI_CLOCK(HSPI_), 1<<31);
            // SET_PERI_REG_MASK(SPI_CLOCK(SPI_), (1 << 31));//set spi sysclock to 80MHz instead 40MHz

           
           

        }
}






 void setDataBits(uint16_t bits) {
    *(uint32_t*)&SPI1U1 = 0;//*(uint32_t*)&
    *(uint32_t*)&SPI1U1 = --bits << SPILMOSI;//
}

void write(uint8_t data) {//SPI_INTR_DISABLE();
    while (SPI1CMD & SPIBUSY);
    setDataBits(8);
    *ADDR_8BIT_SPI1W0 = data;
    SPI1CMD |= SPIBUSY;
    //while (SPI1CMD & SPIBUSY); // CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI_), SPI_TRANS_DONE); //SPI_INTR_ENABLE();
    //uint8_t *cbuf = reinterpret_cast<uint8_t*>(buf);
    
}


void write16(uint16_t data) {//SPI_INTR_DISABLE();  
    //while (SPI1CMD & SPIBUSY);
    setDataBits(16);
    *ADDR_16BIT_SPI1W0 = data ;
    SPI1CMD |= SPIBUSY;
    //while (SPI1CMD & SPIBUSY); //CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI_), SPI_TRANS_DONE); SPI_INTR_ENABLE();
}
void write32(uint32_t data) {//SPI_INTR_DISABLE();
    //while (SPI1CMD & SPIBUSY);
    setDataBits(32);
    SPI1W0 = data;
    SPI1CMD |= SPIBUSY;
    //while (SPI1CMD & SPIBUSY);//CLEAR_PERI_REG_MASK(SPI_SLAVE(HSPI_), SPI_TRANS_DONE); SPI_INTR_ENABLE();
}
//setDataBits(32);//100ns
//SPI1W0 =0;//100ns
//--------------------------------------------------------------------------------------------------------
void write2Bytes( uint16_t* dataPtr, uint16_t size) {

    while (size) {
        if (size > 32) {
            write2Bytes_(dataPtr, 32);
            size -= 32;
            dataPtr += 32;
        }
        else {
            write2Bytes_(dataPtr, size);
            size = 0;
        }
    }
}
void write2Bytes_( uint16_t * dataPtr, uint16_t size) {
    //SPI_INTR_ENABLE(); //SPI_INTR_DISABLE();
    while (SPI1CMD & SPIBUSY);
    setDataBits(size*16);
    for (uint16_t i = 0; i < size; i++) {
        *(ADDR_16BIT_SPI1W0 + i) = *(dataPtr + i);
    }
    //__sync_synchronize();
    SPI1CMD |= SPIBUSY;

    //while (SPI1CMD & SPIBUSY);//SPI_INTR_ENABLE();
}
//--------------------------------------------------------------------------------------------------------
void write2Bytes(uint16_t data, uint16_t size) {
    while (size) {
        if (size > 32 ){
            write2Bytes_(data, 32);
            size -= 32;
        }
        else {
            write2Bytes_(data, size);
            size = 0;
        }
    }
}
void write2Bytes_(uint16_t color, uint16_t size) {
    //SPI_INTR_ENABLE(); //SPI_INTR_DISABLE();

    while (SPI1CMD & SPIBUSY);
    setDataBits(size * 16);
    
    for (uint16_t i = 0; i < size; i++) {//////////////////////////////// 3uS
        *(ADDR_16BIT_SPI1W0 + i) = color;
    }
    //__sync_synchronize();
    SPI1CMD |= SPIBUSY;
    //while (SPI1CMD & SPIBUSY);//SPI_INTR_ENABLE();
    
}
//--------------------------------------------------------------------------------------------------------
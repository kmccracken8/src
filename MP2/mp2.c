#include "elecanisms.h"
#include <stdio.h>

#define ENC_MISO            D1
#define ENC_MOSI            D0
#define ENC_SCK             D2
#define ENC_CSn             D3

#define ENC_MISO_DIR        D1_DIR
#define ENC_MOSI_DIR        D0_DIR
#define ENC_SCK_DIR         D2_DIR
#define ENC_CSn_DIR         D3_DIR

#define ENC_MISO_RP         D1_RP
#define ENC_MOSI_RP         D0_RP
#define ENC_SCK_RP          D2_RP

uint16_t even_parity(uint16_t v) {
    v ^= v >> 8;
    v ^= v >> 4;
    v ^= v >> 2;
    v ^= v >> 1;
    return v & 1;
}

WORD enc_readReg() {
    WORD cmd, result, address;
    uint16_t temp;

    address = (WORD)0x3FFF;

    cmd.w = 0x4000 | address.w;         // set 2nd MSB to 1 for a read
    cmd.w |= even_parity(cmd.w) << 15;

    ENC_CSn = 0;

    SPI2BUF = (uint16_t)cmd.b[1];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    SPI2BUF = (uint16_t)cmd.b[0];
    while (SPI2STATbits.SPIRBF == 0) {}
    temp = SPI2BUF;

    ENC_CSn = 1;

    __asm__("nop");     // p.12 of the AS5048 datasheet specifies a minimum
    __asm__("nop");     //   high time of CSn between transmission of 350ns
    __asm__("nop");     //   which is 5.6 Tcy, so do nothing for 6 Tcy.
    __asm__("nop");
    __asm__("nop");
    __asm__("nop");

    ENC_CSn = 0;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[1] = (uint8_t)SPI2BUF;

    SPI2BUF = 0;
    while (SPI2STATbits.SPIRBF == 0) {}
    result.b[0] = (uint8_t)SPI2BUF;

    ENC_CSn = 1;

    return result;
}

int16_t main(void) {
    uint8_t *RPOR, *RPINR;
    uint16_t val, Enew, Eold, Edelta, pos, vel, dir;
    double duty;
    WORD data;

    init_elecanisms();

    // Configure encoder pins and connect them to SPI2
    ENC_CSn_DIR = OUT; ENC_CSn = 1;
    ENC_SCK_DIR = OUT; ENC_SCK = 0;
    ENC_MOSI_DIR = OUT; ENC_MOSI = 0;
    ENC_MISO_DIR = IN;

    D5_DIR = OUT;
    D5 = 0;
    D7_DIR = OUT;
    D7 = 0;

    RPOR = (uint8_t *)&RPOR0;
    RPINR = (uint8_t *)&RPINR0;

    __builtin_write_OSCCONL(OSCCON & 0xBF);
    RPINR[MISO2_RP] = ENC_MISO_RP;
    RPOR[ENC_MOSI_RP] = MOSI2_RP;
    RPOR[ENC_SCK_RP] = SCK2OUT_RP;
    RPOR[D5_RP] = OC1_RP;
    __builtin_write_OSCCONL(OSCCON | 0x40);

    SPI2CON1 = 0x003B;              // SPI2 mode = 1, SCK freq = 8 MHz
    SPI2CON2 = 0;
    SPI2STAT = 0x8000;

    OC1CON1 = 0x1C06;
    OC1CON2 = 0x001F;
    OC1RS = (uint16_t)(FCY / 1e3 - 1.);
    OC1R = 0.25*OC1RS;
    OC1TMR = 0;

    Eold = enc_readReg();
    Enew = enc_readReg();
    pos = enc_readReg();
    Edelta = 0;
    vel = 0;
    dir = 0;

    while (1) {
      Enew = enc_readReg();
      Edelta = Enew - Eold;
      if (abs(Edelta) < 10000) {
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }else if (Edelta > 0) {
        Edelta = Enew - 16384 - Eold;
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }else{
        Edelta = Enew - Eold + 16384;
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }

      if (Enew > 2000 && Enew < 9000){
        pos = Enew - 5400;
      }


      //val = (data.b[0] + 256*data.b[1]) & 0x3FFF;
      // OC1R = 0.8*OC1RS;
      // OC1R = (double)(val/(16384))*OC1RS;
      //duty = val;
      //OC1R = (duty/16384)*OC1RS;
    }
}

/*
** Copyright (c) 2018, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**     2. Redistributions in binary form must reproduce the above copyright
**        notice, this list of conditions and the following disclaimer in the
**        documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
*/

#include "elecanisms.h"
#include "usb.h"
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

#define SPRING              0
#define DAMPER              1
#define TEXTURE             2
#define WALL                3

uint8_t mode;

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

void vendor_requests(void){
    WORD temp;
    uint16_t i;

    switch (USB_setup.bRequest) {
        case SPRING:
            mode = 0;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case DAMPER:
            mode = 1;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case TEXTURE:
            mode = 2;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        case WALL:
            mode = 3;
            BD[EP0IN].bytecount = 0;
            BD[EP0IN].status = UOWN | DTS | DTSEN;
            break;
        default:
            USB_error_flags |= REQUEST_ERROR;
    }
}

int16_t main(void) {
    uint8_t *RPOR, *RPINR;
    int16_t Eval, Enew, Eold, Edelta, pos, vel, dir;
    double duty;
    WORD data;

    mode = 0;

    init_elecanisms();

    // Configure encoder pins and connect them to SPI2
    ENC_CSn_DIR = OUT; ENC_CSn = 1;
    ENC_SCK_DIR = OUT; ENC_SCK = 0;
    ENC_MOSI_DIR = OUT; ENC_MOSI = 0;
    ENC_MISO_DIR = IN;

    D5_DIR = OUT; //PWM output pin setup
    D5 = 0;
    D6_DIR = OUT; //Direction output pin setup
    D6 = 0;
    D7_DIR = OUT; //Enable output pin setup
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

    USB_setup_vendor_callback = vendor_requests;
    init_usb();

    //Reading encoder value
    data = enc_readReg();
    Eval = (data.b[0] + 256*data.b[1]) & 0x3FFF;
    //Variable initialization
    Eold = Eval;
    Enew = Eval;
    pos = Eval;
    Edelta = 0;
    vel = 0;


    while (USB_USWSTAT != CONFIG_STATE) {
#ifndef USB_INTERRUPT
        usb_service();
#endif
    }
    while (1) {
      //Reading encoder value
      data = enc_readReg();
      Eval = (data.b[0] + 256*data.b[1]) & 0x3FFF;
      //Defining current reading and change since last reading
      Enew = Eval;
      Edelta = Enew - Eold;

      if (abs(Edelta) < 10000) {
        //Position and velocity if the encoder hasn't wrapped around
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }else if (Edelta > 0) {
        //Position and velocity if the encoder has wrapped around one way
        Edelta = Enew - 16384 - Eold;
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }else{
        //Position and velocity if the envoder has wrapped around the other way
        Edelta = Enew - Eold + 16384;
        vel = (Edelta + vel)/2;
        pos = pos + Edelta;
        Eold = Enew;
      }

      //Recalibrate position if encoder is within the unique value section
      if (Enew > 2000 && Enew < 9000){
        pos = Enew - 5400;
      }

#ifndef USB_INTERRUPT
        usb_service();
#endif

      switch(mode){
        case 0:
            //Virtual Spring Mode
            //Set direction against position
            if (pos < 0){
              D6 = 0;
            }else{
              D6 = 1;
            }
            //Motor output is proportional to distance from zero
            duty = 1.3*abs(pos);
            OC1R = ((duty/16384))*OC1RS;
          break;
        case 1:
            //Virtual Damper Mode
            //Set direction against velocity
            if (vel < 0){
              D6 = 0;
            }else{
              D6 = 1;
            }
            //Motor output is proportional to velocity
            duty = abs(vel);
            duty = duty/9;
            if (duty > 1){
              //Prevents duty cycle > 1
              duty = 1;
            }else if (duty < 0.15){
              //Removes low duty cycle outputs
              duty = 0;
            }
            OC1R = duty*OC1RS;
          break;
        case 2:
            //Virtual Texture Mode
            //Set direction against velocity
            if (vel < 0){
              D6 = 0;
            }else{
              D6 = 1;
            }
            //Duty is set high/low every 500 encoder units
            //Dividing value controls texture density
            //Higher == more dense
            duty = abs(pos) % 1000;
            if (duty < 500){
              duty = 1;
            }else{
              duty = 0;
            }
            if (abs(vel) < 2){
              //Only output if wheel is moving
              duty = 0;
            }
            OC1R = duty*OC1RS;
          break;
        case 3:
            //Virtual Wall Mode
            //Set direction against position
            if (pos < 0){
              D6 = 0;
            }else{
              D6 = 1;
            }
            //Sets two wall positions on either side of zero
            if (abs(pos) > 4000) {
              duty = 1;
            }else{
              duty = 0;
            }
            OC1R = duty*OC1RS;
          break;
      }
    }
}

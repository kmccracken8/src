#include "elecanisms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

int16_t main(void) {
    uint8_t mode, mode_sel, SW1_pressed, SW2_pressed, SW3_pressed, round_num;
    uint8_t setup, i, counter, lit;
    uint16_t seed, game[255];
    init_elecanisms();

    T1CON = 0x0020;         // Sets Prescaling to 1:64
    PR1 = 0x0C35;           // Sets period register to 3125
                            //250ns*64*3125=50ms

    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    T1CONbits.TON = 1;          // Starts timer


    T2CON = 0x8000;         // Sets Prescaling to 1:1 and starts timer
    PR2 = 0x0001;           // Sets period register to 1
                            //250ns*1*1 = 250ns
    TMR2 = 0;
    IFS0bits.T2IF = 0;




    mode = 0;
    mode_sel = 0;
    SW1_pressed = 0;
    SW2_pressed = 0;
    SW3_pressed = 0;
    round_num = 0;
    setup = 0;
    counter = 1;
    lit = 0;
    seed = 0;

    while (1) {

      switch (mode) {
        case 0:                                        //Startup Mode
          if(IFS0bits.T2IF == 1){
            IFS0bits.T2IF = 0;
            if(seed>32000){
              seed = 0;
            }
            seed = seed + 3;
          }
          if(IFS0bits.T1IF == 1){                      //timer flag
            IFS0bits.T1IF = 0;

            if(SW1 == 0 && SW1_pressed == 0){          //detect sw1
              SW1_pressed = 1;
              resetLED();
              if (mode_sel < 3) {                      //advance mode selected
                mode_sel = mode_sel + 1;
              }else{
                mode_sel = 1;
              }
            }else if(SW1 == 1 && SW1_pressed == 1){
              SW1_pressed = 0;
            }

            if(SW2 == 0 && SW2_pressed == 0){          //detect sw2
              SW2_pressed = 1;
              resetLED();
              mode = mode_sel;
              srand(seed);
              break;
            }else if(SW2 == 1 && SW2_pressed == 1){
              SW2_pressed = 0;
            }

          }

          switch (mode_sel){                           //set led mode indicator
            case 1:
                LED1 = 1;
                break;
            case 2:
                LED2 = 1;
                break;
            case 3:
                LED3 = 1;
                break;
          }
          break;

        case 1:             //Game Mode 1
          if(setup == 0){                                //setup sequence
            for (i = 0; i < 255; i++) {
              game[i] = rand() % 3;
            }
            setup = 1;
          }
          if(round_num == 0){
            lightLED((int8_t)game[round_num]);
          }
          if(IFS0bits.T1IF == 1){                      //timer flag
            IFS0bits.T1IF = 0;
            counter = counter + 1;

            if(SW3 == 0 && SW3_pressed == 0){          //detect sw3
              SW3_pressed = 1;
              round_num = round_num + 1;
              resetLED();
              lightLED((int8_t)game[round_num]);
            }else if(SW3 == 1 && SW3_pressed == 1){
              SW3_pressed = 0;
            }

            if(counter == 80){
              counter = 1;

              //round_num = round_num + 1;
            }
          }

          break;

        case 2:             //Game Mode 2

          break;

        case 3:             //Game Mode 3

          break;

        case 4:             //End Mode

          break;
      }
    }
}

int resetLED(){
    LED1 = 0;
    LED2 = 0;
    LED3 = 0;
}

int lightLED(int id){
    switch (id) {
      case 0:
        LED1 = 1;
        break;
      case 1:
        LED2 = 1;
        break;
      case 2:
        LED3 = 1;
        break;
    }
}

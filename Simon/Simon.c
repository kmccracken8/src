#include "elecanisms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define MODE_SW               D8
#define GO_SW                 D9

#define LED_1                 D4
#define LED_2                 D5
#define LED_3                 D10
#define LED_4                 D7


#define SW_1                  D0
#define SW_2                  D1
#define SW_3                  D2
#define SW_4                  D3


typedef void(*STATE_HANDLER_T)(void);

void modeselect(void);
void gameplay(void);
void scoredisplay(void);

STATE_HANDLER_T state, last_state;
uint16_t mode, mode_sel, GO_SW_pressed, MODE_SW_pressed, SW_1_pressed, SW_2_pressed, SW_3_pressed, SW_4_pressed, round_num;
uint16_t i, j, k, counter, tcount, game_state, lit, unlit, lose, SWval;
uint16_t seed, game[255], input[255];



int16_t main(void){

    init_elecanisms();

    T1CON = 0x0020;         // Sets Prescaling to 1:64 and starts timer
    PR1 = 0x30D4;           // Sets period register
                                //62.5ns*64*12500=50ms
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    T1CONbits.TON = 1;      // Starts timer

    SW_1_pressed = 0;
    SW_2_pressed = 0;
    SW_3_pressed = 0;
    SW_4_pressed = 0;
    MODE_SW_pressed = 0;
    GO_SW_pressed = 0;

    mode_sel = 0;
    tcount = 0;
    lose = 0;

    lit = 250;               //Defining LED on/off times in game
    unlit = 500;

    state = modeselect;
    last_state = (STATE_HANDLER_T)NULL;

    D0_DIR = IN;
    D1_DIR = IN;
    D2_DIR = IN;
    D3_DIR = IN;
    D4_DIR = OUT;
    D5_DIR = OUT;
    D10_DIR = OUT;
    D7_DIR = OUT;
    D8_DIR = IN;
    D9_DIR = IN;

    resetLED();

    while(1){
        state();
    }

}

void modeselect(void){
    if(state != last_state){
        last_state = state;
        T2CON = 0x8000;         // Sets Prescaling to 1:1 and starts timer
        PR2 = 0x0001;           // Sets period register to 1
                                    //62.5ns*1*1 = 62.5ns
        TMR2 = 0;               // set Timer1 count to 0
        IFS0bits.T2IF = 0;      // lower Timer1 interrupt flag
    }

    if(IFS0bits.T2IF == 1){
      IFS0bits.T2IF = 0;
      if(seed>32000){
        seed = 0;
      }
      seed = seed + 3;
    }

    if(IFS0bits.T1IF == 1){                      //timer flag
      IFS0bits.T1IF = 0;
      lightLED(mode_sel);
      if(MODE_SW == 0 && MODE_SW_pressed == 0){          //detect sw1
        MODE_SW_pressed = 1;
        resetLED();
        if (mode_sel < 3) {                      //advance mode selected
          mode_sel = mode_sel + 1;
        }else{
          mode_sel = 0;
        }
      }else if(MODE_SW == 1 && MODE_SW_pressed == 1){
        MODE_SW_pressed = 0;
      }

      if(GO_SW  == 0 && GO_SW_pressed == 0){          //detect sw2
        GO_SW_pressed = 1;
        mode = mode_sel;
        state = gameplay;
      }else if(GO_SW == 1 && GO_SW_pressed == 1){
        GO_SW_pressed = 0;
      }
    }

    if(state != last_state){
      resetLED();
      srand(seed);
    }
}

void gameplay(void){
    if(state != last_state){
        last_state = state;

        for (i = 0; i < 255; i++) {
          game[i] = rand() % 4;
        }
        game_state = 0;
        tcount = 0;
        round_num = 0;
    }

    switch (game_state) {
        case 0:                                         //First wait
          if(IFS0bits.T1IF == 1){
            IFS0bits.T1IF = 0;
            tcount = tcount + 50;
          }
          if(tcount >= 3000){
            game_state = 1;
            tcount = 0;
            round_num = 0;
            j = 0;
            resetLED();
          }
          break;

        case 1:                                         //Flash sequence

          if(IFS0bits.T1IF == 1){
            IFS0bits.T1IF = 0;
            if(tcount == 0){
              lightLED((int8_t)game[j]);
            }
            if(tcount == lit){
              resetLED();
            }
            tcount = tcount + 50;
            if(tcount == lit + unlit){
              tcount = 0;
              j = j + 1;
            }
          }
          if(j > round_num){
              game_state = 2;
              j = 0;
              k = 0;
              SW_1_pressed = 0;
              SW_2_pressed = 0;
              SW_3_pressed = 0;
              SW_4_pressed = 0;
              tcount = 0;
          }
          break;

        case 2:                                         //Take input
          if(IFS0bits.T1IF == 1){
            IFS0bits.T1IF = 0;

            if(SW_1 == 0 && SW_1_pressed == 0){
              SW_1_pressed = 1;
              if(game[k] != 0){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(0);
            }else if(SW_1 == 1 && SW_1_pressed == 1){
              SW_1_pressed = 0;
              resetLED();
            }

            if(SW_2 == 0 && SW_2_pressed == 0){
              SW_2_pressed = 1;
              if(game[k] != 1){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(1);
            }else if(SW_2 == 1 && SW_2_pressed == 1){
              SW_2_pressed = 0;
              resetLED();
            }

            if(SW_3 == 0 && SW_3_pressed == 0){
              SW_3_pressed = 1;
              if(game[k] != 2){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(2);
            }else if(SW_3 == 1 && SW_3_pressed == 1){
              SW_3_pressed = 0;
              resetLED();
            }

            if(SW_4 == 0 && SW_4_pressed == 0){
              SW_4_pressed = 1;
              if(game[k] != 3){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(3);
            }else if(SW_4 == 1 && SW_4_pressed == 1){
              SW_4_pressed = 0;
              resetLED();
            }

            if(tcount >=3000){                                //Lose Condition
              lose = 1;
            }

            tcount = tcount + 50;
          }

          if(k > round_num && tcount >= 1000){
            game_state = 1;
            j = 0;
            k = 0;
            tcount = 0;
            round_num = round_num + 1;
          }
          if(lose == 1){
            state = scoredisplay;
          }

          break;
    }


    if(state != last_state){

    }
}

void scoredisplay(void){
    if(state != last_state){
        last_state = state;
        lightLED(0);
        lightLED(1);
        lightLED(2);
        lightLED(3);
    }

    if(state != last_state){

    }
}

int resetLED(){
    LED_1 = 0;
    LED_2 = 0;
    LED_3 = 0;
    LED_4 = 0;
}

int lightLED(int id){
    switch (id) {
      case 0:
        LED_1 = 1;
        break;
      case 1:
        LED_2 = 1;
        break;
      case 2:
        LED_3 = 1;
        break;
      case 3:
        LED_4 = 1;
        break;
    }
}

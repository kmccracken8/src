#include "elecanisms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define COIN                  SW1
#define MODE_SW               SW1
#define GO_SW                 SW2

#define RED_LED               LED1
//#define YELLOW_LED            D1
#define BLUE_LED              LED3
#define WHITE_LED             LED2

#define RED_SW                SW3
#define BLUE_SW               SW1
//#define YELLOW_SW             D1
#define WHITE_SW              SW2


typedef void(*STATE_HANDLER_T)(void);

void modeselect(void);
void gameplay(void);
void scoredisplay(void);

STATE_HANDLER_T state, last_state;
uint16_t mode, mode_sel, GO_SW_pressed, COIN_on, MODE_SW_pressed, BLUE_SW_pressed, WHITE_SW_pressed, RED_SW_pressed, round_num;
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

    BLUE_SW_pressed = 0;
    WHITE_SW_pressed = 0;
    RED_SW_pressed = 0;
    MODE_SW_pressed = 0;
    GO_SW_pressed = 0;
    COIN_on = 0;

    mode_sel = 0;
    tcount = 0;
    lose = 0;

    lit = 250;               //Defining LED on/off times in game
    unlit = 500;

    state = modeselect;
    last_state = (STATE_HANDLER_T)NULL;

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
        if (mode_sel < 2) {                      //advance mode selected
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
          game[i] = rand() % 3;
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
              BLUE_SW_pressed = 0;
              WHITE_SW_pressed = 0;
              RED_SW_pressed = 0;
              tcount = 0;
          }
          break;

        case 2:                                         //Take input
          if(IFS0bits.T1IF == 1){
            IFS0bits.T1IF = 0;

            if(BLUE_SW == 0 && BLUE_SW_pressed == 0){
              BLUE_SW_pressed = 1;
              if(game[k] != 2){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(2);
            }else if(BLUE_SW == 1 && BLUE_SW_pressed == 1){
              BLUE_SW_pressed = 0;
              resetLED();
            }

            if(WHITE)SW == 0 && WHITE_SW_pressed == 0){
              WHITE_SW_pressed = 1;
              if(game[k] != 1){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(1);
            }else if(WHITE_SW == 1 && WHITE_SW_pressed == 1){
              WHITE_SW_pressed = 0;
              resetLED();
            }

            if(RED_SW == 0 && RED_SW_pressed == 0){
              RED_SW_pressed = 1;
              if(game[k] != 0){                               //Lose Condition
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(0);
            }else if(RED_SW == 1 && RED_SW_pressed == 1){
              RED_SW_pressed = 0;
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
    }

    if(state != last_state){

    }
}

int resetLED(){
    BLUE_LED = 0;
    RED_LED = 0;
    WHITE_LED = 0;
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

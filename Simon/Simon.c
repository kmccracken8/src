#include "elecanisms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef void(*STATE_HANDLER_T)(void);

void modeselect(void);
void gameplay(void);
void scoredisplay(void);

STATE_HANDLER_T state, last_state;
uint16_t mode, mode_sel, SW1_pressed, SW2_pressed, SW3_pressed, round_num;
uint16_t i, j, k, counter, tcount, game_state, lit, unlit, lose, SWval;
uint16_t seed, game[255], input[255];



int16_t main(void){

    init_elecanisms();

    T1CON = 0x0020;         // Sets Prescaling to 1:64 and starts timer
    PR1 = 0x2EF3;           // Sets period register
                                //65?ns*64*Pr=50ms???
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    T1CONbits.TON = 1;      // Starts timer

    SW1_pressed = 0;
    SW2_pressed = 0;
    SW3_pressed = 0;

    mode_sel = 0;
    tcount = 0;
    lose = 0;

    lit = 300;               //Defining LED on/off times in game
    unlit = 600;

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
                                    //250ns*1*1 = 250ns
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
      if(SW1 == 0 && SW1_pressed == 0){          //detect sw1
        SW1_pressed = 1;
        resetLED();
        if (mode_sel < 2) {                      //advance mode selected
          mode_sel = mode_sel + 1;
        }else{
          mode_sel = 0;
        }
      }else if(SW1 == 1 && SW1_pressed == 1){
        SW1_pressed = 0;
      }

      if(SW2 == 0 && SW2_pressed == 0){          //detect sw2
        SW2_pressed = 1;
        mode = mode_sel;
        state = gameplay;
      }else if(SW2 == 1 && SW2_pressed == 1){
        SW2_pressed = 0;
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
          lightLED(2);
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
              SW1_pressed = 0;
              SW2_pressed = 0;
              SW3_pressed = 0;
              tcount = 0;
          }
          break;

        case 2:                                         //Take input
          if(IFS0bits.T1IF == 1){
            IFS0bits.T1IF = 0;

            if(SW1 == 0 && SW1_pressed == 0){
              SW1_pressed = 1;
              if(game[k] != 2){
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(2);
            }else if(SW1 == 1 && SW1_pressed == 1){
              SW1_pressed = 0;
              resetLED();
            }

            if(SW2 == 0 && SW2_pressed == 0){
              SW2_pressed = 1;
              if(game[k] != 1){
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(1);
            }else if(SW2 == 1 && SW2_pressed == 1){
              SW2_pressed = 0;
              resetLED();
            }

            if(SW3 == 0 && SW3_pressed == 0){
              SW3_pressed = 1;
              if(game[k] != 0){
                lose = 1;
              }else{
                k = k + 1;
                tcount = 0;
              }
              lightLED(0);
            }else if(SW3 == 1 && SW3_pressed == 1){
              SW3_pressed = 0;
              resetLED();
            }

            if(tcount >=3000){
              lose = 1;
            }

            tcount = tcount + 50;
          }

          if(k > round_num){
            game_state = 1;
            j = 0;
            k = 0;
            round_num = round_num + 1;
          }
          if(lose = 1){
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

    //STATE TASKS

    if(state != last_state){

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

int checkSW(){
  int8_t output = 20;

  if(SW1 == 0 && SW1_pressed == 0){
    SW1_pressed = 1;
  }else if(SW1 == 1 && SW1_pressed == 1){
    SW1_pressed = 0;
    resetLED();
  }
  if(SW2 == 0 && SW2_pressed == 0){
    SW2_pressed = 1;
  }else if(SW2 == 1 && SW2_pressed == 1){
    SW2_pressed = 0;
    resetLED();
  }
  if(SW3 == 0 && SW3_pressed == 0){
    SW3_pressed = 1;
  }else if(SW3 == 1 && SW3_pressed == 1){
    SW3_pressed = 0;
    resetLED();
  }

  if(SW1_pressed + SW2_pressed + SW3_pressed == 0){
    output = 20;
  }else if(SW1_pressed + SW2_pressed + SW3_pressed != 1){
    output = 10;
  }else if(SW1_pressed){
    output = 2;
  }else if(SW2_pressed){
    output = 1;
  }else if(SW3_pressed){
    output = 0;
  }

  return output;
}

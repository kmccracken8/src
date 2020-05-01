#include "elecanisms.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GO_SW                 D9

#define LED_1                 D4
#define LED_2                 D5
#define LED_3                 D7
#define LED_4                 D8


#define SW_1                  D0
#define SW_2                  D1
#define SW_3                  D2
#define SW_4                  D3

#define A                     D10
#define B                     D11
#define C                     D12
#define D                     D13
#define E                     A1
#define F                     A3
#define G                     A5


typedef void(*STATE_HANDLER_T)(void);

void pregame(void);
void gameplay(void);
void scoredisplay(void);

STATE_HANDLER_T state, last_state;
uint16_t mode, mode_sel, GO_SW_pressed, SW_1_pressed, SW_2_pressed, SW_3_pressed, SW_4_pressed, round_num;
uint16_t i, j, k, counter, tcount, game_state, lit, unlit, lose, SWval, dig, dig1, dig2, dig3, tempo;
uint16_t seed, game[255], input[255];



int16_t main(void){

    init_elecanisms();

    T1CON = 0x0020;         // Sets Prescaling to 1:64
    PR1 = 0x30D4;           // Sets period register
                                //62.5ns*64*12500=50ms
    TMR1 = 0;               // set Timer1 count to 0
    IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
    T1CONbits.TON = 1;      // Starts timer

    SW_1_pressed = 0;
    SW_2_pressed = 0;
    SW_3_pressed = 0;
    SW_4_pressed = 0;
    GO_SW_pressed = 0;

    tcount = 0;
    lose = 0;

    lit = 250;               //Defining LED on/off times in game
    unlit = 500;

    state = pregame;
    last_state = (STATE_HANDLER_T)NULL;

    D0_DIR = IN;
    D1_DIR = IN;
    D2_DIR = IN;
    D3_DIR = IN;
    D4_DIR = OUT;
    D5_DIR = OUT;
    D7_DIR = OUT;
    D8_DIR = OUT;
    D9_DIR = IN;


    ANSB = 0;
    D10_DIR = OUT;
    D11_DIR = OUT;
    D12_DIR = OUT;
    D13_DIR = OUT;
    A1_DIR = OUT;
    A3_DIR = OUT;
    A5_DIR = OUT;

    displaychar(35);

    resetLED();

    while(1){
        state();
    }

}

void pregame(void){
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
      if(GO_SW  == 0 && GO_SW_pressed == 0){          //detect Go Switch
        GO_SW_pressed = 1;
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
            if(tcount >= 0 && tcount < 1000){
              displaychar(3);
            }else if(tcount >= 1000 && tcount < 2000){
              displaychar(2);
            }else if(tcount >= 2000 && tcount < 3000){
              displaychar(1);
            }
          }
          if(tcount >= 3000){
            game_state = 1;
            displaychar(35);
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
        tcount = 0;
    }

    if(IFS0bits.T1IF == 1){
      IFS0bits.T1IF = 0;
      tcount = tcount + 50;

      if(tcount % 350 <= 125){ //Flash main lights
        lightLED(0);
        lightLED(1);
        lightLED(2);
        lightLED(3);
      }else{
        resetLED();
      }

      if(round_num >= 100){
        dig = 3;
      }else if (round_num >= 10){
        dig = 2;
      }else{
        dig = 1;
      }

      tempo = 800;

      dig3 = (round_num - (round_num % 100))/100;
      dig2 = ((round_num - dig3*100) - ((round_num - dig3*100) % 10))/10;
      dig1 = round_num - dig3*100 - dig2*10;

      if(tcount == 1*tempo){
        displaychar(28);
      }else if(tcount == 2*tempo){
        displaychar(12);
      }else if(tcount == 3*tempo){
        displaychar(24);
      }else if(tcount == 4*tempo){
        displaychar(27);
      }else if(tcount == 5*tempo){
        displaychar(14);
      }else if(tcount == 6*tempo){
        displaychar(dig3);
      }else if(tcount == 7*tempo - 250){
        displaychar(35);
      }else if(tcount == 7*tempo){
        displaychar(dig2);
      }else if(tcount == 8*tempo - 250){
        displaychar(35);
      }else if(tcount == 8*tempo){
        displaychar(dig1);
      }

      if(GO_SW  == 0 && GO_SW_pressed == 0){          //detect Go Switch
        GO_SW_pressed = 1;
        state = pregame;
      }else if(GO_SW == 1 && GO_SW_pressed == 1){
        GO_SW_pressed = 0;
      }

      if(tcount >= 24*tempo){
        state = pregame;
      }
    }

    if(state != last_state){
        tcount = 0;
        displaychar(35);
        lose = 0;
        resetLED();
    }
}

int resetLED(){
    __asm__("nop");
    LED_1 = 0;
    __asm__("nop");
    LED_2 = 0;
    __asm__("nop");
    LED_3 = 0;
    __asm__("nop");
    LED_4 = 0;
    __asm__("nop");
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

int displaychar(int digit){
    switch (digit) {
      case 0:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 1:
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 2:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 3:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 4:
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 5:
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 6:
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 7:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 8:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 9:
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 10: //A
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 11:  //b
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 12:  //C
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 13:  //D
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 14:  //E
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 15:  //F
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 16:  //G
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 17:  //H
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 18:  //I
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 19:  //J
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 21:  //l
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 23:  //n
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 24:  //o
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 25:  //P
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 26:  //q
          A = 1;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 27:  //r
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 28:  //S
          A = 1;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 29:  //t
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 30:  //U
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 1;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
      case 34:  //y
          A = 0;
          __asm__("nop");
          B = 1;
          __asm__("nop");
          C = 1;
          __asm__("nop");
          D = 1;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 1;
          __asm__("nop");
          G = 1;
          __asm__("nop");
          break;
      case 35:  //NULL
          A = 0;
          __asm__("nop");
          B = 0;
          __asm__("nop");
          C = 0;
          __asm__("nop");
          D = 0;
          __asm__("nop");
          E = 0;
          __asm__("nop");
          F = 0;
          __asm__("nop");
          G = 0;
          __asm__("nop");
          break;
    }
}

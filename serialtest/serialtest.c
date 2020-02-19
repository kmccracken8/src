#include "elecanisms.h"
#include "ajuart.h"
#include <stdio.h>

int16_t main(void) {
    init_elecanisms();
    init_ajuart();

    T1CON = 0x0020;       // set Timer1 period to 0.5s
    PR1 = 0x7A11;

    TMR1 = 0;             // set Timer1 count to 0
    IFS0bits.T1IF = 0;    // lower Timer1 interrupt flag
    T1CONbits.TON = 1;    // turn on Timer1

    LED2 = ON;
    LED3 = ON;

    while(1){
      if(IFS0bits.T1IF == 1){
        IFS0bits.T1IF = 0;      // lower Timer1 interrupt flag
        LED2 = !LED2;
        if(SW2 == 0){
          printf("Hello");
          LED3 = !LED3;
        }
      }
    }
}

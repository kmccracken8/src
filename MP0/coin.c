#include "elecanisms.h"

int16_t main(void) {
    init_elecanisms();

    while(!PORTDbits.RD5){
        LED1 = 1;
    }
}

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include "nokia.h"
#include "fruity_crush.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"


int main(void){
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    Nokia5110_Init();
    Nokia5110_Clear();
    habilita_matriz_botoes2();
    jogo();
    return 0;
}

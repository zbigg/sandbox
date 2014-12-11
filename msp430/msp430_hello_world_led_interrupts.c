#include <msp430.h>


#define LED_0 BIT0 
#define LED_1 BIT6
#define LED_OUT P1OUT
#define LED_DIR P1DIR

unsigned int timerCount = 0;

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop watchdog timer
    LED_DIR |= (LED_0 + LED_1); // Set P1.0 and P1.6 to output direction
    LED_OUT &= ~(LED_0 + LED_1); // Set the LEDs off

    CCTL0 = CCIE;
    TACTL = TASSEL_2 + MC_2; // Set the timer A to SMCLCK, Continuous

    P1DIR |= 0x01;

    __enable_interrupt();

    __bis_SR_register(LPM0_bits + GIE); // LPM0 with interrupts enabled
}


// Timer A0 interrupt service routine
// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) Timer_A (void)
#else
#error Compiler not supported!
#endif

{                                          
    timerCount = (timerCount + 1) % 8;
    if(timerCount ==0) {
        P1OUT ^= (LED_0 + LED_1);
        //P1OUT ^= 0x01;
    }
}
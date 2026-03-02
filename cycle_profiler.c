/*
 * EZ Scope - Cycle Profiler
 * * This diagnostic script measures the exact number of CPU clock cycles required to 
 * capture one 10-bit ADC sample on the MSP430G2553. It uses the hardware Timer A0 
 * to profile both the hardware conversion time and the C-code software overhead.
 *
 * Measured Result: 49 Cycles @ 16 MHz (approx. 3.06 us per sample, or 326 kSPS).
 */

#include <msp430.h>

void Init_System(void);

// Global variables for debugging (View these in the CCS "Expressions" Window)
volatile unsigned int start_time;
volatile unsigned int end_time;
volatile unsigned int total_cycles_adc;
volatile int adc_result;

void main(void) {
    Init_System();
    
    // --- SETUP TIMER TO COUNT CYCLES ---
    // TASSEL_2 = Use SMCLK (16 MHz, synchronized with CPU)
    // MC_2     = Continuous Mode (Counts 0 to 65535 repeatedly)
    // TACLR    = Clear timer before starting
    TA0CTL = TASSEL_2 + MC_2 + TACLR;

    while (1) {
        // =======================================================
        // PROFILE: ADC SAMPLE + SOFTWARE OVERHEAD
        // =======================================================
        
        // 1. Capture Timer Value BEFORE starting
        start_time = TA0R; 

        // 2. Start ADC (Enable + Start Conversion)
        ADC10CTL0 |= ENC + ADC10SC;

        // 3. Wait for conversion to complete
        // (Hardware limits: 4 cycles for sampling + 13 cycles for conversion)
        while (!(ADC10CTL0 & ADC10IFG)); 
        
        // 4. Read and shift data (10-bit to 8-bit mapping)
        adc_result = ADC10MEM >> 2;
        
        // 5. Clear interrupt flag
        ADC10CTL0 &= ~ADC10IFG;          
        
        // 6. Capture Timer Value AFTER completion
        end_time = TA0R;
        
        // 7. Calculate Difference
        // RESULT: 'total_cycles_adc' will hold the exact count (Expected: 49)
        total_cycles_adc = end_time - start_time;
        
        // Stop ADC logic to reset for the next loop iteration
        ADC10CTL0 &= ~ENC; 
        
        // BREAKPOINT HERE: Inspect 'total_cycles_adc' in the debugger
        __no_operation(); 
    }
}

void Init_System(void) {
    // Stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;   
    
    // Calibrate DCO to 16 MHz
    if (CALBC1_16MHZ==0xFF) while(1);
    DCOCTL = 0;
    BCSCTL1 = CALBC1_16MHZ;
    DCOCTL = CALDCO_16MHZ;

    // ADC Setup
    // CONSEQ_0    = Single Channel, Single Conversion
    // ADC10SSEL_2 = Use SMCLK (16 MHz) so ADC runs at the same speed as CPU
    ADC10CTL1 = INCH_4 + CONSEQ_0 + ADC10SSEL_2; 
    ADC10AE0 |= BIT4; 
    
    // ADC10SHT_0  = 4 clock cycles for sample-and-hold time
    ADC10CTL0 = ADC10SHT_0 + MSC + ADC10ON; 
}
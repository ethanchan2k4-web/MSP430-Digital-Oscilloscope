#include "intrinsics.h"
#include "msp430g2553.h"
#include <msp430.h>

#define NPOINTS 400

void Init(void);
void Init_ADC(void);
void Init_UART(void);

// Global Variables
volatile unsigned char v[NPOINTS];
volatile char received_char = 0;
volatile int task_ready = 0;

void main(void) {
  Init();
  Init_UART();
  Init_ADC();

  // Enable USCI_A0 RX interrupt to wait for MATLAB command
  IE2 |= UCA0RXIE;

  // Activate Global Interrupts
  __bis_SR_register(GIE);

 

  while (1) {
    if (task_ready == 1) {

      UCA0TXBUF = 'U';             // Send 'U' out of P1.2
      // Start ADC conversion sequence
      // Enable ADC interrupts to handle data collection
      ADC10CTL0 |= ADC10IE;
      
      // Reset task flag to wait for next command
      task_ready = 0; 
      
    } else {
      // Enter Low Power Mode 0 with interrupts enabled while waiting
      __bis_SR_register(LPM0_bits + GIE);
    }
  }
}

void Init(void) {
  // Stop watchdog timer
  WDTCTL = WDTPW + WDTHOLD;

  // Set system clock to 16 MHz
  if (CALBC1_16MHZ==0xFF) while(1);
  DCOCTL = 0;
  BCSCTL1 = CALBC1_16MHZ;
  DCOCTL = CALDCO_16MHZ;

  // Set P1.0 to output (Green LED)
  P1DIR |= BIT0;
  P1OUT = 0x00;
}

void Init_ADC(void) {
  // ADC10CTL1: Input Channel 4 (P1.4), Mode 2 (Repeat single channel)
  ADC10CTL1 = INCH_4 + CONSEQ_2; 

  // Enable analog input on P1.4
  ADC10AE0 |= BIT4; 

  // ADC10CTL0: SHT_0 (4 ADC clocks), MSC (Multiple Sample Conversion), ADC On
  ADC10CTL0 = ADC10SHT_0 + MSC + ADC10ON;

  // Enable Conversion (ENC) and Start Conversion (ADC10SC)
  ADC10CTL0 |= ENC + ADC10SC;
}

void Init_UART(void) {
  // Configure P1.1 (RX) and P1.2 (TX) for UART
  P1SEL |= BIT1 + BIT2;
  P1SEL2 |= BIT1 + BIT2;

  // Baud Rate Calculation for 16MHz / 115200
  // 16000000 / 115200 = 138.88
  UCA0BR0 = 138;      // Low byte
  UCA0BR1 = 0;        // High byte
  UCA0MCTL = UCBRS_7; // Modulation UCBRSx = 7

  // Use SMCLK for UART
  UCA0CTL1 |= UCSSEL_2;

  // Release UART Reset
  UCA0CTL1 &= ~UCSWRST;
  
}

// ISR: Receive Character from MATLAB
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void) {
  received_char = UCA0RXBUF; // Read buffer
  task_ready = 1;            // Flag main loop to start
  __bic_SR_register_on_exit(LPM0_bits); // Wake up CPU
}

// ISR: ADC Conversion Complete
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
  P1OUT |= BIT0; // LED ON (Profiling start)
  
  unsigned int i;
  for (i = 0; i < NPOINTS; i++) {
    // Wait for conversion to complete
    while (!(ADC10CTL0 & ADC10IFG));
    
    // Shift 10-bit result (0-1023) to 8-bit (0-255)
    v[i] = (ADC10MEM >> 2); 
    
    // Clear flag
    ADC10CTL0 &= ~ADC10IFG;
  }
  
  // Disable ADC interrupt to stop sampling
  ADC10CTL0 &= ~ADC10IE; 
  
  // Enable UART TX interrupt to send data
  IE2 |= UCA0TXIE; 
}

// ISR: Transmit Data to MATLAB
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void) {
  P1OUT &= ~BIT0; // LED OFF (Profiling end)
  
  unsigned int i;
  for (i = 0; i < NPOINTS; i++) {
    // Wait for TX buffer to be ready
    while (!(IFG2 & UCA0TXIFG));
    
    // Send data
    UCA0TXBUF = v[i];
  }
  
  // Disable TX interrupt after sending all points
  IE2 &= ~UCA0TXIE; 
}
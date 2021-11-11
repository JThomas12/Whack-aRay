#include <time.h>
#include <stdlib.h>
#include <portDefinitions>

void SetupDigitalGPIO(unsigned int port, unsigned int pinMask, int direction)
{
    /*Function for setting up a digital GPIO Pin

     This function will do all the register configs for a give port and pin.

     Examples
     -------
     SetupOnDigitalGPIO(PortF, pin3, INPUT, INTERRUPT) // Set up an input with interrupts on PF3.
     SetupOnDigitalGPIO(PortE, pin4, OUTPUT, NoINTERRUPT) // Set up an output with no interrupts on PE4.
     Notes
     -----
     We assume a pullup resistor.
     We also assume an edge based trigger.
     It is best the header with the #defines for each port and pin are included.

     Inputs
     ------
     (*((volatile unsigned long *) port  - Base address of the port being used.
     (*((volatile unsigned long *) pin   - Bit mask of the pin being used
     int direction - Boolean flag for direction of the pin. 1 = Output. 0 = input
     int UseInterrupts - Boolean flag to use interrupts. 1 = use them. 0 = don't
     */

    //Define the generic pointers
    unsigned int volatile *pRCGCGPIO = (unsigned int *) RCGCGPIO;
    unsigned int volatile *pGPIOLOCK = (unsigned int *) (port + GPIOLOCK);
    unsigned int volatile *pGPIOCR = (unsigned int *) (port + GPIOCR);
    unsigned int volatile *pGPIODIR = (unsigned int *) (port + GPIODIR);
    unsigned int volatile *pGPIOAFSEL = (unsigned int *) (port + GPIOAFSEL);
    unsigned int volatile *pGPIOPUR = (unsigned int *) (port + GPIOPUR);
    unsigned int volatile *pGPIODEN = (unsigned int *) (port + GPIODEN);

    // activate the clocks for the port
    int clocks;
    switch ((int) port)
    {
    case PortA:
        clocks = ClocksA; break;
    case PortB:
        clocks = ClocksB; break;
    case PortC:
        clocks = ClocksC; break;
    case PortD:
        clocks = ClocksD; break;
    case PortE:
        clocks = ClocksE; break;
    case PortF:
        clocks = ClocksF; break;
    default:
        clocks = ClocksF; //ERROR. TODO: Add an exception to handle this. Send to Error ISR or something
    }

    *pRCGCGPIO |= clocks;
    while ((*pRCGCGPIO & clocks) == 0);
    if(port == PortF){
        *pGPIOLOCK = GPIOKEY;
        *pGPIOCR |= pinMask;
    }
    if (direction == 0)
    {
        *pGPIODIR &= ~pinMask;
        *pGPIOPUR |= pinMask;
    }
    else if (direction == 1)
    {
        *pGPIODIR |= pinMask;
    }

    *pGPIOAFSEL &= ~pinMask;
    *pGPIODEN |= pinMask;

}
// portPin is an array of unsigned integers
// portPin[0] is the port base address
// portPin[1] is the pin mask
void turnOn(unsigned int portPin[2])
{
    unsigned int volatile *pGPIODATA = (unsigned int *) (portPin[0] + GPIODATA);
    *pGPIODATA = *pGPIODATA |portPin[1];
}

void turnOff(unsigned int portPin[2])
{
    unsigned int volatile *pGPIODATA = (unsigned int *) (portPin[0] + GPIODATA);
    *pGPIODATA = *pGPIODATA & ~portPin[1];
}

void setupBoard(){
    SetupDigitalGPIO(PortA, pin2, OUTPUT);
        SetupDigitalGPIO(PortA, pin3, OUTPUT);
        SetupDigitalGPIO(PortA, pin4, OUTPUT);
        SetupDigitalGPIO(PortA, pin5, OUTPUT);
        SetupDigitalGPIO(PortA, pin6, OUTPUT);
        SetupDigitalGPIO(PortA, pin7, OUTPUT);
        SetupDigitalGPIO(PortB, pin0, OUTPUT);
}

void displayNum (int num) {
    //Define segments of 7 segment display

    unsigned int segA[2] = {PortA, pin6};
    unsigned int segB[2] = {PortA, pin5};
    unsigned int segC[2] = {PortA, pin4};
    unsigned int segD[2] = {PortA, pin3};
    unsigned int segE[2] = {PortA, pin2};
    unsigned int segF[2] = {PortA, pin7};
    unsigned int segG[2] = {PortB, pin0};

    turnOff(segA);
    turnOff(segB);
    turnOff(segC);
    turnOff(segD);
    turnOff(segE);
    turnOff(segF);
    turnOff(segG);

    switch(num)
        {
            case 8 :
                turnOn(segG);
            case 0 :
                turnOn(segA);
                turnOn(segB);
                turnOn(segC);
                turnOn(segD);
                turnOn(segE);
                turnOn(segF);
                break;
            case 2:
                turnOn(segA);
                turnOn(segB);
                turnOn(segD);
                turnOn(segE);
                turnOn(segG);
                break;
            case 3:
                turnOn(segD);
                turnOn(segG);
            case 7:
                turnOn(segA);
                turnOn(segB);
                turnOn(segC);
                break;
            case 6:
                turnOn(segE);
            case 5:
                turnOn(segA);
                turnOn(segC);
                turnOn(segD);
                turnOn(segF);
                turnOn(segG);
                break;
            case 9:
                turnOn(segA);
            case 4:
                turnOn(segF);
                turnOn(segG);
            case 1:
                turnOn(segB);
                turnOn(segC);
                break;

        }
        return;
}

void sleep(int delay){
    int i =0;
    for(i = 0; i < 2000000*delay; i++){}
    //This function temporarly halts the run of the program
    //time_t volatile current_time, start_time;
    //current_time = time(NULL);
    //start_time = current_time;
    /*while(current_time < start_time + seconds){
        current_time = time(NULL);
    }*/
}


int randInt(int low, int high){
    // returns a random integer from low up to but not including high
   return (int)rand()*(high-low)+low;
}

void turnOnNextLight( unsigned int volatile *lightPort, int delay){
    //This function turns on the next light and returns the the time the light will time out
    sleep(delay);
    *lightPort = *lightPort | 0x08;
}


int main(void) {

    /*
     * First, define the pointers to the key registers needed for configuring
     * the general purpose I/Os for Port F.  Note that the use of volatile ensures that
     * the compiler always carries out the memory accesses, rather than optimizing
     * them out (for example, if the access is in a loop).
     */

    // pRCGCGPIO is a pointer to the General-Purpose Input/Output Run Mode Clock
    // Gating Control Register (p 340, DS)
    unsigned int volatile *pRCGCGPIO = (unsigned int *) (0x400FE000 + 0x608);

    // pGPIOLOCK_PortE is a pointer to the GPIO Lock register for port F (p 684, DS)
    unsigned int volatile *pGPIOLOCK_PortE = (unsigned int *)(0x40024000 + 0x520);

    // pGPIOCR_PortE is a pointer to the GPIO Commit register for port F (p 685, DS)
    unsigned int volatile *pGPIOCR_PortE = (unsigned int *)(0x40024000 + 0x524);

    // pGPIODIR_PortE is a pointer to the GPIO Direction register for port F (p 663, DS)
    unsigned int volatile *pGPIODIR_PortE = (unsigned int *) (0x40024000 + 0x400);

    // pGPIOAFSEL is a pointer to the GPIO Alternate Function Select register for port F (p 672, DS)
    unsigned int volatile *pGPIOAFSEL_PortE = (unsigned int *) (0x40024000 + 0x420);

    // pGPIOPUR is a pointer to the GPIO Pull-Up Select register for port F (p 677, DS)
    unsigned int volatile *pGPIOPUR_PortE = (unsigned int *) (0x40024000 + 0x510);

    // pGPIODEN is a pointer to the GPIO Digital Enable register for port F (p 683, DS)
    unsigned int volatile *pGPIODEN_PortE = (unsigned int *) (0x40024000 + 0x51C);

    // pGPIODATA is a pointer to the GPIO Data register for port F (p 662, DS)
    unsigned int volatile *pGPIODATA_PortE = (unsigned int *) (0x40024000 + 0x3FC);

    unsigned int volatile *pGPIOPDR_PortE = (unsigned int *) (0x40024000 + 0x514);



    int switch_position, count;

    /*
     * Now step through the cook book of steps.
     */

    // Step 1a: Turn on the clocks for port F.
    //  Note:   port F clocks are controlled by bit 5 (R5) of *pRCGCGPIO.
    //          In _binary_ this would correspond to a mask of 0b10.0000.
    //          In _hex_ this would correspond to a mask of 0x20
    *pRCGCGPIO = *pRCGCGPIO | 0x0010;

    // Step 1b: Check to be sure the clocks have started.
    //          This can take a few clock cycles.
    //          Keep checking bit 5 until it is no longer 0.
    while ( (*pRCGCGPIO & 0x0010) == 0 ) ; // Good thing this is volatile!

    // Step 2a: Unlock Port F
    //          Port F has NMI connected to PF0.  The M4 thus has a lock on this.
    //          The key to the lock is to write the value 0x4C4F434B to GPIOLOCK.
    *pGPIOLOCK_PortE    = 0x4C4F434B;

    // Step 2b: Enable us to commit to all controls in Port F for PF[4:0]
    *pGPIOCR_PortE = *pGPIOCR_PortE | 0x1F;

    // Let's use PF4 to read SW1.  Therefore it should be
    // an input.


    // Step 3b: Set the pulldown resistor on the switch.  This will
    //          give a 1 (16) if not pressed and 0 if pressed.
    *pGPIOPDR_PortE = *pGPIOPDR_PortE | 0x0010;

    // Step 5:  Enable the pin
    *pGPIODEN_PortE = *pGPIODEN_PortE | ~0xFFEF;


    // Let's use PF3 to control the green LED.  Therefore it should be
    // an output.

    // Step 3:  Set the direction of the pin to be used.  The pins
    //          can be configured as either input or output.
    //          In _binary_ this would correspond to a mask of 0b1000
    //          In _hex_ this would correspond ot a mask of 0x08;
    *pGPIODIR_PortE = *pGPIODIR_PortE | 0x08;

    // Step 4:  Set the pin to be used as a general purpose I/O pin.
    //          This means we clear the bit corresponding to PF3.
    *pGPIOAFSEL_PortE = *pGPIOAFSEL_PortE & ~0x08;

    // Step 5:  Enable the pin
    *pGPIODEN_PortE = *pGPIODEN_PortE | 0x08;
    int i;
    *pGPIODATA_PortE = *pGPIODATA_PortE | 0x08;




    int points = 0, clicked = 0, timeOutSeconds = 2, sleepTime;

    time_t currentTime, offTime, onTime = 0;

    currentTime = time(NULL);
    // the time the light will be turned of
    offTime = currentTime + timeOutSeconds;

    setupBoard();
    displayNum(points);
    while (1)
    {
        currentTime = time(NULL);

        // turns of the light if it has been on longer than the timeout time
        if (currentTime >= offTime){
            *pGPIODATA_PortE = *pGPIODATA_PortE & ~0x08;

            // halts the program for 1-2 seconds then turns on the light again

            //onTime = randInt(1,3);

            turnOnNextLight(pGPIODATA_PortE,1);
            offTime = time(NULL) + timeOutSeconds;

            clicked = 0;

        }

        // (re)read the switch position...
        switch_position = *pGPIODATA_PortE & 0x10; // Good thing this is volatile!
        if (switch_position == 16){
            if (currentTime < offTime && clicked == 0){
                *pGPIODATA_PortE = *pGPIODATA_PortE & ~0x08;
                clicked = 1;
                points += 1;

                displayNum(points);

                turnOnNextLight(pGPIODATA_PortE,1);
                offTime = time(NULL) + timeOutSeconds;
                clicked = 0;

                //offTime = turnOnNextLight(pGPIODATA_PortE, timeOutSeconds);
                //clicked = 0;


                // halts the program for 1-2 seconds then turns on the light again





            }
            // Turn on LED Your code here
            // Delay for a bit.
            //for(i = 0; i < 500000; i++) {};
        }



    }

    return 0;
}

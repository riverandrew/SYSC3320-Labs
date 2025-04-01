// experiment1.c: Reading Swithces and Controlling LEDs
// Andrew Rivera
// student id: 101192264
// Axi Timer Experiment

#include "xparameters.h"
#include "xil_types.h"
#include "xtmrctr.h"
#include "xil_io.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xgpio.h"
#include <stdio.h>
#include <unistd.h>

/* LED GPIO Definitions */
#define LED_GPIO_DEVICE_ID XPAR_AXI_GPIO_0_DEVICE_ID
#define LED_CHANNEL 1

/* 1. Instance of the Interrupt Controller */
XScuGic InterruptController;

/* The configuration parameters of the controller */
static XScuGic_Config *GicConfig;

/* 2. Timer and GPIO Instances */
XTmrCtr TimerInstancePtr; // Timer
XGpio GpioLeds; // GPIO

int test = 0;
volatile int led_toggle_flag = 0;

/* 3. Interrupt Handler */
void Timer_InterruptHandler(void *CallBackRef)
{
    led_toggle_flag = 1;

    XTmrCtr_Reset((XTmrCtr *)CallBackRef, 0);

    xil_printf("Interrupt Triggered! Flag set for LED Toggle. \n");

}


/* Interrupt Setup */
int SetUpInterruptSystem(XScuGic *XScuGicInstancePtr)
{
    /* Connect the interrupt controller interrupt handler */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 XScuGicInstancePtr);

    /* Enable interrupts in the ARM */
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}

/* Initialize the Interrupt Controller */
int ScuGicInterrupt_Init(u16 DeviceId, XTmrCtr *TimerInstancePtr)
{
    int Status;

    // Interrupt controller initialization
    GicConfig = XScuGic_LookupConfig(DeviceId);
    if (NULL == GicConfig)
    {
        return XST_FAILURE;
    }

    Status = XScuGic_CfgInitialize(&InterruptController, GicConfig, GicConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    // Interrupt system setup
    Status = SetUpInterruptSystem(&InterruptController);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    // 4. Connect Timer interrupt to the interrupt controller
    Status = XScuGic_Connect(&InterruptController, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR,
    						 (Xil_ExceptionHandler)Timer_InterruptHandler,
							 (void *)TimerInstancePtr);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    // Enable Timer interrupt
    XScuGic_Enable(&InterruptController, XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR);

    return XST_SUCCESS;
}

u32 main_counter ;
int main()
{
	xil_printf("Andrew Rivera 101192264 \n");
    int xStatus;

    // 5. Initialize GPIO for LED control
    xStatus = XGpio_Initialize(&GpioLeds, LED_GPIO_DEVICE_ID);
    if (xStatus != XST_SUCCESS){
        xil_printf("LED GPIO intialization Failed. \n");
        return XST_FAILURE;
    }

    // 6. set GPIO direction
	XGpio_SetDataDirection(&GpioLeds, LED_CHANNEL, 0x00000000); //LED as output

    // 7. Timer counter initialization
    xStatus = XTmrCtr_Initialize(&TimerInstancePtr, XPAR_AXI_TIMER_0_DEVICE_ID);
    if (xStatus != XST_SUCCESS){
        xil_printf("Timer counter intialization Failed. \n");
        return XST_FAILURE;
    }

    // Set Timer Handler
    XTmrCtr_SetHandler(&TimerInstancePtr, (XTmrCtr_Handler)Timer_InterruptHandler, &TimerInstancePtr);

    // Initialize timer pointer with base address
    unsigned int *timer_ptr = (unsigned int *)XPAR_AXI_TIMER_0_BASEADDR;

    // 8. Load Timer Load Register (TLR)
    //3sec = (2^32 - TLR + 2) *(1/50MHz)
    *(timer_ptr + 1) = 4144967298; // 3 second delay


    // 9. Configure timer: Generate mode, count up, interrupt enabled
    //0x20C
    *(timer_ptr) = 0x0F4;

    // 10. Initialize and enable interrupt system
    xStatus = ScuGicInterrupt_Init(XPAR_SCUGIC_SINGLE_DEVICE_ID, &TimerInstancePtr);
    if (xStatus != XST_SUCCESS){
        xil_printf("Timer counter intialization Failed. \n");
        return XST_FAILURE;
    }

    // 11. Start Timer
    *(timer_ptr) = 0x0D4;


    // 12. Infinite loop to keep the program running
    int counter = 0;
    while (1)
    {
    	sleep(1);
    	counter++;
    	xil_printf("Counter: %d\n", counter);
        if (led_toggle_flag)
        {
            // Toggle LED here
            static int led_state = 0x00;
            led_state = !led_state;
            XGpio_DiscreteWrite(&GpioLeds, LED_CHANNEL, led_state);
            xil_printf("LED status: %s\n", led_state ? "ON" : "OFF");
            led_toggle_flag = 0;
        }

    }

    return 0;
}

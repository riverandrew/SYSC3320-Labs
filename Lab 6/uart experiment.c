// experiment2.c: UART communication protocol
// Andrew Rivera
// student id: 101192264
// Uart LED Experiment

#include "xparameters.h"
#include "xuartps.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "sleep.h"

#define UART_DEVICE_ID  XPAR_XUARTPS_0_DEVICE_ID  // Use UART0 (change to _1 if using UART1)
#define GPIO_DEVICE_ID  XPAR_AXI_GPIO_0_DEVICE_ID // Update with your AXI GPIO ID

// 1. create instances of uartps and gpio
XUartPs UartPs;
XGpio GpioLeds;

int main() {
    xil_printf("UART LED Control Started. Send '1', '0', 'A', or 'B' to control LEDs.\n");

    //  Initialize UART
    XUartPs_Config *Config = XUartPs_LookupConfig(UART_DEVICE_ID);
    XUartPs_CfgInitialize(&UartPs, Config, Config->BaseAddress);

    // 2. Initialize GPIO and specify direction
    XGpio_Initialize(&GpioLeds, GPIO_DEVICE_ID);
    XGpio_SetDataDirection(&GpioLeds, 1, 0); // Set LEDs as Output


    // 3. Clear UART Buffers Before Use
    XUartPs_SetOptions(&UartPs, XUARTPS_OPTION_RESET_TX);
    XUartPs_SetOptions(&UartPs, XUARTPS_OPTION_RESET_RX);


    u8 led_state = 0x0;  // Initial LED state (all off)

    while (1) {
        if (XUartPs_IsReceiveData(UartPs.Config.BaseAddress)) {
            u8 recv_byte = XUartPs_RecvByte(UartPs.Config.BaseAddress);
            xil_printf("Received: %c\n", recv_byte);

			 // 4. write a switch-case statement to check recv_byte
			 // and update led_state accordingly
            switch(recv_byte) {

                // If the character is “1”: turn ON all LEDs
                case 0x31:
                    led_state = 0x0F;
                    xil_printf("LED status: ON 0x0F. \n");
                    XGpio_DiscreteWrite(&GpioLeds, 1, led_state);
                    break;

                // If the character is “0”: turn OFF all LEDs
                case 0x30:
                    led_state = 0x00;
                    xil_printf("LED status: OFF 0x00. \n");
                    XGpio_DiscreteWrite(&GpioLeds, 1, led_state);
                    break;

                // If the character is “A”: turn ON LED 3 and LED 1.
                case 0x41:
                    led_state = 0x0A;
                    xil_printf("LED status: LED3 and LED1 ON. 0x0A\n");
                    XGpio_DiscreteWrite(&GpioLeds, 1, led_state);
                    break;

                // If the character is “B”: Blink all LEDs for 5 times with a 1-second delay interval
                case 0x42:
                    xil_printf("LED status: BLINKING. \n");
                    for (int i = 0; i < 5; i++){
                        XGpio_DiscreteWrite(&GpioLeds, 1, 0x0F); // ON
                        xil_printf("LED status: BLINKING 0x0F \n");
                        sleep(1);
                        XGpio_DiscreteWrite(&GpioLeds, 1, 0x00); // OFF
                        xil_printf("LED status: BLINKING  0x00 \n");
                        sleep(1);
                    }
                    break;

                default:
                    xil_printf("Andrew Rivera 101192264 \n");
                    break;
            }
            // 5. Update LED State
            led_state = 0x0; // Reset state back to off 0x00.

        }
    }
}

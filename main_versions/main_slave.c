/*
 * main.c
 *
 *  Created on: 09.09.2022
 *  Authors:    Przemysław Kurzak
 *      		Jakub Dzialowy
 */

#include "xil_cache.h"
#include "xparameters.h"
#include "PmodBLE.h"
#include "platform.h"
#include "xgpio.h"

//required definitions for sending & receiving data over the host board's UART port
#include "xuartps.h"
typedef XUartPs SysUart;
#define SysUart_Send            XUartPs_Send
#define SysUart_Recv            XUartPs_Recv
#define SYS_UART_DEVICE_ID      XPAR_PS7_UART_1_DEVICE_ID
#define BLE_UART_AXI_CLOCK_FREQ 100000000
#define BAUD_RATE				115200

//leds
#define LED_ID 					XPAR_AXI_GPIO_0_DEVICE_ID
#define LED_CHANNEL 			1

//objects
PmodBLE myDevice;
SysUart myUart;
XGpio led;

//functions
void Blink();
void BleInitialize();
void CommunicationRun();
void SysUartInit();

void Blink(int decValtoBinary)
{
	XGpio_DiscreteWrite(&led, LED_CHANNEL, (unsigned char) decValtoBinary);
}

void BleInitialize()
{
    SysUartInit();
    BLE_Begin (
        &myDevice,
        XPAR_PMODBLE_0_S_AXI_GPIO_BASEADDR,
        XPAR_PMODBLE_0_S_AXI_UART_BASEADDR,
        BLE_UART_AXI_CLOCK_FREQ,
		BAUD_RATE
    );
}

void CommunicationRun()
{
    u8 dataBuffer[1];
    int dataReceive;

    xil_printf("Starting...\r\n");
    xil_printf("Device: SLAVE\r\n");

    while(1) {
    	dataReceive = SysUart_Recv(&myUart, dataBuffer, 1);
        if (dataReceive != 0)
        {
            BLE_SendData(&myDevice, dataBuffer, 1);

        }

        dataReceive = BLE_RecvData(&myDevice, dataBuffer, 1);
        if (dataReceive != 0)
        {
        	if(dataReceive >= 1 && dataReceive <= 9)
        	{
        		//leds on - show binary value of argument
        		//Example:
        		//if "dataReceive" == "2", then 0010
        		//only LD1 on
        		Blink(dataReceive);
        	}
        }
    }
}

void SysUartInit()
{
    XUartPs_Config *myUartCfgPtr;
    myUartCfgPtr = XUartPs_LookupConfig(SYS_UART_DEVICE_ID);
    XUartPs_CfgInitialize(&myUart, myUartCfgPtr, myUartCfgPtr->BaseAddress);
}

int main()
{
	init_platform();
	int Status = XGpio_Initialize(&led, LED_ID);
	if(Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	BleInitialize();
	SysUartInit();
	CommunicationRun();
    return XST_SUCCESS;
}

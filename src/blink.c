

#include <stdio.h>
#include <stdlib.h>

// need this for false declaration
#include <stdbool.h>
#include "ftd2xx.h"

int main(int argc, char* argv[])
{
	FT_HANDLE ftHandle;            // Handle of the FTDI device
	FT_STATUS ftStatus;            // Result of each D2XX call
	DWORD dwNumDevs;               // The number of devices
	unsigned int uiDevIndex = 0xF; // The device in the list that we'll use
	BYTE byOutputBuffer[16];        // Buffer to hold MPSSE commands and data
	// to be sent to the FT2232H
	BYTE byInputBuffer[16];         // Buffer to hold data read from the FT2232H
	DWORD dwCount = 0;             // General loop index
	DWORD dwNumBytesToSend = 0;    // Index to the output buffer
	DWORD dwNumBytesSent = 0;      // Count of the actual bytes send - used with the FT_Write
	DWORD dwNumBytesToRead = 0;    // Number of bytes available to read in the driver's input buffer
	DWORD dwNumBytesRead = 0;      // Count of actual bytes read 0 used with FT_Read
	DWORD dwClockDivisor = 0x05DB; // Value of clock divisor, SCL Frequency =
	//     60/((1+0x05DB*2) (MHz) = 1Mhz
	int i;
	unsigned int c;
	int count=5, milisecs = 1000;//by default led will blink 5 times waiting 1 seconds between on/off

	if(argc == 3){ //get command line parameters to blink leds and time interval between on/off
		count = atoi(argv[1]);
		milisecs = atoi(argv[2]);
	}
	// Does an FTDI device exist?

	printf("Checking for FTDI devices...\n");

	ftStatus = FT_CreateDeviceInfoList(&dwNumDevs);
	// Get the number of FTDI devices
	if (ftStatus != FT_OK)         // Did the command execute OK?
	{
		printf("Error in getting the number of devices\n");
		return 1;                  // Exit with error
	}

	if (dwNumDevs < 1)             // Exit if we don't see any
	{
		printf("There are no FTDI devices installed\n");
		return 1;                  // Exit with error
	}

	printf("%d FTDI devices found \
         - the count includes individual ports on a single chip\n", dwNumDevs);

	// Open the port we assume port 0

	printf("\nAssume first device has the MPSSE and open it...\n");

	ftStatus = FT_Open(0, &ftHandle);

	if (ftStatus != FT_OK)
	{
		printf("Open Failed with error %d\n", ftStatus);
		return 1;                  // Exit with error
	}

	// Configure port parameters

	printf("\nConfiguring port for MPSSE use...\n");

	ftStatus |= FT_ResetDevice(ftHandle); // Reset USB Device

	if (ftStatus != FT_OK)         // Did the command execute OK?
	{
		printf("ResetDevice did not return properly\n");
		return 1;                  // Exit with error
	}


	// Purge USB receive buffer first by reading out all old data from FT2232H recieve buffer
	ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
	// Get the number of bytes in the FT2232H recieve buffer

	if ((ftStatus == FT_OK) && (dwNumBytesToRead > 0))
		FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
	// Read out the data from FT2232H receive buffer

	ftStatus |= FT_SetUSBParameters(ftHandle, 65536, 65535);
	// set the USB request transfer sizes to 64k

	ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0);
	// Disable event and error characters

	ftStatus |= FT_SetTimeouts(ftHandle, 0, 5000);
	// Sets the read and write timeouts in milliseconds

	ftStatus |= FT_SetLatencyTimer(ftHandle, 1);
	// Set the latency timer to 1mS (default is 16mS)

	ftStatus |= FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0x00, 0x00);
	// Turn on flow control to synchronize IN requests

	ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00);
	// Reset controller

	ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02);
	// Enable MPSSE mode

	if (ftStatus != FT_OK)
	{
		printf("Error in initializing the MPSSE %d\n", ftStatus);
		FT_Close(ftHandle);
		return 1;                   // Exit with error
	}

	sleep(1);

	// Enable internal loop-back

	byOutputBuffer[dwNumBytesToSend++] = 0x84; // Enable Loopback

	ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	// send off the loopback command

	dwNumBytesToSend = 0; // Reset output buffer pointer

	sleep(1); // Wait for all the USB stuff to complete and work

	// Check the receive buffer - it should be empty
	ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);

	if (dwNumBytesToRead != 0)
	{
		printf("Error 0 - MPSSE receive buffer should be empty %d - dwNumBytesToRead %d\n", ftStatus, dwNumBytesToRead);
		FT_SetBitMode(ftHandle, 0x0, 0x00);
		// Reset the port to disable MPSSE
		FT_Close(ftHandle); // Close the USB port
		return 1;  // exit with error
	}

	// synchronize the MPSSE by sending a bogus opcode (0xAB),
	// the MPSSE will respond with a "Bad Command" (0xFA) followed by
	// the bogus opcode itself

	byOutputBuffer[dwNumBytesToSend++] = 0xAB;
	// Add bogus command 0xAB to the queue

	ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	// Send off the BAD command
	dwNumBytesToSend = 0; // Reset output buffer pointer

	do
	{
		ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
		// Get the number of bytes in the device input buffer
	} while ((dwNumBytesToRead == 0) && (ftStatus == FT_OK));
	// or Timeout

	bool bCommandEchod = false;

	ftStatus = FT_Read(ftHandle, &byInputBuffer, dwNumBytesToRead, &dwNumBytesRead);
	// Read out the data from the input buffer

	printf("%d byte(s) read from buffer:", dwNumBytesRead);

	for (i=0; i<dwNumBytesRead;i++)
		printf("[%x]",byInputBuffer[i]); //Print read buffer bytes from ftdi
	printf("\n");

	for (dwCount = 0; dwCount < dwNumBytesRead - 1; dwCount++)
		// check if bad command and echo command are received
	{
		if ((byInputBuffer[dwCount] == 0xFA) && (byInputBuffer[dwCount+1] == 0xAB))
		{ bCommandEchod = true;
		break;
		}
	}
	if (bCommandEchod == false)
	{
		printf("Error in synchronizing the MPSSE\n");
		FT_Close(ftHandle);
		return 1;  // Exit with error
	}

	// Disable internal loop-back

	byOutputBuffer[dwNumBytesToSend++] = 0x84; //actually this line disables loopback, but my board doesnt have phsysically loopback connection right now, so I kept loop back configuration
	// disable the loopback command 0x85
	ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	// send off the loopback command

	dwNumBytesToSend = 0; // reset output buffer pointer

	// check the receive buffer - it should be empty
	ftStatus = FT_GetQueueStatus(ftHandle, &dwNumBytesToRead);
	// get the number of bytes in the FT2232H receive buffer

	if (dwNumBytesToRead != 0)
	{
		printf("Error 1 - MPSSE receive buffer should be empty\n", ftStatus);
		FT_SetBitMode(ftHandle, 0x0, 0x00);
		// Reset the port to disable MPSSE
		FT_Close(ftHandle); // Close the USB Port
		return 1;
	}

	// Configure the MPSSE settings for JTAG
	// multiple commands can be sent to the MPSSE with one FT_Write

	dwNumBytesToSend = 0; // start with a fresh index

	/*FT2232H specific commands removed from here. Becuase my board is FT2232D*/

	// Set TCK frequency
	// TCK = 60MHz / ((1 + [(1 + 0xValueH*256) or (0xValueL])*2)

	byOutputBuffer[dwNumBytesToSend++] = '\x86';
	// command to set clock divisor

	byOutputBuffer[dwNumBytesToSend++] = dwClockDivisor & 0xFF;
	// set 0xValueL of clock divisor

	byOutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & 0xFF;
	// set 0xValueH of clock divisor

	ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	// send off the clock divisor commands

	dwNumBytesToSend = 0; // reset output buffer pointer

	// Set initial states of the MPSSE interface
	// - low byte, both pin directions and output values
	// Pin name Signal Direction Config Initial State Config
	// ADBUS0 TCK/SK output 1 high 1
	// ADBUS1 TDI/DO output 1 low 0
	// ADBUS2 TDO/DI input  0     0
	// ADBUS3 TMS/CS output 1 high 1
	// ADBUS4 GPIOL0 output 1 low 0
	// ADBUS5 GPIOL1 output 1 low 0
	// ADBUS6 GPIOL2 output 1 high 1
	// ADBUS7 GPIOL3 output 1 high 1

	byOutputBuffer[dwNumBytesToSend++] = 0x80;
	// configure data bits low-byte of MPSSE port

	byOutputBuffer[dwNumBytesToSend++] = 0xC9;
	// initial state config above

	byOutputBuffer[dwNumBytesToSend++] = 0xFB;
	// direction config above

	ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
	// send off the low GPIO config commands

	dwNumBytesToSend = 0;
	// reset output buffer pointer

	// Note that since the data in subsequent sections will be clocked on the rising edge, the
	// inital clock state of high is selected. Clocks will be generated as high-low-high.
	// For example, in this case, data changes on the rising edge to give it enough time
	// to have it available at the device, which will accept data *into* the target device
	// on the falling edge.
	// Set initial states of the MPSSE interface
	// - high byte, both pin directions and output values
	// Pin name Signal Direction Config Initial State Config
	// ACBUS0 GPIOH0 input 0 0
	// ACBUS1 GPIOH1 input 0 0
	// ACBUS2 GPIOH2 input 0 0
	// ACBUS3 GPIOH3 input 0 0
	// ACBUS4 GPIOH4 input 0 0
	// ACBUS5 GPIOH5 input 0 0
	// ACBUS6 GPIOH6 input 0 0
	// ACBUS7 GPIOH7 input 0 0

	printf("Led will blink %d times with %d milisecs on/off time.\n", count, milisecs);

	c  = 1;

	do{
		byOutputBuffer[dwNumBytesToSend++] = 0x82;
		// configure data bits low-byte of MPSSE port

		byOutputBuffer[dwNumBytesToSend++] = 0x01&c++;
		// initial state config above

		byOutputBuffer[dwNumBytesToSend++] = 0x01;
		// direction config above;

		ftStatus = FT_Write(ftHandle, byOutputBuffer, dwNumBytesToSend, &dwNumBytesSent);
		// send off the high GPIO config commands
		dwNumBytesToSend = 0;
		usleep(milisecs*1000);
	}while(c<count*2);

	printf("Led blinked %d times!\n",c/2);

	FT_SetBitMode(ftHandle, 0x0, 0x00);
	// reset the port to disable MPSSE
	FT_Close(ftHandle);
	// close the USB port

	return 0;
}

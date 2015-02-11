#include<stdio.h>
#include"../include/ftcjtag.h"

int main(int argc, char* argv[])
{
	WriteDataByteBuffer WriteDataBuffer;
	ReadDataByteBuffer ReadDataBuffer;

	DWORD dwNumBytesReturned = 0;

	FTC_STATUS Status = FTC_SUCCESS;
	DWORD dwNumDevices = 0;

	FTC_HANDLE ftHandle;
	DWORD dwClockDivisor = 600;


	/*
	Status = JTAG_GetNumDevices(&dwNumDevices);

	if(Status == FTC_SUCCESS)
		printf("There are %d JTAG devices.\n", dwNumDevices);
	else{
		printf("No Jtag device detected\nExiting program.\n");
		exit(1);
	}
	 */
	Status = JTAG_Open(&ftHandle);

	if(Status == FTC_SUCCESS)
		printf("JTAG device opened.\n");
	else{
		printf("Err:%d Couldn't open JTAG device.\n", Status);
		exit(1);
	}

	sleep(1);

	Status = JTAG_InitDevice(ftHandle, dwClockDivisor);


	if(Status == FTC_SUCCESS)
		printf("JTAG init successful.\n");
	else{
		printf("Err:Couldn't init JTAG device.\n");
	}

	sleep(1);

	// Add values to write buffer
	WriteDataBuffer[0] = '\x0E'; //SCAN IDCODE IR instruction value

	// Write buffer to device
	Status = JTAG_Write(ftHandle, TRUE, 4, &WriteDataBuffer, 65535, RUN_TEST_IDLE_STATE);

	sleep(1);

	if(Status == FTC_SUCCESS)
		printf("JTAG command write successful.\n");
	else{
		printf("JTAG command write failed.\n", Status);
		Status = JTAG_Close(ftHandle);
		exit(1);
	}
	Status = JTAG_Read(ftHandle, FALSE, 32, &ReadDataBuffer, &dwNumBytesReturned, RUN_TEST_IDLE_STATE);
	if(Status == FTC_SUCCESS)
		printf("JTAG command read successful.\n");
	else{
		printf("JTAG command read failed.\n", Status);
		Status = JTAG_Close(ftHandle);
		exit(1);
	}

	printf("0x%02x%02x%02x%02x\n",ReadDataBuffer[3],ReadDataBuffer[2],ReadDataBuffer[1],ReadDataBuffer[0]);

	sleep(1);

	Status = JTAG_Close(ftHandle);

	if(Status == FTC_SUCCESS)
		printf("JTAG device closed.\n");
	else{
		printf("Err:%d Couldn't close JTAG device.\n", Status);
	}
	return 0;
}

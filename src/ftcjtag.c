#include<stdio.h>
#include"../include/ftcjtag.h"

int main(int argc, char* argv[])
{

	FTC_STATUS Status = FTC_SUCCESS;
	DWORD dwNumDevices = 0;
	Status = JTAG_GetNumDevices(&dwNumDevices);

	if(Status == FTC_SUCCESS)
		printf("There are %d JTAG devices.\n", dwNumDevices);
	else{
		printf("No Jtag device detected\nExiting program.\n");
		exit(1);
	}

	return 0;
}

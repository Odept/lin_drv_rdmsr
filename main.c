#include "api.h"

#include <stdio.h>
#include <fcntl.h>		// open
#include <sys/ioctl.h>	// ioctl
#include <errno.h>


static int ioctl_rdmsr(int f_drv, ulong f_msr, ulong* f_pVal)
{
	ulong val = f_msr;
	int err = ioctl(f_drv, IOCTL_MSR_READ, &val);
	if(err < 0)
	{
		printf("ERROR: failed to send IOCTL_MSR_READ (%d)\n", err);
		return -ENOTTY;
	}

	*f_pVal = val;
	return 0;
}


int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Usage: <rdmsr> msr_number\n");
		return 0;
	}

	char* errstr = NULL;
	ulong msr = strtol(argv[1], &errstr, 0);
	if(*errstr)
	{
		printf("ERROR: invalid MSR\n");
		return -EINVAL;
	}

	int file = open("/dev/"DEV_NAME, O_RDWR);
	if(file < 0)
	{
		printf("ERROR: failed to open the device \"/dev/"DEV_NAME"\"\n");
		return -EBADF;
	}

	ulong val;
	if(ioctl_rdmsr(file, msr, &val) == 0)
		printf("MSR 0x%08X = 0x%016lX\n", (uint)msr, val);

	close(file);

	return 0;
}


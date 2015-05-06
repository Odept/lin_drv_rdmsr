#ifndef __API_H__
#define __API_H__

#include <linux/ioctl.h>


typedef unsigned int	uint;
typedef unsigned long	ulong;


#define DEV_NAME	"rdmsr"

#define IOCTL_MAGIC	31337
#define IOCTL_MSR_READ _IOR(IOCTL_MAGIC, 0, ulong*)


//#define MSR_VMX_CR4_FIXED0 0x488
//#define MSR_VMX_CR4_FIXED1 0x489

#endif // __API_H__


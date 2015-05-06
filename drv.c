/**
 * sudo insmod <name> - start driver
 * sudo  rmmod <name> -  stop driver
 */
#include "api.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>		// cdev_init
#include <linux/device.h>	// class_create
//#include <asm/uaccess.h>


static void drv_msg(const char* f_fmt, ...)
{
	char buf[256];

	va_list args;
	va_start(args, f_fmt);

	sprintf(buf, KERN_INFO "rdmsr: ");
	vsprintf(buf + strlen(buf), f_fmt, args);
	strcat(buf, "\n");
	printk(buf);

	va_end(args);
}
#define TRACE(MSG) drv_msg MSG

static void drv_error(const char* f_fmt, ...)
{
	char buf[256];

	va_list args;
	va_start(args, f_fmt);

	sprintf(buf, KERN_ALERT "rdmsr: ERROR(");
	vsprintf(buf + strlen(buf), f_fmt, args);
	strcat(buf, ")\n");
	printk(buf);

	va_end(args);
}
#define ERROR(MSG) drv_error MSG

// ====================================
static ulong read_msr(uint f_msr)
{
	uint eax, edx;
	rdmsr(f_msr, eax, edx);
	return (((ulong)edx) << 32) | eax;
}

// ============================================================================
//static uint s_open;
static int drv_open(struct inode* f_inode, struct file* f_file)
{
	TRACE(("open (%p, %p)", f_inode, f_file));
	return 0;
}


static int drv_close(struct inode* f_inode, struct file* f_file)
{
	TRACE(("close (%p, %p)", f_inode, f_file));
	return 0;
}


static long drv_ioctl(struct file* f_file, unsigned int f_ioctl, unsigned long f_param)
{

	switch(f_ioctl)
	{
		case IOCTL_MSR_READ:
		{
			ulong* param;
			uint msr;

			param = (ulong*)f_param;
			if(*param >= (1llu << (sizeof(uint) * 8)))
			{
				ERROR(("MSR is out of range (0x%016llX)", *param));
				return EDOM;
			}

			msr = (uint)*param;
			TRACE(("Read MSR 0x%X", msr));
			*param = read_msr(msr);
		}
			break;

		default:
			ERROR(("invalid IOCTL 0x%X", f_ioctl));
			return -EINVAL;
	}

	return 0;
}

// ====================================
#define FIRST_MINOR	0
#define MINOR_CNT	1

static struct file_operations s_ops =
{
	.owner		= THIS_MODULE,
	.open			= drv_open,
	.release		= drv_close,
	.unlocked_ioctl	= drv_ioctl
};


static dev_t s_dev;
static struct cdev s_cdev;
static struct class* s_class;


static int __init mod_init(void)
{
	int err;
	struct device* dev;

	TRACE(("load (x%u)", sizeof(void*) * 8));

	err = alloc_chrdev_region(&s_dev, FIRST_MINOR, MINOR_CNT, "drv_rdmsr");
	if(err < 0)
	{
		ERROR(("failed to alloc a char device region (%d)", err));
		return err;
	}

	cdev_init(&s_cdev, &s_ops);

	err = cdev_add(&s_cdev, s_dev, MINOR_CNT);
	if(err < 0)
	{
		ERROR(("failed to add the char device (%d)", err));
		unregister_chrdev_region(s_dev, MINOR_CNT);
		return err;
	}

	s_class = class_create(THIS_MODULE, "char");
	if( IS_ERR(s_class) )
	{
		ERROR(("failed to create a char class"));
		cdev_del(&s_cdev);
		unregister_chrdev_region(s_dev, MINOR_CNT);
		return PTR_ERR(s_class);
	}

	dev = device_create(s_class, NULL, s_dev, NULL, DEV_NAME);
	if( IS_ERR(dev) )
	{
		ERROR(("failed to crate a device (%d)", err));
		class_destroy(s_class);
		cdev_del(&s_cdev);
		unregister_chrdev_region(s_dev, MINOR_CNT);
		return PTR_ERR(dev);
	}

	TRACE(("driver loaded (/dev/"DEV_NAME")"));
	return 0;
}


static void __exit mod_reset(void)
{
	TRACE(("unload"));

	device_destroy(s_class, s_dev);
	class_destroy(s_class);
	cdev_del(&s_cdev);
	unregister_chrdev_region(s_dev, MINOR_CNT);
}


module_init(mod_init );
module_exit(mod_reset);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anonymous");
MODULE_DESCRIPTION("Read MSR Char Driver");


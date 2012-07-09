/*-----------------------------------------------------------------------------
  파 일 : dev_gpio.c
  설 명 : GPIO F Port 및 GPG5, GPG11 Port를 제어하는 드라이버이다.
  작 성 : tsheaven@falinux.com
  날 짜 : 2008-06-20
  주 의 :

-------------------------------------------------------------------------------*/
#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h> 
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/ioport.h>
#include <linux/slab.h>     // kmalloc() 
#include <linux/poll.h>     // poll
#include <linux/proc_fs.h>
#include <linux/time.h>
#include <linux/timer.h>

//#include <linux/irq.h>
#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>

//#nclude <asm/unistd.h>
#include <asm/arch/regs-gpio.h>


#include "dev_gpio.h"

/* local value  -----------------------------------------------------------------*/
#define	ON		1
#define	OFF		0


/* local variable  --------------------------------------------------------------*/
static int usage   = 0;
static int showmsg = 0;
static int major   = GPIO_MAJOR_DEF;

/* implementaion ==============================================================*/
void set_gpio_a (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO A %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPA0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPA1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPA2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPA3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPA4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPA5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPA6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPA7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPA8, val ); break;
		default : break;
	}		
}

void set_gpio_b (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO B %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPB0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPB1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPB2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPB3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPB4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPB5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPB6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPB7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPB8, val ); break;
		default : break;
	}		
}

void set_gpio_c (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO C %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPC0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPC1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPC2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPC3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPC4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPC5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPC6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPC7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPC8, val ); break;
		default : break;
	}		
}

void set_gpio_d (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO D %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPD0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPD1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPD2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPD3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPD4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPD5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPD6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPD7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPD8, val ); break;
		default : break;
	}		
}

void set_gpio_e (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO E %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPE0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPE1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPE2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPE3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPE4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPE5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPE6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPE7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPE8, val ); break;
		default : break;
	}		
}

void set_gpio_f (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO F %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPF0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPF1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPF2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPF3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPF4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPF5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPF6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPF7, val ); break;
		default : break;
	}		
}

void set_gpio_g (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO G %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPG0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPG1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPG2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPG3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPG4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPG5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPG6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPG7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPG8, val ); break;
		default : break;
	}		
}

void set_gpio_h (char gpb_no, unsigned char val)
{
	if (showmsg) printk("GPIO H %x = %x\n", gpb_no, val); 
		
	switch( gpb_no )
	{
		case 0	: s3c2410_gpio_setpin( S3C2410_GPH0, val ); break;
		case 1	: s3c2410_gpio_setpin( S3C2410_GPH1, val ); break;
		case 2	: s3c2410_gpio_setpin( S3C2410_GPH2, val ); break;
		case 3	: s3c2410_gpio_setpin( S3C2410_GPH3, val ); break;
		case 4	: s3c2410_gpio_setpin( S3C2410_GPH4, val ); break;
		case 5	: s3c2410_gpio_setpin( S3C2410_GPH5, val ); break;
		case 6	: s3c2410_gpio_setpin( S3C2410_GPH6, val ); break;
		case 7	: s3c2410_gpio_setpin( S3C2410_GPH7, val ); break;
		case 8	: s3c2410_gpio_setpin( S3C2410_GPH8, val ); break;
		default : break;
	}		
}

static int gpio_ioctl( struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg )
{
	int size = 0;;
	int err = 0;
	dip_sw_info dip_rd;
	unsigned short *dip;


	size = _IOC_SIZE(cmd);

	//printk("size = %d\n", size);
	if(size)
	{
		// be careful. if you want to read from memory, use VERIFY_WRITE.
		if (_IOC_DIR(cmd) & _IOC_READ) 
			//err = verify_area(VERIFY_WRITE, (void *) arg, size);
			err = access_ok(VERIFY_WRITE, (void *) arg, size);
		
		//printk("err = %d\n", err);
	}

	//printk(" %s IOCTL (%d)\n", DEV_NAME, usage );
/*
	switch (cmd)
	{
	case IOCTL_GPIO_SET	: set_gpio_f( arg, 1 ); return 0;
	case IOCTL_GPIO_CLR	: set_gpio_f( arg, 0 ); return 0;
	}							  
*/		
	switch(cmd)
	{
		case IOCTL_GPIO_A_ON:	set_gpio_a(arg, ON);	break;
		case IOCTL_GPIO_B_ON:	set_gpio_b(arg, ON);	break;
		case IOCTL_GPIO_C_ON:	set_gpio_c(arg, ON);	break;
		case IOCTL_GPIO_D_ON:	set_gpio_d(arg, ON);	break;
		case IOCTL_GPIO_E_ON:	set_gpio_e(arg, ON);	break;
		case IOCTL_GPIO_F_ON:	set_gpio_f(arg, ON);	break;
		case IOCTL_GPIO_G_ON:	set_gpio_g(arg, ON);	break;
		case IOCTL_GPIO_H_ON:	set_gpio_h(arg, ON);	break;
			
		case IOCTL_GPIO_A_OFF:	set_gpio_a(arg, OFF);	break;
		case IOCTL_GPIO_B_OFF:	set_gpio_b(arg, OFF);	break;
		case IOCTL_GPIO_C_OFF:	set_gpio_c(arg, OFF);	break;
		case IOCTL_GPIO_D_OFF:	set_gpio_d(arg, OFF);	break;
		case IOCTL_GPIO_E_OFF:	set_gpio_e(arg, OFF);	break;
		case IOCTL_GPIO_F_OFF:	set_gpio_f(arg, OFF);	break;
		case IOCTL_GPIO_G_OFF:	set_gpio_g(arg, OFF);	break;
		case IOCTL_GPIO_H_OFF:	set_gpio_h(arg, OFF);	break;
		
		case IOCTL_GET_DIP_SWITCH:	
			dip = (unsigned short *) 0xf9000004;
			//printk(">>>>>>>>>> %x \n", *dip);
			dip_rd.chk = 1;
			dip_rd.info = (unsigned short)*dip & 0xff;
			dip_rd.size = sizeof(unsigned short);
			copy_to_user ((void *)arg, (const void *)&dip_rd, (unsigned long)size);
			break;
	}

	return -EINVAL;
}

static int gpio_open( struct inode *inode, struct file *filp )
{
	usage ++;

	if (showmsg) printk(" %s OPEN (%d)\n", DEV_NAME, usage );
    return 0;
}

static int gpio_release( struct inode *inode, struct file *filp)

{
	usage --;          
    
	if (showmsg) printk(" %s CLOSE\n", DEV_NAME );
    return 0;
}

static struct file_operations gpio_fops =
{
	open    : gpio_open, 	
	release : gpio_release, 
	//read    : gpio_read,		// not used.
	ioctl   : gpio_ioctl,
};

static __init int gpio_init( void )
{
	// register module
	major &= 0xff;
	if( !register_chrdev( major, DEV_NAME, &gpio_fops ) )  
	{
		printk("register device %s OK (major=%d)\n\n", DEV_NAME, major );
	}
	else
	{        
		printk("unable to get major %d for %s \n\n", major, DEV_NAME );
		return -EBUSY;
	}

    return 0;		
}

static __exit void gpio_exit( void )
{
	// unregister moudle
    unregister_chrdev( major, DEV_NAME );
    printk(" unregister %s OK\n\n", DEV_NAME );
}

/*-----------------------------------------------------------------------------*/

module_init(gpio_init);
module_exit(gpio_exit);
module_param(major, int, 0);
module_param(showmsg, int, 0);
MODULE_AUTHOR("jong2ry@imecasys.com");
MODULE_LICENSE("Dual BSD/GPL");

/* end */


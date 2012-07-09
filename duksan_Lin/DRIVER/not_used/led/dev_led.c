
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <linux/fs.h>          
#include <linux/errno.h>       
#include <linux/types.h>       
#include <linux/fcntl.h>       

#include <asm/uaccess.h>
#include <asm/io.h>

#include <linux/time.h>
#include <linux/timer.h>

#include "dev_led.h"

#define   KERNELTIMER_WRITE_ADDR           0x0378

#define   TIME_STEP                       (5*HZ/10)

unsigned char led_con_reg = 0xff;
static int major  = LED_DEF;

typedef struct
{
        struct timer_list  timer;            
	unsigned long      led;
} __attribute__ ((packed)) KERNEL_TIMER_MANAGER;

static KERNEL_TIMER_MANAGER *ptrmng = NULL;

void kerneltimer_timeover(unsigned long arg );

void kerneltimer_registertimer( KERNEL_TIMER_MANAGER *pdata, unsigned long timeover )
{
     init_timer( &(pdata->timer) );
     pdata->timer.expires  = get_jiffies_64() + timeover;
     pdata->timer.data     = (unsigned long) pdata      ;
     pdata->timer.function = kerneltimer_timeover       ;
     add_timer( &(pdata->timer) );
}

void kerneltimer_timeover(unsigned long arg )
{
	KERNEL_TIMER_MANAGER *pdata = NULL;     
	unsigned short *preg;

	if( arg )
	{
		pdata = ( KERNEL_TIMER_MANAGER * ) arg;

      	if (pdata->led > 0)
	  	{
	  	  	pdata->led = 0;
		  	led_con_reg |= 0x01;
      	}
	  	else
	  	{
	      	pdata->led = 1;
		  	led_con_reg &= 0xfe;
	  	}
		preg = (unsigned short *) 0xf9000000;
		*preg = led_con_reg;
	  
	  kerneltimer_registertimer( pdata , TIME_STEP );
   }
}

int kerneltimer_init(void)
{
	ptrmng = kmalloc( sizeof( KERNEL_TIMER_MANAGER ), GFP_KERNEL );
    if( ptrmng == NULL ) return -ENOMEM;
     
    memset( ptrmng, 0, sizeof( KERNEL_TIMER_MANAGER ) );
     
    ptrmng->led = 0;
    kerneltimer_registertimer( ptrmng, TIME_STEP );
   	
   	printk("register device %s OK (major=%d)\n\n", DEV_NAME, major );
	return 0;
}

void kerneltimer_exit(void)
{
    if( ptrmng != NULL ) 
    {
        del_timer( &(ptrmng->timer) );
        kfree( ptrmng );
    }    
    printk(" unregister %s OK\n\n", DEV_NAME );
}

module_init(kerneltimer_init);
module_exit(kerneltimer_exit);
MODULE_AUTHOR("jong2ry@imecasys.com");
MODULE_LICENSE("Dual BSD/GPL");


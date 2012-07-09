/*-----------------------------------------------------------------------------
  파 일 : net32_dev.c
  설 명 : Duksan IDC에서 NET32 Serial Driver와 통신하기 위한 모듈이다.
  작 성 : jong2ry@imecasys.com
  날 짜 : 2011-03-15
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

#include <asm/uaccess.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>

#include <asm/arch/regs-gpio.h>


#define  DEV_ADDR_DIP_SW			(S3C2410_VA_DIP)+(0x00000004)
#define  DEV_ADDR_DO_BASE			(S3C2410_VA_DIP)+(0x00000008)
#define  DEV_ADDR_ANALOG_BASE		(S3C2410_VA_DIP)+(0x0000000C)

#define  DEV_NAME				"s3c2410_net32"
#define  MAJOR_NUMBER   		254
#define  BUFF_SIZE      		2048

// 항상 s3c2410.c의 내용과 동일해야 한다. 
#define		NET32_TOKEN					0x80		// net32 token message
#define		NET32_REPORT_MSG			0x20		// net32 report message
#define		NET32_COMMAND_MSG			0x40		// net32 command message
#define		NET32_REQUIRE_MSG			0x00		// net32 require message
#define		NET32_INFO_MSG				0x8a		// net32 information message
#define		NET32_BOOT_MSG				0x88		// net32 boot message

// 항상 s3c2410.c의 내용과 동일해야 한다. 
#define		NET32_MAX_QUEUE_SIZE		2048		//queue size
#define		NET32_QUEUE_FULL			-1			//queue full
#define		NET32_QUEUE_EMPTY			-2			//queue empty
#define 	NET32_QUEUE_SUCCESS			1			//queue success

// 항상 s3c2410.c의 내용과 동일해야 한다. 
#define 	NET32_QUEUE_NOT_BUSY		0			//queue not busy
#define 	NET32_QUEUE_BUSY			1			//queue busy

// 항상 STRUCTURE.h의 내용과 동일해야 한다. 
// net32 token message type
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_TOKEN_T;

// 항상 STRUCTURE.h의 내용과 동일해야 한다. 
// net32 basic message type
typedef struct {
	unsigned char cHeader;
	unsigned char cCmd;
	unsigned char cLength;
	unsigned char cDbgNoq;
	unsigned char cDbgNiq;
	unsigned char cPcm;
	unsigned char cPno;
	unsigned char cVal[4];
	unsigned char cChkSum;
} __attribute__ ((packed)) NET32_MSG_T;

// net32 queue data 
// 항상 s3c2410.c의 내용과 동일해야 한다. 
typedef struct {
	short pcm;
	short pno;
	float value;
	unsigned int type;
	unsigned char msg[12];
} net32_data;

static char		*g_pRxMsg = NULL;
static char		*g_pTxMsg = NULL;
static int		sz_data = 0;
static int		g_nMyId = 0;
static int		g_nDipSw = 0;


// s3c2410.c 에서 extern symbol된 함수이다.
extern int pop_net32(net32_data *pPoint);
extern int push_net32(net32_data *pPoint); 


static int s3c2410_net32_open( struct inode *inode, struct file *filp ) 
// ----------------------------------------------------------------------------
// MODULE OPEN FUNCTION
{
	return 0;
}


static int s3c2410_net32_release( struct inode *inode, struct file *filp )
// ----------------------------------------------------------------------------
// MODULE RELEASE FUNCTION
{
	return 0;
}




static void s3c2410_net32_handler (char *pBuf, size_t length)
{
	int i = 0;
	net32_data point;
	NET32_MSG_T *pMsg = (NET32_MSG_T *)pBuf;	
	NET32_TOKEN_T *pToken = (NET32_TOKEN_T *)pBuf;
	
	if ( pMsg->cCmd == 'R' ) {
		//printk("R\n");
		point.msg[0] = 0x00;		// fixed
		point.msg[1] = NET32_REPORT_MSG;
		point.msg[2] = 0x09;	// fixed data length
		point.msg[3] = g_nMyId;
		point.msg[4] = pMsg->cPno;
		point.msg[5] = pMsg->cVal[0];
		point.msg[6] = pMsg->cVal[1];
		point.msg[7] = pMsg->cVal[2];
		point.msg[8] = pMsg->cVal[3];
		point.msg[9] = 0x00;	// ChkSum
	    for ( i = 0; i < 9; i++ )
			point.msg[9] -= point.msg[i];								

		point.type = NET32_REPORT_MSG;
		push_net32(&point);
	}

	if ( pMsg->cCmd == 'C' ) {
		//printk("C\n");
		point.msg[0] = pMsg->cPcm;
		point.msg[1] = NET32_COMMAND_MSG;
		point.msg[2] = 0x09;	// fixed data length
		point.msg[3] = g_nMyId;
		point.msg[4] = pMsg->cPno;
		point.msg[5] = pMsg->cVal[0];
		point.msg[6] = pMsg->cVal[1];
		point.msg[7] = pMsg->cVal[2];
		point.msg[8] = pMsg->cVal[3];
		point.msg[9] = 0x00;	// ChkSum
	    for ( i = 0; i < 9; i++ )
	        point.msg[9] -= point.msg[i];								

		point.type = NET32_COMMAND_MSG;
		push_net32(&point);
	}			

	if ( pMsg->cCmd == 'r' ) {
		printk("r\n");
		point.msg[0] = pMsg->cPcm;
		point.msg[1] = NET32_REQUIRE_MSG;
		point.msg[2] = 0x05;	// fixed data length
		point.msg[3] = g_nMyId;
		point.msg[4] = pMsg->cPno;
		point.msg[5] = 0x00;	// ChkSum
	    for ( i = 0; i < 5; i++ )
	        point.msg[5] -= point.msg[i];								

		point.type = NET32_REQUIRE_MSG;
		push_net32(&point);
	}	

	if ( pToken->cCmd == 'T' ) {
		//printk("T\n");
		//p->cDbgNiq = pMsg->cDbgNiq;
		//p->cDbgNoq = pMsg->cDbgNoq;		
	}
}


static ssize_t s3c2410_net32_write( struct file *filp, const char *buf, size_t count, loff_t *f_pos )
// ----------------------------------------------------------------------------
// MODULE WRITE FUNCTION
{	
	if (BUFF_SIZE < count)  
		sz_data  = BUFF_SIZE;
	sz_data  = count;

	copy_from_user( g_pRxMsg, buf, sz_data );	

	if ( sz_data > 0 ) { 
		s3c2410_net32_handler(g_pRxMsg, sz_data);
	}
	
	return 1;
}


static ssize_t s3c2410_net32_read( struct file *filp, char *buf, size_t count, loff_t *f_pos )
// ----------------------------------------------------------------------------
// MODULE READ FUNCTION
{
	int nRtnQueue;
	net32_data point;

	memset(&point.msg[0], 0x00, 12);
	nRtnQueue = pop_net32(&point);
	
	if ( nRtnQueue == NET32_QUEUE_SUCCESS ) {
		copy_to_user( buf, &point.msg[0], sizeof(point.msg) );		
		return sizeof(point.msg);
	}
	else 
		return -1;
	
	return -1;
}


// ----------------------------------------------------------------------------
// fops
static struct file_operations net32_fops = {
	.read 		= s3c2410_net32_read,
	.write 		= s3c2410_net32_write,
	.open 		= s3c2410_net32_open,
	.release 	= s3c2410_net32_release
};


int __init s3c2410_net32_init( void )
// ----------------------------------------------------------------------------
// MODULE INIT
{
	int nRtnQueue = 0;
	net32_data sData;
	
	register_chrdev( MAJOR_NUMBER, DEV_NAME, &net32_fops );
	
	g_pRxMsg = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );
	g_pTxMsg = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );
	memset( g_pRxMsg, 0, BUFF_SIZE );
	memset( g_pTxMsg, 0, BUFF_SIZE );
	
	printk("[NET32] initialized\n");

	// get dip switch value.
	// dips switc 회로에서 on/off가 반대로 되어 있기 때문에 주의해야 한다. 
	g_nDipSw = ~(readw(DEV_ADDR_DIP_SW)) & 0x000000ff ;
	g_nMyId = g_nDipSw  & 0x1f;
	printk("[NET32] my_id %d (0x%x)\n", g_nMyId, g_nDipSw);
	

	sData.type = NET32_INFO_MSG;
	nRtnQueue = push_net32(&sData);
	
	if ( nRtnQueue == NET32_QUEUE_SUCCESS )
		printk("[NET32] state msg push success\n");
	else 
		printk("[NET32] state msg push fail\n");
		
	return 0;
}


void __exit s3c2410_net32_exit( void )
// ----------------------------------------------------------------------------
// MODULE EXIT
{
	unregister_chrdev( MAJOR_NUMBER, DEV_NAME );
	kfree( g_pRxMsg );
	kfree( g_pTxMsg );
	printk( "[NET32] exited\n");
}


module_init( s3c2410_net32_init );
module_exit( s3c2410_net32_exit );
MODULE_AUTHOR("jong2ry@imecasys.com");
MODULE_LICENSE("Dual BSD/GPL");

/* end */


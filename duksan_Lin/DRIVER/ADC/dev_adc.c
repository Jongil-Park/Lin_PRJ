/*-----------------------------------------------------------------------------
  파 일 : dev_adc.c
  설 명 : Duksan IDC에서 ADC값을 읽어오기 위한 모듈이다.
  작 성 : jong2ry@imecasys.com
  날 짜 : 2010-07-06
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

#define  DEV_NAME				"s3c2410_adc"
#define  MAJOR_NUMBER   		255
#define  BUFF_SIZE      		2048

/* from map.h
#define S3C24XX_VA_GPIO	   S3C2410_ADDR(0x00E00000)		// GPIO
#define S3C2400_PA_GPIO	   (0x15600000)
#define S3C2410_PA_GPIO	   (0x56000000)
#define S3C24XX_SZ_GPIO	   SZ_1M 
 
#define S3C2410_VA_DIP	   		S3C2410_ADDR(0x09000000)			// duksan 추가
#define S3C2410_PA_DIP	   		(0x10000000)
#define S3C2410_SZ_DIP	   		SZ_1M 
*/
#define  DEV_ADDR_DIP_SW			(S3C2410_VA_DIP)+(0x00000004)
#define  DEV_ADDR_DO_BASE			(S3C2410_VA_DIP)+(0x00000008)
#define  DEV_ADDR_ANALOG_BASE		(S3C2410_VA_DIP)+(0x0000000C)

#define  DEV_ADDR_PORTB_CON			(S3C24XX_VA_GPIO)+(0x00000010)
#define  DEV_ADDR_PORTB_DAT			(S3C24XX_VA_GPIO)+(0x00000014)
#define  DEV_ADDR_PORTB_PULLUP		(S3C24XX_VA_GPIO)+(0x00000018)

#define  DEV_SEL_ANALOG_OUT			0x00D8

#define  DEV_REQUEST_ID				1
#define  DEV_RESPONSE_ID			0

static char *gp_buffer = NULL;
static char *gp_txmsg = NULL;
static int   sz_data = 0;

unsigned int g_nFlagId = 0;				// id request flag
unsigned int g_nMyId = 0;				// pcm_id
unsigned int g_nDipSw = 0;				// dip switch
unsigned int g_nRequestPno = 0;		// request pcm number
unsigned int g_nRequestType = 0;		// request type
//unsigned int g_nId2 = 0;				// id2

// from point_handler.c
typedef struct {
	unsigned char c_pno;
	unsigned char c_factoryReset;
	unsigned char c_type;
	unsigned char c_length;
	unsigned int  n_val;
	unsigned char c_chksum;
}__attribute__((packed)) PNT_DEV_MSG_T;

// from define.h  
// point-define의 type
#define DFN_PNT_VR						0
#define DFN_PNT_DO						1
#define DFN_PNT_VO						2
#define DFN_PNT_DI2S					3
#define DFN_PNT_JPT						4
#define DFN_PNT_VI						5
#define DFN_PNT_CI						6
#define DFN_PNT_ID						7

typedef struct {
	unsigned short w_doval;
	unsigned int n_val[24];
}__attribute__((packed)) DEV_ADC_VALUE_T;


char gc_dobit[8] = {0x01, 0x02, 0x04, 0x08,	0x10, 0x20, 0x40, 0x80};

volatile DEV_ADC_VALUE_T g_point;



void dev_wait_adc(unsigned int n_clk)
// ----------------------------------------------------------------------------
// ADC WAIT TIMER
// Description : while을 사용한 Timer....
// Arguments   : n_clk		Timer value.
// Returns     : none
{
	while(n_clk--);
} 



static int dev_dout_write(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// DIGITAL VALUE OUTPUT TO PNO 0 ~ 7
// Description : Digital Output Port를 제어한다. (pno 0 ~ 7)
// Arguments   : p_msg		Is a pointer of receive-message
// Returns     : 0			Always 0
{
	// range of digital point is 0 ~ 7
	if ( p_msg->c_pno < 8 ) {
		// check do bit table
		if ( p_msg->n_val > 0) {
			g_point.w_doval |= gc_dobit[p_msg->c_pno];
		}
		else {
			g_point.w_doval &= ~gc_dobit[p_msg->c_pno];
		}
		
		// write do value
		writeb(g_point.w_doval, DEV_ADDR_DO_BASE);
	}
	return 0;
}



static void dev_dac_write(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// CONTROLED LTC1257
// Description : ltc1257을 제어해서 출력값을 내보낸다.
// 				 GPB0 : TXS  (OUTPUT)  
// 				 GPB1 : RXS  (INPUT) 
// 				 GPB2 : CKS  (OUTPUT)
// 				 GPB3 : *DAC (OUTPUT)
// 				 GPB4 : *ADC (OUTPUT) 
// Arguments   : p_msg		Is a pointer of receive-message
// Returns     : none
{
	int i;
	unsigned short w_aoval_bit;
    
	s3c2410_gpio_setpin(S3C2410_GPB3, 1);			//*DAC high
	s3c2410_gpio_setpin(S3C2410_GPB0, 0);			//TXS low

    for ( i = 0; i < 12; i++ ) {
		s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
		dev_wait_adc(70);
		s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
		w_aoval_bit = ( p_msg->n_val >> (11 - i) ) & 0x0001;
		
        if ( w_aoval_bit ) 
			s3c2410_gpio_setpin(S3C2410_GPB0, 1);	//TXS high
		else
			s3c2410_gpio_setpin(S3C2410_GPB0, 0);	//TXS low
		
		dev_wait_adc(70);
    }
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);			//CKS high
	
	dev_wait_adc(35);
	s3c2410_gpio_setpin(S3C2410_GPB3, 0);			//*DAC low
	dev_wait_adc(35);

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);			//CKS low
	
	dev_wait_adc(35);
	s3c2410_gpio_setpin(S3C2410_GPB3, 1);			//*DAC high
	dev_wait_adc(35);

	s3c2410_gpio_setpin(S3C2410_GPB2, 1);			//CKS high
}



static unsigned short dev_adc_read(int ch_sel)
{
	int i;
	unsigned short adc_rxs;
	
	s3c2410_gpio_setpin(S3C2410_GPB4, 0);		//*ADC low
	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
	s3c2410_gpio_setpin(S3C2410_GPB0, 1);		//TXS high (start bit is 1)
	dev_wait_adc(70);
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
	dev_wait_adc(70);								//SGL/DIFF bit is always 1 (Single-ended Mode)	
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);							

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low

	//If ODD/SIGN bit is 0, CH0 is selected
	if ( ch_sel == 0 )
		s3c2410_gpio_setpin(S3C2410_GPB0, 0);	//TXS low 

	dev_wait_adc(70);							
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);							

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low

	//If MSBF bit is 1, MSB(Dout) is first
	if ( ch_sel == 0 ) {
		s3c2410_gpio_setpin(S3C2410_GPB0, 1);	//TXS high
	}

	//MSBF bit is always							
	dev_wait_adc(70);							
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);							

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
	dev_wait_adc(70);													
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);							

	adc_rxs = s3c2410_gpio_getpin(S3C2410_GPB1) & 0x002;

	for ( i = 0; i < 11; i++ ) {
		s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
		dev_wait_adc(70);													
		s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
		dev_wait_adc(70);													
		adc_rxs += (s3c2410_gpio_getpin(S3C2410_GPB1) & 0x002) << (10 - i);
	}

	s3c2410_gpio_setpin(S3C2410_GPB2, 0);		//CKS low
	dev_wait_adc(70);													
	s3c2410_gpio_setpin(S3C2410_GPB2, 1);		//CKS high
	dev_wait_adc(70);													
	adc_rxs += (s3c2410_gpio_getpin(S3C2410_GPB1) & 0x002) >> 1;
	s3c2410_gpio_setpin(S3C2410_GPB4, 1);		// *ADC high

	return adc_rxs;
}



static void dev_analog_select(unsigned int n_type, PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// ANALOG POINT SELECT
// Description : Analog Output Port를 선택한다.
// Arguments   : p_msg		Is a pointer of receive-message
// Returns     : none
{
	unsigned int n_pno = 0, n_msel = 0;
	
	if ( p_msg->c_pno < 16 ) {
		n_pno = p_msg->c_pno - 8;
		n_msel = 0xf8;
		n_msel &= 0xdf;				// MSEL[3:1] Masking
		n_msel |= n_pno;			// Point Masking
	}
	else if ( p_msg->c_pno < 24 ) {
		n_pno = p_msg->c_pno - 16;
		n_msel = 0xf8;
		n_msel &= 0xf7;				// MSEL[3:1] Masking
		n_msel |= n_pno;			// Point Masking
	}
	else if ( p_msg->c_pno < 32 ) {
		n_pno = p_msg->c_pno - 24;
		n_msel = 0xf8;
		n_msel &= 0xef;				// MSEL[3:1] Masking
		n_msel |= n_pno;			// Point Masking
	}

	writeb( n_msel, DEV_ADDR_ANALOG_BASE);
}



static void dev_aout_write(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// ANALOG VALUE OUTPUT TO PNO 8 ~ 15
// Description : Analog Output Port를 제어한다. (pno 8 ~ 15)
// Arguments   : p_msg		Is a pointer of receive-message
// Returns     : none
{
	// range of analog point is 8 ~ 15
	if ( p_msg->c_pno > 7 && p_msg->c_pno < 16 ) {
		writeb( 0xfc, DEV_ADDR_ANALOG_BASE );				// ready analog out
		g_point.n_val[p_msg->c_pno - 8] = p_msg->n_val;		// get analog value.
		dev_dac_write(p_msg);								// set analog value at dac convertor.
		dev_wait_adc(300);	
		
		dev_analog_select( p_msg->c_type, p_msg );			// select anolog output point.
		dev_wait_adc(300);
	}
}


static void dev_sort_analog_list(unsigned short *pw_data, unsigned short w_cnt)
{
    int i = 0;
    int j = 0;
    unsigned short w_buf = 0;

    for ( i = 0; i < (w_cnt - 1); i++ ) {
        for ( j = i+1; j < w_cnt; j++ ) {
            if ( pw_data[i] > pw_data[j] ) { 
                w_buf = pw_data[i];
                pw_data[i] = pw_data[j];
                pw_data[j] = w_buf;
            }
        }
    }
}



static void dev_ain_read(PNT_DEV_MSG_T *p_msg)
// ----------------------------------------------------------------------------
// ANALOG VALUE INPUT TO PNO 16 ~ 31
// Description : Analog Input Port를 제어한다. (pno 16 ~ 31)
// Arguments   : p_msg		Is a pointer of receive-message
// Returns     : none
{
	int i, j, k;
	unsigned short adc_val = 0x0000;
    unsigned short w_adc_val[8];
    unsigned short w_adc_buf[8];
    unsigned short w_adc_avrbuf[8];

	// range of analog point is 16 ~ 31
	if ( p_msg->c_pno > 15 && p_msg->c_pno < 32 ) {
		// Type에 맞도록 ADC 값을 생성한다.
		// Output과 다르게 먼저 point를 생성한 다음. ADC값을 읽어온다. 
		switch ( p_msg->c_type ) {
			
			// adc 값을 생성하기 위해 6번의 adc값을 읽어온다. 그중에 가운데 4개값의 평균값을 낸다 
			// 그 평균값을 4번 생성한 다음 그값의 가운데 값 2개의 평균을 낸다.
			// 그 평균값을 4번 생성한 다음 그값의 가운데 값 2개의 평균 구해 adc 값을 생성한다.
			// 총 96번의 adc를 가지고 평균을 내는 것이다.
			case DFN_PNT_DI2S:
			case DFN_PNT_VI:
			case DFN_PNT_CI:
			case DFN_PNT_JPT:
				dev_analog_select( p_msg->c_type, p_msg );			// select anolog output point.
				dev_wait_adc(300);

				for ( k = 0; k < 4; k++ ) {
					for ( j = 0; j < 4; j++ ) {
						for ( i = 0; i < 6; i++ ) {
							if ( p_msg->c_type == DFN_PNT_JPT ) 
								w_adc_val[i] = dev_adc_read(1);
							else
								w_adc_val[i] = dev_adc_read(0);
						}    
						dev_sort_analog_list( w_adc_val, 6 );
						w_adc_buf[j] = (w_adc_val[2] + w_adc_val[3]) / 2;
					}
					dev_sort_analog_list( w_adc_buf, 4 );
					w_adc_avrbuf[k] = (w_adc_buf[1] + w_adc_buf[2]) / 2;
				}
				dev_sort_analog_list( w_adc_avrbuf, 4 );

				adc_val = (w_adc_avrbuf[1] + w_adc_avrbuf[2]) / 2;
				break;

			default:
				break;
		}

		// 8 값은 pno를 16~31을 0 ~ 23의 값으로 맞춰주기 위하여 사용하였다. 
		// g_point.n_val의 변수값의 최대 갯수가 24개 이기 때문이다. 
		g_point.n_val[p_msg->c_pno - 8] = adc_val;			
	}
}



static int s3c2410_adc_open( struct inode *inode, struct file *filp ) 
{
	return 0;
}



static int s3c2410_adc_release( struct inode *inode, struct file *filp )
{
	return 0;
}



static ssize_t s3c2410_adc_write( struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
	PNT_DEV_MSG_T *p_msg;
	unsigned short wSwitch = 0;

	if (BUFF_SIZE < count)  
		sz_data  = BUFF_SIZE;
	sz_data  = count;

	copy_from_user( gp_buffer, buf, sz_data );

	p_msg = (PNT_DEV_MSG_T *) gp_buffer;

	g_nRequestPno = p_msg->c_pno;
	g_nRequestType = p_msg->c_type;

	wSwitch = (s3c2410_gpio_getpin(S3C2410_GPB10) >> 10) & 0x0001;

	switch ( g_nRequestType ) {
		case DFN_PNT_VR:
			break;

		case DFN_PNT_DO:
			dev_dout_write(p_msg);
			break;

		case DFN_PNT_VO:
			dev_aout_write(p_msg);
			break;

		case DFN_PNT_DI2S:
		case DFN_PNT_JPT:
		case DFN_PNT_VI:
		case DFN_PNT_CI:
			dev_ain_read(p_msg);
			break;

		case DFN_PNT_ID:
			g_nFlagId = DEV_REQUEST_ID;
			break;

		default:
			break;
	}

	return count;
}



static ssize_t s3c2410_adc_read( struct file *filp, char *buf, size_t count, loff_t *f_pos )
{
	int i = 0;
	PNT_DEV_MSG_T *p_msg;
	unsigned short wSwitch = 0;

	p_msg = (PNT_DEV_MSG_T *) gp_txmsg;

	wSwitch = (s3c2410_gpio_getpin(S3C2410_GPB10) >> 10) & 0x0001;

	if ( g_nFlagId == DEV_REQUEST_ID ) {
		p_msg->c_pno = 0;
		p_msg->c_factoryReset = wSwitch;
		p_msg->c_type = DFN_PNT_ID;
		p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
		//printk("+ 1 p_msg->c_length = %d\n", p_msg->c_length);
		p_msg->n_val = g_nMyId;
		p_msg->c_chksum = 0;
		for( i = 0; i < (p_msg->c_length - 1); i++) {
			p_msg->c_chksum -= gp_txmsg[i];
			//printk("+ 1 %x\n", gp_txmsg[i]);			
		}
		//printk("+ 1 %x\n", p_msg->c_chksum);
	
		g_nFlagId = DEV_RESPONSE_ID;

		copy_to_user( buf, gp_txmsg, count );
		return count;
	}
	else {
		if ( g_nRequestPno < 16 ) {
			return -1;
		}
		else {
			p_msg->c_pno = g_nRequestPno;
			p_msg->c_factoryReset = wSwitch;
			p_msg->c_type = g_nRequestType;
			p_msg->c_length = (char)sizeof(PNT_DEV_MSG_T);
			//printk("+ 2 p_msg->c_length = %d\n", p_msg->c_length);
			p_msg->n_val = g_point.n_val[g_nRequestPno - 8];
			p_msg->c_chksum = 0;
			for( i = 0; i < (p_msg->c_length - 1); i++) {
				p_msg->c_chksum -= gp_txmsg[i];
				//printk("+ 2 %x\n", gp_txmsg[i]);		
			}
			//printk("+ 2 %x\n", p_msg->c_chksum);		

			copy_to_user( buf, gp_txmsg, count );
			return count;
		}
	}

	return -1;
}



static struct file_operations adc_fops = {
	.read 		= s3c2410_adc_read,
	.write 		= s3c2410_adc_write,
	.open 		= s3c2410_adc_open,
	.release 	= s3c2410_adc_release
};

/*
// 사용자가 cat등을 통해서 /proc/id2파일을 열면 
// 커널은 이 함수를 호출해서 해당 정보를 넘겨준다. 
static int proc_read_id2(char *page, char **start, off_t off,
			   int count, int *eof, void *data)
{
	sprintf(page, "%d", g_nId2);
	return 1;
}
*/


int __init s3c2410_adc_init( void )
// ----------------------------------------------------------------------------
// MODULE INIT
// Description : ADC 모듈울 초기화 합니다. 
// Arguments   : none
// Returns     : 0			Always 0
{
	//struct proc_dir_entry *root_proc_file = NULL;

	register_chrdev( MAJOR_NUMBER, DEV_NAME, &adc_fops );
	gp_buffer = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );
	gp_txmsg = (char*) kmalloc( BUFF_SIZE, GFP_KERNEL );

	memset( gp_buffer, 0, BUFF_SIZE );
	memset( gp_txmsg, 0, BUFF_SIZE );
	
	printk("[ADC] initialized\n");
	
	// get dip switch value.
	// dips switc 회로에서 on/off가 반대로 되어 있기 때문에 주의해야 한다. 
	g_nDipSw = ~(readw(DEV_ADDR_DIP_SW)) & 0x000000ff ;
	g_nMyId = g_nDipSw  & 0x1f;
	g_nFlagId = DEV_RESPONSE_ID;
	printk("[ADC] my_id %d (0x%x)\n", g_nMyId, g_nDipSw);
	
	//root_proc_file = create_proc_entry("id2", S_IFREG | S_IRUSR, NULL);
	//root_proc_file->read_proc = proc_read_id2;
	
	// port B configure
	s3c2410_gpio_cfgpin(S3C2410_GPB0, S3C2410_GPB0_OUTP);	//TXS Output
	s3c2410_gpio_cfgpin(S3C2410_GPB1, S3C2410_GPB1_INP);	//RXS Input
	s3c2410_gpio_cfgpin(S3C2410_GPB2, S3C2410_GPB2_OUTP);	//CKS Output
	s3c2410_gpio_cfgpin(S3C2410_GPB3, S3C2410_GPB3_OUTP);	//*DAC Output
	s3c2410_gpio_cfgpin(S3C2410_GPB4, S3C2410_GPB4_OUTP);	//*ADC Output
	
	s3c2410_gpio_cfgpin(S3C2410_GPB10, S3C2410_GPB10_INP);	//S3 Input (Factory Reset)

	s3c2410_gpio_pullup(S3C2410_GPB0, 0);	//Pull-up EN
	s3c2410_gpio_pullup(S3C2410_GPB1, 0);	//Pull-up EN	
	s3c2410_gpio_pullup(S3C2410_GPB2, 0);	//Pull-up EN	
	s3c2410_gpio_pullup(S3C2410_GPB3, 0);	//Pull-up EN	
	s3c2410_gpio_pullup(S3C2410_GPB4, 0);	//Pull-up EN	
	s3c2410_gpio_pullup(S3C2410_GPB10, 0);	//Pull-up EN	
	printk("[ADC] port B init\n");

	printk("[ADC] 2011-03-09 Version 1.4\n");
	
	return 0;
}


void __exit s3c2410_adc_exit( void )
{
	unregister_chrdev( MAJOR_NUMBER, DEV_NAME );
	kfree( gp_buffer );
	kfree( gp_txmsg );
	printk( "[ADC] exited\n");
}


module_init( s3c2410_adc_init );
module_exit( s3c2410_adc_exit );
MODULE_AUTHOR("jong2ry@imecasys.com");
MODULE_LICENSE("Dual BSD/GPL");

/* end */


/*-----------------------------------------------------------------------------
  files 	: net32_dev.h
  author	: jong2ry@imecasys.com
  date 		: 2011-03-15
-------------------------------------------------------------------------------*/
#ifndef _DEV_GPIO_H_
#define _DEV_GPIO_H_

/* define -------------------------------------------------------------------*/
#define		GPIO_MAJOR_DEF		218
#define		DEV_NAME			"S3C2410 GPIO Module ver 1.00"
#define 	VERSION_STR 		"Ver 1.00"

typedef struct
{
	int chk;
	unsigned long	size;
	unsigned short	info;
}__attribute__((packed)) dip_sw_info;

/* command of ioctl-function  -------------------------------------------------------*/
#define		GPIO_IOCTL_MAGIC	'h'

#define		IOCTL_GPIO_SET		_IOW( GPIO_IOCTL_MAGIC, 1, int )
#define		IOCTL_GPIO_CLR		_IOW( GPIO_IOCTL_MAGIC, 2, int )

#define		IOCTL_GPIO_A_ON		_IOW( GPIO_IOCTL_MAGIC, 1, int )
#define		IOCTL_GPIO_B_ON		_IOW( GPIO_IOCTL_MAGIC, 2, int )
#define		IOCTL_GPIO_C_ON		_IOW( GPIO_IOCTL_MAGIC, 3, int )
#define		IOCTL_GPIO_D_ON		_IOW( GPIO_IOCTL_MAGIC, 4, int )
#define		IOCTL_GPIO_E_ON		_IOW( GPIO_IOCTL_MAGIC, 5, int )
#define		IOCTL_GPIO_F_ON		_IOW( GPIO_IOCTL_MAGIC, 6, int )
#define		IOCTL_GPIO_G_ON		_IOW( GPIO_IOCTL_MAGIC, 7, int )
#define		IOCTL_GPIO_H_ON		_IOW( GPIO_IOCTL_MAGIC, 8, int )

#define		IOCTL_GPIO_A_OFF	_IOW( GPIO_IOCTL_MAGIC, 9, int )
#define		IOCTL_GPIO_B_OFF	_IOW( GPIO_IOCTL_MAGIC, 10, int )
#define		IOCTL_GPIO_C_OFF	_IOW( GPIO_IOCTL_MAGIC, 11, int )
#define		IOCTL_GPIO_D_OFF	_IOW( GPIO_IOCTL_MAGIC, 12, int )
#define		IOCTL_GPIO_E_OFF	_IOW( GPIO_IOCTL_MAGIC, 13, int )
#define		IOCTL_GPIO_F_OFF	_IOW( GPIO_IOCTL_MAGIC, 14, int )
#define		IOCTL_GPIO_G_OFF	_IOW( GPIO_IOCTL_MAGIC, 15, int )
#define		IOCTL_GPIO_H_OFF	_IOW( GPIO_IOCTL_MAGIC, 16, int )

#define		IOCTL_GET_DIP_SWITCH	_IOR( GPIO_IOCTL_MAGIC, 17, dip_sw_info )

#endif  // _DEV_GPIO_H_



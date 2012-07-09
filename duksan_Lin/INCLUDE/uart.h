#ifndef UART_MANAGER_H
#define UART_MANAGER_H

// define ============================================================

// GPIO Module =======================================================
#define		GPIO_MAJOR_DEF		218
#define		DEV_NAME			"S3C2410 GPIO Module ver 1.00"
#define		GPIO_IOCTL_MAGIC	'h'

// variable ==========================================================
int net32Fd;
int gpioFd;


#define UART2DEVICE "/dev/s3c2410_serial2"
#define UART1DEVICE "/dev/s3c2410_serial1"

#define UART1_TX_EN 	ioctl( gpioFd, _IOW( GPIO_IOCTL_MAGIC, 2, int ), 5 );
#define UART2_TX_EN 	ioctl( gpioFd, _IOW( GPIO_IOCTL_MAGIC, 2, int ), 6 );
#define UART1_TX_DIS 	ioctl( gpioFd, _IOW( GPIO_IOCTL_MAGIC, 10, int ), 5 );
#define UART2_TX_DIS 	ioctl( gpioFd, _IOW( GPIO_IOCTL_MAGIC, 10, int ), 6 );

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <asm/io.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define		GPIO_MAJOR_DEF		218
#define		DEV_NAME			"S3C2410 GPIO Module ver 1.00"
#define 	VERSION_STR 		"Ver 1.00"
#define		GPIO_IOCTL_MAGIC	'h'

#define		IOCTL_GET_DIP_SWITCH	_IOR( GPIO_IOCTL_MAGIC, 17, dip_sw_info )

typedef struct
{
	int chk;
	unsigned long	size;
	unsigned short	info;
}__attribute__((packed)) dip_sw_info;

int dbg_show = 0;
char text_in[4096];

/****************************************************************/	
static int	
dev_open( char *fname, unsigned char major, unsigned char minor )
/****************************************************************/
{
	int	dev;
	
	dev = open( fname, O_RDWR|O_NDELAY );
	if (dev == -1 )
	{
		mknod( fname, (S_IRWXU|S_IRWXG|S_IFCHR), (major<<8)|(minor) );
		
		dev = open( fname, O_RDWR|O_NDELAY );
		if (dev == -1 )
		{
			printf( " Device OPEN FAIL %s\n", fname );
			return -1;
		}
	}
        
	return dev;
}

/****************************************************************/
int main(void)
/****************************************************************/
{
	int i = 0;
	FILE *fp;
	FILE *fp_out;
	int dev_fd;
	dip_sw_info dip;	
	dip_sw_info pre_dip;	
	char index[2];
	int access_flag = 0;
	
	while(1)
	{	
		memset (text_in, 0x00, sizeof(text_in));
		dev_fd = dev_open("/dev/gpio", GPIO_MAJOR_DEF, 0 );
		
		for(;;) {
			dip.size = 0;
			ioctl( dev_fd, IOCTL_GET_DIP_SWITCH, &dip ); 
			
			if (pre_dip.info != dip.info) {
				access_flag = 0;
				memcpy(&pre_dip, &dip, sizeof(dip));
				//if (dbg_show) printf("\n\ndip.info = %d\n", dip.info);
				if((fp_out = fopen("/etc/rc.d/rc.M", "r+")) != NULL) {
					//if (dbg_show) printf("+ fp_out open\n");	
					sleep(1);
					continue;
				}
				
				// open file.
				if((fp = fopen("/etc/rc.d/rc.M", "r+")) != NULL) {
					//if (dbg_show) printf("+ fp open\n");	
					while (fgets(text_in, 256, fp) != NULL) {
						//printf("file read ok\n");	
						for(i = 0; i < 256; i++) {
							if (strncmp( &text_in[i], "'[PCM", 5) == NULL && access_flag == 0) {
								index[0] = (dip.info / 10) + 48;
								index[1] = (dip.info % 10) + 48;
								strncpy(&text_in[i+5], index, sizeof(index));
								//if (dbg_show) printf("change = %s\n", text_in);
								access_flag = 1;
							}						
						}
						fputs(text_in, fp_out);
						//printf("file write ok\n");	
					}
					fclose(fp);
					fclose(fp_out);	
					//if (dbg_show) 
					//	printf("- fp_out close\n");				
					//if (dbg_show)
					//	 printf("- fp close\n");	
					sleep(1);			
				}
			}
		}
		close( dev_fd );
		continue;
	}
	return 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <strings.h>
#include <netdb.h>

#define BAUDRATE B115200		// 임시로 적용. 실제로는 64000bps로 적용.
#define MODEMDEVICE "/dev/s3c2410_serial1"


void my_sleep(int sec,int usec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 usec		Is a micro-second value. 
// Returns     : none
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;                
    select(0,NULL,NULL,NULL,&tv);
    return;
}

/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	int fd;
	struct termios oldtio, newtio;

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0) { 
		fprintf(stderr, "\n[NET32] : Serial FD Open fail ");
		fflush(stderr);
		perror(MODEMDEVICE); 
		system("reboot");
	}
	
	if (tcgetattr(fd, &oldtio) < 0)
	{
		fprintf(stderr, "\n[NET32] : error in tcgetattr ");
		fflush(stderr);
		system("reboot");
	}
	
	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN]  = 1;
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	while(1) {
		my_sleep(5,0);	
	}
}




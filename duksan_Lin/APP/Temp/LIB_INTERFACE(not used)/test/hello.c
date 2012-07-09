#include <stdio.h>

extern void pset (int pcm, int pno, float val);
extern float pget (int pcm, int pno);
extern void InitUart2(int baudrate);

int do_interface(void)
{
	int i = 0;
	int pcm = 0;
	int pno = 0;
	int recv_length = 0;
	float val = 0;
	unsigned char buf[128];

	InitUart2(9600);
	//InitUart2(115200);

	while(1)
	{
		SendUart2("123456789012345678901234", 20);
		recv_length = RecvUart2(buf, 128);
		pset(1, 1, 1);
		sleep(1);
		//SendUart2("test uart2", 10);
		SendUart2("123456789012345678901234", 20);
		recv_length = RecvUart2(buf, 128);
		pset(1, 2, 1);
		sleep(1);
		//SendUart2("test uart2", 10);
		SendUart2("123456789012345678901234", 20);
		recv_length = RecvUart2(buf, 128);
		pset(1, 3, 1);
		sleep(1);
		//SendUart2("test uart2", 10);
		SendUart2("123456789012345678901234", 20);
		recv_length = RecvUart2(buf, 128);
		pset(1, 4, 1);
		sleep(1);
	}

	val = pget(1, 1);
	printf("val = %f\n", val);
	val = pget(1, 2);
	printf("val = %f\n", val);
	val = pget(1, 3);
	printf("val = %f\n", val);
	val = pget(1, 4);
	printf("val = %f\n", val);

	printf("short = %d\n", sizeof(unsigned short));
}




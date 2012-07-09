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
#include <pthread.h>
//

#define CMD_GATEWAY_IP				"gatewayip"
#define CMD_IP_ADDR					"ipaddr"

#define FIND_COMMAND				1
#define NOT_FIND_COMMAND			0

#define OPEN_OPTION_WR				1
#define OPEN_OPTION_W				2

#define COMMAND_COUNT				64
#define COMMAND_SIZE				64

typedef struct 
{
	char name[COMMAND_SIZE];
	char value[COMMAND_SIZE];	
}cmdinfo;

int dbg_show = 0;
cmdinfo pre_cmd[COMMAND_COUNT];

/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	int i = 0;
	int cnt = 0;
	FILE *fp = NULL;
	int status = 0;
	unsigned int ip = 0;
	unsigned int ip1 = 0;
	unsigned int ip2 = 0;
	char cmd[COMMAND_SIZE];
	char value[COMMAND_SIZE];
	char chIp[COMMAND_SIZE];
	int ipLength = 0;

	memset(&pre_cmd, 0x00, sizeof(pre_cmd));
	memset(cmd, 0x00, sizeof(cmd));
	memset(value, 0x00, sizeof(value));	
	memset(chIp, 0x00, sizeof(chIp));	

	// open file.
	if((fp = fopen("config.dat", "r")) == NULL)
	{
		if (fp != NULL)  fclose(fp);
		printf("[ERROR] File Open with Option 'r'\n");		
		if((fp = fopen("config.dat", "w")) == NULL)
		{
			if (fp != NULL)  fclose(fp);
			printf("[ERROR] File Open with Option 'w'\n");		
		}
		else
			status = OPEN_OPTION_W;
	}
	else
	{
		if (fp != NULL)  fclose(fp);
		if((fp = fopen("config.dat", "r+")) == NULL)
		{
			if (fp != NULL)  fclose(fp);
			printf("[ERROR] File Open with Option 'r+'\n");		
		}
		else
			status = OPEN_OPTION_WR;
	}

	if (status == OPEN_OPTION_W)
	{
		if (fp != NULL)  fclose(fp);
		return;		
	}
	else
	{
		// search file.
		while(!feof(fp))
		{
			memset(cmd, 0x00, sizeof(cmd));
			memset(value, 0x00, sizeof(value));
			fscanf(fp, "%s %s\n", &pre_cmd[cnt].name, &pre_cmd[cnt].value);	
			cnt++;
		}		
	}
	if (fp != NULL)  fclose(fp);

	// file re-open
	if((fp = fopen("configEth0", "w")) == NULL)
	{
		if (fp != NULL)  fclose(fp);
		printf("[ERROR] File Open with Option 'w'\n");		
	}
	else
	{
		for (i = 0; i < cnt; i++)
		{
			if (strncmp(&pre_cmd[i].name, "ipaddr", 5) == NULL)
				break;	
		}			
		printf("pre_cmd[%d].name = %s\n", i, pre_cmd[i].name);
		printf("pre_cmd[%d].value = %s\n", i, pre_cmd[i].value);
		ipLength = strlen(pre_cmd[i].value) - 1;
		//printf("ipLength= %d\n", ipLength);
		chIp[2] = pre_cmd[i].value[ipLength];
		chIp[1] = pre_cmd[i].value[ipLength - 1];
		chIp[0] = pre_cmd[i].value[ipLength - 2];
		//printf("chIp 0 = %s\n", chIp); 
		//printf("%c\n", chIp[0]); 
		//printf("%c\n", chIp[1]); 
		//printf("%c\n", chIp[2]); 

		if (chIp[0] == '.')
			chIp[0] = 0x30;		

		ip = ((chIp[0]  - 0x30) * 100) + ((chIp[1]  - 0x30) * 10) + (chIp[2]  - 0x30);
		printf("ip = %d\n", ip);
				
		// file write.
		if (ip >= 100)
		{
			ip1 = ip - 100;
			ip2 = 1;
		}
		else if (ip >= 200)
		{
			ip1 = ip - 200;
			ip2 = 2;
		}
		else
		{
			ip1 = ip;
			ip2 = 0;
		}
		
		fprintf(fp, "ifconfig eth0 down\n");
		fprintf(fp, "ifconfig eth0 hw ether 00:33:10:10:%02d:%02d\n", ip2,  ip1);
		fprintf(fp, "ifconfig eth0 %s\n", pre_cmd[i].value);
		fprintf(fp, "ifconfig eth0 up\n");
	}

	if (fp != NULL)  fclose(fp);
	return 0;
}

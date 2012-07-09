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
#include <sys/poll.h>		// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

/*******************************************************************/
// define
/*******************************************************************/
#define OPEN_OPTION_WR				1	// file open option
#define OPEN_OPTION_W				2

#define DATA_SIZE					32	// argument length

typedef struct {
	char argv[DATA_SIZE];
	char value[DATA_SIZE];
}cfg_info;

/*******************************************************************/
// variable
/*******************************************************************/
/*
char *g_pCfgTable[] = {		// file data table
	"ipaddr",
	"gatewayip",
	"stationip",
	"target",
	"net32",
	"bacnet",
	"subio" ,
	"apg",
	"ccms",
	"dnp",
	"light",
	"mdsvr",
	"mac"
};
*/
cfg_info g_temp[24];		// file data


/*******************************************************************/
static cfg_info *New_Cfg(void)
/*******************************************************************/
{
	cfg_info *p;

	p = malloc( sizeof(cfg_info) );
	memset( p, 0, sizeof(cfg_info) );
	return p;
}

/*
static int Search_CfgTable(cfg_info *p)
{
	int ret = 0;
	int table_count = 0;

	table_count = sizeof(g_pCfgTable) / sizeof(char *);
	//printf("table_count = %d\n", table_count);
	//printf("sizeof(g_pCfgTable) = %d\n", sizeof(g_pCfgTable));
	//printf("sizeof(char) = %d\n", sizeof(char *));

	for (ret = 0; ret < table_count; ret++) {
		if ( !strcmp(p->argv, g_pCfgTable[ret]) ) {
			//printf("string same\n");
			return ret;
		} 
	}

	return -1;
}
*/

/*******************************************************************/
static void Cfg_File_Handler(cfg_info *p)
/*******************************************************************/
{
	int i = 0;
	int cnt = 0;
	FILE *fp = NULL;
	int status = 0;

	memset( &g_temp, 0, sizeof(g_temp) );

	// open file.
	if((fp = fopen("/duksan/CONFIG/config.dat", "r")) == NULL)	{
		printf("[ERROR] File Open with Option 'r'\n");		
		if (fp != NULL)  
			fclose(fp);

		if((fp = fopen("/duksan/CONFIG/config.dat", "w")) == NULL)	{
			if (fp != NULL)  
				fclose(fp);
			printf("[ERROR] File Open with Option 'w'\n");		
			return;
		}
		else
			status = OPEN_OPTION_W;
	}
	else {
		if (fp != NULL)  
			fclose(fp);

		if((fp = fopen("/duksan/CONFIG/config.dat", "r+")) == NULL) {
			if (fp != NULL)  
				fclose(fp);
			printf("[ERROR] File Open with Option 'r+'\n");	
			return;	
		}
		else
			status = OPEN_OPTION_WR;
	}

	// scan file or write value
	if (status == OPEN_OPTION_W) {
		fprintf(fp,"%s %s\n", p->argv, p->value); 
		return;		
	}
	else {		// scan file.
		cnt = 0;
		while(!feof(fp)) {
			fscanf(fp, "%s %s\n", (char *)&g_temp[cnt].argv, (char *)&g_temp[cnt].value);	
			cnt++;
		}
	}

	if (fp != NULL)  
		fclose(fp);

	// file re-open and write value
	if((fp = fopen("/duksan/CONFIG/config.dat", "w")) == NULL) {
		if (fp != NULL)  
			fclose(fp);
		printf("[ERROR] File Open with Option 'w'\n");
		return;		
	}
	else {
		for (i = 0; i < cnt; i++) {
			if ( !strcmp(p->argv, g_temp[i].argv) ) 
				fprintf(fp,"%s %s\n", p->argv, p->value); 		
			else 
				fprintf(fp,"%s %s\n", g_temp[i].argv, g_temp[i].value); 		
		} 
	}

	if (fp != NULL)  
		fclose(fp);
	return;
}


/*******************************************************************/
static void Network_File_Handler(cfg_info *p)
/*******************************************************************/
{
	int i = 0;
//	int j = 0;
	int cnt = 0;
//	int token = 0;
//	char ip[3];
	FILE *fp = NULL;

	if((fp = fopen("/duksan/CONFIG/config.dat", "r")) == NULL) {
		printf("[ERROR] File Open with Option 'r'\n");
		return;		
	}
	else {
		cnt = 0;
		while(!feof(fp)) {
			fscanf(fp, "%s %s\n", (char *)&g_temp[cnt].argv, (char *)&g_temp[cnt].value);	
			cnt++;
		}
		fclose(fp);
	}

	// Write IP Address
	if((fp = fopen("/duksan/CONFIG/Eth0Ip.sh", "w")) == NULL) {
		printf("[ERROR] Eth0Ip.sh File Open with Option 'w'\n");
		return;		
	}
	else {
		for (i = 0; i < cnt; i++) {
			if ( !strcmp(g_temp[i].argv, "ipaddr") ) 
				fprintf(fp,"ifconfig eth0 %s\n", g_temp[i].value);
		} 
		fclose(fp);
		fp = NULL;
	}

	// Write Gateway IP Address
	if((fp = fopen("/duksan/CONFIG/Eth0Gw.sh", "w")) == NULL) {
		printf("[ERROR] Eth0Gw.sh File Open with Option 'w'\n");
		return;		
	}
	else {
		for (i = 0; i < cnt; i++) {
			if ( !strcmp(g_temp[i].argv, "gatewayip") ) 
				fprintf(fp,"route add default gw %s dev eth0\n", g_temp[i].value);
		} 
		fclose(fp);
		fp = NULL;
	}

	if ( strcmp(p->argv, "mac") ) 
		return;

	// Write MAC Address
	if((fp = fopen("/duksan/CONFIG/Eth0Mac.sh", "w")) == NULL) {
		printf("[ERROR] Eth0Mac.sh File Open with Option 'w'\n");
		return;		
	}
	else {
		if ( !strcmp(p->argv, "mac") ) 
			fprintf(fp,"ifconfig eth0 hw ether %s\n", 
					p->value);
		fclose(fp);
		fp = NULL;
	}
}

/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	cfg_info *pCfg;
	//int ret = 0;

	if ( argc != 3 ) {
		printf("Wrong argument (argc = %d)\n", argc);
		return -1;
	}
	
	pCfg = New_Cfg();
	
	if ( strlen(argv[1]) > DATA_SIZE ) {
		printf("Wrong argv[1] length\n");
		goto out;
	}

	if ( strlen(argv[2]) > DATA_SIZE ) {
		printf("Wrong argv[2] length\n");
		goto out;
	}

	memcpy (&pCfg->argv, argv[1], strlen(argv[1]) );
	memcpy (&pCfg->value, argv[2], strlen(argv[2]) );

	printf("[Command] %s %s\n", pCfg->argv, pCfg->value);
	/*
	ret = Search_CfgTable(pCfg);
	
	if ( ret < 0 ) {
		printf("Wrong argv[1]\n");
		goto out;
	}
	*/

	Cfg_File_Handler(pCfg);
	Network_File_Handler(pCfg);

out:
	free(pCfg);
	return 1;
}




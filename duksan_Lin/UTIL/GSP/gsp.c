
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
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>		// use poll event


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "cfg_mgr.h"										// elba manager

#define MAX_BUFFER_SIZE					4096

#define CFG_VALUE_SIZE					16			// config.dat value size
#define CFG_COMMAND_SIZE				32			// config.dat command size
#define CFG_COMMAND_COUNT				16			// config.dat command count

#define	MSG_FINDTARGET			1
#define MSG_SETTARGET			2


typedef struct {
	int fd;
	unsigned char *txbuf;
	unsigned char *rxbuf;
	unsigned char *tempbuf;
	int recvLength;
	int tempWp;
	int bufSize;
	int ntime_request;
	char targetIp[CFG_VALUE_SIZE];
}UDP_CFG_T;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// MFC로 생성된 GSP 프로그램에서 
// 사용되는 구조체와 동일한 포멧을 가져야 한다. 
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
typedef struct __attribute__ ( (__packed__)) {
	unsigned char stx;
	unsigned char mac[6];
	unsigned char ip[4];
	unsigned char gw[4];
	unsigned char subnet[4];
	unsigned char server[4];
	unsigned char used_3is;
	unsigned char used_Net32;
	unsigned char used_Apg;
	unsigned char used_Subio;
	unsigned char used_MODBUS;
	unsigned char used_BACnet;
	unsigned char used_IFace_Light;
	unsigned char used_CCMS;
	unsigned char used_SMS;
	unsigned char rel_kernel_num;
	unsigned char rel_app_num;
	unsigned char dummy05;
	unsigned char dummy06;
	unsigned char dummy07;
	unsigned char dummy08;
	unsigned char dummy09;
	unsigned char etx;
}UDP_CFG_MSG_T;

#define DATA_SIZE					32	// argument length

typedef struct {
	char argv[DATA_SIZE];
	char value[DATA_SIZE];
}cfg_info;

cfg_info g_temp[10];		// file data


unsigned char gc_cfg_rx_msg[MAX_BUFFER_SIZE];
unsigned char gc_cfg_tx_msg[MAX_BUFFER_SIZE];
unsigned char gc_cfg_tmp_msg[MAX_BUFFER_SIZE];

int g_nFindMsgFlag = 0;


void cfg_sleep(int sec, int usec) 
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = usec;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


UDP_CFG_T *new_cfg(void)
{
	UDP_CFG_T *pCfg;
	
	pCfg = (UDP_CFG_T *)malloc( sizeof(UDP_CFG_T) );
	
	pCfg->fd = -1;
	pCfg->rxbuf = (unsigned char *)&gc_cfg_rx_msg;
	pCfg->txbuf = (unsigned char *)&gc_cfg_tx_msg;
	pCfg->tempbuf = (unsigned char *)&gc_cfg_tmp_msg;
	pCfg->tempWp = 0;
	pCfg->recvLength = 0;
	pCfg->bufSize = MAX_BUFFER_SIZE;

	return pCfg;
}


void cfg_close(UDP_CFG_T *p)
{
	close(p->fd);

	p->fd = -1;
	p->tempWp = 0;
	p->recvLength = 0;

	memset( p->tempbuf, 0, MAX_BUFFER_SIZE );
	memset( p->rxbuf, 0, MAX_BUFFER_SIZE );
	memset( p->txbuf, 0, MAX_BUFFER_SIZE );
}


void cfg_SendMsg()
{
	int s, addrsize;

	struct sockaddr_in server_addr;

	char buf[1024];

	if((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0){
		printf("can`t create socket\n");
		exit(0);
	}


	/* echo 서버의 소켓주소 구조체 작성 */
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
	server_addr.sin_port = htons(9602);


	/*구조체의 크기를 구한다 */
	addrsize = sizeof(struct sockaddr);

	/* echo 서버로 메세지 송신 */
	if(sendto(s, buf, 32, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))<0) {
		printf("sendto error!\n");
		exit(0);
	}
}


int cfg_ParsingMsg(UDP_CFG_T *p)
{
	int i = 0;
	int nRecvLength = 0;
	unsigned char *pBuf;

	fprintf( stdout, "+ RecvLength = %d \n", p->recvLength  );
	fflush( stdout );		

	pBuf = (unsigned char *)p->rxbuf;
	nRecvLength = p->recvLength;

	for ( i = 0; i < nRecvLength; i++ ) {
		fprintf( stdout, "%x ", pBuf[i] );
	}
	fprintf( stdout, "\n");
	fflush( stdout );

	if ( pBuf[0] == '<' && pBuf[nRecvLength - 1] == '>'  ) {

		// get find target message
		if ( strncmp(&pBuf[1], "FindTarget", 10) == 0 ) {
			fprintf( stdout, "+ Get FindTarget Msg OK \n"  );
			fflush( stdout );			
			return MSG_FINDTARGET;	
		}
	}

	if ( p->recvLength == sizeof(UDP_CFG_MSG_T) ) {
		return MSG_SETTARGET;
	}

	return -1;
}


void cfg_GetIp(unsigned char *pBuf, UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// FIND IP ADDRESS
// Description		: Ip address 얻는다. 
// Arguments		: pBuf			temp-buffer
// 					  p				UDP_CFG_T pointer
// Returns			: none
//
{
	int i = 0;
	char *pToken;
	int nStrLen = 0;
	unsigned char temp[64];
	char delims[] = ".\n";
	int nCount = 0;
	unsigned char tempIp[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;

	nStrLen = strlen(pBuf);
	memset(temp, 0x00, sizeof(temp) );
	memset(tempIp, 0x00, sizeof(tempIp) );
	memcpy(temp, pBuf, nStrLen);

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempIp[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 4 ) 
		return;

	for ( i = 0; i < nCount; i++ ) 
		pPkt->ip[i] = tempIp[i];
}


void cfg_GetSubnet(unsigned char *pBuf, UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// FIND SUBNET MASK
// Description		: Subnet mask 얻는다. 
// Arguments		: pBuf			temp-buffer
// 					  p				UDP_CFG_T pointer
// Returns			: none
//
{
	int i = 0;
	char *pToken;
	int nStrLen = 0;
	unsigned char temp[64];
	char delims[] = ".\n";
	int nCount = 0;
	unsigned char tempSubnet[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;

	nStrLen = strlen(pBuf);
	memset(temp, 0x00, sizeof(temp) );
	memset(tempSubnet, 0x00, sizeof(tempSubnet) );
	memcpy(temp, pBuf, nStrLen);

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempSubnet[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 4 ) 
		return;

	for ( i = 0; i < nCount; i++ ) 
		pPkt->subnet[i] = tempSubnet[i];
}



void cfg_GetMac(unsigned char *pBuf, UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// FIND MAC ADDRESS
// Description		: MAC address를 얻는다. 
// Arguments		: pBuf			temp-buffer
// 					  p				UDP_CFG_T pointer
// Returns			: none
//  
{
	int i = 0;
	char *pToken;
	int nStrLen = 0;
	unsigned char temp[64];
	char delims[] = ":\n";
	int nCount = 0;
	unsigned char tempMac[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;

	nStrLen = strlen(pBuf);
	memset(temp, 0x00, sizeof(temp) );
	memset(tempMac, 0x00, sizeof(tempMac) );
	memcpy(temp, pBuf, nStrLen);

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempMac[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 6 ) 
		return;

	for ( i = 0; i < nCount; i++ ) 
		pPkt->mac[i] = tempMac[i];
}


void cfg_GetServer(unsigned char *pBuf, UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// FIND SERVER IP ADDRESS
// Description		: Server ip address 얻는다. 
// Arguments		: pBuf			temp-buffer
// 					  p				UDP_CFG_T pointer
// Returns			: none
//
{
	int i = 0;
	char *pToken;
	int nStrLen = 0;
	unsigned char temp[64];
	char delims[] = ".\n";
	int nCount = 0;
	unsigned char tempServer[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;

	nStrLen = strlen(pBuf);
	memset(temp, 0x00, sizeof(temp) );
	memset(tempServer, 0x00, sizeof(tempServer) );
	memcpy(temp, pBuf, nStrLen);

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempServer[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 4 ) 
		return;

	for ( i = 0; i < nCount; i++ ) 
		pPkt->server[i] = tempServer[i];
}


void cfg_GetGateway(unsigned char *pBuf, UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// FIND GATEWAY IP ADDRESS
// Description		: Gateway ip address 얻는다. 
// Arguments		: pBuf			temp-buffer
// 					  p				UDP_CFG_T pointer
// Returns			: none
//
{
	int i = 0;
	char *pToken;
	int nStrLen = 0;
	unsigned char temp[64];
	char delims[] = ".\n";
	int nCount = 0;
	unsigned char tempGw[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;

	nStrLen = strlen(pBuf);
	memset(temp, 0x00, sizeof(temp) );
	memset(tempGw, 0x00, sizeof(tempGw) );
	memcpy(temp, pBuf, nStrLen);

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempGw[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 4 ) 
		return;

	for ( i = 0; i < nCount; i++ ) 
		pPkt->gw[i] = tempGw[i];
}



void cfg_GetMyIfc(UDP_CFG_T *p)
{
    // 이더넷 데이터 구조체  
    struct ifreq *ifr; 
    struct sockaddr_in *sin; 
    struct sockaddr *sa; 
 
    // 이더넷 설정 구조체 
    struct ifconf ifcfg; 
    int fd; 
    int n; 
    int numreqs = 30; 

	unsigned char temp[64];


	fd = socket(AF_INET, SOCK_DGRAM, 0); 
 
    // 이더넷 설정정보를 가지고오기 위해서  
    // 설정 구조체를 초기화하고   
    // ifreq데이터는 ifc_buf에 저장되며,  
    // 네트워크 장치가 여러개 있을 수 있으므로 크기를 충분히 잡아주어야 한다.   
    // 보통은 루프백주소와 하나의 이더넷카드, 2개의 장치를 가진다. 
    memset(&ifcfg, 0, sizeof(ifcfg)); 
    ifcfg.ifc_buf = NULL; 
    ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
    ifcfg.ifc_buf = malloc(ifcfg.ifc_len); 
 
    for(;;) 
    { 
        ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
        ifcfg.ifc_buf = realloc(ifcfg.ifc_buf, ifcfg.ifc_len); 
        if (ioctl(fd, SIOCGIFCONF, (char *)&ifcfg) < 0) 
        { 
            perror("SIOCGIFCONF "); 
            return; 
        } 
        // 디버깅 메시지 ifcfg.ifc_len/sizeof(struct ifreq)로 네트워크  
        // 장치의 수를 계산할 수 있다.   
        // 물론 ioctl을 통해서도 구할 수 있는데 그건 각자 해보기 바란다. 
        //printf("%d : %d \n", ifcfg.ifc_len, sizeof(struct ifreq)); 
        break; 
    } 
 
    // 주소를 비교해 보자.. ifcfg.ifc_req는 ifcfg.ifc_buf를 가리키고 있음을  
    // 알 수 있다.  
    //printf("address %d\n", &ifcfg.ifc_req); 
    //printf("address %d\n", &ifcfg.ifc_buf); 
 
    // 네트워크 장치의 정보를 얻어온다.   
    // 보통 루프백과 하나의 이더넷 카드를 가지고 있을 것이므로  
    // 2개의 정보를 출력할 것이다.  
    ifr = ifcfg.ifc_req; 
    for (n = 0; n < ifcfg.ifc_len; n+= sizeof(struct ifreq)) 
    { 
		//printf("[%s]\n", ifr->ifr_name); 

		if ( strncmp((char *)&ifr->ifr_name, "eth0", 4) != 0 ) {
			ifr++; 
			continue;
		}

		// 주소값을 출력하고 루프백 주소인지 확인한다. 
        //printf("[%s]\n", ifr->ifr_name); 
        sin = (struct sockaddr_in *)&ifr->ifr_addr; 
        //printf("IP    %s [%d]\n", inet_ntoa(sin->sin_addr), strlen(inet_ntoa(sin->sin_addr)) ); 
		memset(temp, 0x00, sizeof(temp));
		sprintf(temp, "%s", inet_ntoa(sin->sin_addr));
		//printf("IP >> %s\n", temp);

		cfg_GetIp(temp, p);
        if ( (sin->sin_addr.s_addr) == INADDR_LOOPBACK)	{ 
            printf("Loop Back\n"); 
        } 
        else { 
            // 루프백장치가 아니라면 MAC을 출력한다. 
            ioctl(fd, SIOCGIFHWADDR, (char *)ifr); 
            sa = &ifr->ifr_hwaddr; 
            //printf("MAC %s [%d]\n", ether_ntoa((struct ether_addr *)sa->sa_data), strlen(ether_ntoa((struct ether_addr *)sa->sa_data))); 
			memset(temp, 0x00, sizeof(temp));
			sprintf(temp, "%s", ether_ntoa((struct ether_addr *)sa->sa_data));
			cfg_GetMac(temp, p);
        } 

		// 네트워크 마스팅 주소 
		ioctl(fd, SIOCGIFNETMASK, (char *)ifr); 
		sin = (struct sockaddr_in *)&ifr->ifr_addr; 
		//printf("MASK  %s [%d]\n", inet_ntoa(sin->sin_addr), strlen(inet_ntoa(sin->sin_addr))); 
		memset(temp, 0x00, sizeof(temp));
		sprintf(temp, "%s", inet_ntoa(sin->sin_addr));
		cfg_GetSubnet(temp, p);


        // 브로드 캐스팅 주소  
        //ioctl(fd,  SIOCGIFBRDADDR, (char *)ifr); 
        //sin = (struct sockaddr_in *)&ifr->ifr_broadaddr; 
        //printf("BROD  %s [%d]\n", inet_ntoa(sin->sin_addr), strlen(inet_ntoa(sin->sin_addr)) ); 
		// 네트워크 마스팅 주소 
        //ioctl(fd, SIOCGIFNETMASK, (char *)ifr); 
        //sin = (struct sockaddr_in *)&ifr->ifr_addr; 
        //printf("MASK  %s [%d]\n", inet_ntoa(sin->sin_addr), strlen(inet_ntoa(sin->sin_addr))); 
        // MTU값 
        //ioctl(fd, SIOCGIFMTU, (char *)ifr); 
        //printf("MTU   %d\n", ifr->ifr_mtu,); 
        //printf("\n"); 

        ifr++; 
    } 
}


void cfg_GetMySetting(UDP_CFG_T *p)
{
	int cnt = 0;
	FILE *fp = NULL;
	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->txbuf;
	unsigned char temp[64];
	unsigned char bufVersion[8];


	if ( (fp = fopen("/duksan/CONFIG/config.dat", "r")) == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		return;		
	}
	else {
		cnt = 0;
		while(!feof(fp)) {
			fscanf(fp, "%s %s\n", (char *)&g_temp[cnt].argv, (char *)&g_temp[cnt].value);	
			//printf("Setting %s  %s  \n", (char *)&g_temp[cnt].argv, (char *)&g_temp[cnt].value); 

			if ( !strcmp(g_temp[cnt].argv, "gatewayip") ) {
				memset(temp, 0x00, sizeof(temp));
				sprintf(temp, "%s", (char *)&g_temp[cnt].value);
				//printf("string same gatewayip %s\n", temp);
				cfg_GetGateway(temp, p);
			} 

			if ( !strcmp(g_temp[cnt].argv, "stationip") ) {
				memset(temp, 0x00, sizeof(temp));
				sprintf(temp, "%s", (char *)&g_temp[cnt].value);
				//printf("string same stationip %s\n", temp);
				cfg_GetServer(temp, p);
			} 

			if ( !strcmp(g_temp[cnt].argv, "net32") ) {
				//printf("string same net32 \n");
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_Net32 = 1;
				else
					pPkt->used_Net32 = 0;	
			} 

			if ( !strcmp(g_temp[cnt].argv, "target") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_3is = 1;
				else
					pPkt->used_3is = 0;	
			} 

			if ( !strcmp(g_temp[cnt].argv, "bacnet") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_BACnet = 1;
				else
					pPkt->used_BACnet = 0;	
			} 

			if ( !strcmp(g_temp[cnt].argv, "light") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_IFace_Light = 1;
				else
					pPkt->used_IFace_Light = 0;	
			} 
			
			if ( !strcmp(g_temp[cnt].argv, "apg") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_Apg = 1;
				else
					pPkt->used_Apg = 0;	
			} 
			
			if ( !strcmp(g_temp[cnt].argv, "subio") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_Subio = 1;
				else
					pPkt->used_Subio = 0;	
			} 
			
			if ( !strcmp(g_temp[cnt].argv, "mdsvr") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_MODBUS = 1;
				else
					pPkt->used_MODBUS = 0;	
			} 
			
			if ( !strcmp(g_temp[cnt].argv, "ccms") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_CCMS = 1;
				else
					pPkt->used_CCMS = 0;	
			} 

			if ( !strcmp(g_temp[cnt].argv, "sms") ) {
				if ( !strcmp(g_temp[cnt].value, "on") )
					pPkt->used_SMS = 1;
				else
					pPkt->used_SMS = 0;	
			} 
						
			cnt++;
		}
		fclose(fp);
	}
	
	// Get kernel version
	if ( (fp = fopen("/duksan/FILE/kernel_version", "r")) == NULL ) {
		printf("[GSP] 'kernel_version' File Open Error\n");
		pPkt->rel_kernel_num = 0;
	}
	else {
		while(!feof(fp)) {
			fscanf(fp, "%s", bufVersion);	
		}	
		
		pPkt->rel_kernel_num = atoi(bufVersion);
		fclose(fp);		
	}	

	// Get application version
	if ( (fp = fopen("/duksan/FILE/app_version", "r")) == NULL ) {
		printf("[GSP] 'app_version' File Open Error\n");
		pPkt->rel_app_num = 0;
	}
	else {
		while(!feof(fp)) {
			fscanf(fp, "%s", bufVersion);	
		}	
				
		pPkt->rel_app_num = atoi(bufVersion);
		fclose(fp);
	}		
}




int cfg_CheckMyMac(UDP_CFG_T *p)
{
    // 이더넷 데이터 구조체  
    struct ifreq *ifr; 
    struct sockaddr_in *sin; 
    struct sockaddr *sa; 
 
    // 이더넷 설정 구조체 
    struct ifconf ifcfg; 
    int fd; 
    int n; 
    int numreqs = 30; 

	unsigned char temp[64];

	int i = 0;
	char *pToken;
	//int nStrLen = 0;
	char delims[] = ":\n";
	int nCount = 0;
	unsigned char tempMac[32];
	unsigned int nTempVal = 0;

	// packet structure
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->rxbuf;


	fd = socket(AF_INET, SOCK_DGRAM, 0); 
 
    // 이더넷 설정정보를 가지고오기 위해서  
    // 설정 구조체를 초기화하고   
    // ifreq데이터는 ifc_buf에 저장되며,  
    // 네트워크 장치가 여러개 있을 수 있으므로 크기를 충분히 잡아주어야 한다.   
    // 보통은 루프백주소와 하나의 이더넷카드, 2개의 장치를 가진다. 
    memset(&ifcfg, 0, sizeof(ifcfg)); 
    ifcfg.ifc_buf = NULL; 
    ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
    ifcfg.ifc_buf = malloc(ifcfg.ifc_len); 
 
    for (;;) { 
        ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
        ifcfg.ifc_buf = realloc(ifcfg.ifc_buf, ifcfg.ifc_len); 
        if (ioctl(fd, SIOCGIFCONF, (char *)&ifcfg) < 0)	{ 
            perror("SIOCGIFCONF "); 
            return -1; 
        } 

        break; 
    } 
  
    // 네트워크 장치의 정보를 얻어온다.   
    // 보통 루프백과 하나의 이더넷 카드를 가지고 있을 것이므로  
    // 2개의 정보를 출력할 것이다.  
    ifr = ifcfg.ifc_req; 
    for (n = 0; n < ifcfg.ifc_len; n+= sizeof(struct ifreq)) { 

		if ( strncmp((char *)&ifr->ifr_name, "eth0", 4) != 0 ) {
			ifr++; 
			continue;
		}
        if ( (sin->sin_addr.s_addr) == INADDR_LOOPBACK)	{ 
            printf("Loop Back\n"); 
        } 
        else { 
            // 루프백장치가 아니라면 MAC을 출력한다. 
            ioctl(fd, SIOCGIFHWADDR, (char *)ifr); 
            sa = &ifr->ifr_hwaddr; 
            printf("MAC %s [%d]\n", ether_ntoa((struct ether_addr *)sa->sa_data), strlen(ether_ntoa((struct ether_addr *)sa->sa_data))); 
			memset(temp, 0x00, sizeof(temp));
			sprintf(temp, "%s", ether_ntoa((struct ether_addr *)sa->sa_data));
        } 
		ifr++; 
    } 

	memset(tempMac, 0x00, sizeof(tempMac) );

	nCount = 0;
	pToken = strtok(temp, delims);
	while( pToken != NULL ) {
		nTempVal = (unsigned int)atoi(pToken);
		tempMac[nCount] = (unsigned char)nTempVal;
		pToken = strtok(NULL, delims);
		nCount++;
	}

	if ( nCount > 6 ) 
		return -1;

	for ( i = 0; i < nCount; i++ ) {
		//printf("%x  ::  %x\n", pPkt->mac[i] , tempMac[i]); 
		if ( pPkt->mac[i] != tempMac[i] ) {
			return -1;
		}
	}

	return 1;
}


void cfg_SetIfc(UDP_CFG_T *p)
{
	int i = 0;
	int cnt = 0;
	FILE *fp = NULL;
	UDP_CFG_MSG_T *pPkt;
	pPkt = (UDP_CFG_MSG_T *)p->rxbuf;
	//unsigned char temp[64];

	if ( (fp = fopen("/duksan/CONFIG/config.dat", "r")) == NULL ) {
		printf("[ERROR] File Open with Option 'r'\n");
		return;		
	}
	else {
		cnt = 0;
		while(!feof(fp)) {
			fscanf(fp, "%s %s\n", (char *)&g_temp[cnt].argv, (char *)&g_temp[cnt].value);	

			if ( !strcmp(g_temp[cnt].argv, "ipaddr") ) {
				memset((char *)&g_temp[cnt].value, 0x00, sizeof(g_temp[cnt].value));
				printf("IP %d.%d.%d.%d\n", pPkt->ip[0], pPkt->ip[1], pPkt->ip[2], pPkt->ip[3] ); 
				sprintf((char *)&g_temp[cnt].value, "%d.%d.%d.%d", pPkt->ip[0], pPkt->ip[1], pPkt->ip[2], pPkt->ip[3] ); 
			} 

			if ( !strcmp(g_temp[cnt].argv, "gatewayip") ) {
				memset((char *)&g_temp[cnt].value, 0x00, sizeof(g_temp[cnt].value));
				printf("GWIP %d.%d.%d.%d\n", pPkt->gw[0], pPkt->gw[1], pPkt->gw[2], pPkt->gw[3] ); 
				sprintf((char *)&g_temp[cnt].value, "%d.%d.%d.%d", pPkt->gw[0], pPkt->gw[1], pPkt->gw[2], pPkt->gw[3] ); 
			} 

			if ( !strcmp(g_temp[cnt].argv, "stationip") ) {
				memset((char *)&g_temp[cnt].value, 0x00, sizeof(g_temp[cnt].value));
				printf("SERVER %d.%d.%d.%d\n", pPkt->server[0], pPkt->server[1], pPkt->server[2], pPkt->server[3] ); 
				sprintf((char *)&g_temp[cnt].value, "%d.%d.%d.%d", pPkt->server[0], pPkt->server[1], pPkt->server[2], pPkt->server[3] ); 
			} 

			if ( !strcmp(g_temp[cnt].argv, "net32") ) {
				memset((char *)&g_temp[cnt].value, 0x00, sizeof(g_temp[cnt].value));
				if ( pPkt->used_Net32 > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 

			if ( !strcmp(g_temp[cnt].argv, "target") ) {
				if ( pPkt->used_3is > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 
			
			if ( !strcmp(g_temp[cnt].argv, "bacnet") ) {
				if ( pPkt->used_BACnet > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 		

			if ( !strcmp(g_temp[cnt].argv, "light") ) {
				if ( pPkt->used_IFace_Light > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 	

			if ( !strcmp(g_temp[cnt].argv, "apg") ) {
				if ( pPkt->used_Apg > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 								


			if ( !strcmp(g_temp[cnt].argv, "subio") ) {
				if ( pPkt->used_Subio > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 					

			if ( !strcmp(g_temp[cnt].argv, "mdsvr") ) {
				if ( pPkt->used_MODBUS > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 				

			if ( !strcmp(g_temp[cnt].argv, "ccms") ) {
				if ( pPkt->used_CCMS > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 				
			
			if ( !strcmp(g_temp[cnt].argv, "sms") ) {
				if ( pPkt->used_SMS > 0  ) 
					memcpy( (char *)&g_temp[cnt].value, "on", sizeof("on") );
				else
					memcpy( (char *)&g_temp[cnt].value, "off", sizeof("off") );
			} 							
			cnt++;
		}
		fclose(fp);
	}
	
	// file re-open and write value
	if((fp = fopen("/duksan/CONFIG/config.dat", "w")) == NULL) {
		if (fp != NULL)  
			fclose(fp);
		printf("[ERROR] File Open with Option 'w'\n");
		return;		
	}
	else {
		for (i = 0; i < cnt; i++) 
			fprintf(fp,"%s %s\n", g_temp[i].argv, g_temp[i].value); 		
	}		
	fclose(fp);


	// Write IP Address
	if((fp = fopen("/duksan/CONFIG/Eth0Ip.sh", "w")) == NULL) {
		printf("[ERROR] Eth0Ip.sh File Open with Option 'w'\n");
		return;		
	}
	else {
		fprintf(fp,"ifconfig eth0 %d.%d.%d.%d\n", 
				pPkt->ip[0], pPkt->ip[1], pPkt->ip[2], pPkt->ip[3] );
		fclose(fp);
		fp = NULL;
	}

	// Write Gateway IP Address
	if((fp = fopen("/duksan/CONFIG/Eth0Gw.sh", "w")) == NULL) {
		printf("[ERROR] Eth0Gw.sh File Open with Option 'w'\n");
		return;		
	}
	else {
		fprintf(fp,"route add default gw %d.%d.%d.%d dev eth0\n", 
				pPkt->gw[0], pPkt->gw[1], pPkt->gw[2], pPkt->gw[3]);
		fclose(fp);
		fp = NULL;
	}

	chdir("/duksan/CONFIG");
	system("./network.sh");
}

int cfg_SendPacket(UDP_CFG_T *p)
{
	int ret = 0;
	int one = 1;
	int send_sock;
	struct sockaddr_in broad_addr;

	send_sock = socket( PF_INET, SOCK_DGRAM, 0 );

	memset( &broad_addr, 0, sizeof(broad_addr) );
	broad_addr.sin_family = AF_INET;
	broad_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
	broad_addr.sin_port = htons( 9602 );

	if ( setsockopt(send_sock, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one) ) < 0) {
		fprintf( stdout, "+ Socket Error\n" );
		fflush( stdout );
		return -1;
	}

	ret = sendto(send_sock, p->txbuf, sizeof(UDP_CFG_MSG_T), 0,
		   (struct sockaddr *)&broad_addr, sizeof(broad_addr));

	close(send_sock);

	return ret;
}


int cfg_RecvPacket(UDP_CFG_T *p)
// ----------------------------------------------------------------------------
// RECEIVE UDP PACKET MESSAGE
// Description		: GSP 프로그램으로부터 오는 Message를 Parsing한다. 
// Arguments		: arg		argument
// Returns			: exit(1)
//   
{
	//int i = 0;
	int ret = 0;
	int one = 1;
	struct timeval timeo;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	//char message[1024];
	int str_len = 0;
	int clnt_addr_size = 0;
	UDP_CFG_MSG_T *pMsg;
	int nRetVal = 0;
	int nChkMac = 0;

	p->fd = socket( PF_INET, SOCK_DGRAM, 0 );
	if( p->fd < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( p->fd );
		return -1;					
	}

	memset( &serv_addr, 0, sizeof(serv_addr) );
	timeo.tv_sec = 0;
	timeo.tv_usec = 10000;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons( 9602 );

	if ( setsockopt(p->fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one) ) < 0) {
		fprintf( stdout, "+ Socket Error\n" );
		fflush( stdout );
		return 1;
	}

	if ( bind( p->fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1 ) {
		fprintf( stdout, "+ Socket bind error\n" );
		fflush( stdout );
		close( p->fd );
		return -1;
	}
		
	// Receive Packet
	for ( ;; ) {
		clnt_addr_size = sizeof(clnt_addr);
		str_len = recvfrom(p->fd, p->rxbuf, 1024, 0,
						   (struct sockaddr *)&clnt_addr, &clnt_addr_size);
		p->recvLength = str_len;

		if ( str_len > 0) {
			pMsg = (UDP_CFG_MSG_T *)p->txbuf;

			fprintf( stdout, "+ str_len = %d\n", str_len );
			fflush( stdout );
			nRetVal = cfg_ParsingMsg(p);

			switch( nRetVal ) {
			case MSG_FINDTARGET:
				fprintf( stdout, "+ MSG_FINDTARGET\n" );
				cfg_GetMyIfc(p);
				cfg_GetMySetting(p);
				ret = cfg_SendPacket(p);
				
				// Find Message를 받고 나서 자신의 정보를 UDP로 쏘면,
				// receive를 했을 때, Data를 다시 받아 오기 때문에 
				// Flag를 사용해서 막는다. 
				g_nFindMsgFlag = 1;
				
				/*
				ret = sendto(p->fd, p->txbuf, sizeof(UDP_CFG_MSG_T), 0,
					   (struct sockaddr *)&clnt_addr, clnt_addr_size);
				*/
				break;

			case MSG_SETTARGET:
				fprintf( stdout, "+ MSG_SETTARGET\n" );
				nChkMac = cfg_CheckMyMac(p);
				if ( nChkMac > 0 ) {
					fprintf( stdout, "+ g_nFindMsgFlag = %d\n", g_nFindMsgFlag );
					fflush( stdout );
					
					// Find Message를 받고 나서 자신의 정보를 UDP로 쏘면,
					// receive를 했을 때, Data를 다시 받아 오기 때문에 
					// Flag를 사용해서 막는다. 
					if ( g_nFindMsgFlag == 1 ) {
						g_nFindMsgFlag = 0;
						break;	
					}
									
					fprintf( stdout, "+ Network Setting.\n" );
					cfg_SetIfc(p);
				}
				
				break;
			}

			fprintf( stdout, "+ Send return = %d\n", ret );
			fflush( stdout );
		}
	}
	return -1;
}


/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	UDP_CFG_T *pCfg;
	
	pCfg = new_cfg();
	if ( pCfg == NULL ) {
		fprintf( stdout, "Can't create CFG Server.\n" );
		fflush( stdout );
		exit(1);
	}

	while (1) {
		if ( cfg_RecvPacket( pCfg ) < 0 ) {
			cfg_sleep( 3, 0 );
			cfg_close( pCfg );
			continue;	
		}
		continue;
	}

	exit(1);
}




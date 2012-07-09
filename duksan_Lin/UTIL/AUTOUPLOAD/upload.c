#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>


typedef struct {
	unsigned short index;
	unsigned char dummy;
	unsigned char chkSum;
}__attribute__ ( (__packed__)) txmsg_info ;

typedef struct {
	unsigned short index;
	unsigned short length;
	unsigned char data[512];
	unsigned char chkSum;
}__attribute__ ( (__packed__)) rxmsg_info;


typedef struct {
	int socket;
	int client_socket;
	struct sockaddr_in   server_addr;
	struct sockaddr_in   client_addr;
	txmsg_info *pTx;
	rxmsg_info *pRx;
	FILE *fp;
}__attribute__ ( (__packed__)) upload_info;

#define MAX_FILE_SIZE		524288	// 1024*512
#define MAX_BUFFER			8192

/*
********************************************************************* 
* CREATE POINTER OF UPLOAD_INFO STRUCTURE 
* 
* Description	: This function is called by main to crate pointer of upload_info struture
* 
* Arguments		: none
* 
* Returns		: pUpload		Is a pointer of upload_info struture.
* 
********************************************************************* 
*/
upload_info *New_Upload(void)
{
	upload_info *pUpload = NULL;
	
	pUpload = (upload_info *)malloc( sizeof(upload_info) );
	memset( (unsigned char *)pUpload, 0, sizeof(upload_info) );	

	pUpload->socket = -1;
	pUpload->pTx = (txmsg_info *)malloc( sizeof(txmsg_info) );
	memset( (unsigned char *)pUpload->pTx, 0, sizeof(txmsg_info) );		
	pUpload->pRx = (rxmsg_info *)malloc( sizeof(rxmsg_info) );
	memset( (unsigned char *)pUpload->pRx, 0, sizeof(rxmsg_info) );				

	pUpload->fp = NULL;

	return pUpload;
}


/*******************************************************************/
int Upload_Open(upload_info *p)
/*******************************************************************/
{
	int option;

	p->socket  = socket( PF_INET, SOCK_STREAM, 0);
	if( -1 == p->socket ) {
		fprintf(stderr, "Can't create Socket\n"); 
		return -1;
	}
	
	option = 1;
	setsockopt( p->socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );

	memset( &p->server_addr, 0, sizeof(p->server_addr) );
	p->server_addr.sin_family     = AF_INET;
	p->server_addr.sin_port       = htons( 9090 );
	p->server_addr.sin_addr.s_addr= htonl( INADDR_ANY );
	
	if( -1 == bind( p->socket, 
					(struct sockaddr*)&p->server_addr, 
					sizeof( p->server_addr ) ) )  {
		fprintf(stderr, "Bind Error Upload Socket \n"); 
		return -1;
	}

	return p->socket;
}


/*******************************************************************/
int Upload_Accept(upload_info *p)
/*******************************************************************/
{
	int client_addr_size;

	if( -1 == listen( p->socket, 5 ) ) {
		fprintf(stderr, "Listen Error Upload Socket \n"); 
		return -1;
	}
	
	fprintf(stderr, "Upload server Wait client Socket \n"); 
	fflush(stderr);

	client_addr_size  = sizeof( p->client_addr);
	p->client_socket  = accept( p->socket, 
								(struct sockaddr*)&p->client_addr, 
								&client_addr_size);
	
	if ( -1 == p->client_socket) {
		fprintf(stderr, "Accept Error Upload Socket \n"); 
		return -1;
	}

	return p->client_socket;
}


/*
********************************************************************* 
* RECEIVE DATA
* 
* Description	: This function is called by main to receive data.
* 
* Arguments		: p				Is a pointer of upload_info struture.
* 
* Returns		: retVal		Is a length of data from client
* 
********************************************************************* 
*/
int Upload_OnReceive(upload_info *p)
{
	int retVal;
	rxmsg_info *pMsg = (rxmsg_info *)p->pRx;

	memset ( pMsg, 0, sizeof(rxmsg_info) );
	retVal = read ( p->client_socket, pMsg, 1024 );
	/*
	fprintf( stderr, "%d, length = %d, chksum = 0x%x\n", 
			 pMsg->index, 
			 pMsg->length, 
			 pMsg->chkSum );
	fflush( stderr );
	*/
	return retVal;
}


/*
********************************************************************* 
* SEND ACK MESSAGE
* 
* Description	: This function is called by main to send ack message.
* 
* Arguments		: p				Is a pointer of upload_info struture.
* 
* Returns		: 1       If the call was successful
*              	  -1      If not
* 
* 
********************************************************************* 
*/
int Upload_OnSend(upload_info *p)
{
	int i = 0;
	unsigned char chkSum = 0;
	rxmsg_info *pMsg = (rxmsg_info *)p->pRx;
	txmsg_info *pAck = (txmsg_info *)p->pTx;

	memset( pAck, 0, sizeof(txmsg_info) );
	pAck->index = pMsg->index;
	chkSum = 0;
	for ( i = 0; i < pMsg->length; i++ ) {
		chkSum -= pMsg->data[i];
	}
	pAck->chkSum = chkSum;

	write ( p->client_socket, pAck, sizeof(txmsg_info) );

	if ( pMsg->chkSum == pAck->chkSum ) {
		fwrite( pMsg->data, 1, pMsg->length, p->fp);

		return 1;
	}
	else {
		fprintf( stderr, "Error %d, pMsg->chkSum = 0x%x, pAck->chkSum = 0x%x\n", 
				 pAck->index,
				 pMsg->chkSum, 
				 pAck->chkSum );
		fflush( stderr );
		return -1;
	}

}


/*******************************************************************/
void upload_Close(upload_info *p)
/*******************************************************************/
{
	close( p->socket );
	close( p->client_socket );
	fprintf(stderr,"Close Upload Socket.\n");
	sleep(3);
}


/*
********************************************************************* 
* OPEN FILE
* 
* Description	: This function is called by main to open file.
* 
* Arguments		: p				Is a pointer of upload_info struture.
* 
* Returns		: none
* 
********************************************************************* 
*/
void upload_FileOpen(upload_info *p)
{
	p->fp = fopen( "/duksan/APP/duksan_upload\0", "wb" );

	chmod ("/duksan/APP/duksan_upload\0", 0x0777);

	if ( p->fp == (FILE*) 0 ) {
		fprintf( stderr, "Upload File open error\n");
		fflush( stderr );
		return;
	}	

	return;
}


/*
********************************************************************* 
* CLOSE FILE
* 
* Description	: This function is called by main to close file.
* 
* Arguments		: p				Is a pointer of upload_info struture.
* 
* Returns		: none
* 
********************************************************************* 
*/
void upload_FileClose(upload_info *p)
{
	fclose(p->fp);
}




/*
********************************************************************* 
* COPY FILE
* 
* Description	: This function is called by main to copy file.
* 
* Arguments		: none.
* 
* Returns		: none
* 
********************************************************************* 
*/
void upload_FileCopy(void)
{
   FILE    *fp_sour;
   FILE    *fp_dest;
   char     buff[1024];
   size_t   n_size;
                                          
   fp_sour  = fopen( "/duksan/APP/duksan_upload\0"  , "r");
   fp_dest  = fopen( "/duksan/APP/duksan\0", "w");
                            
   while( 0 < (n_size = fread( buff, 1, 1024, fp_sour)))
   {
      fwrite( buff, 1, n_size, fp_dest);
   }                            

   fclose( fp_sour);
   fclose( fp_dest);
}

void upload_Reset(void)
{
	system("reboot");
}


/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	upload_info *pUpload = NULL;

	pUpload = New_Upload();
	if (pUpload == NULL) {
		fprintf(stderr,"Can't create UPLOAD.\n");
		return -1;
	}

	while (1) {
		if ( Upload_Open(pUpload) < 0 ) {
			fflush(stderr);
			sleep(1);
			continue;	
		}	

		for(;;) {
			if (Upload_Accept(pUpload) > 0 ) {

				upload_FileOpen(pUpload);

				while( Upload_OnReceive(pUpload) != 0 ){

					if (Upload_OnSend(pUpload) < 0) {
						upload_Close(pUpload);
						upload_FileClose(pUpload);
						break;
					}
				}

				upload_Close(pUpload);
				upload_FileClose(pUpload);
				fflush(stderr);
				upload_Reset();
				//upload_FileCopy();
				break;
			}
			else {
				fflush(stderr);
				break;
			}
			continue;
		}		

	}	

	return -1;
}




#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/tcp.h>
//
//
#include "global.h"
#include "struct.h"
#include "dnp_observer.h"
//
int debug_dnp_tx = 1;
int debug_dnp_rx = 1;
//
// dnp
unsigned char dnp_dl_control;
short dnp_master_addr;
short dnp_slave_addr;
unsigned char trans_seq;
unsigned char app_seq;
unsigned char link_status = 0;
//
int unsol_seq = 16;
//
int DNP_BUF_CNT;
int server_socket = -1;
struct sockaddr_in svr_dnp;
int nDnpClientCnt = 0;
//mutex
pthread_mutex_t dnpObserver_mutex = PTHREAD_MUTEX_INITIALIZER;
//
//extern DNP_DIO_Point DNP_di_point[MAX_DNP_DI_POINT];
//extern DNP_DIO_Point DNP_do_point[MAX_DNP_DO_POINT];
//extern DNP_AIO_Point DNP_ai_point[MAX_DNP_AI_POINT];
//extern DNP_AIO_Point DNP_ao_point[MAX_DNP_AO_POINT];
//
//
static const unsigned short dnp_crc16tbl[256] = // Look up table values
{
0x0000, 0x365E, 0x6CBC, 0x5AE2, 0xD978, 0xEF26, 0xB5C4, 0x839A,
0xFF89, 0xC9D7, 0x9335, 0xA56B, 0x26F1, 0x10AF, 0x4A4D, 0x7C13,
0xB26B, 0x8435, 0xDED7, 0xE889, 0x6B13, 0x5D4D, 0x07AF, 0x31F1,
0x4DE2, 0x7BBC, 0x215E, 0x1700, 0x949A, 0xA2C4, 0xF826, 0xCE78,
0x29AF, 0x1FF1, 0x4513, 0x734D, 0xF0D7, 0xC689, 0x9C6B, 0xAA35,
0xD626, 0xE078, 0xBA9A, 0x8CC4, 0x0F5E, 0x3900, 0x63E2, 0x55BC,
0x9BC4, 0xAD9A, 0xF778, 0xC126, 0x42BC, 0x74E2, 0x2E00, 0x185E,
0x644D, 0x5213, 0x08F1, 0x3EAF, 0xBD35, 0x8B6B, 0xD189, 0xE7D7,
0x535E, 0x6500, 0x3FE2, 0x09BC, 0x8A26, 0xBC78, 0xE69A, 0xD0C4,
0xACD7, 0x9A89, 0xC06B, 0xF635, 0x75AF, 0x43F1, 0x1913, 0x2F4D,
0xE135, 0xD76B, 0x8D89, 0xBBD7, 0x384D, 0x0E13, 0x54F1, 0x62AF,
0x1EBC, 0x28E2, 0x7200, 0x445E, 0xC7C4, 0xF19A, 0xAB78, 0x9D26,
0x7AF1, 0x4CAF, 0x164D, 0x2013, 0xA389, 0x95D7, 0xCF35, 0xF96B,
0x8578, 0xB326, 0xE9C4, 0xDF9A, 0x5C00, 0x6A5E, 0x30BC, 0x06E2,
0xC89A, 0xFEC4, 0xA426, 0x9278, 0x11E2, 0x27BC, 0x7D5E, 0x4B00,
0x3713, 0x014D, 0x5BAF, 0x6DF1, 0xEE6B, 0xD835, 0x82D7, 0xB489,
0xA6BC, 0x90E2, 0xCA00, 0xFC5E, 0x7FC4, 0x499A, 0x1378, 0x2526,
0x5935, 0x6F6B, 0x3589, 0x03D7, 0x804D, 0xB613, 0xECF1, 0xDAAF,
0x14D7, 0x2289, 0x786B, 0x4E35, 0xCDAF, 0xFBF1, 0xA113, 0x974D,
0xEB5E, 0xDD00, 0x87E2, 0xB1BC, 0x3226, 0x0478, 0x5E9A, 0x68C4,
0x8F13, 0xB94D, 0xE3AF, 0xD5F1, 0x566B, 0x6035, 0x3AD7, 0x0C89,
0x709A, 0x46C4, 0x1C26, 0x2A78, 0xA9E2, 0x9FBC, 0xC55E, 0xF300,
0x3D78, 0x0B26, 0x51C4, 0x679A, 0xE400, 0xD25E, 0x88BC, 0xBEE2,
0xC2F1, 0xF4AF, 0xAE4D, 0x9813, 0x1B89, 0x2DD7, 0x7735, 0x416B,
0xF5E2, 0xC3BC, 0x995E, 0xAF00, 0x2C9A, 0x1AC4, 0x4026, 0x7678,
0x0A6B, 0x3C35, 0x66D7, 0x5089, 0xD313, 0xE54D, 0xBFAF, 0x89F1,
0x4789, 0x71D7, 0x2B35, 0x1D6B, 0x9EF1, 0xA8AF, 0xF24D, 0xC413,
0xB800, 0x8E5E, 0xD4BC, 0xE2E2, 0x6178, 0x5726, 0x0DC4, 0x3B9A,
0xDC4D, 0xEA13, 0xB0F1, 0x86AF, 0x0535, 0x336B, 0x6989, 0x5FD7,
0x23C4, 0x159A, 0x4F78, 0x7926, 0xFABC, 0xCCE2, 0x9600, 0xA05E,
0x6E26, 0x5878, 0x029A, 0x34C4, 0xB75E, 0x8100, 0xDBE2, 0xEDBC,
0x91AF, 0xA7F1, 0xFD13, 0xCB4D, 0x48D7, 0x7E89, 0x246B, 0x1235
};

//
/****************************************************************/
unsigned short dnp_mkcrc16(unsigned char *p, int len)
/****************************************************************/
{
	unsigned short crc = 0;
	//
	while ( len-- )
		crc = (crc >> 8) ^ dnp_crc16tbl[(unsigned char)crc ^ *((unsigned char *)p)++];
    crc = ~crc;
	//
	return	crc;
} 

/****************************************************************/
void mk_lpdu(int client_sock, unsigned char *apdu, int apdu_length)
/****************************************************************/
{
    int i,j,k;
    unsigned char  buf[400];
    unsigned char crc_data[20];
    unsigned short mk_crc_data;
    int cnt;
    int dl_ptr = 0;
	//
    if (debug_dnp_rx)
        printf("DNPd: %s() apdu_length %d \n", __FUNCTION__, apdu_length);
	//
    buf[dl_ptr++] = 0x05;                        // header
    buf[dl_ptr++] = 0x64;                        // header
    buf[dl_ptr++] = apdu_length + 5;             // length
    //buf[dl_ptr++] = dnp_dl_control - 0x80;      // control
    buf[dl_ptr++] = 0x44;      // control
    buf[dl_ptr++] = (dnp_master_addr & 0x00ff); // dest addr
    buf[dl_ptr++] = (dnp_master_addr >> 8);   
    buf[dl_ptr++] = (dnp_slave_addr & 0x00ff);  // source addr
    buf[dl_ptr++] = (dnp_slave_addr >> 8);
    //
    mk_crc_data = dnp_mkcrc16(buf, dl_ptr);
    //
    buf[dl_ptr++] = (unsigned char)(mk_crc_data & 0x00ff);     // crc16
    buf[dl_ptr++] = (unsigned char)(mk_crc_data >> 8);         // crc16
    //
    cnt = apdu_length;
    //
    i = 0;
    j = 0;
    //
    while(cnt--)
    {
        crc_data[i++] = apdu[j++];
        //
        if ((i >= 16) || (j >= apdu_length))
        {
            for(k = 0; k < i; k++)
                buf[dl_ptr++] = crc_data[k];
            //
            mk_crc_data = dnp_mkcrc16(crc_data, i);
            //
            buf[dl_ptr++] = (unsigned char)(mk_crc_data & 0x00ff);
            buf[dl_ptr++] = (unsigned char)(mk_crc_data >> 8);
            //
            i = 0;
        }
    }
	//
    if (debug_dnp_rx)
    {
        printf(">>>>>> DNP Tx(%d): \n", client_sock);
        for (i = 0;i < dl_ptr; i++)
        {
            printf("%02x ", buf[i]);
                if (!((i+1) % 16))
            printf("\n");
        }
        printf("\n");
        printf("\n");
    }
   
	//usleep(300000);

    // 07.07.05 modified. by Jong2ry
    // Send function us Socket API Function
    // Success : Return Value is Data Count
    // Failure : Return Socket Error     
#if 1
    // ORG 
	send(client_sock, buf, dl_ptr, 0);
#else
    if( send(client_sock, buf, dl_ptr, 0) != dl_ptr)
    {
        if (debug_dnp_rx)
            printf("DEBUG : Socket Send Error!!! Send Try Again \n");
        if( send(client_sock, buf, dl_ptr, 0) != dl_ptr)
        {
            if (debug_dnp_rx)
                printf("DEBUG : Socket Send Fail!!!\n");
        }
    }
#endif
}  

/****************************************************************/
void mk_tpdu(int client_sock, unsigned char *apdu, int apdu_length)
/****************************************************************/
{
    unsigned char buf[250];
	//
    // printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
	//
    if(trans_seq > 63)
        trans_seq = 0;
    //
    //printf("Make TPDU\n");
    if(apdu_length > 499)
    {
        apdu[0] = 0x41;
        mk_lpdu(client_sock, apdu, 250);
        //
        buf[0] = 0x02;
        memcpy(&buf[1], &apdu[250], 249);      
        mk_lpdu(client_sock, buf, 250);
        //
        buf[0] = 0x83;
        memcpy(&buf[1], &apdu[499], apdu_length-499);      
        mk_lpdu(client_sock, buf, apdu_length-498);
    } 
    else if(apdu_length > 250)
    {
        apdu[0] = 0x41;
        mk_lpdu(client_sock, apdu, 250);
        //
        buf[0] = 0x82;
        memcpy(&buf[1], &apdu[250], apdu_length-250);      
        mk_lpdu(client_sock, buf, apdu_length-249);
    }
    else
    {
        mk_lpdu(client_sock, apdu, apdu_length);
    }
}

/****************************************************************/
void mk_unsol(int client_sock)
/****************************************************************/
{
    unsigned char buf[1024];
    DNP_DIO_Point *dio_p;
    int pno=0, buf_ptr=0;
    unsigned char range_ptr;
    int  range_val=0; 
	//
    //printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
    //
    if (unsol_seq > 31)
        unsol_seq = 16;
	//
    buf[buf_ptr++] = 0xc1;              // transport layer header
    buf[buf_ptr++] = 0xc0 + unsol_seq;  // AC
    buf[buf_ptr++] = 0x82;              // FC 0x82, 130, Unsolicited Message
    buf[buf_ptr++] = 0x00;              // IIN
    buf[buf_ptr++] = 0x00;              // IIN
    //
    buf[buf_ptr++] = 0x02;              // object group
    buf[buf_ptr++] = 0x01;              // object variation
    //
    buf[buf_ptr++] = 0x27;              //  qualifier field
    range_ptr = buf_ptr;
    buf[buf_ptr++] = 0x01;              // range_filed default value (1 data)
    //buf[buf_ptr++] = 0x00;            // range_filed default value (1 data)
	//
    for (pno = 0; pno < MAX_DNP_DI_POINT; pno++)
    {
        dio_p = &DNP_di_point[pno];
		//    
        if (dio_p->Unsol)
        {
            if(dio_p->Unsol != client_sock)
            {
                printf("DNPd: Send DI Unsol message, pno %d, PLCaddr %d, value %d, client_sock %d \n", pno, dio_p->PLCaddr, dio_p->val, client_sock);
                buf[buf_ptr++] = pno & 0xff;
                buf[buf_ptr++] = (pno & 0xff00) >> 8;
                //
                if(dio_p->val)
                    //buf[buf_ptr++] = 0x80;    // First Bit(0) is 1, By DNP Protocol. First Bit is Device status(1 : on-line, 0 : off-line) 
                    buf[buf_ptr++] = 0x81;                    
                else
                    //buf[buf_ptr++] = 0x00;    // First Bit(0) is 1, By DNP Protocol. First Bit is Device status(1 : on-line, 0 : off-line)
                    buf[buf_ptr++] = 0x01;                    
                //   
                range_val++;
                //
                if(dio_p->Unsol == 1)
                    dio_p->Unsol = client_sock;
                else
                    dio_p->Unsol = 0;
                //
                //dio_p->PrevVal = dio_p->val;                    
            }
        }
    }
	//
    buf[range_ptr] = range_val & 0xff;
    //buf[range_ptr+1] = (range_val & 0xff00) >> 8;
    //
    mk_tpdu(client_sock, buf, buf_ptr);
    //
    unsol_seq++;
} 

/****************************************************************/
void mk_unsol_do(int client_sock)
/****************************************************************/
{
    int i = 0;
    unsigned char buf[1024];
    DNP_DIO_Point *dio_p;
    int pno=0, buf_ptr=0;
    unsigned char range_ptr;
    int  range_val=0; 
    //

    printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
  
    for (pno = 0; pno < MAX_DNP_DO_POINT; pno++)
    {
        buf_ptr = 0;    // 20071109
        range_ptr = 0;  // 20071109
        range_val = 0;  // 20071109
    
    if (unsol_seq > 31)
        unsol_seq = 16;
    //
    buf[buf_ptr++] = 0xc1;              // transport layer header
    buf[buf_ptr++] = 0xc0 + unsol_seq;  // AC
    buf[buf_ptr++] = 0x82;              // FC
    buf[buf_ptr++] = 0x00;              // IIN
    buf[buf_ptr++] = 0x00;              // IIN
    //
    buf[buf_ptr++] = 10;                // object group
    buf[buf_ptr++] = 0x02;              // object variation
    //
    buf[buf_ptr++] = 0x28;              //  qualifier field
    range_ptr = buf_ptr;
    buf[buf_ptr++] = 0x01;              // range_filed default value (1 data)
    buf[buf_ptr++] = 0x00;              // range_filed default value (1 data)
      
    //    for(pno = 0; pno < MAX_DNP_DO_POINT; pno++)   // 20071109 ORG
    //    {                                             // 20071109 ORG
        dio_p = &DNP_do_point[pno];
        //
        if(dio_p->Unsol)
        {
            if(dio_p->Unsol != client_sock)
            {
                    //wdog_reload_cnt();      // 20071114
                    printf("DNPd: Send DO Unsol message, pno %d, PLCaddr %d, ONaddr %d, OFFaddr %d, value %d\n", pno, dio_p->PLCaddr, dio_p->ONaddr, dio_p->OFFaddr, dio_p->val);
                buf[buf_ptr++] = pno & 0xff;
                buf[buf_ptr++] = (pno & 0xff00) >> 8;

                if(dio_p->val)
                    buf[buf_ptr++] = 0x81;
                else
                    buf[buf_ptr++] = 0x01;

                range_val++;
          
                if(dio_p->Unsol == 1)
                    dio_p->Unsol = client_sock;
                else
                    dio_p->Unsol = 0;

                    // 20071109 add                        
                    buf[range_ptr] = range_val & 0xff;
                    buf[range_ptr+1] = (range_val & 0xff00) >> 8;
                    
                    mk_tpdu(client_sock, buf, buf_ptr);
                  
                    unsol_seq++;
                
                    printf("DNPd: DO Unsol - ");  
                    for (i = 0; i < buf_ptr; i++)
                    {
                        if (buf[i] <= 0x0f)
                            printf("0");
                        printf("%x ", buf[i]);
            }
                    printf("\n");
                    // 20071109 add
                        
        }
                // dio_p->Unsol = 0;
    }
    //    }     // 20071109 ORG

#if 0      
        buf[range_ptr] = range_val & 0xff;
        buf[range_ptr+1] = (range_val & 0xff00) >> 8;

        mk_tpdu(client_sock, buf, buf_ptr);
      
        unsol_seq++;

        printf("DNPd: DO Unsol - ");  
        for (i = 0; i < buf_ptr; i++)
        {
            if (buf[i] <= 0x0f)
                printf("0");
            printf("%x ", buf[i]);
        }
        printf("\n");
#endif
    }   // 20071109       
} 

/****************************************************************/
void mk_unsol_ao(int client_sock)
/****************************************************************/
{
    int i = 0;
    int pno = 0, buf_ptr = 0;
    unsigned char buf[64];
    DNP_AIO_Point *aio_p;
    unsigned char range_ptr;
    int  range_val = 0; 
    double ai_point;
    short analog_val;
    
    printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
    
    if (unsol_seq > 31)
        unsol_seq = 16;
    
    buf[buf_ptr++] = 0xc1;              // transport layer header
    buf[buf_ptr++] = 0xc0 + unsol_seq;  // AC
    buf[buf_ptr++] = 0x82;              // FC
    buf[buf_ptr++] = 0x00;              // IIN
    buf[buf_ptr++] = 0x00;              // IIN
    
    buf[buf_ptr++] = 0x28;              // object group
    buf[buf_ptr++] = 0x02;              // object variation
    
    buf[buf_ptr++] = 0x27;              // qualifier field
    range_ptr = buf_ptr;
    buf[buf_ptr++] = 0x01;              // range_filed default value (1 data)
    //buf[buf_ptr++] = 0x00;              // range_filed default value (1 data)
    
    for (pno = 0; pno < MAX_DNP_AO_POINT; pno++)
    {
        aio_p = &DNP_ao_point[pno];
        
        if(aio_p->Unsol)
        {
            if(aio_p->Unsol != client_sock)
            {
                buf[buf_ptr++] = pno & 0xff;
                buf[buf_ptr++] = (pno & 0xff00) >> 8;

                buf[buf_ptr++] = 0x01;          // status flag - online    
                // buf[var_ptr] = 2;       // 16bit analog output status
                aio_p = &DNP_ao_point[pno];
                //buf[ptr++] = 1;             // flag
                ai_point = aio_p->val;
                analog_val = ai_point;
                buf[buf_ptr++] = analog_val & 0xff;
                analog_val = analog_val >> 8;
                buf[buf_ptr++] = analog_val & 0xff;
                //buf[buf_ptr++] = 0x00;          // ??? AO is 3 bytes
                //if (aio_p->val)
                //    buf[buf_ptr++] = 0x80;
                //else
                //    buf[buf_ptr++] = 0x00;
                range_val++;

                //wdog_reload_cnt();      // 20071114
                analog_val = ai_point;      // to display correctly, analog_val is shifted right at above.
                printf("DNPd: Send AO Unsol message, pno %d, PLCaddr %d, value %d, PrevVal 0x%x \n", pno, aio_p->PLCaddr, analog_val);
                printf("DNPd: val 0x%x, PrevVal 0x%x \n", aio_p->val, aio_p->PrevVal);
                
                if (aio_p->Unsol == 1)
                    aio_p->Unsol = client_sock;
                else
                    aio_p->Unsol = 0;
            }
            //aio_p->Unsol = 0;
        }
    }
  
    //buf[range_ptr] = range_val & 0xff;
    //buf[range_ptr+1] = (range_val & 0xff00) >> 8;
    
    mk_tpdu(client_sock, buf, buf_ptr);
    
    unsol_seq++;
    
    for (i = 0; i < buf_ptr; i++)
    {
        if (buf[i] <= 0x0f)
            printf("0");
        printf("%x ", buf[i]);
    }
    printf("\n");

} 

void mk_unsol_ao_from_other_hmi(int client_sock, int dnpno)
{
    int i = 0;
    int pno = 0, buf_ptr = 0;
    unsigned char buf[64];
    DNP_AIO_Point *aio_p;
    unsigned char range_ptr;
    int  range_val = 0; 
    double ai_point;
    short analog_val;
    
    printf("DNPd: %s() client_sock %d, dnpno %d \n", __FUNCTION__, client_sock, dnpno);
    
    if (unsol_seq > 31)
        unsol_seq = 16;
    
    buf[buf_ptr++] = 0xc1;              // transport layer header
    buf[buf_ptr++] = 0xc0 + unsol_seq;  // AC
    buf[buf_ptr++] = 0x82;              // FC
    buf[buf_ptr++] = 0x00;              // IIN
    buf[buf_ptr++] = 0x00;              // IIN
    
    buf[buf_ptr++] = 0x28;              // object group
    buf[buf_ptr++] = 0x02;              // object variation
    
    buf[buf_ptr++] = 0x27;              // qualifier field
    range_ptr = buf_ptr;
    buf[buf_ptr++] = 0x01;              // range_filed default value (1 data)
    //buf[buf_ptr++] = 0x00;              // range_filed default value (1 data)
    
    //for (pno = 0; pno < MAX_DNP_AO_POINT; pno++)
    //{
        aio_p = &DNP_ao_point[dnpno];
        
        //if(aio_p->Unsol)
        //{
            //if(aio_p->Unsol != client_sock)
            //{
                buf[buf_ptr++] = dnpno & 0xff;
                buf[buf_ptr++] = (dnpno & 0xff00) >> 8;

                buf[buf_ptr++] = 0x01;          // status flag - online    
                // buf[var_ptr] = 2;       // 16bit analog output status
                aio_p = &DNP_ao_point[dnpno];
                //buf[ptr++] = 1;             // flag
                ai_point = aio_p->val;
                analog_val = ai_point;
                buf[buf_ptr++] = analog_val & 0xff;
                analog_val = analog_val >> 8;
                buf[buf_ptr++] = analog_val & 0xff;
                //buf[buf_ptr++] = 0x00;          // ??? AO is 3 bytes
                //if (aio_p->val)
                //    buf[buf_ptr++] = 0x80;
                //else
                //    buf[buf_ptr++] = 0x00;
                range_val++;

                //wdog_reload_cnt();      // 20071114
                analog_val = ai_point;      // to display correctly, analog_val is shifted right at above.
                printf("DNPd: AO Unsol msg, dnpno %d, PLCaddr %d, value %d, PrevVal 0x%x \n", dnpno, aio_p->PLCaddr, analog_val);
                printf("DNPd: AO Unsol msg, val 0x%x, PrevVal 0x%x \n", aio_p->val, aio_p->PrevVal);
      
                aio_p->PrevVal = aio_p->val;    // data sync...
                          
                //if (aio_p->Unsol == 1)
                //    aio_p->Unsol = client_sock;
                //else
                //    aio_p->Unsol = 0;
            //}
            //aio_p->Unsol = 0;
        //}
    //}
  
    //buf[range_ptr] = range_val & 0xff;
    //buf[range_ptr+1] = (range_val & 0xff00) >> 8;
    
    mk_tpdu(client_sock, buf, buf_ptr);
    
    unsol_seq++;
    
    for (i = 0; i < buf_ptr; i++)
    {
        if (buf[i] <= 0x0f)
            printf("0");
        printf("%x ", buf[i]);
    }
    printf("\n");

} 

/****************************************************************/
void check_unsol(int client_sock)
/****************************************************************/
{
    int pno = 0;
    DNP_DIO_Point *dio_p;
    DNP_AIO_Point *aio_p;
    //if (debug_dnp_rx)
    //    printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
	//
    for (pno = 0; pno < MAX_DNP_DI_POINT; pno++)
    {
        dio_p = &DNP_di_point[pno];
        //
        if(dio_p->Unsol)
        {
            if(dio_p->Unsol != client_sock)
            {
				mk_unsol(client_sock);
                break;
            }
        }
	}
    //
    for (pno = 0; pno < MAX_DNP_DO_POINT; pno++)
    {
        dio_p = &DNP_do_point[pno];
        //
        if((dio_p->Unsol) && (dio_p->EnUnsol))
        {
            if(dio_p->Unsol != client_sock)
            {
                mk_unsol_do(client_sock);
                break;
            }
        }
	}
	//	//
    for (pno = 0; pno < MAX_DNP_AO_POINT; pno++)
    {
        //
		//printf("check AO Pno = %d\n", pno, val);
		//
		aio_p = &DNP_ao_point[pno];
        //
		//printf("check AO Pno = %d, Val = %f, unsol = %d\n", pno, aio_p->val, aio_p->Unsol);
		//
        if (aio_p->Unsol)
        {
			//printf("check AO Pno = %d\n", pno);
			//
            if (aio_p->Unsol != client_sock)
            {
                mk_unsol_ao(client_sock);
                break;
            }
        }
    }
	//
}

//
// Read point
//
/****************************************************************/
void rd_point(int client_sock, unsigned char *apdu, int apdu_length)
/****************************************************************/
{
    unsigned char buf[2048];
    unsigned char obj_group, obj_var, q_code;
    int r_ptr=3, ptr=0, bi_ptr=0, ai_ptr=0, var_ptr=0;
    DNP_DIO_Point *dio_p;
    DNP_AIO_Point *aio_p;
    int pno, i;
    int first_pno, final_pno;
    unsigned char binary_point[48];
    double ai_point, dio_point;
    short analog_val;
    //
    unsigned char dio_val=0, dio_val_chk=0, dio_p_val=0;
    //
	memset(buf, 0x00, sizeof(buf));
	//
    if (debug_dnp_rx)
        printf("DNPd: %s() \n", __FUNCTION__);
	//
    // transport layer 추가
    buf[ptr++] = 0xc0 + trans_seq;    // ptr : make apdu pointer
    //
    buf[ptr++] = 0xe0 + app_seq;      // application header
    buf[ptr++] = 0x81;                // 0x81, 129, Response
    buf[ptr++] = 0x00;                // ignore IIN
    buf[ptr++] = 0x00;                // ignore IIN
    //
    // printf("r_ptr = %d \n",r_ptr);
    // printf("apdu_length = %d \n",apdu_length);
	//
    while (r_ptr < apdu_length)
    {   
        obj_group = apdu[r_ptr++];          // r_ptr : read apdu pointer
        obj_var = apdu[r_ptr++];
        q_code = apdu[r_ptr++];
		//
        if(obj_group == CLASS_0123)
        {
            if(obj_var == 1)
                obj_group = BINARY_INPUT;    // DI point
            else if(obj_var == 2)
                obj_group = BINARY_OUTPUT;
            else if(obj_var == 3)
                obj_group = ANALOG_INPUT;
            else
            {
                printf("class 4 \n");
                obj_group = ANALOG_OUTPUT;
            }
        }
		//
        buf[ptr++] = obj_group;
        var_ptr = ptr;
        buf[ptr++] = 1;                 // variation is fixed to 1
        buf[ptr++] = q_code;
		//
        //printf("DEBUG: q_code 0x%x \n", q_code);
        switch(q_code)
        {
            case 0x00:    // range : first point number(8bit), final point number(8bit)
                first_pno = apdu[r_ptr++];
                final_pno = apdu[r_ptr++];
                buf[ptr++] = first_pno;
                buf[ptr++] = final_pno;      
                break;

            case 0x01:    // range : first point number(16bit), final point number(16bit)
                first_pno = apdu[r_ptr++];
                buf[ptr++] = first_pno;
                first_pno = first_pno + apdu[r_ptr] * 256;    // Add, bug fix for dunji
                buf[ptr++] = apdu[r_ptr++];
                final_pno = apdu[r_ptr++];
                buf[ptr++] = final_pno;
                final_pno = final_pno + apdu[r_ptr] * 256;    // Add, bug fix for dunji
                buf[ptr++] = apdu[r_ptr++];
                //printf("DEBUG: first_pno %d, final_pno %d \n", first_pno, final_pno);
                break;

            case 0x06:   // all point
                first_pno = 1;
                if(obj_group < 10)
                    final_pno = MAX_DNP_DI_POINT-1;
                else if(obj_group < 20)
                    final_pno = MAX_DNP_DO_POINT-1;
                else if(obj_group < 40)
                    final_pno = MAX_DNP_AI_POINT-1;
                else
                    final_pno = MAX_DNP_AO_POINT-1;
                break;

            case 0x07:
                first_pno=0;
                final_pno = apdu[r_ptr++];
                buf[ptr++] = final_pno;
                break;

            case 0x17:
                printf("DNPd: q-code 0x17 \n");
                return;    

            default:
                printf("DNPd: Unknown q-code 0x%x \n", q_code);
                return;
        }
		//
        for (pno = first_pno; pno < (final_pno+1); pno++)
        {
            //printf("DEBUG: pno %d \n", pno);
            //p = &ex_ptbl[dnp_slave_addr][pno];
            //p = &ptbl[pno];
            if(pno < 0)
            {
                printf("DNPd: Point number is minus (%d). \n", pno);
                return;
            }
             // 
            switch(obj_group)
            {
                case BINARY_OUTPUT:
                case CONTROL_BLOCK:
                    ////buf[var_ptr] = 2;
                    //dio_p = &DNP_do_point[pno];
                    //dio_p_val = dio_p->val;
                    //
                    //if((pno != final_pno)&&(dio_val_chk < 7))
                    //{
                    //    dio_p_val = dio_p_val << dio_val_chk;
                    //    dio_val += dio_p_val;
                    //    dio_val_chk++;          
                    //}
                    //else
                    //{
                    //    dio_p_val = dio_p_val << dio_val_chk;
                    //    dio_val += dio_p_val;
                    //    buf[ptr++] = dio_val;
                    //    dio_val = 0;
                    //    dio_val_chk = 0;
                    //}
                    //
                    if(pno >= MAX_DNP_DO_POINT)
                    {
                        printf("DNPd: [DO] Point number[%d] is over the MAX_POINT number!!!\n", pno);
                        return;
                    }
                    
                    buf[var_ptr] = 2;                       // 16bit analog output status
                    dio_p = &DNP_do_point[pno];
                    
					//buf[ptr++] = 0x01;                      // flag
                    //dio_point = dio_p->val;
                    //analog_val = dio_point;
                    //buf[ptr++] = analog_val & 0xff;
                    //analog_val = analog_val >> 8;
                    //buf[ptr++] = analog_val & 0xff;
					
					if (dio_p->val)
						buf[ptr++] = 0x81;
					else
						buf[ptr++] = 0x01;

                    continue;
                    
                case BINARY_INPUT:
                case BINARY_INPUT_CHANGE:
                    //dio_p = &DNP_di_point[pno];
                    //dio_p_val = dio_p->val;
                    //
                    //if((pno != final_pno)&&(dio_val_chk < 7))
                    //{
                    //    dio_p_val = dio_p_val << dio_val_chk;
                    //    dio_val += dio_p_val;
                    //    dio_val_chk++;
                    //}
                    //else
                    //{
                    //    dio_p_val = dio_p_val << dio_val_chk;
                    //    dio_val += dio_p_val;
                    //    //printf("DEBUG DI dio_val 0x%x \n", dio_val);
                    //    buf[ptr++] = dio_val;
                    //    dio_val = 0;
                    //    dio_val_chk = 0;
                    //}
                    //
                    if(pno >= MAX_DNP_DI_POINT)
                    {
                        printf("DNPd: [DI] point number[%d] is over the MAX_POINT number!!!\n", pno);
                        return;
                    }   

                    buf[var_ptr] = 2;                       // 16bit analog output status
                    dio_p = &DNP_di_point[pno];

					if (dio_p->val)
						buf[ptr++] = 0x81;
					else
						buf[ptr++] = 0x01;

                    //buf[ptr++] = 0x01;                      // flag
                    //dio_point = dio_p->val;
                    //analog_val = dio_point;
                    //buf[ptr++] = analog_val & 0xff;
                    //analog_val = analog_val >> 8;
                    //buf[ptr++] = analog_val & 0xff;
                    continue;
                    
                case ANALOG_INPUT:
                case FROZEN_ANALOG_INPUT:
                case ANALOG_CHANGE_EVENT:
                case FROZEN_ANALOG_EVENT:
                    buf[var_ptr] = 2;         // 16bit analog input without flag
                    aio_p = &DNP_ai_point[pno];
                    buf[ptr++] = 1;           // flag
                    // for only ahyun
                    //ai_point = aio_p->val - aio_p->MinVal;
                    //ai_point *= 16000;
                    //ai_point = ai_point/(aio_p->MaxVal - aio_p->MinVal);
                    ai_point = aio_p->val - aio_p->MinVal;
                    // end
                    analog_val = ai_point;
                    //printf("DEBUG:: Pno = %d,  AI 0x%x \n", pno, analog_val);
                    buf[ptr++] = analog_val & 0xff;
                    analog_val = analog_val >> 8;
                    buf[ptr++] = analog_val & 0xff;
					//
                    if (pno >= MAX_DNP_AI_POINT)
                    {
                        printf("DNPd: [AI] Point number[%d] is over the MAX_POINT number!!!", pno);
                        return;
                    }   
                    continue;
                
                case ANALOG_OUTPUT:
                case ANALOG_OUTPUT_BLOCK:
                    buf[var_ptr] = 2;       // 16bit analog output status
                    aio_p = &DNP_ao_point[pno];
                    buf[ptr++] = 1;         // flag
                    ai_point = aio_p->val;
                    //ai_point *= 16000;
                    //ai_point = ai_point/(aio_p->MaxVal - aio_p->MinVal);
                    analog_val = ai_point;
                    buf[ptr++] = analog_val & 0xff;
                    analog_val = analog_val >> 8;
                    buf[ptr++] = analog_val & 0xff;
                    //
                    if(pno >= MAX_DNP_AO_POINT)
                    {
                        printf("DNPd: [AO] Point number[%d] is over the MAX_POINT number!!! \n", pno);
                        printf("obj_g = %d , obj_var = %d , q_code = %d , first = %d , final_pno = %d \n",obj_group, obj_var, q_code, first_pno,final_pno);          
                        return;
                    }
                    continue;
    
                case CLASS_0123:
                    printf("DNPd: Class object!!! \n");      
    
                default:
                    printf("DNPd: Unknown obj_group (%d) \n", obj_group);
                    return;
            }
        }
    }    
    //
#if 0
    for(i = 0; i < ptr; i++)
    {
        if (buf[i] < 0x10)
            printf("0");
        printf("%x ", buf[i]);
    
        if (!((i+1) % 16))
            printf("\n");
    }
    printf("\n");
#endif
    //
    mk_tpdu(client_sock, buf, ptr);
}

void wr_point(int client_sock, unsigned char* apdu, int apdu_length)
{
    int i;
    unsigned char obj_group, obj_var, q_field, q_code, i_size;
    int cnt, ptr, pno;
    long range_field;
    unsigned char buf[10];
    float pval=0;
    unsigned int ival = 0, ival1 = 0x00, ival2 = 0x00;
    //Point *p;
    DNP_DIO_Point *dio_p;
    DNP_AIO_Point *aio_p;

    if (debug_dnp_tx)
        printf("DNPd: %s() \n", __FUNCTION__);
    
#if 0  
    printk("DNPd: %s() apdu_length %d \n", __FUNCTION__, apdu_length);
    for(i=0;i<apdu_length;i++)
    {
        if (apdu[i] < 0x10)
            printk("0");
        printk("%x ",apdu[i]);
        
        if (!((i+1) % 16))
            printk("\n");    
    }
    printk("\n");
#endif

    ptr = 3;
    cnt = apdu_length;
  
    //printk("ptr %d obj_group 0x%x \n", ptr, apdu[ptr]);    
    obj_group = apdu[ptr++];
    //printk("ptr %d obj_var 0x%x \n", ptr, apdu[ptr]);    
    obj_var = apdu[ptr++];
    //printk("ptr %d q_field 0x%x \n", ptr, apdu[ptr]);    
    q_field = apdu[ptr++];
      
    // index size : 0(no data number), 1(8bit data number), 2(16bit data number), 3(32bit data number)
    // qualifier code : 7~9 range represent quantity (8bit, 16bit, 32bit)
    i_size = q_field >> 4;          // Index Size
    q_code = q_field & 0xf;         // Qualifier Code
    //printk("i_size 0x%x q_code 0x%x \n", i_size, q_code);
    
    if (q_code == 0x07)
    {
        range_field = apdu[ptr++];
    }
    else if(q_code == 0x08)
    {
        range_field = apdu[ptr++];
        range_field += (apdu[ptr++] << 8);
    }
    else if(q_code == 0x09)
    {
        range_field = apdu[ptr++];
        range_field += (apdu[ptr++] << 8);
        range_field += (apdu[ptr++] << 16);
        range_field += (apdu[ptr++] << 24);
    }
    else
    {
        printf("DNPd: q_code err : %x \n", q_code);    
        return;
    }
    
    if (range_field > 256)
    {
        printf("DNPd: Range_field is over 256!!! \n");
        return;
    }

    while(range_field--)
    {
        if (i_size == 1)
        {
            pno = apdu[ptr++];
        }
        else if(i_size == 2)
        {
            pno = apdu[ptr++];
            pno += (apdu[ptr++] << 8);
        }
        else if(i_size == 3)
        {
            pno = apdu[ptr++];
            pno += (apdu[ptr++] << 8);
            pno += (apdu[ptr++] << 16);
            pno += (apdu[ptr++] << 24);
        }
      
        //printk(">>> ptr %d \n", ptr);
        if(obj_group == 12)
        {
            if(obj_var == 1)
            {
                if(pno >= MAX_DNP_DO_POINT)
                {
                    printf("DNPd: DO write point number is over MAX_POINT (pno 0x%x MAX_DNP_DO_POINT 0x%x) !!!\n", pno, MAX_DNP_DO_POINT);
                    return;
                }

                dio_p = &DNP_do_point[pno];
                //ptr += 10;
                ptr = 10;
                ival = apdu[ptr++];
            
                // 0x81 정지  81 open  0 수동
                // 0x41 기동  41 close 1 자동 
                // if (ival > 0)
                if(ival == 0x81)
                    dio_p->WrVal = 0;
                else if (ival == 0x41)
                    dio_p->WrVal = 1;
                else
                    printf("DNPd: ptr %d, ival 0x%x \n", ptr, ival);
        
                dio_p->WrFlag = 1;
                
                printf("DNPd: DO %d point (PLCaddr %d, ONaddr %d, OFFaddr %d) set to %d, ival 0x%x \n", pno, dio_p->PLCaddr, dio_p->ONaddr, dio_p->OFFaddr, dio_p->WrVal, ival);
                //dio_p->Unsol = 1;
            }
            // else if(obj_var == 2)
            // pval = (apdu[ptr++] >> 7);
            else
            {
                printf("DNPd: obj_group 10 variance error : %x \n", obj_var);
                return;
            }        
        }
        else if(obj_group == 41)
        {
/*
            if(obj_var == 1)
            {
                ptr++;
                pval = apdu[ptr++];
                pval += (apdu[ptr++] << 8);
                pval += (apdu[ptr++] << 16);
                pval += (apdu[ptr++] << 24);
            }
*/    
            if(obj_var == 2)
            {
                //ival = apdu[ptr++];
                ////printk("[%d-%d]",ival,apdu[ptr-1]);
                //ival += (apdu[ptr++] << 8);
                ////printk("[%d-%d]",ival,apdu[ptr-1]);

                // 20071120 move to here!!! control status //ptr++;  //control status

                //printk("ptr %d ival 0x%x 0x%x \n", ptr, apdu[ptr], apdu[ptr+1]);
                //ival = apdu[ptr++] + (apdu[ptr++] << 8);
                ival1 = apdu[ptr++];
                ival2 = (apdu[ptr++] << 8);
                ival = ival1 + ival2;
                //printk("[%d-%d]",ival,apdu[ptr-1]);
                //printk("ptr %d ival 0x%x 2,1 0x%x 0x%x\n", ptr, ival, ival2, ival1);
                //ival += (apdu[ptr++] << 8);
                //printk("[%d-%d]",ival,apdu[ptr-1]);
                
                // 20071120 move to here!!! control status
                ptr++;      // 20071114 move to here!!! control status
                // 20071120 move to here!!! control status
            }
            else
            {
                printf("DNPd: obj_group 41 variance error : 0x%x \n", obj_var);
                return;
            }
        
            if (pno >= MAX_DNP_AO_POINT)
            {
                printf("DNPd: AO write point number is over MAX_POINT (pno 0x%x MAX_DNP_AO_POINT 0x%x) !!!\n", pno, MAX_DNP_AO_POINT);
                return;
            }
        
            //p = &ex_ptbl[dnp_slave_addr][pno];
            aio_p = &DNP_ao_point[pno];
/*
            pval = ival;
            pval /= 16000;
            pval *= (aio_p->MaxVal - aio_p->MinVal);
            pval += aio_p->MinVal;
*/
            if(aio_p->scale == 0.001)
                ival = ival * 1000;
            
            if(ival > aio_p->MaxVal)
                ival = aio_p->MaxVal;
            else if(ival < 0)
                ival = 0;
          
            //aio_p->val = ival;
            aio_p->PrevVal = ival;
            aio_p->WrFlag = 1;
            printf("DNPd: AO %d point (PLCaddr %d) set to %d, 0x%x \n", pno, aio_p->PLCaddr, ival, ival);
        }
        else
        {
            printf("DNPd: <Write point> Unknown object!!! \n");
            return;
        }          
                                              
        //printk("pset(%d,%d,%d)\n",dnp_slave_addr, pno, pval);
          
        //aPset(dnp_slave_addr,pno,pval);        
    }
      
    buf[0] = 0xc0 + trans_seq;
    buf[1] = 0xe0 + (app_seq & 0x0f);       //application header
    buf[2] = 0x81;                          // response
    buf[3] = 0x00;                          // ignore IIN
    buf[4] = 0x00;                          // ignore IIN
  
    for (i = 3; i < apdu_length; i++)
        buf[i+2] = apdu[i];
    
    // printk("send wr_response\n");  
    mk_tpdu(client_sock, buf, apdu_length+2);
}

/****************************************************************/
void send_confirm(int client_sock)
/****************************************************************/
{
    int i = 0;
    int ptr = 0;
    unsigned char buf[32];
    unsigned short mk_crc_data;
	//
    // printf("send confirm!! \n");
    if (debug_dnp_rx)
        printf("DNPd: %s() \n", __FUNCTION__);
	//
    buf[ptr++] = 0x05;
    buf[ptr++] = 0x64;
    buf[ptr++] = 0x08;
    //buf[ptr++] = 0x44 + (dnp_dl_control & 0x20);
    buf[ptr++] = 0x44;
    buf[ptr++] = (dnp_master_addr & 0x00ff); // dest addr
    buf[ptr++] = (dnp_master_addr >> 8);   
    buf[ptr++] = (dnp_slave_addr & 0x00ff);  // source addr
    buf[ptr++] = (dnp_slave_addr >> 8);
	//
    mk_crc_data = dnp_mkcrc16(buf, ptr);
	//
    buf[ptr++] = (mk_crc_data & 0x00ff);     // crc16
    buf[ptr++] = (mk_crc_data >> 8);         // crc16
	//
    buf[ptr++] = 0xc0 + trans_seq;      // trans header
    
    // !!!!!!!!!!!!!!!!!!!!!!! This Point Must Modify
    buf[ptr++] = 0xc0 + app_seq;        // app header
    // !!!!!!!!!!!!!!!!!!!!!!!       
	//
    buf[ptr++] = 0x00;                  // FC : confirm message
    //buf[ptr++] = 0x00;                // IIN
    //buf[ptr++] = 0x00;                // IIN
	//
    mk_crc_data = dnp_mkcrc16(&buf[10], 3);
	//
    buf[ptr++] = (mk_crc_data & 0x00ff);     // crc16
    buf[ptr++] = (mk_crc_data >> 8);         // crc16
	//
    if (debug_dnp_rx)
    {
        printf(">>>>>> DNP Tx: \n");
        for (i = 0;i < 16; i++)
        {
            printf("%02x ", buf[i]);
                if (!((i+1) % 16))
            printf("\n");
        }
        //debug_date();
        printf("\n");
        printf("\n");
    }
	//
    // send(client_sock, buf, 18, 0); /* MSG_DONTWAIT */
    // send(client_sock, buf, ptr, 0);
    if (send(client_sock, buf, ptr, 0) < 0)
    {
        printf("DNPd: send_confirm failed!!!\n");
    }
}        

//
// TPDU (Transport Protocol Data Unit) is included.
// 
/****************************************************************/
void rd_apdu(int client_sock, unsigned char *apdu, int cnt)
/****************************************************************/
{
    int i, apdu_ptr;
    unsigned char  AC_header, FC_header;
    // byte obj_group, obj_var, qual_code;
   
    if (debug_dnp_rx) 
        printf("DNPd: %s() cnt %d \n", __FUNCTION__, cnt);
	//
#if 1
    // TPDU data dump routine - debug
    if (debug_dnp_rx)
    {
        for(i = 0; i < cnt; i++)
        {
            if (apdu[i] < 0x10)
                printf("0");
            printf("%x ", apdu[i]);
            
            if (!((i+1) % 16))
                printf("\n");
        }
        printf("\n");
    }
#endif
	//
    // transport layer protocol
    if (apdu[0] < 0xc0)    // FIR : 1 , FIN :1 
    {
        printf("DNPd: TPDU header error !!! \n");
        return;
    }
	//
    apdu_ptr = 0;
    trans_seq = apdu[apdu_ptr++] & 0x3f;
    //trans_seq++;
    //
    // application layer  
    AC_header = apdu[apdu_ptr++];
    FC_header = apdu[apdu_ptr++];
    //
    app_seq = AC_header & 0x0f;
    //printf("DNPd: app_seq 0x%x \n", app_seq);
    //app_seq++;
	//
    if(AC_header < 0xc0)
    {
        printf("DNPd: apdu header error!!! \n");
        return;
    }
    else if(AC_header > 0xdf)
    {
        send_confirm(client_sock);
    }
	//
    if(FC_header == DNP_CONFIRM)  // ignore DNP confirm message
    {
        //printf("dnp confirm msg!!\n");
        return;  
    }
	//
    // obj_group = apdu[apdu_ptr++];
    // obj_var = apdu[apdu_ptr++];
    // qual_code = apdu[apdu_ptr++] & 0x0f;
  
    if(FC_header == DNP_READ)
    {
        //printf("DEBUG: fc_header : 0x01\n");
        //printf("DEBUG: Read Point\n");
        rd_point(client_sock, apdu, cnt);
    }  
    else if(FC_header == DNP_DOPERATE)
    {
        //printf("DEBUG: fc_header : 0x02\n");
        //printf("DEBUG: Write Point\n");
        wr_point(client_sock,apdu,cnt);
    }  
}

/****************************************************************/
void dnp_reset()
/****************************************************************/
{
    link_status = 1;
	//
    if (debug_dnp_rx)
        printf("DNPd: DNP Reset !!! \n");          
}

/****************************************************************/
void send_ACK(int client_sock)
/****************************************************************/
{
    int i = 0;
    int ptr = 0;
    unsigned char buf[32];
    unsigned short mk_crc_data;
	//
    if (debug_dnp_rx)
        printf("DNPd: %s() client_sock %d \n", __FUNCTION__, client_sock);
	//
    buf[ptr++] = 0x05;
    buf[ptr++] = 0x64;
    buf[ptr++] = 0x05;
    buf[ptr++] = 0x00;
    buf[ptr++] = (dnp_master_addr & 0x00ff); // dest addr
    buf[ptr++] = (dnp_master_addr >> 8);   
    buf[ptr++] = (dnp_slave_addr & 0x00ff);  // source addr
    buf[ptr++] = (dnp_slave_addr >> 8);
    //
    mk_crc_data = dnp_mkcrc16(buf, ptr);
	//      
    buf[ptr++] = (mk_crc_data & 0x00ff);     // crc16
    buf[ptr++] = (mk_crc_data >> 8);         // crc16
    //
    // if (send(client_sock, buf, ptr, 0))
    // {
    // }
    // else
    // {
    //     printf("DNPd: send_ack failed!!!\n");
    // }
    //
    if (debug_dnp_rx)
    {
        printf(">>>>>> DNP Tx: \n");
        for (i = 0;i < 10; i++)
        {
            printf("%02x ", buf[i]);
                if (!((i+1) % 16))
            printf("\n");
        }
        //debug_date();
        printf("\n");
        printf("\n");
    }
    //
    if (send(client_sock, buf, ptr, 0) < 0)
    {
        printf("DNPd: send_ACK failed!!!\n");
    }
}

//
// Data Link Layer
//
/****************************************************************/
void rd_lpdu(int client_sock, unsigned char *buf, int cnt)
/****************************************************************/
{
    int i,j,k,t;
    int dl_ptr = 0;
    unsigned char crc_data[20];
    unsigned short mk_crc_data;
    unsigned char crc_chk1, crc_chk2;
    unsigned char dl_msg[300];
    unsigned char lpdu_control;
    short lpdu_d_addr, lpdu_s_addr;
	//
    if (debug_dnp_rx)
        printf("DNPd: %s() cnt %d \n", __FUNCTION__, cnt);
	//
    // Data link layer is 10 bytes.
    for(i = 0; i < 10; i++)
        crc_data[i] = buf[i];
	//
	k = i;  // Position 10 start TPDU
	//
	mk_crc_data = dnp_mkcrc16(crc_data, 8);
	//
    crc_chk1 = (unsigned char)(mk_crc_data >> 8); 
    crc_chk2 = (unsigned char) mk_crc_data;
	//
    // chk_crc_data = (word)(crc_data[k-1] << 8);
    // chk_crc_data += crc_data[k-2];
    //
    // Datalink layer 10byte crc check
	if ((crc_chk1 != crc_data[k-1]) || (crc_chk2 != crc_data[k-2]))
    {
        printf("DNPd: CRC Error 1, CRC 0x%02x%02x, crc_chk1-2 0x%02x%02x \n", 
                    crc_data[k-1], crc_data[k-2], crc_chk1, crc_chk2);
        //printf("DNPd: mk_crc_data : %d\n",(mk_crc_data>>8)&0x00ff);
        //printf("DNPd: mk_crc_data : %d\n",(byte)mk_crc_data);
        //printf("DNPd: mk_crc_data : %d\n",mk_crc_data);
        return;
    }
	//
    // lpdu_rx.length = crc_data[2];    
	lpdu_control = crc_data[3];                   // data link function code
	lpdu_d_addr = (crc_data[5]<<8)+crc_data[4];   // destination address
	lpdu_s_addr = (crc_data[7]<<8)+crc_data[6];   // source address                           
	//
#if 0
    if(lpdu_d_addr != DNP_MY_ADDR)    // destination addr is not my addr.
    {
        printf("addr error!! \n");
        return;
    }    	  
#endif
	//
    dnp_dl_control = lpdu_control;  
    dnp_master_addr = lpdu_s_addr;    // source addr = master addr
    dnp_slave_addr = lpdu_d_addr;     // source addr = master addr
	//		
    // if there is a date for transport layer
    // except 10byte for datalink layer
    // crc data 2byte for 16byte data
    // go to transport layer except crc data?
    k = 10;   
    t = cnt - 10;
	while(k < (cnt))
	{
        i = 0;	
        while ((i<18)&&(i<t))
        {
            crc_data[i] = buf[k+i];
            i++;			
        }
		//
        t -= i;
        k += i;
		//
		mk_crc_data = dnp_mkcrc16(crc_data, i-2);
		//
        crc_chk1 = (unsigned char)(mk_crc_data>>8); 
        crc_chk2 = (unsigned char) mk_crc_data;
		//
        // CRC check
        if ((crc_chk1 != crc_data[i-1]) || (crc_chk2 != crc_data[i-2]))
        {
            // lpdu_rx.flag = RESET;
            printf("DNPd: CRC error 2, crc_data1-2 0x%x 0x%x, crc_chk1-2 0x%x 0x%x \n",
                        crc_data[i-1], crc_data[i-2], crc_chk1, crc_chk2);
            return;	
        }
        //
        for (j = 0; j < i-2; j++)
            dl_msg[dl_ptr++] = crc_data[j];
	}
	//
    // lpdu_rx.lsdu[l] = '\0';
    // Send_Tcp_Data(client_sock, lpdu_rx.lsdu);
    if (lpdu_control == 0x80)
    {
        printf("DEBUG: ACK message receive!!!\n");
        return;
    }
	//
    lpdu_control &= 0x0f;
    switch(lpdu_control)
    {
        // define RESET_LINK 0x00
        case RESET_LINK:
            dnp_reset();
            send_ACK(client_sock);
            break;

        // define REQUEST_CONFIRM 0x03 
        case REQUEST_CONFIRM:       // data link layer require confirm message?
            send_ACK(client_sock);  // confirm message

        // define REQUEST_UNCONFIRM 0x04
        case REQUEST_UNCONFIRM:     // data link layer don't require confirm message?
            //if(PLC_comm)
            rd_apdu(client_sock, dl_msg, dl_ptr); // go to application layer (include transport layer protocol)
            break;

        default:
            break;
    } 
}

/****************************************************************/
void get_command(int client_sock, char *recv_sock,int msg_length)
/****************************************************************/
{
    int i;
    int cnt;  
    int dnp_length, leng_cnt;
    int msg_cnt = 0;
    int dnp_ptr = 0;
    static unsigned char dnp_msg[300];
    //
    if (debug_dnp_tx)
        printf("DNPd: %s() msg_length %d \n", __FUNCTION__, msg_length);
	//
    while(dnp_ptr < msg_length)
    {
        if((recv_sock[dnp_ptr]!= 0x05)||(recv_sock[dnp_ptr+1]!=0x64))   // header 0x0564 error
        {
            printf("DNPd: Header 0x0564 error!!! [%X][%X] \n", recv_sock[dnp_ptr], recv_sock[dnp_ptr+1]);
            return;
        }
		//
        dnp_length = recv_sock[dnp_ptr+2];   
		//
        i = 20;
        cnt = dnp_length - 5;
        leng_cnt = 0;
		//
        while(i--)
        {
            if(cnt > 0)
            {
                leng_cnt++;
                cnt -= 16;
            }
            else
                break;
        }
		//
        dnp_length += 5;
        dnp_length += (leng_cnt * 2);             // dnp msg total length
		//
        for(i = 0; i < dnp_length; i++)
            dnp_msg[i] = recv_sock[dnp_ptr+i];
        //
        //link_status = 1;  
		//
        rd_lpdu(client_sock,dnp_msg,dnp_length);  // go to datalink layer  
        // Send_Tcp_Data(client_sock, dnp_msg);
        dnp_ptr += dnp_length;
    }
}

/****************************************************************/
void dnp_thread(int new_sock)
/****************************************************************/
{
    int i = 0;
	int nConnectFlag = 0;
    int sockopt_rval = 0;
    struct timeval tv;
    unsigned char buf[DNP_BUF_SIZE+1];
    //
    printf("DNPd : DNP thread is created (sock %d). \n", new_sock);
    //
    // unsolicit message is slow for one thread.
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    sockopt_rval = setsockopt(new_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    printf("DNPd : setsockopt sockopt_rval %d \n", sockopt_rval);  
	//
    while(1)
    {
        pthread_mutex_lock(&dnpObserver_mutex);
        //
		//
		if (link_status == 1)
			check_unsol(new_sock);
		//
        DNP_BUF_CNT = recv(new_sock, buf, DNP_BUF_SIZE, 0);
        // DNP_BUF_CNT = recv(new_sock, buf, DNP_BUF_SIZE, MSG_PEEK);
        // DNP_BUF_CNT = recv(new_sock, buf, DNP_BUF_SIZE, MSG_DONTWAIT);
        // printf("new_sock %d, link_status %d, DNP_BUF_CNT %d \n", new_sock, link_status, DNP_BUF_CNT);
		//
		//printf("recv buf = %d\n", DNP_BUF_CNT);
        if (DNP_BUF_CNT > 0) 
        {
            // printf(">>>>>> DNP_BUF_CNT %d \n", DNP_BUF_CNT);
            if (strlen(buf) >= DNP_BUF_SIZE)
                buf[DNP_BUF_SIZE] = '\0';
			//
            if (debug_dnp_rx)
            {
                printf("<<<<<< DNP Rx: DNP_BUF_CNT %d \n", DNP_BUF_CNT);
                /* debug packet dump here */
                for (i = 0;i < DNP_BUF_CNT; i++)
                {
                    printf("%02x ", buf[i]);
                    if (!((i+1) % 16))
                        printf("\n");
                }
                printf("\n");
                printf("\n");
            }
			//
			if (debug_dnp_rx)
				printf("+ DNPd : Get_Command(%d)\n", new_sock);
            
			get_command(new_sock, buf, DNP_BUF_CNT);  // go to Get Command
			
			if (debug_dnp_rx)
				printf("- DNPd : Get_Command(%d)\n", new_sock);
        }
        else if(DNP_BUF_CNT == 0)
        {
            // printf("rcv return %d\n", DNP_BUF_CNT);
            printf("DNPd : Socket closed (sock: %d). \n", new_sock);
            close(new_sock);
            link_status = 0;
            pthread_mutex_unlock(&dnpObserver_mutex);
            break;
        }
        else
        {
        }
        //
        pthread_mutex_unlock(&dnpObserver_mutex);
        //thread_sleep(1);    // MMC thread hang???
    }
    printf("DNPd : DNP_thread exit (sock: %d). \n", new_sock);
	nDnpClientCnt--;
	pthread_exit(0);
}

/****************************************************************/
void accept_request(void)
/****************************************************************/
{
	pthread_t new_dnp_id;
	char *query_string = NULL;
    int dnp_client = -1;
    int svr_dnp_len;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
	int thread_policy;	
	int th_id;
	int th_status;
	int rr_min_priority, rr_max_priority;
	//
    DNP_BUF_CNT = 0;
	nDnpClientCnt = 0;
	memset(&svr_dnp, 0x00, sizeof(svr_dnp)); 
	//
    do 
    {
        svr_dnp_len = sizeof(svr_dnp);
		dnp_client = accept(server_socket, 
						(struct sockaddr *)&svr_dnp,
						&svr_dnp_len);
        //   
        if (dnp_client == -1)
        {
            if(debug_dnp_tx)
				printf("DNPd: Accept failed!!! nDnpClientCnt = %d\n", nDnpClientCnt);    
			sleep(1);
            continue;
        }                             
   	
		nDnpClientCnt++;
   		
        printf("DNPd: Connection from %s [%d] \n", inet_ntoa(svr_dnp.sin_addr), svr_dnp.sin_port);
		//
		if(pthread_attr_init(&thread_attr) != 0)
		{
			printf("Thread attr init error\n");
			close(dnp_client);
			continue;
		}

		pthread_attr_getschedpolicy(&thread_attr, &thread_policy);
		pthread_attr_getschedparam(&thread_attr, &thread_param);
		//
		printf("Default policy is %s, priority is %d\n", 
				(thread_policy == SCHED_FIFO ? "FIFO"
				 : (thread_policy == SCHED_RR ? "RR"
					: (thread_policy == SCHED_OTHER ? "OTHER"
						: "unknown"))), thread_param.sched_priority);
		
		//
		rr_min_priority = sched_get_priority_min(SCHED_RR);
		rr_max_priority = sched_get_priority_max(SCHED_RR);
		printf("[Round Robin] Min : %d, Max : %d\n", rr_min_priority, rr_max_priority);
		//
		th_id = pthread_create(&new_dnp_id, NULL, (void*(*)(void*)) dnp_thread, 
							   (void*) dnp_client);
		printf("Dnp_Observer_Main thread created. Id = %d\n", new_dnp_id);
		//
		dnp_client = -1;    
    } while (1) ;
}  

/****************************************************************/
void dnp_observer_main(void* arg)
/****************************************************************/
{
	//Variables
	int i;
	int one = 1;
	//	
	struct timeval tv;        
	//
	//int server_socket;
	int client_socket; 
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	//
	int fd;
	int sin_size;
	int server_port;
	int status;
	//
	//Initialize
	i = 0;
	//
	tv.tv_sec = 0;
	tv.tv_usec = 10000; //0.01sec
	//
	server_socket = 0;
	client_socket = 0;
	memset(&server_addr, 0, sizeof(server_addr));
	memset(&client_addr, 0, sizeof(client_addr));
	//
	sin_size = sizeof(client_addr);
	server_port = DNP_SERVER_PORT;
	//	
	//Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);
	//	
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	printf("Interface Server Started, Port [%d]\n", server_port);
	//
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server_port);
	//
	//setsockopt(server_socket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	//
	if(bind(server_socket, (struct sockaddr *)&server_addr, 
			sizeof(server_addr)) < 0)
	{	
		printf("[ERROR] In Bind\n");
		close(server_socket);
		return;
	}
	//
	if(listen(server_socket, 10) < 0)
	{
		printf("[ERROR] In Listen\n");
		close(server_socket);
		return;
	}
	//
	accept_request();
}


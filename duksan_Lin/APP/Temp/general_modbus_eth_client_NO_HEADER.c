/*
 * apps/plc_thread.c
 *
 * (C) Copyright 2005  DUKSAN MECASYS CO., LTD.
 * All rights reserved.
 *
 * This program contains confidential information of DUKSAN MECASYS CO., LTD.
 * and unauthorized distribution of this program, or any portion of it, are
 * prohibited.
 *
 * 2008-2-11	Initial version, tyKim.
 */
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

#include "general_modbus_eth_client_NO_HEADER.h"

int debug_general_modbus_eth_client = 1;

unsigned char rx_buf[MAX_BUF];
unsigned char tx_buf[MAX_BUF];

static void mod_sleep(int sec, int msec) 
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = msec * 1000;
    select( 0, NULL, NULL, NULL, &tv );
	return;
}


/********************************************************************************/
void 
Ethernet_Mod_Buf_Clear()
/********************************************************************************/
{                                 
	memset(&tx_buf, 0x00, MAX_BUF);
    memset(&rx_buf, 0x00, MAX_BUF);
}

/********************************************************************************/
int 
Ethernet_Get_Byte_Size(int num_of_points)
/********************************************************************************/
{
    if((num_of_points % 8) == 0)
        return (num_of_points / 8);
    else
        return (num_of_points / 8) + 1;
}

/********************************************************************************/
void 
Ethernet_Make_Byte(int* data, byte* temp_value)
/********************************************************************************/
{
    byte b_temp;
    b_temp = (data[0] << 0) | (data[1] << 1) | (data[2] << 2) | (data[3] << 3) | 
             (data[4] << 4) | (data[5] << 5) | (data[6] << 6) | (data[7] << 7);
             
    *temp_value = b_temp;
    
    return;             
}                 

/********************************************************************************/
int 
Ethernet_Modbus_Request_Write(int client_socket, int txmsg_length)
/********************************************************************************/
{
    //int i;             
    if(send(client_socket, tx_buf, txmsg_length, 0) <= 0)
    {
        printf("TX_msg Sending Error...\n");
        return 0;
    }
    else
    {
        printf("TX_msg Sending Success...\n");
        return 1;
    }
}

/********************************************************************************/
int 
Ethernet_Modbus_Response_Read(int client_socket)
/********************************************************************************/
{
    mod_sleep(0, 300);
    int i = 0;
    int rxmsg_length = 0;
    
    rxmsg_length = recv(client_socket, rx_buf, MAX_BUF, 0);
    
    if (rxmsg_length <= 0)
        printf("RX_msg Receiving Error...\n");
    else 
    {
        printf("RX_msg : ");
        for(i = 0; i < rxmsg_length; i++)
        {
            printf("0x%02X ", rx_buf[i]);
        }
        printf("\n");
    }
        
    return rxmsg_length;
}

/********************************************************************************/
int 
Ethernet_Chk_Rx_Read_Msg(int rxmsg_length)
/********************************************************************************/
{
	if(rxmsg_length == 0)
        return 0;
    
    short temp_crc = 0;
    short rxmsg_crc = 0;
    byte* bp = (byte*) rx_buf;
    bp = bp + rxmsg_length - 2;
                      
    byte* bp1 = (byte*) &rxmsg_crc;
                      
    bp1[0] = (byte)bp[0];
    bp1[1] = (byte)bp[1];
    
    temp_crc = (0x00FF & CRC16_hi(rx_buf, rxmsg_length - 2)) | (0x00FF & CRC16_lo(rx_buf, rxmsg_length - 2)) << 8;
    
    if(rxmsg_crc == temp_crc)
	   	return 1;
    else
	    return 0;
}

/********************************************************************************/
void 
Ethernet_Modbus_Data_Analyze_0102(short num_of_points, byte *piarr_data)
/********************************************************************************/
{
	int i = 0;
    int byte_cnt = Ethernet_Get_Byte_Size(num_of_points);
    
    byte* bp = (byte*) rx_buf;
    bp = bp + 3;
                           
    for(i = 0; i < byte_cnt; i++)
    {
        piarr_data[i] = bp[i];
        if(debug_general_modbus_eth_client == 1)
            printf("Data[%d] : 0x%02X\n", i, piarr_data[i]);
    }      
}

/********************************************************************************/
void 
Ethernet_Modbus_Data_Analyze_0304(short num_of_points, short *piarr_data)
/********************************************************************************/
{
	int i = 0;
    int byte_cnt = num_of_points * sizeof(short);
    
    byte* bp = (byte*) rx_buf;
    bp = bp + 3;
    
    byte* cp = (byte*) piarr_data;
        
    for(i = 0; i < byte_cnt; i++)
    {
        cp[i+1] = bp[i];
        cp[i] = bp[i+1];
        i++;
    }
    
    if(debug_general_modbus_eth_client == 1)
    {
        for(i = 0; i < num_of_points; i++)
        {
            printf("Data[%d] = 0x%02X, %hd\n", i, piarr_data[i], piarr_data[i]);
        }
    }                 
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function Code 0x01 (Read Coil Status, 0X references)
// Function Code 0x02 (Read Input Status, 1X references) 
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
int 
Ethernet_Modbus_Request_0102(int socket, unsigned char unit_id, unsigned char func, 
                             short addr, short num_of_points, byte* piarr_data)
/********************************************************************************************/
{
    int i = 0;
    unsigned char txmsg_id = unit_id;
    unsigned char txmsg_func = func;
    short txmsg_addr = htons(addr);
    short txmsg_num_of_points = htons(num_of_points);
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + 
                       sizeof(txmsg_num_of_points) + sizeof(txmsg_crc);
    int rxmsg_length;
                        
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
    memset(bp, 0x00, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_addr, 2);
    memcpy(bp+4, &txmsg_num_of_points, 2);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+6, &txmsg_crc, 2);
    
    // DEBUG CODE    
    if(debug_general_modbus_eth_client == 1)
    {
        if(txmsg_func == 1)
            printf("TX_msg 01 : ");
        else if(txmsg_func == 2)
            printf("TX_msg 02 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    // Step 2 : Send Out Tx Message
    if(Ethernet_Modbus_Request_Write(socket, txmsg_length) <= 0)
        return -1;
    
    // Step 3 : Send In Rx Message
    //if(rxmsg_length = Ethernet_Modbus_Response_Read(socket) <= 0)
    //    return -1;
    rxmsg_length = Ethernet_Modbus_Response_Read(socket);
    if( rxmsg_length <= 0)
        return -1;

                   
    // Step 4 : Check CRC Data
    if(Ethernet_Chk_Rx_Read_Msg(rxmsg_length) == 1)
    {
        Ethernet_Modbus_Data_Analyze_0102(num_of_points, piarr_data);
        
        if(debug_general_modbus_eth_client == 1)
            printf("MODBUS CRC check Ok.\n");
            
        Ethernet_Mod_Buf_Clear();
        return 1;
    }
    else
    {
        if(debug_general_modbus_eth_client == 1)
            printf("MODBUS CRC check Error.\n");
            
        Ethernet_Mod_Buf_Clear();
        return 0;
    }        
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function Code 0x03 (Read Holding Registers, 4X references)
// Function Code 0x04 (Read Input Registers, 3X references)
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
int 
Ethernet_Modbus_Request_0304(int socket, unsigned char unit_id, unsigned char func, 
                             short addr, short num_of_points, short* piarr_data)
/********************************************************************************************/
{
    int i = 0;
    unsigned char txmsg_id = unit_id;
    unsigned char txmsg_func = func;
    short txmsg_addr = htons(addr);
    short txmsg_num_of_points = htons(num_of_points);
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + 
                       sizeof(txmsg_num_of_points) + sizeof(txmsg_crc);
    int rxmsg_length;
                        
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, 0x00, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_addr, 2);
    memcpy(bp+4, &txmsg_num_of_points, 2);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+6, &txmsg_crc, 2);
    
    // DEBUG CODE    
    if(debug_general_modbus_eth_client == 1)
    {
        if(txmsg_func == 3)
            printf("TX_msg 03 : ");
        if(txmsg_func == 4)
            printf("TX_msg 04 : ");
            
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
        printf("\n");
    }                         
    
    // Step 2 : Send Out Tx Message
    if(Ethernet_Modbus_Request_Write(socket, txmsg_length) == 0)
        return 0;
    
    // Step 3 : Send In Rx Message
    rxmsg_length = Ethernet_Modbus_Response_Read(socket);
                   
    // Step 4 : Check CRC Data
    if(Ethernet_Chk_Rx_Read_Msg(rxmsg_length) == 1)
    {
        Ethernet_Modbus_Data_Analyze_0304(num_of_points, piarr_data);
        
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS CRC check Ok.\n");
          
        Ethernet_Mod_Buf_Clear();
        return 1;
    }
    else
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS CRC check Error.\n"); 
          
        Ethernet_Mod_Buf_Clear();
        return 0;
    }    
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function Code 0x05 (Force Single Coil, 0X references)
// Function Code 0x06 (Preset Single Registers, 4X references)
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/    
int 
Ethernet_Modbus_Request_0506(int socket, unsigned char unit_id, 
                             unsigned char func, short addr, short piarr_data)
/********************************************************************************************/
{
    int i = 0;
    unsigned char txmsg_id = unit_id;
    unsigned char txmsg_func = func;
    short txmsg_addr = htons(addr);
    short txmsg_send_data = htons(piarr_data);
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + 
                       sizeof(txmsg_send_data) + sizeof(txmsg_crc);
    int rxmsg_length;
                        
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, 0x00, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_addr, 2);
    memcpy(bp+4, &txmsg_send_data, 2);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+6, &txmsg_crc, 2);
    
    // DEBUG CODE    
    if(debug_general_modbus_eth_client == 1)
    {
        if(txmsg_func == 5)
            printf("TX_msg 05 : ");
        if(txmsg_func == 6)
            printf("TX_msg 06 : ");
            
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
        printf("\n");
    }    
    
    
    // Step 2 : Send Out Tx Message
    if(Ethernet_Modbus_Request_Write(socket, txmsg_length) == 0)
        return 0;
    
    // Step 3 : Send In Rx Message
    rxmsg_length = Ethernet_Modbus_Response_Read(socket);
                       
    // Step 4 : Check CRC Data
    
    if(Ethernet_Chk_Rx_Read_Msg(rxmsg_length) == 1)
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS CRC check Ok.\n");
          
        Ethernet_Mod_Buf_Clear();
        return 1;
    }
    else
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS CRC check Error.\n"); 
          
        Ethernet_Mod_Buf_Clear();
        return 0;
    }    
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function Code 0x0F (Force Multiple Coils, 0X references)
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
int 
Ethernet_Modbus_Request_0F(int socket, unsigned char unit_id, unsigned char func, 
                           short addr, short num_of_points, unsigned char byte_size, 
                           byte* piarr_data)
/********************************************************************************************/
{
    int i = 0;
    unsigned char txmsg_id = unit_id;
    unsigned char txmsg_func = func;
    
    short txmsg_addr = htons(addr);
    short txmsg_num_of_points = htons(num_of_points);
    unsigned char txmsg_byte_size = byte_size;
    short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + 
                       sizeof(txmsg_num_of_points) + sizeof(txmsg_byte_size) + 
                       txmsg_byte_size + sizeof(txmsg_crc);
    int rxmsg_length;
                        
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
    memset(bp, 0x00, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_addr, 2);
    memcpy(bp+4, &txmsg_num_of_points, 2);
    memcpy(bp+6, &txmsg_byte_size, 1);
    memcpy(bp+7, piarr_data, txmsg_byte_size);
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+7+txmsg_byte_size, &txmsg_crc, 2);

    // DEBUG CODE    
    if(debug_general_modbus_eth_client == 1)
    {
        printf("TX_msg 0F : ");
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
        printf("\n");
    }    
    
    
    // Step 2 : Send Out Tx Message
    if(Ethernet_Modbus_Request_Write(socket, txmsg_length) == 0)
        return 0;
    
    // Step 3 : Send In Rx Message
    rxmsg_length = Ethernet_Modbus_Response_Read(socket);
                   
    // Step 4 : Check CRC Data
    if(Ethernet_Chk_Rx_Read_Msg(rxmsg_length) == 1)
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS 0x0F CRC check Ok.\n");
          
        Ethernet_Mod_Buf_Clear();
        return 1;
    }
    else
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS 0x0F CRC check Error.\n"); 
          
        Ethernet_Mod_Buf_Clear();
        return 0;
    }    
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Function Code 0x10 (Preset Multiple Registers, 4X references)
//////////////////////////////////////////////////////////////////////////////////////////////
/********************************************************************************************/
int 
Ethernet_Modbus_Request_10(int socket, unsigned char unit_id, unsigned char func, 
                           short addr, short num_of_points, unsigned char byte_size, short* piarr_data)
/********************************************************************************************/
{
    int i = 0;
    unsigned char txmsg_id = unit_id;
    unsigned char txmsg_func = func;
    short txmsg_addr = htons(addr);
    short txmsg_num_of_points = htons(num_of_points);
    unsigned char txmsg_byte_size = byte_size;
    short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + 
                       sizeof(txmsg_num_of_points) + sizeof(txmsg_byte_size) + 
                       txmsg_byte_size + sizeof(txmsg_crc);
    int rxmsg_length;
    short stemp;
                        
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
    memset(bp, 0x00, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_addr, 2);
    memcpy(bp+4, &txmsg_num_of_points, 2);
    memcpy(bp+6, &txmsg_byte_size, 1);
    
    for(i = 0; i < num_of_points; i++)
    {
        stemp = htons(piarr_data[i]);
        memcpy(bp+7+(2*i), &stemp, 2);
    }
    
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+7+txmsg_byte_size, &txmsg_crc, 2);
    
    // DEBUG CODE    
    if(debug_general_modbus_eth_client == 1)
    {
        printf("TX_msg 10 : ");
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
        printf("\n");
    }    
    
    
    // Step 2 : Send Out Tx Message
    if(Ethernet_Modbus_Request_Write(socket, txmsg_length) == 0)
        return 0;
    
    // Step 3 : Send In Rx Message
    rxmsg_length = Ethernet_Modbus_Response_Read(socket);
                   
    // Step 4 : Check CRC Data
    if(Ethernet_Chk_Rx_Read_Msg(rxmsg_length) == 1)
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS 0x10 CRC check Ok.\n");
          
        Ethernet_Mod_Buf_Clear();
        return 1;
    }
    else
    {
        if(debug_general_modbus_eth_client == 1)
          printf("MODBUS 0x10 CRC check Error.\n"); 
          
        Ethernet_Mod_Buf_Clear();
        return 0;
    }    
}                                
                                      

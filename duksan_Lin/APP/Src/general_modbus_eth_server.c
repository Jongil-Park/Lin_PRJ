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

#include "general_modbus_eth_server.h"                                               
#include "general_modbus_eth_client.h"                                               

int debug_MBAP_general_modbus_eth_server = 1;
                  

extern int MBAP_Ethernet_Modbus_Request_Read(int fd, char* rx_buf)
{
    int i, rxmsg_length;
    rxmsg_length = recv(fd, rx_buf, MAX_BUF, 0);
    
    printf("Rx_msg : ");
    for(i=0; i<rxmsg_length; i++)
    {
        printf("0x%02X ", rx_buf[i]);
    }
    printf("\n");
    
    return rxmsg_length;
} 
    
extern int MBAP_Ethernet_Modbus_Check_Rxmsg(char* rx_buf, int rxmsg_length)
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
    
extern void MBAP_Ethernet_Modbus_Get_Values_0102(char* rx_buf, byte* slave_addr, short* memory_addr, short* num_of_points, byte* byte_count)
{
    byte* bp;
    
    *slave_addr = rx_buf[0];
    
    bp = (byte*) memory_addr;
    bp[0] = rx_buf[3];
    bp[1] = rx_buf[2];       
    
    bp = (byte*) num_of_points;
    bp[0] = rx_buf[5];
    bp[1] = rx_buf[4];                                                                                        

    //*byte_count = Ethernet_Get_Byte_Size(*num_of_points);
    *byte_count = MBAP_Ethernet_Get_Byte_Size(*num_of_points);
    
    return;
}

extern void MBAP_Ethernet_Modbus_Get_Values_0304(char* rx_buf, byte* slave_addr, short* memory_addr, short* num_of_points, byte* byte_count)
{
    byte* bp;
    
    *slave_addr = rx_buf[0];
    
    bp = (byte*) memory_addr;
    bp[0] = rx_buf[3];
    bp[1] = rx_buf[2];       
    
    bp = (byte*) num_of_points;
    bp[0] = rx_buf[5];
    bp[1] = rx_buf[4];                                                                                        

    *byte_count = *num_of_points * 2;
    
    return;
}

extern void MBAP_Ethernet_Modbus_Get_Values_0506(char* rx_buf, byte* slave_addr, short* memory_addr, short* receive_data)
{
    byte* bp;
    
    *slave_addr = rx_buf[0];
    
    bp = (byte*) memory_addr;
    bp[0] = rx_buf[3];
    bp[1] = rx_buf[2];       
                 
    bp = (byte*) receive_data;
    bp[0] = rx_buf[5];
    bp[1] = rx_buf[4];
    
    return;
}

extern void MBAP_Ethernet_Modbus_Get_Values_0F(char* rx_buf, byte* slave_addr, short* memory_addr, short* num_of_points, byte* byte_count, byte* receive_data)
{
    int i;
    byte* bp;
    
    *slave_addr = rx_buf[0];
    
    bp = (byte*) memory_addr;
    bp[0] = rx_buf[3];
    bp[1] = rx_buf[2];       
                 
    bp = (byte*) num_of_points;
    bp[0] = rx_buf[5];
    bp[1] = rx_buf[4];
    
    *byte_count = rx_buf[6];
    
    for(i = 0; i < *byte_count; i++)
    {
        receive_data[i] = rx_buf[i+7];
    } 
    
    return;
}

extern void MBAP_Ethernet_Modbus_Get_Values_10(char* rx_buf, byte* slave_addr, short* memory_addr, short* num_of_points, byte* byte_count, short* receive_data)
{
    int i = 0, j = 0;
    byte* bp;
    
    *slave_addr = rx_buf[0];
    
    bp = (byte*) memory_addr;
    bp[0] = rx_buf[3];
    bp[1] = rx_buf[2];       
                 
    bp = (byte*) num_of_points;
    bp[0] = rx_buf[5];
    bp[1] = rx_buf[4];
    
    *byte_count = rx_buf[6];
                    
    for(i = 0; i < *byte_count; i++)
    {
        bp = (byte*) &receive_data[j];
        bp[0] = rx_buf[i+8];
        bp[1] = rx_buf[i+7];
        i++;
        j++;
    } 
    
    return;
}

/*
extern int MBAP_Ethernet_Modbus_Response_0102(int fd, char* tx_buf, byte slave_addr, byte function_code, byte byte_count, byte* send_data)
{
    int i = 0;
    unsigned char txmsg_id = slave_addr;
    unsigned char txmsg_func = function_code;
    byte txmsg_byte_count = byte_count;
    byte* txmsg_send_data = send_data;
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_byte_count) + 
                       byte_count + sizeof(txmsg_crc);
                            
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, NULL, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_byte_count, 1);
    memcpy(bp+3, txmsg_send_data, byte_count);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+txmsg_length-2, &txmsg_crc, 2); 	
	
    if(debug_MBAP_general_modbus_eth_server == 1)
    {
        if(txmsg_func == 1)
            printf("TX_msg 01 : ");
            
        else if(txmsg_func == 2)
            printf("TX_msg 02 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    if(send(fd, tx_buf, txmsg_length, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return 0;
    }
} 

extern int MBAP_Ethernet_Modbus_Response_0304(int fd, char* tx_buf, byte slave_addr, byte function_code, byte byte_count, byte* send_data)
{
    int i = 0;
    unsigned char txmsg_id = slave_addr;
    unsigned char txmsg_func = function_code;
    byte txmsg_byte_count = byte_count;
    byte* txmsg_send_data = send_data;
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_byte_count) + 
                       byte_count + sizeof(txmsg_crc);
    
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, NULL, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_byte_count, 1);
    memcpy(bp+3, txmsg_send_data, byte_count);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+txmsg_length-2, &txmsg_crc, 2); 	
	
    if(debug_MBAP_general_modbus_eth_server == 1)
    {
        if(txmsg_func == 3)
            printf("TX_msg 03 : ");
            
        else if(txmsg_func == 4)
            printf("TX_msg 04 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    if(send(fd, tx_buf, txmsg_length, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return 0;
    }
}

extern int MBAP_Ethernet_Modbus_Response_0506(int fd, char* tx_buf, byte slave_addr, byte function_code, short memory_addr, short send_data)
{
    int i = 0;
    unsigned char txmsg_id = slave_addr;
    unsigned char txmsg_func = function_code;
    short txmsg_memory_addr = htons(memory_addr);
    short txmsg_send_data = htons(send_data);
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_memory_addr) + sizeof(txmsg_send_data) + sizeof(txmsg_crc);
    
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, NULL, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_memory_addr, 2);
    memcpy(bp+4, &txmsg_send_data, 2);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+txmsg_length-2, &txmsg_crc, 2); 	
	
    if(debug_MBAP_general_modbus_eth_server == 1)
    {
        if(txmsg_func == 5)
            printf("TX_msg 05 : ");
            
        else if(txmsg_func == 6)
            printf("TX_msg 06 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    if(send(fd, tx_buf, txmsg_length, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return 0;
    }
}   

extern int MBAP_Ethernet_Modbus_Response_0F10(int fd, char* tx_buf, byte slave_addr, byte function_code, short memory_addr, short num_of_points)
{
    int i = 0;
    unsigned char txmsg_id = slave_addr;
    unsigned char txmsg_func = function_code;
    short txmsg_memory_addr = htons(memory_addr);
    short txmsg_num_of_points = htons(num_of_points);
	short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_memory_addr) + sizeof(txmsg_num_of_points) + sizeof(txmsg_crc);
    
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, NULL, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_memory_addr, 2);
    memcpy(bp+4, &txmsg_num_of_points, 2);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+txmsg_length-2, &txmsg_crc, 2); 	
	
    if(debug_MBAP_general_modbus_eth_server == 1)
    {
        if(txmsg_func == 0x0F)
            printf("TX_msg 0F : ");
            
        else if(txmsg_func == 0x10)
            printf("TX_msg 10 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    if(send(fd, tx_buf, txmsg_length, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return 0;
    }
}     

extern int MBAP_Ethernet_Modbus_Respone_Error(int fd, char* tx_buf, byte slave_addr, byte exception_code)
{
    int i = 0;
    unsigned char txmsg_id = slave_addr;
    unsigned char txmsg_func = 0x81;
    unsigned char txmsg_exception_code = exception_code;
    short txmsg_crc;
    byte* bp = (byte*) tx_buf;
    int txmsg_length = sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_exception_code) + sizeof(txmsg_crc);
    
    // Step 1 : Make Struct Tx Message
    // !!!! Comunicataion use Big Endian. Because Buffer Potision Change!!!
	
    memset(bp, NULL, MAX_BUF);
    memcpy(bp, &txmsg_id, 1);
    memcpy(bp+1, &txmsg_func, 1);
    memcpy(bp+2, &txmsg_exception_code, 1);
        
    txmsg_crc = (0x00FF & CRC16_hi(tx_buf, txmsg_length - 2)) | (0x00FF & CRC16_lo(tx_buf, txmsg_length - 2)) << 8;
    memcpy(bp+txmsg_length-2, &txmsg_crc, 2); 	
	
    if(debug_MBAP_general_modbus_eth_server == 1)
    {
        printf("TX_msg 81 : ");
        
        for (i = 0; i < txmsg_length; i++)
            printf("0x%02X ", tx_buf[i]);
            
        printf("\n");
    }
    
    if(send(fd, tx_buf, txmsg_length, 0) > 0)
    {
        printf("TX_msg Sending Success...\n\n");
        return 1;
    }
    else
    {
        printf("TX_msg Sending Failed...\n\n");        
        return 0;
    }
}
*/


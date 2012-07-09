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

#define MAX_ETHERNET_BUF             2048
#define SUCCESS                     1
#define FAIL                        0
#define byte						unsigned char

#include "general_modbus_eth_client.h"      



int debug_MBAP_general_modbus_eth_client = 0;

unsigned char rx_buf[MAX_ETHERNET_BUF];
unsigned char tx_buf[MAX_ETHERNET_BUF];


/********************************************************************************/
void 
MBAP_Ethernet_Mod_Buf_Clear()
/********************************************************************************/
{                                 
	memset(tx_buf, 0, sizeof(rx_buf));
	memset(rx_buf, 0, sizeof(tx_buf));
}

//Get Byte Size 
/********************************************************************************/
int 
MBAP_Ethernet_Get_Byte_Size(int num_of_points)
/********************************************************************************/
{
	if((num_of_points % 8) == 0)
		return (num_of_points / 8);
	else
		return (num_of_points / 8) + 1;
}


//Make Byte Packet
/********************************************************************************/
void 
MBAP_Ethernet_Make_Byte(int* data, unsigned char* temp_value)
/********************************************************************************/
{
	unsigned char b_temp;
	b_temp = (data[0] << 0) | (data[1] << 1) | (data[2] << 2) | (data[3] << 3) | 
		(data[4] << 4) | (data[5] << 5) | (data[6] << 6) | (data[7] << 7);
	
	*temp_value = b_temp;
	
	return;             
}                 

/********************************************************************************/
int 
MBAP_Ethernet_Modbus_Request_Write(int client_socket, short txmsg_func, 
								   int txmsg_total_length)
/********************************************************************************/
{
	int i;
	
	if(send(client_socket, tx_buf, txmsg_total_length, 0) <= 0)
	{
		printf("[Error] TX_msg Sending Error\n");
		return FAIL;
	}
	else
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
		{
			printf("[OK] TX_msg Sending Success\n");
			printf("TX_MSG %02X : \n", txmsg_func);
			
			for (i = 0; i < txmsg_total_length; i++)
			{
				printf("%02X ", tx_buf[i]);
				if (!((i+1) % 16))
					printf("\n");
			}            
			printf("\n");
		}
		
		return SUCCESS;
	}
}

/********************************************************************************/
int 
MBAP_Ethernet_Modbus_Response_Read(int client_socket)
/********************************************************************************/
{
	int i = 0;
	
	int rxmsg_length = 0;
	
	rxmsg_length = recv(client_socket, rx_buf, MAX_ETHERNET_BUF, 0);
	
	if (rxmsg_length <= 0)
	{
		printf("[Error] RX_msg Receiving\n");
	}
	else 
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
		{
			printf("[OK] RX_msg Receiving\n"); 
			printf("RX_msg : \n");
			for(i = 0; i < rxmsg_length; i++)
			{
				printf("%02X ", rx_buf[i]);
				if (!((i+1) % 16))
					printf("\n");
			}
			printf("\n");
		}
	}
	
	return rxmsg_length;
}
                                                 
/********************************************************************************/
int 
MBAP_Ethernet_Chk_Rx_Read_Msg(short transaction, unsigned char unit_id)
/********************************************************************************/
{
	return SUCCESS;
}

/********************************************************************************/
void 
MBAP_Ethernet_Modbus_Data_Analyze_0102(short num_of_points, unsigned char *piaar_data)
/********************************************************************************/
{
	int i = 0;
	int j = 0;
	int byte_cnt = MBAP_Ethernet_Get_Byte_Size(num_of_points);

	unsigned char* bp = (unsigned char*) rx_buf;
	bp = bp + 9;
	
	for(i = 0; i < byte_cnt; i++)
	{
		for(j = 0; j < 8; j++)
		{
			piaar_data[(8 * i) + j] = (bp[i] >> j) & 1;
			if(debug_MBAP_general_modbus_eth_client == 1)
				printf("Data[%d] : 0x%02X\n", 8 * i + j, piaar_data[8 * i + j]);
		}
	}      
}

/********************************************************************************/
void 
MBAP_Ethernet_Modbus_Data_Analyze_0304(short num_of_points, short *piaar_data)
/********************************************************************************/
{
	int i = 0;
	int byte_cnt = num_of_points * sizeof(short);
	
	unsigned char* bp = (unsigned char*) rx_buf;
	bp = bp + 9;
	
	unsigned char* cp = (unsigned char*) piaar_data;
	
	for(i = 0; i < byte_cnt; i++)
	{
		cp[i+1] = bp[i];
		cp[i] = bp[i+1];
		i++;
	}
	
	if(debug_MBAP_general_modbus_eth_client == 1)
	{
		for(i = 0; i < num_of_points; i++)
		{
			 printf("Data[%d] = 0x%02X, %hd\n", i, piaar_data[i], piaar_data[i]);
		}
	}                 
}

//////////////////////////////////////////////////////////////////////////////////
// Function Code 0x01 (Read Coil Status, 0X references)
// Function Code 0x02 (Read Input Status, 1X references) 
/////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
int 
MBAP_Ethernet_Modbus_Request_0102(int socket, short transaction, unsigned char unit_id, 
		                          unsigned char func, short addr, 
								  short num_of_points, unsigned char* piarr_data)
/********************************************************************************/
{
	//Variables
	int i;
	unsigned char* bp;
	//
	short txmsg_transaction;
	short txmsg_protocol;
	unsigned char txmsg_id;
	unsigned char txmsg_func;
	short txmsg_addr;
	short txmsg_num_of_points;
	short txmsg_length;
	//
	int rxmsg_length;
	int txmsg_total_length;

	
	//Initialize
	i = 0;
	bp = (unsigned char*) tx_buf;
	//	
	txmsg_transaction = htons(transaction); 
	txmsg_protocol = 0;
	txmsg_id = unit_id;
	txmsg_func = func;
	txmsg_addr = htons(addr);
	txmsg_num_of_points = htons(num_of_points);
	txmsg_length = htons(sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + sizeof(txmsg_num_of_points));	
	//
	rxmsg_length = 0;
	txmsg_total_length = htons(txmsg_length) + 6;
	
	// Step 1 : Make Tx Message
	memset(bp, 0, MAX_ETHERNET_BUF);
	
	//
	// transaction
	// 00 00 
	//       protocol
	//       00 00 
	//  
	//             00 00 00 000
	//
	// 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
	// modbus protocol

	memcpy(bp+0,  &txmsg_transaction,   sizeof(txmsg_transaction));
	memcpy(bp+2,  &txmsg_protocol,      sizeof(txmsg_protocol)); 
	memcpy(bp+4,  &txmsg_length,        sizeof(txmsg_length));
	memcpy(bp+6,  &txmsg_id,            sizeof(txmsg_id));
	memcpy(bp+7,  &txmsg_func,          sizeof(txmsg_func));
	memcpy(bp+8,  &txmsg_addr,          sizeof(txmsg_addr));
	memcpy(bp+10, &txmsg_num_of_points, sizeof(txmsg_num_of_points));
	
	// Step 2 : Send Out Tx Message
	if(MBAP_Ethernet_Modbus_Request_Write(socket, txmsg_func, txmsg_total_length) <= 0) 
		return FAIL;
	
	// Step 3 : Recv In Rx Message
	//if(rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket) <= 0)
	//	return FAIL;
	rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket);
	if( rxmsg_length <= 0)
		return FAIL;
	
		
	// Step 4 : Check Data
	if(MBAP_Ethernet_Chk_Rx_Read_Msg(transaction, unit_id) == SUCCESS)
	{
		MBAP_Ethernet_Modbus_Data_Analyze_0102(num_of_points, piarr_data);
		//
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[OK] Response Message Check\n");
		//	
		MBAP_Ethernet_Mod_Buf_Clear();
		return SUCCESS;
	}
	else
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[ERROR] Response Message Check\n");
		//	
		MBAP_Ethernet_Mod_Buf_Clear();
		return FAIL;
	}        
}

////////////////////////////////?////////////////////////////////////////////////
// Function Code 0x03 (Read Holding Registers, 4X references)
// Function Code 0x04 (Read Input Registers, 3X references)
/////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
int 
MBAP_Ethernet_Modbus_Request_0304(int socket, short transaction, unsigned char unit_id, 
		                          unsigned char func, short addr, short num_of_points, 
								  short* piarr_data)
/********************************************************************************/
{	
	//Variables
	int i;
	unsigned char* bp;
	//
	short txmsg_transaction;
	short txmsg_protocol;
	unsigned char txmsg_id;
	unsigned char txmsg_func;
	short txmsg_addr;
	short txmsg_num_of_points;
	short txmsg_length;
	//
	int rxmsg_length;
	int txmsg_total_length;

	
	//Initialize
	i = 0;
	bp = (unsigned char*) tx_buf;
	//
	txmsg_transaction = htons(transaction); 
	txmsg_protocol = 0;
	txmsg_id = unit_id;
	txmsg_func = func;
	txmsg_addr = htons(addr);
	txmsg_num_of_points = htons(num_of_points);
	txmsg_length = htons(sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + sizeof(txmsg_num_of_points));
	//	
	rxmsg_length = 0;
	txmsg_total_length = htons(txmsg_length) + 6;
	

	// Step 1 : Make Tx Message
	memset(bp, 0, MAX_ETHERNET_BUF);
	
	memcpy(bp+0,  &txmsg_transaction,   sizeof(txmsg_transaction));
	memcpy(bp+2,  &txmsg_protocol,      sizeof(txmsg_protocol)); 
	memcpy(bp+4,  &txmsg_length,        sizeof(txmsg_length));
	memcpy(bp+6,  &txmsg_id,            sizeof(txmsg_id));
	memcpy(bp+7,  &txmsg_func,          sizeof(txmsg_func));
	memcpy(bp+8,  &txmsg_addr,          sizeof(txmsg_addr));
	memcpy(bp+10, &txmsg_num_of_points, sizeof(txmsg_num_of_points));
	
	
	// Step 2 : Send Out Tx Message
	if(MBAP_Ethernet_Modbus_Request_Write(socket, txmsg_func, txmsg_total_length) <= 0)
		return FAIL;
	
	// Step 3 : Send In Rx Message
	//if(rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket) <= 0)
	//	return FAIL;
	rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket);
	if( rxmsg_length <= 0)
		return FAIL;

	
	
	// Step 4 : Check Data
	if(MBAP_Ethernet_Chk_Rx_Read_Msg(transaction, unit_id) == SUCCESS)
	{
		MBAP_Ethernet_Modbus_Data_Analyze_0304(num_of_points, piarr_data);
		
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[OK] Response Message Check\n");
			
			MBAP_Ethernet_Mod_Buf_Clear();
		return SUCCESS;
	}
	else
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[ERROR] Response Message Check\n"); 
			
		MBAP_Ethernet_Mod_Buf_Clear();
		return FAIL;
	}    
}

/////////////////////////////////////////////////////////////////////////////////
// Function Code 0x05 (Force Single Coil, 0X references)
// Function Code 0x06 (Preset Single Registers, 4X references)
/////////////////////////////////////////////////////////////////////////////////
/********************************************************************************/
int 
MBAP_Ethernet_Modbus_Request_0506(int socket, short transaction, unsigned char unit_id, 
		                          unsigned char func, short addr, short send_data)
/********************************************************************************/
{	
	//Variables
	int i;
	unsigned char* bp;
	//
	short txmsg_transaction;
	short txmsg_protocol;
	unsigned char txmsg_id;
	unsigned char txmsg_func;
	short txmsg_addr;
	short txmsg_send_data;
	//short txmsg_num_of_points;
	short txmsg_length;
	//
	int rxmsg_length;
	int txmsg_total_length;

	
	//Initialize
	i = 0;
	bp = (unsigned char*) tx_buf;
	//
	txmsg_transaction = htons(transaction); 
	txmsg_protocol = 0;
	txmsg_id = unit_id;
	txmsg_func = func;
	txmsg_addr = htons(addr);
	txmsg_send_data = htons(send_data);
	txmsg_length = htons(sizeof(txmsg_id) + sizeof(txmsg_func) + sizeof(txmsg_addr) + sizeof(txmsg_send_data));
	//
	rxmsg_length = 0;
	txmsg_total_length = htons(txmsg_length) + 6;
	
	
	// Step 1 : Make Tx Message
	memset(bp, 0, MAX_ETHERNET_BUF);
	
	memcpy(bp+0,  &txmsg_transaction,   sizeof(txmsg_transaction));
	memcpy(bp+2,  &txmsg_protocol,      sizeof(txmsg_protocol)); 
	memcpy(bp+4,  &txmsg_length,        sizeof(txmsg_length));
	memcpy(bp+6,  &txmsg_id,            sizeof(txmsg_id));
	memcpy(bp+7,  &txmsg_func,          sizeof(txmsg_func));
	memcpy(bp+8,  &txmsg_addr,          sizeof(txmsg_addr));
	memcpy(bp+10, &txmsg_send_data,     sizeof(txmsg_send_data));
	
	// Step 2 : Send Out Tx Message
	if(MBAP_Ethernet_Modbus_Request_Write(socket, txmsg_func, txmsg_total_length) <= 0)
		return FAIL;
	
	// Step 3 : Send In Rx Message
	//if(rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket) <= 0)
	//	return FAIL;
	rxmsg_length = MBAP_Ethernet_Modbus_Response_Read(socket);
	if( rxmsg_length <= 0)
		return FAIL;

	
	// Step 4 : Check Data
	if(MBAP_Ethernet_Chk_Rx_Read_Msg(transaction, unit_id) == SUCCESS)
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[OK] Response Message Check\n");
		//	
		MBAP_Ethernet_Mod_Buf_Clear();
		return SUCCESS;
	}
	else
	{
		if(debug_MBAP_general_modbus_eth_client == 1)
			printf("[ERROR] Response Message Check\n"); 
		MBAP_Ethernet_Mod_Buf_Clear();
		return FAIL;
	}    
}



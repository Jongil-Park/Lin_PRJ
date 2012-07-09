#ifndef GENERAL_MODBUS_ETH_CLIENT_H
#define GENERAL_MODBUS_ETH_CLIENT_H



void MBAP_Ethernet_Mod_Buf_Clear(void);
//
int MBAP_Ethernet_Get_Byte_Size(int num_of_points);
//
void MBAP_Ethernet_Make_Byte(int* data, unsigned char* temp_value);
//
int MBAP_Ethernet_Modbus_Request_Write(int client_socket, short txmsg_func,
									   int txmsg_total_length);
//
int MBAP_Ethernet_Modbus_Response_Read(int client_socket);
//
int MBAP_Ethernet_Chk_Rx_Read_Msg(short transaction, unsigned char unit_id);
//
void MBAP_Ethernet_Modbus_Data_Analyze_0102(short num_of_points, unsigned char *piaar_data);
//
void MBAP_Ethernet_Modbus_Data_Analyze_0304(short num_of_points, short *piaar_data);
//
int MBAP_Ethernet_Modbus_Request_0102(int socket, short transaction, unsigned char unit_id,
		                              unsigned char func, short addr,
									  short num_of_points, unsigned char* piarr_data);
//
int MBAP_Ethernet_Modbus_Request_0304(int socket, short transaction, unsigned char unit_id,
		                              unsigned char func, short addr, short num_of_points,
									  short* piarr_data);
//
int MBAP_Ethernet_Modbus_Request_0506(int socket, short transaction, unsigned char unit_id,
		                              unsigned char func, short addr, short send_data);

#endif

/*************************************************************************
* Copyright (C) 2006 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

/* command line tool that sends a BACnet service, and displays the reply */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>       /* for time */
#include <errno.h> /* errno */
#include <string.h> /* strerror() */

#define MY_PRINT_ENABLED 0

#include "bacdef.h"
#include "config.h"
#include "bactext.h"
#include "bacerror.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "npdu.h"
#include "apdu.h"
#include "device.h"
#include "net.h"
#include "datalink.h"
#include "whois.h"
/* some demo stuff needed */
#include "filename.h"
#include "handlers.h"
#include "client.h"
#include "txbuf.h"
#include "dlenv.h"

/* duksan include file. jong2ry */
#include <pthread.h>
#include <sys/poll.h>		// use poll event
#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "bacnet.h"

#define INTERLOCK_00		0
#define INTERLOCK_01		1
#define INTERLOCK_02		2
#define INTERLOCK_03		3
#define INTERLOCK_04		4
#define INTERLOCK_05		5
#define INTERLOCK_06		6
#define INTERLOCK_07		7
#define INTERLOCK_08		8
#define INTERLOCK_09		9
#define INTERLOCK_10		10
#define INTERLOCK_11		11
#define INTERLOCK_12		12
#define INTERLOCK_13		13
#define INTERLOCK_14		14
#define INTERLOCK_15		15
#define INTERLOCK_16		16
#define INTERLOCK_17		17
#define INTERLOCK_18		18
#define INTERLOCK_19		19

#define INTERLOCK_PCM		11
#define EHP_PCM				0


/* duksan readprop data. jong2ry */
//_ReadData_t ReadData[READ_MAX_DATA];

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* global variables used in this file */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
static int32_t Target_Object_Index = BACNET_ARRAY_ALL;

static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;

extern bacnetFileInfo bacnetFile[65536];

extern aoPtbl_t aoPtbl[MAX_OBJECT_INSTANCE];
extern aiPtbl_t aiPtbl[MAX_OBJECT_INSTANCE];
extern doPtbl_t doPtbl[MAX_OBJECT_INSTANCE];
extern diPtbl_t diPtbl[MAX_OBJECT_INSTANCE];
extern msoPtbl_t msoPtbl[MAX_OBJECT_INSTANCE];
extern msiPtbl_t msiPtbl[MAX_OBJECT_INSTANCE];
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

extern int pSet(int pcm, int pno, float value);

extern pthread_mutex_t bacnetAccess_mutex;

extern int bacnetObjCnt;
extern pthread_mutex_t bacnetQ_mutex;
extern point_queue bacnet_message_queue;
extern queue_status status;
extern bacnetFileInfo bacnetFile[65536];

extern int bacnetObjCnt;
int inchun_seabu = 1;
int BAC_DbgShow = 0;


extern int BACnet_writeprop(
	uint32_t DeviceInstance, 
	uint32_t ObjectType, 
	uint32_t ObjectInstance,
	point_info setpoint
	);

/****************************************************************/
void put_bacnet_message_queue(int pcm, int pno, float value)
/****************************************************************/
{
	point_info point;

	point.pcm = pcm;
	point.pno = pno;
	point.value = value;

	pthread_mutex_lock(&bacnetQ_mutex);
	putq(&bacnet_message_queue, &point);
	pthread_mutex_unlock(&bacnetQ_mutex);
	//status.bacnet_message_in++;

	//printf("DEBUG : putQ to bacnet_message_queue pcm = %d pno = %d, val = %f\n", point.pcm, point.pno, point.value);
}


/****************************************************************/
int get_bacnet_message_queue(point_info *pPoint)
/****************************************************************/
{
	int result = ERROR;

	pthread_mutex_lock(&bacnetQ_mutex);
	result = getq(&bacnet_message_queue, pPoint);
	pthread_mutex_unlock(&bacnetQ_mutex);

	if(result == SUCCESS)
	{
		//status.bacnet_message_out++;
		return SUCCESS;
	}
	else
		return ERROR;
}


/****************************************************************/
int check_bacnet_message_queue(point_info point, int *pType)
/****************************************************************/
{
	int index = 0;
	
	printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Check  %d %d %f\n", point.pcm, point.pno, point.value);
	if(point.pcm != ST_PCM_ONOFF && 
			point.pcm != ST_PCM_SETTEMP && 
			point.pcm != ST_PCM_MODE && 
			point.pcm != ST_PCM_AHU)
		return -1;

	if(point.pcm == ST_PCM_AHU)
		return bacnetObjCnt-1;
		
	for (index = 0; index < bacnetObjCnt; index++)
	{
		//printf("point.pcm = %d, file.pno = %d\n", point.pcm, bacnetFile[index].pno);
		if(point.pno == bacnetFile[index].pno)
		{
		printf(">>>>>>>>>>>>>>>>>>>>>>>> return index %d. %f\n", index, point.value);
		
		switch(point.pcm)
		{
			case ST_PCM_ONOFF:	   	*pType = BAC_TYPE_BO;	break;
			case ST_PCM_SETTEMP:  	*pType = BAC_TYPE_AV;	break;
			case ST_PCM_MODE:		*pType = BAC_TYPE_MSO;	break;
		}
		return index;
		}
	}
	return -1;
}


/****************************************************************/
void inchun_interlock(void)
/****************************************************************/
{
	//int st = 0;
	int result = 0;

	//23, 24
	result = g_fExPtbl[EHP_PCM][26] + g_fExPtbl[EHP_PCM][25];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 0 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 0 , 0);
	}
	//printf("Check = %d, %d\n", st++, result);
	
	//27,2A,2B,2C
	result = g_fExPtbl[EHP_PCM][29] + 
		g_fExPtbl[EHP_PCM][32] +
		g_fExPtbl[EHP_PCM][33] +
		g_fExPtbl[EHP_PCM][34];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 1 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 1 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//30,31
	result = g_fExPtbl[EHP_PCM][36] + g_fExPtbl[EHP_PCM][37];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 2 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 2 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//32,33
	result = g_fExPtbl[EHP_PCM][38] + g_fExPtbl[EHP_PCM][39];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 3 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 3 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//34
	result = g_fExPtbl[EHP_PCM][40];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 4 , 1);
		put_bacnet_message_queue(INTERLOCK_PCM, 4, 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 4 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//42,43,44,45
	result = g_fExPtbl[EHP_PCM][53] + 
		g_fExPtbl[EHP_PCM][54] +
		g_fExPtbl[EHP_PCM][55] +
		g_fExPtbl[EHP_PCM][56];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 5 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 5 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//40,41
	result = g_fExPtbl[EHP_PCM][51] + g_fExPtbl[EHP_PCM][52];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 6 , 1);
		put_bacnet_message_queue(INTERLOCK_PCM, 6, 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 6 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//54,55,56,57
	result = g_fExPtbl[EHP_PCM][71] + 
		g_fExPtbl[EHP_PCM][72] +
		g_fExPtbl[EHP_PCM][73] +
		g_fExPtbl[EHP_PCM][74];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 7 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 7 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//49,4A
	result = g_fExPtbl[EHP_PCM][60] + g_fExPtbl[EHP_PCM][61];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 8 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 8 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//50,51
	result = g_fExPtbl[EHP_PCM][67] + g_fExPtbl[EHP_PCM][68];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 9 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 9 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//4B,4C,4D,4E,4F
	result = g_fExPtbl[EHP_PCM][62] + 
		g_fExPtbl[EHP_PCM][63] +
		g_fExPtbl[EHP_PCM][64] +
		g_fExPtbl[EHP_PCM][65] +
		g_fExPtbl[EHP_PCM][66];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 10 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 10 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//58,59,5A
	result = g_fExPtbl[EHP_PCM][75] + 
		g_fExPtbl[EHP_PCM][76] +
		g_fExPtbl[EHP_PCM][77];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 11 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 11 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//68,69
	result = g_fExPtbl[EHP_PCM][91] + g_fExPtbl[EHP_PCM][92];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 12 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 12 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//66,67
	result = g_fExPtbl[EHP_PCM][89] + g_fExPtbl[EHP_PCM][90];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 13 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 13 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);
	
	//78,79,7A
	result = g_fExPtbl[EHP_PCM][104] + 
		g_fExPtbl[EHP_PCM][105] +
		g_fExPtbl[EHP_PCM][106];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 14 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 14 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);

	//73,74,75
	result = g_fExPtbl[EHP_PCM][99] + 
		g_fExPtbl[EHP_PCM][100] +
		g_fExPtbl[EHP_PCM][101];
	if(result > 0)	
	{
		pSet(INTERLOCK_PCM, 15 , 1);
	}
	else
	{			
		pSet(INTERLOCK_PCM, 15 , 0);
	}	
	//printf("Check = %d, %d\n", st++, result);	
}






/* for debugging... */
// 091122 jong2ry. create myhandler
static void MyPrintReadPropertyData(
    BACNET_READ_PROPERTY_DATA * data)
{
    BACNET_APPLICATION_DATA_VALUE value;        /* for decode value data */
    int len = 0;
    uint8_t *application_data;
    int application_data_len;
    bool first_value = true;
    bool print_brace = false;
    char stream[32];

    if (data) 
    	{
/*
#if 0
        if (data->array_index == BACNET_ARRAY_ALL)
            fprintf(stderr, "%s #%u %s\n",
                bactext_object_type_name(data->object_type),
                data->object_instance,
                bactext_property_name(data->object_property));
        else
            fprintf(stderr, "%s #%u %s[%d]\n",
                bactext_object_type_name(data->object_type),
                data->object_instance,
                bactext_property_name(data->object_property),
                data->array_index);
#endif
*/
        application_data = data->application_data;
        application_data_len = data->application_data_len;
        /* FIXME: what if application_data_len is bigger than 255? */
        /* value? need to loop until all of the len is gone... */
        for (;;) 
        {
            len =
                bacapp_decode_application_data(application_data,
                (uint8_t) application_data_len, &value);
            if (first_value && (len < application_data_len)) 
            {
                first_value = false;
/*
#if MY_PRINT_ENABLED
                fprintf(stdout, "{");
#endif
*/
                print_brace = true;
            }
			
			// 091122 jong2ry.
			#if MY_PRINT_ENABLED
				printf("data->object_type = %d\n", data->object_type);	
				printf("data->object_instatnce = %d\n", data->object_instance);	
				printf("data->object_property = %d\n", data->object_property);	
	            printf("get Value = %f \n", (float)value.type.Real);
			#endif	
			
			// Only ReadProperty		
			if (data->object_property == 85) 		
			{
				switch(data->object_type)
				{
					case OBJECT_ANALOG_INPUT:
						aiPtbl[data->object_instance].Value = (float)value.type.Real;
						
						#if MY_PRINT_ENABLED		
							printf("Ptbl val = %f\n", 
								aiPtbl[data->object_instance].Value);
						#endif
						break;	

					case OBJECT_ANALOG_VALUE:
						aoPtbl[data->object_instance].Value = (float)value.type.Real;
						
						#if MY_PRINT_ENABLED		
							printf("(ao)Ptbl val = %f\n", 
								aoPtbl[data->object_instance].Value);
						#endif
						break;	

					case OBJECT_BINARY_INPUT:
						memset(stream, 0x00, sizeof(stream));
						sprintf((char *)&stream, "%s", bactext_binary_present_value_name(value.type.Enumerated));						
						//printf("IN %s\n", bactext_binary_present_value_name(value.type.Enumerated));
						if (strncmp((char *)&stream, "active",6) == 0)
						{
							diPtbl[data->object_instance].Value = 1;
							//printf("ON\n");
						}
						else
							diPtbl[data->object_instance].Value = 0;
						
						#if MY_PRINT_ENABLED		
							printf("(di)Ptbl val = %d\n", 
								diPtbl[data->object_instance].Value);
						#endif
						break;	

					case OBJECT_BINARY_OUTPUT:
						memset(stream, 0x00, sizeof(stream));
						sprintf(stream, "%s", bactext_binary_present_value_name(value.type.Enumerated));						
						//printf("IN %s\n", bactext_binary_present_value_name(value.type.Enumerated));
						if (strncmp(stream, "active",6) == 0)
						{
							doPtbl[data->object_instance].Value = 1;
							//printf("ON\n");
						}
						else
							doPtbl[data->object_instance].Value = 0;
						
						#if MY_PRINT_ENABLED		
							printf("(do)Ptbl val = %d\n", 
								doPtbl[data->object_instance].Value);
						#endif
						break;	

					case OBJECT_MULTI_STATE_OUTPUT:
						msoPtbl[data->object_instance].Value = value.type.Signed_Int - 1;	//!!!! Value add -1					
						#if MY_PRINT_ENABLED		
							printf("(mso)Ptbl val = %d\n", 
								msoPtbl[data->object_instance].Value);
						#endif
						break;							

					case OBJECT_MULTI_STATE_INPUT:
						msiPtbl[data->object_instance].Value = value.type.Signed_Int - 1;	//!!!! Value add -1					
						#if MY_PRINT_ENABLED		
							printf("(msi)Ptbl val = %d\n", 
								msoPtbl[data->object_instance].Value);
						#endif
						break;	
					
					default:
						break;
				}
			}
			//bacapp_print_value(stdout, &value, data->object_property);
            if (len) {
                if (len < application_data_len) {
                    application_data += len;
                    application_data_len -= len;
                    /* there's more! */
/*
#if MY_PRINT_ENABLED
                    fprintf(stdout, ",");
#endif
*/
                } else
                    break;
            } else
                break;
        }
/*
#if MY_PRINT_ENABLED
        if (print_brace)
            fprintf(stdout, "}");
        fprintf(stdout, "\r\n");
#endif
*/
    }
}


// 091122 jong2ry. create my handler
static void my_handler_read_property_ack(
    uint8_t * service_request,
    uint16_t service_len,
    BACNET_ADDRESS * src,
    BACNET_CONFIRMED_SERVICE_ACK_DATA * service_data)
{
    int len = 0;
    BACNET_READ_PROPERTY_DATA data;

    (void) src;
    (void) service_data;        /* we could use these... */
    len = rp_ack_decode_service_request(service_request, service_len, &data);
#if 0
    fprintf(stderr, "Received Read-Property Ack!\n");
#endif
    if (len > 0)
	{
		MyPrintReadPropertyData(&data);
	}
}

static void MyErrorHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    BACNET_ERROR_CLASS error_class,
    BACNET_ERROR_CODE error_code)
{
    /* FIXME: verify src and invoke id */
    (void) src;
    (void) invoke_id;
    printf("BACnet Error: %s: %s\r\n",
        bactext_error_class_name((int) error_class),
        bactext_error_code_name((int) error_code));
    Error_Detected = true;
}

static void MyAbortHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t abort_reason,
    bool server)
{
    /* FIXME: verify src and invoke id */
    (void) src;
    (void) invoke_id;
    (void) server;
    printf("BACnet Abort: %s\r\n",
        bactext_abort_reason_name((int) abort_reason));
    Error_Detected = true;
}

static void MyRejectHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id,
    uint8_t reject_reason)
{
    /* FIXME: verify src and invoke id */
    (void) src;
    (void) invoke_id;
    printf("BACnet Reject: %s\r\n",
        bactext_reject_reason_name((int) reject_reason));
    Error_Detected = true;
}

static void Init_Service_Handlers(
    void)
{
    Device_Init();
    handler_read_property_object_set(OBJECT_DEVICE,
        Device_Encode_Property_APDU, Device_Valid_Object_Instance_Number);
    /* we need to handle who-is
       to support dynamic device binding to us */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_WHO_IS, handler_who_is);
    
    /* handle i-am to support binding to other devices */
    apdu_set_unconfirmed_handler(SERVICE_UNCONFIRMED_I_AM, handler_i_am_bind);
    
    /* set the handler for all the services we don't implement
       It is required to send the proper reject message... */
    apdu_set_unrecognized_service_handler_handler
        (handler_unrecognized_service);
        
    /* we must implement read property - it's required! */
    apdu_set_confirmed_handler(SERVICE_CONFIRMED_READ_PROPERTY,
        handler_read_property);
        
    /* handle the data coming back from confirmed requests */
	// 091122 jong2ry. change ack_handler function.
	//apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
     //   handler_read_property_ack);
	apdu_set_confirmed_ack_handler(SERVICE_CONFIRMED_READ_PROPERTY,
		my_handler_read_property_ack);
	 
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_READ_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}


int BACnet_readprop(uint32_t Device_Instance, uint32_t ObjectType, uint32_t ObjectInstance, uint32_t ObjectProperty)
{
    BACNET_ADDRESS src = {
        0
    };  /* address where message came from */
    uint16_t pdu_len = 0;
    unsigned timeout = 100;     /* milliseconds */
    unsigned max_apdu = 0;
    time_t elapsed_seconds = 0;
    time_t last_seconds = 0;
    time_t current_seconds = 0;
    time_t timeout_seconds = 0;
    uint8_t invoke_id = 0;
    bool found = false;

	//setuup command
	Target_Device_Object_Instance = Device_Instance;
	Target_Object_Type = ObjectType;
	Target_Object_Instance = ObjectInstance;
	Target_Object_Property = ObjectProperty;

	//setuup my info
	Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
	address_init();
	Init_Service_Handlers();
	
	/* configure the timeout values */
    last_seconds = time(NULL);
    timeout_seconds = (apdu_timeout() / 1000) * apdu_retries();
    /* try to bind with the device */
    found =
        address_bind_request(Target_Device_Object_Instance, &max_apdu,
        &Target_Address);
    if (!found) {
        Send_WhoIs(Target_Device_Object_Instance,
            Target_Device_Object_Instance);
		Error_Detected = false;	
    }

	//printf("\n\nfound - 1 (%d)\n", found);    
	/* loop forever */
    for (;;) {
        /* increment timer - exit if timed out */
        current_seconds = time(NULL);

        /* at least one second has passed */
        if (current_seconds != last_seconds)
            tsm_timer_milliseconds(((current_seconds - last_seconds) * 1000));
		
		if (Error_Detected)
            break;
   		/* wait until the device is bound, or timeout and quit */
        if (!found) {
            found =
                address_bind_request(Target_Device_Object_Instance, &max_apdu,
                &Target_Address);
   			//printf("not find invoke_id = %d\n", invoke_id);
        }
        
        //printf("found - 2 (%d)\n", found);    
        if (found) {
            //printf("invoke_id = %d\n", invoke_id);                    
            if (invoke_id == 0) {
                invoke_id =
                    Send_Read_Property_Request(Target_Device_Object_Instance,
                    Target_Object_Type, Target_Object_Instance,
                    Target_Object_Property, Target_Object_Index);
				//printf("find invoke_id = %d\n", invoke_id);                    
            } else if (tsm_invoke_id_free(invoke_id))
                break;
            else if (tsm_invoke_id_failed(invoke_id)) {
                fprintf(stderr, "\rError: TSM Timeout!\r\n");
                tsm_free_invoke_id(invoke_id);
                Error_Detected = true;
                /* try again or abort? */
                break;
            }
        } else {
            /* increment timer - exit if timed out */
            elapsed_seconds += (current_seconds - last_seconds);
            if (elapsed_seconds > timeout_seconds) {
                printf("\rError: APDU Timeout!\r\n");
                Error_Detected = true;
                break;
            }
        }

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);
		
        /* process */
        if (pdu_len) 
        {
        	npdu_handler(&src, &Rx_Buf[0], pdu_len);
			/*
			printf("pdu_len = %d\n", pdu_len);
			for (i = 0; i < pdu_len; i++)
				printf("%x ", Rx_Buf[i]);
			printf("\n");
	        */
        }

        /* keep track of time for next check */
        last_seconds = current_seconds;
    }

    if (Error_Detected)
        return 1;
    return 0;
}


void bacnet_update_ptbl(int pcm, int pno, uint32_t *pObjectType, uint32_t *pObjectInstance)
{
	switch(*pObjectType)
	{
		case OBJECT_ANALOG_INPUT:
			pSet(pcm, pno, aiPtbl[*pObjectInstance].Value);						
			printf("AI pcm = %d, pno = %d, %f \n", pcm, pno, aiPtbl[*pObjectInstance].Value);
			break;
			
		case OBJECT_ANALOG_VALUE:
			pSet(pcm, pno, aoPtbl[*pObjectInstance].Value);
			printf("AO pcm = %d, pno = %d, %f \n", pcm, pno, aoPtbl[*pObjectInstance].Value);
			break;
		
		case OBJECT_BINARY_INPUT:
			pSet(pcm, pno, diPtbl[*pObjectInstance].Value);
			if (pcm == 0)
				pSet(1, pno, diPtbl[*pObjectInstance].Value);
			if (pcm == 1)
				pSet(0, pno, diPtbl[*pObjectInstance].Value);
			printf("DI pcm = %d, pno = %d, %d \n", pcm, pno, diPtbl[*pObjectInstance].Value);
			break;
		
		case OBJECT_BINARY_OUTPUT:
			pSet(pcm, pno, doPtbl[*pObjectInstance].Value);
			if (pcm == 0)
				pSet(1, pno, doPtbl[*pObjectInstance].Value);
			if (pcm == 1)
				pSet(0, pno, doPtbl[*pObjectInstance].Value);
			printf("DO pcm = %d, pno = %d, %d \n", pcm, pno, doPtbl[*pObjectInstance].Value);
			break;						
		
		case OBJECT_MULTI_STATE_OUTPUT:
			pSet(pcm, pno, msoPtbl[*pObjectInstance].Value);
			printf("MSO pcm = %d, pno = %d, %d \n", pcm, pno, doPtbl[*pObjectInstance].Value);
			break;			
						
		case OBJECT_MULTI_STATE_INPUT:
			pSet(pcm, pno, msiPtbl[*pObjectInstance].Value);
			printf("MSI pcm = %d, pno = %d, %d \n", pcm, pno, doPtbl[*pObjectInstance].Value);
			break;					
			
		default:
			break;
	} // switch(objectType) end	
}

/****************************************************************/
void setup_my_info(void)
/****************************************************************/
{
	Device_Set_Object_Instance_Number(BACNET_MAX_INSTANCE);
	address_init();
	Init_Service_Handlers();
}


/****************************************************************/
void bacnet_write_For_AHU(int index, point_info point, int type)
/****************************************************************/
{
	int i = 0;
	int j = 0;
	int bacnetErr = 0;
	int retryCnt = 3;
	uint32_t DeviceInstance = 0;
	uint32_t ObjectType = 0;
	uint32_t ObjectInstance = 0;
	uint32_t ObjectProperty = 0;
	//int pointOffset = 0;
	int ahuTable[64] = {1, 4,	//OnOff Status
				13, 2,		//SetTemp Status
				7, 14,		//Mode Status
				42, 2,		//COOL_CMD_OA_Damper Status
				44, 2,		//COOL_CMD_EA_Damper Status
				46, 2,		//COOL_CMD_MIX_Damper Status
				48, 2,		//HEAT_CMD_OA_Damper Status
				50, 2,		//HEAT_CMD_EA_Damper Status
				52, 2,		//HEAT_CMD_MIX_Damper Status				
				54, 2,		//NOMAL_CMD_OA_Damper Status
				56, 2,		//NOMAL_CMD_EA_Damper Status
				58, 2};		//NOMAL_CMD_MIX_Damper Status	

	int pnoTable[64] = {10, 0,	//OnOff Statua
				10, 2,		//SetTemp Status
				10, 18,		//Mode Status
				10, 9,		//COOL_CMD_OA_Damper Status
				10, 10,		//COOL_CMD_EA_Damper Status
				10, 11,		//COOL_CMD_MIX_Damper Status
				10, 12,		//HEAT_CMD_OA_Damper Status
				10, 13,		//HEAT_CMD_EA_Damper Status
				10, 14,		//HEAT_CMD_MIX_Damper Status				
				10, 15,		//NOMAL_CMD_OA_Damper Status
				10, 16,		//NOMAL_CMD_EA_Damper Status
				10, 17};		//NOMAL_CMD_MIX_Damper Status				

	if(point.pcm != 10)
		return;	
	/*
	printf("index = %d\n", index);
	printf("bacnetFile[index].type  = %d, %d\n", bacnetFile[index].type, (bacnetFile[index].type*65536));
	printf("bacnetFile[index].device  = %d, %d\n", bacnetFile[index].device, (bacnetFile[index].device*4906));
	printf("bacnetFile[index].unit  = %d, %d\n", bacnetFile[index].unit, (bacnetFile[index].unit*256));
	*/
	
	DeviceInstance =  9000 + bacnetFile[index].device;
	DeviceInstance =  DeviceInstance + (bacnetFile[index].type * 16);
	ObjectInstance = (bacnetFile[index].type*65536) +
		(bacnetFile[index].device*4096) +
		(bacnetFile[index].unit*256);
	ObjectProperty = 85;		

	for (i = 0; i < 12; i++)
	{
		index = i*2;	
		if(point.pno == pnoTable[index + 1])
		{
			ObjectType = ahuTable[index + 1];
			//if(BAC_DbgShow)
		//	{
				printf("+ Wp(AHU) (%d) :: %d %d %d %d (%d, %d)\n", 
					bacnetFile[index].pno, 
					DeviceInstance, 
					ObjectType, 
					ObjectInstance + ahuTable[index], 
					ObjectProperty, 
					ahuTable[index],
					ahuTable[index + 1]);						
		//	}
			
			// bacnet writeproperty
			for (j = 0; j < retryCnt; j++)
			{
				pthread_mutex_lock(&bacnetAccess_mutex);
				BACnet_writeprop(DeviceInstance,
					ObjectType, 
					ObjectInstance + ahuTable[index],
					point);
				pthread_mutex_unlock(&bacnetAccess_mutex);
			}
				
			// bacnet readproperty
			pthread_mutex_lock(&bacnetAccess_mutex);
			bacnetErr = 1;
			bacnetErr = BACnet_readprop(DeviceInstance, 
				ObjectType, 
				ObjectInstance + ahuTable[index], 
				ObjectProperty);
			pthread_mutex_unlock(&bacnetAccess_mutex);				
			
			if(bacnetErr == 1)
			{
				printf("- COMM Error(%d) = %d  %d %d %d \n", 
					bacnetFile[index].pno, 
					DeviceInstance, 
					ObjectType, 
					ObjectInstance, 
					ObjectProperty);	
					return;
			}		
		}
	}		
}


/****************************************************************/
void bacnet_write(int index, point_info point, int type)
/****************************************************************/
{
	int i = 0;
	int retryCnt = 3;
	int bacnetErr = 0;
	uint32_t DeviceInstance = 0;
	uint32_t ObjectType = 0;
	uint32_t ObjectInstance = 0;
	uint32_t ObjectProperty = 0;
	int pointOffset = 0;

	if(inchun_seabu) 
	{
		if(point.pcm == 10)
		{
			bacnet_write_For_AHU(index, point, type);
			return;
		}
	}
	
	DeviceInstance =  9000 + bacnetFile[index].device;
	DeviceInstance =  DeviceInstance + (bacnetFile[index].type * 16);
	ObjectType = type;
	switch(point.pcm)
	{
		case ST_PCM_ONOFF:	  pointOffset = ST_WRITE_ONOFF;	  break;
		case ST_PCM_SETTEMP:  pointOffset = ST_WRITE_SETTEMP; break;
		case ST_PCM_MODE:	  pointOffset = ST_WRITE_MODE;	  break;
	}
	ObjectInstance = bacnetFile[index].type*65536 +
		bacnetFile[index].device*4096 +
		bacnetFile[index].unit*256 +
		pointOffset;
	ObjectProperty = 85;
	
	//if(BAC_DbgShow)
	//{
		printf("+ Wp (%d) :: %d %d %d %d %f\n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty,
			point.value);	
	//}

	// bacnet writeproperty
	for (i = 0; i < retryCnt; i++)
	{
		pthread_mutex_lock(&bacnetAccess_mutex);
		BACnet_writeprop(DeviceInstance,
			ObjectType, 
			ObjectInstance,
			point);
		pthread_mutex_unlock(&bacnetAccess_mutex);
	}
		
	// bacnet readproperty
	pthread_mutex_lock(&bacnetAccess_mutex);
	bacnetErr = 1;
	bacnetErr = BACnet_readprop(DeviceInstance, 
					ObjectType, 
					ObjectInstance, 
					ObjectProperty);
	pthread_mutex_unlock(&bacnetAccess_mutex);

	// Error Check
	if(bacnetErr == 1)
	{
		printf("- COMM Error(%d) = %d  %d %d %d \n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty);	
			return;
	}

	if(BAC_DbgShow)
	{
		printf("- Wp (%d) :: %d %d %d %d %f\n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty,
			point.value);	
	}
}



/****************************************************************/
void read_readPoint(int index, int pointOffset)
/****************************************************************/
{
	int bacnetErr = 0;
	int pcm = 0;
	uint32_t DeviceInstance = 0;
	uint32_t ObjectType = 0;
	uint32_t ObjectInstance = 0;
	uint32_t ObjectProperty = 0;
	
	//Inchun iface = not read AHU
	if(inchun_seabu) 
	{	
		if(bacnetFile[index].type == 2)
			return;
	}
		
	DeviceInstance =  9000 + bacnetFile[index].device;
	DeviceInstance =  DeviceInstance + (bacnetFile[index].type * 16);
	switch(pointOffset)
	{
		case ST_READ_ONOFF:	   	ObjectType = BAC_TYPE_BI;		break;
		case ST_READ_SETTEMP:  	ObjectType = BAC_TYPE_AI;		break;
		case ST_READ_MODE:		ObjectType = BAC_TYPE_MSI;		break;
		case ST_READ_TEMP:		ObjectType = BAC_TYPE_AI;		break;
		case ST_READ_ALRAM:		ObjectType = BAC_TYPE_BI;		break;
		case ST_READ_FILTER:	ObjectType = BAC_TYPE_BI;		break;
	}
	ObjectInstance = (bacnetFile[index].type*65536) +
		(bacnetFile[index].device*4096) +
		(bacnetFile[index].unit*256) +
		pointOffset;
	ObjectProperty = 85;
/*
	printf("bacnetFile[index].type  = %d, %d\n", bacnetFile[index].type, (bacnetFile[index].type*65536));
	printf("bacnetFile[index].device  = %d, %d\n", bacnetFile[index].device, (bacnetFile[index].device*4906));
	printf("bacnetFile[index].unit  = %d, %d\n", bacnetFile[index].unit, (bacnetFile[index].unit*256));
	printf("pointOffset  = %d, %d\n", pointOffset, pointOffset);
*/

	//Inchun iface
	if(inchun_seabu) 
	{
		if (bacnetFile[index].type == 1 &&
					pointOffset == ST_READ_SETTEMP)
			return;
	
		if (bacnetFile[index].type == 1 &&
					pointOffset == ST_READ_TEMP)
			return;
		
		if (bacnetFile[index].type == 0 &&
					pointOffset == ST_READ_FILTER)
			return;
	}
	
	//if(BAC_DbgShow)
	//{
		printf("+ Rp(Read) (%d) :: %d  %d %d %d \n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty);		
	//}

	// bacnet readproperty
	pthread_mutex_lock(&bacnetAccess_mutex);
	bacnetErr = 1;
	bacnetErr = BACnet_readprop(DeviceInstance, 
					ObjectType, 
					ObjectInstance, 
					ObjectProperty);
	pthread_mutex_unlock(&bacnetAccess_mutex);
	
	switch(pointOffset)
	{
		case ST_READ_ONOFF:	   	pcm = ST_PCM_ONOFF;		break;
		case ST_READ_SETTEMP:  	pcm = ST_PCM_SETTEMP;	break;
		case ST_READ_MODE:		pcm = ST_PCM_MODE;		break;
		case ST_READ_TEMP:		pcm = ST_PCM_TEMP;		break;
		case ST_READ_ALRAM:		pcm = ST_PCM_ALRAM;		break;
		case ST_READ_FILTER:	pcm = ST_PCM_FILTER;	break;
	}

	if(bacnetErr == 1)
	{
		printf("- COMM Error(%d) = %d  %d %d %d \n\n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty);	
			pSet(pcm, bacnetFile[index].pno, 0);
			if(pcm == 0)
				pSet(pcm+1, bacnetFile[index].pno, 0);
			return;
	}

	// value write in point-table.
	bacnet_update_ptbl(pcm, 
		bacnetFile[index].pno, 
		&ObjectType, 
		&ObjectInstance);

	if(BAC_DbgShow)
	{
		printf("- Rp(Read) (%d) :: %d  %d %d %d \n", 
			bacnetFile[index].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance, 
			ObjectProperty);		
	}
}

/****************************************************************/
void read_readPoint_for_AHU(void)
/****************************************************************/
{
	int i = 0;
	int bacnetErr = 0;
	//int pcm = 0;
	int index  = 0;
	uint32_t DeviceInstance = 0;
	uint32_t ObjectType = 0;
	uint32_t ObjectInstance = 0;
	uint32_t ObjectProperty = 0;

	int ahuTable[64] = {2, 3,	//OnOff Statua
				19, 0,		//SetTemp Status
				8, 13,		//Mode Status
				14, 0,		//VentTemp Status
				28, 0,		//OutTemp Status
				30, 0,		//MixTemp Status
				43, 0,		//COOL_CMD_OA_Damper Status
				45, 0,		//COOL_CMD_EA_Damper Status
				47, 0,		//COOL_CMD_MIX_Damper Status
				49, 0,		//HEAT_CMD_OA_Damper Status
				51, 0,		//HEAT_CMD_EA_Damper Status
				53, 0,		//HEAT_CMD_MIX_Damper Status				
				55, 0,		//NOMAL_CMD_OA_Damper Status
				57, 0,		//NOMAL_CMD_EA_Damper Status
				59, 0,		//NOMAL_CMD_MIX_Damper Status	
				40, 0,		//EA_Damper Status
 				39, 0,		//OA_Damper Status
				41, 0};		//MIX_Damper Status

	int pnoTable[64] = {10, 0,	//OnOff Statua
				10, 2,		//SetTemp Status
				10, 18,		//Mode Status
				10, 3,		//VentTemp Status
				10, 4,		//OutTemp Status
				10, 5,		//MixTemp Status
				10, 9,		//COOL_CMD_OA_Damper Status
				10, 10,		//COOL_CMD_EA_Damper Status
				10, 11,		//COOL_CMD_MIX_Damper Status
				10, 12,		//HEAT_CMD_OA_Damper Status
				10, 13,		//HEAT_CMD_EA_Damper Status
				10, 14,		//HEAT_CMD_MIX_Damper Status				
				10, 15,		//NOMAL_CMD_OA_Damper Status
				10, 16,		//NOMAL_CMD_EA_Damper Status
				10, 17,		//NOMAL_CMD_MIX_Damper Status				
				10, 6,		//EA_Damper Status
 				10, 7,		//OA_Damper Status
				10, 8};		//MIX_Damper Status


	for (i = 0; i < bacnetObjCnt; i++)
	{
		if(bacnetFile[i].type  != 2)
			continue;

		DeviceInstance =  9000 + bacnetFile[i].device;
		DeviceInstance =  DeviceInstance + (bacnetFile[i].type * 16);
		ObjectInstance = (bacnetFile[i].type*65536) +
							(bacnetFile[i].device*4096) +
							(bacnetFile[i].unit*256);
		ObjectProperty = 85;		
	}

	for (i = 0; i < 18; i++)
	{
		//printf("################################################################\n"); 
		index = i*2;		
		ObjectType = ahuTable[index + 1];
		printf("+ Rp(Read) (%d)  :: %d  %d %d %d (%d, %d)\n", 
			bacnetFile[i].pno, 
			DeviceInstance, 
			ObjectType, 
			ObjectInstance + ahuTable[index], 
			ObjectProperty, 
			ahuTable[index],
			ahuTable[index + 1]);		
			
		// bacnet readproperty
		pthread_mutex_lock(&bacnetAccess_mutex);
		bacnetErr = 1;
		bacnetErr = BACnet_readprop(DeviceInstance, 
						ObjectType, 
						ObjectInstance + ahuTable[index], 
						ObjectProperty);
		pthread_mutex_unlock(&bacnetAccess_mutex);

		if(bacnetErr == 1)
		{
			setup_my_info();					
			printf("COMM Error(%d) = %d  %d %d %d \n", 
				bacnetFile[i].pno, 
				DeviceInstance, 
				ObjectType, 
				ObjectInstance, 
				ObjectProperty);	
				pSet(pnoTable[index], pnoTable[index+1], 0);
				return;
		}
		else
		{
		
		switch(ObjectType)
		{
			case OBJECT_ANALOG_INPUT:
				pSet(pnoTable[index], pnoTable[index+1], aiPtbl[ObjectInstance + ahuTable[index]].Value);						
				printf("AI pcm = %d, pno = %d, %f \n", pnoTable[index], pnoTable[index+1], aiPtbl[ObjectInstance + ahuTable[index]].Value);
				break;
				
			case OBJECT_ANALOG_VALUE:
				pSet(pnoTable[index], pnoTable[index+1], aoPtbl[ObjectInstance + ahuTable[index]].Value);
				printf("AO pcm = %d, pno = %d, %f \n", pnoTable[index], pnoTable[index+1], aoPtbl[ObjectInstance + ahuTable[index]].Value);
				break;
			
			case OBJECT_BINARY_INPUT:
				pSet(pnoTable[index], pnoTable[index+1], diPtbl[ObjectInstance + ahuTable[index]].Value);
				printf("DI pcm = %d, pno = %d, %d \n", pnoTable[index], pnoTable[index+1], diPtbl[ObjectInstance + ahuTable[index]].Value);
				break;
			
			case OBJECT_BINARY_OUTPUT:
				pSet(pnoTable[index], pnoTable[index+1], doPtbl[ObjectInstance + ahuTable[index]].Value);
				printf("DO pcm = %d, pno = %d, %d \n", pnoTable[index], pnoTable[index+1], doPtbl[ObjectInstance + ahuTable[index]].Value);
				break;						
			
			case OBJECT_MULTI_STATE_OUTPUT:
				pSet(pnoTable[index], pnoTable[index+1], msoPtbl[ObjectInstance + ahuTable[index]].Value);
				printf("MSO pcm = %d, pno = %d, %d \n", pnoTable[index], pnoTable[index+1], msoPtbl[ObjectInstance + ahuTable[index]].Value);
				break;			
							
			case OBJECT_MULTI_STATE_INPUT:
				pSet(pnoTable[index], pnoTable[index+1], msiPtbl[ObjectInstance + ahuTable[index]].Value);
				printf("MSI pcm = %d, pno = %d, %d \n", pnoTable[index], pnoTable[index+1], msiPtbl[ObjectInstance + ahuTable[index]].Value);
				break;					
				
			default:
				break;
		} // switch(objectType) end	
		}
	}
}

void read_OnOff(void)
{
	int j = 0;
	int st = 0;
	int type = 0;
	int index = 0;
	point_info point;

	if(inchun_seabu) read_readPoint_for_AHU();	// AHU Read
	for (j = 0; j < bacnetObjCnt; j++)
	{
		st = ST_READ_ONOFF;		
		inchun_interlock();
				
		//get write queue
		if (get_bacnet_message_queue(&point) == SUCCESS)			
		{
			index = check_bacnet_message_queue(point, &type);
			printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> find index %d\n", index);
			if (index >= 0)
				bacnet_write(index, point, type);
		}				
		read_readPoint(j, st);
	}

}


/****************************************************************/
int *bacnet_readprop_main(void* arg)
/****************************************************************/
{
	//int i = 0;
	int j = 0;
	int st = 0;
	int index = 0;
	//int loopCnt1 = 0;
	int loopCnt2 = 0;
	int loopCnt3 = 0;
	//int writeSt = 0;
	int type = 0;
	point_info point;
	struct timespec ts;
	int check_onoff = 0;
	
	ts.tv_sec = 0;
	ts.tv_nsec = 1000000; //0.01sec		
	st = ST_READ_TEMP;
	
	while(1)// while super loop start
	{
		//datalink layer init
		datalink_cleanup();
		dlenv_init();

		if(inchun_seabu) read_readPoint_for_AHU();

		// Loop forever.
		for(;;)
		{
			for (j = 0; j < bacnetObjCnt; j++)
			{
				inchun_interlock();
				
				//get write queue
				if (get_bacnet_message_queue(&point) == SUCCESS)
				{
					index = check_bacnet_message_queue(point, &type);
						printf(">>>>>>>>>>>>>>>>>>>>>>>>>>> find index %d\n", index);
					if (index >= 0)
					{
						bacnet_write(index, point, type);
						if (point.pcm == 0)
							check_onoff++;
					}
				}				
			
				//printf("\ni = %d, st = %d\n", j, st); 
				nanosleep(&ts, NULL);			 						
				read_readPoint(j, st);
			}

			if(check_onoff > 0)
				read_OnOff();
			check_onoff = 0;

			if(inchun_seabu) read_readPoint_for_AHU();	// AHU Read
			switch(st)
			{
				case ST_READ_TEMP:
					st = ST_READ_ONOFF;		
					break;

				case ST_READ_ONOFF:
					st = ST_READ_SETTEMP;	
					break;
/*
					if (loopCnt1 > 1)
					{
						loopCnt1 = 0;
						st = ST_READ_SETTEMP;	
					}
					else
					{
						if(inchun_seabu) read_readPoint_for_AHU();				// AHU Read
						loopCnt1++;
						st = ST_READ_ONOFF;
					}
					break;
*/				
				case ST_READ_SETTEMP:
					st = ST_READ_MODE;		
					break;
					
				case ST_READ_MODE:
					st = ST_READ_ALRAM;
					break;
					
				case ST_READ_ALRAM:
					if (loopCnt2 > 2)
					{
						loopCnt2 = 0;
						st = ST_READ_FILTER;	
					}
					else
					{
						loopCnt2++;
						st = ST_READ_TEMP;
					}
					break;

				case ST_READ_FILTER:
					if (loopCnt3 > 2)
					{
						loopCnt3 = 0;
						st = ST_READ_TEMP;
					}
					else
					{
						loopCnt3++;
						st = ST_READ_ONOFF;
					}
					break;							
					
				default :
					st = ST_READ_TEMP;
					break;
			}
			
		}
	}//while super loop end
	return 0;
}


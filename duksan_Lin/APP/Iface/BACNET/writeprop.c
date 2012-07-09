/**************************************************************************
*
* Copyright (C) 2006-2007 Steve Karg <skarg@users.sourceforge.net>
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

/* command line tool that sends a BACnet service, and displays the response */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>       /* for time */
#include <string.h>
#include <errno.h>
#include <ctype.h>      /* toupper */
#include "bactext.h"
#include "iam.h"
#include "arf.h"
#include "tsm.h"
#include "address.h"
#include "config.h"
#include "bacdef.h"
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

/* duksan */
#include <pthread.h>
#include <sys/poll.h>		// use poll event


#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"
#include "queue_handler.h"									// queue handler
#include "bacnet.h"

/* duksan */
extern pthread_mutex_t bacnetQ_mutex;
extern pthread_mutex_t bacnetAccess_mutex;
extern point_queue bacnet_message_queue;

extern aoPtbl_t aoPtbl[MAX_OBJECT_INSTANCE];
extern aiPtbl_t aiPtbl[MAX_OBJECT_INSTANCE];
extern doPtbl_t doPtbl[MAX_OBJECT_INSTANCE];
extern diPtbl_t diPtbl[MAX_OBJECT_INSTANCE];
extern msoPtbl_t msoPtbl[MAX_OBJECT_INSTANCE];
extern msiPtbl_t msiPtbl[MAX_OBJECT_INSTANCE];

aoPtbl_t aoPrePtbl[MAX_OBJECT_INSTANCE];
doPtbl_t doPrePtbl[MAX_OBJECT_INSTANCE];
msoPtbl_t msoPrePtbl[MAX_OBJECT_INSTANCE];

extern int pSet(int pcm, int pno, float value);
extern float g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

#ifndef MAX_PROPERTY_VALUES
#define MAX_PROPERTY_VALUES 64
#endif

/* buffer used for receive */
static uint8_t Rx_Buf[MAX_MPDU] = { 0 };

/* global variables used in this file */
static uint32_t Target_Device_Object_Instance = BACNET_MAX_INSTANCE;
static uint32_t Target_Object_Instance = BACNET_MAX_INSTANCE;
static BACNET_OBJECT_TYPE Target_Object_Type = OBJECT_ANALOG_INPUT;
static BACNET_PROPERTY_ID Target_Object_Property = PROP_ACKED_TRANSITIONS;
/* array index value or BACNET_ARRAY_ALL */
static int32_t Target_Object_Property_Index = BACNET_ARRAY_ALL;
static BACNET_APPLICATION_DATA_VALUE
    Target_Object_Property_Value[MAX_PROPERTY_VALUES];

/* 0 if not set, 1..16 if set */
static uint8_t Target_Object_Property_Priority = 0;

static BACNET_ADDRESS Target_Address;
static bool Error_Detected = false;

#if 0
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

static void MyWritePropertySimpleAckHandler(
    BACNET_ADDRESS * src,
    uint8_t invoke_id)
{
    (void) src;
    (void) invoke_id;
    printf("\r\nWriteProperty Acknowledged!\r\n");
}
#endif

#if 0
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
    /* handle the ack coming back */
    apdu_set_confirmed_simple_ack_handler(SERVICE_CONFIRMED_WRITE_PROPERTY,
        MyWritePropertySimpleAckHandler);
    /* handle any errors coming back */
    apdu_set_error_handler(SERVICE_CONFIRMED_WRITE_PROPERTY, MyErrorHandler);
    apdu_set_abort_handler(MyAbortHandler);
    apdu_set_reject_handler(MyRejectHandler);
}
#endif

int BACnet_writeprop(
	uint32_t DeviceInstance, 
	uint32_t ObjectType, 
	uint32_t ObjectInstance,
	point_info setpoint
	)
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
    //char *value_string = NULL;
    bool status = false;
    //int args_remaining = 0, tag_value_arg = 0, i = 0;
    //BACNET_APPLICATION_TAG property_tag;
    //uint8_t context_tag = 0;
    char value_stream[24];

	/*
	printf("+ Wp (%d)  :: %d  %d %d %f\n", 
		setpoint.pno, 
		DeviceInstance, 
		ObjectType, 
		ObjectInstance, 
		setpoint.value);	
	*/

    /* decode the command line parameters */
    Target_Device_Object_Instance = DeviceInstance;
    Target_Object_Type = ObjectType;
    Target_Object_Instance = ObjectInstance;
    Target_Object_Property = 85;
    Target_Object_Property_Priority = 1;
    Target_Object_Property_Index = -1;

	//printf("ObjectType = %d\n", ObjectType); 
	//printf("OBJECT_BINARY_OUTPUT = %d\n", OBJECT_BINARY_OUTPUT);
	//printf("OBJECT_ANALOG_VALUE = %d\n", OBJECT_ANALOG_VALUE);
	//printf("OBJECT_MULTI_STATE_OUTPUT = %d\n", OBJECT_MULTI_STATE_OUTPUT);

	if (ObjectType == OBJECT_BINARY_OUTPUT && setpoint.value == 1)
	{
	    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_ENUMERATED, "1",
	    	&Target_Object_Property_Value[0]);    		
	}		
	else if (ObjectType == OBJECT_BINARY_OUTPUT && setpoint.value == 0)
	{
	    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_ENUMERATED, "0",
	    	&Target_Object_Property_Value[0]);    		
	}
	else if (ObjectType == OBJECT_ANALOG_VALUE)
	{
	    memset(value_stream, 0x00, sizeof(value_stream));
		sprintf(value_stream, "%.0f", setpoint.value);
	    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_REAL, value_stream,
	    	&Target_Object_Property_Value[0]);    		
	}		
	else if (ObjectType == OBJECT_MULTI_STATE_OUTPUT)
	{
	    memset(value_stream, 0x00, sizeof(value_stream));
		sprintf(value_stream, "%.0f", setpoint.value + 1); 	// !!! value add +1 
	    status = bacapp_parse_application_data(BACNET_APPLICATION_TAG_UNSIGNED_INT, value_stream,
	    	&Target_Object_Property_Value[0]);    		
	}			
	else
	{
		printf("Type Error (%d)\n", ObjectType);
		return 0;
	}

    if (!status) {
        /* FIXME: show the expected entry format for the tag */
        fprintf(stderr, "Error: unable to parse the tag value\r\n");
        return 1;
    }
    Target_Object_Property_Value[0].next = NULL;

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
    }

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
        }
        if (found) {
            if (invoke_id == 0) {
                invoke_id =
                    Send_Write_Property_Request(Target_Device_Object_Instance,
                    Target_Object_Type, Target_Object_Instance,
                    Target_Object_Property, &Target_Object_Property_Value[0],
                    Target_Object_Property_Priority,
                    Target_Object_Property_Index);
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
                Error_Detected = true;
                printf("\rError: APDU Timeout!\r\n");
                break;
            }
        }

        /* returns 0 bytes on timeout */
        pdu_len = datalink_receive(&src, &Rx_Buf[0], MAX_MPDU, timeout);

        /* process */
        if (pdu_len) {
            npdu_handler(&src, &Rx_Buf[0], pdu_len);
        }

        /* keep track of time for next check */
        last_seconds = current_seconds;
    }
    if (Error_Detected)
        return 1;
    return 0;
}



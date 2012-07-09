/******************************************************************/
// file : 	ghp_comm.h
// date : 	2010.03.03.
// author : jong2ry
/******************************************************************/
#define POLL_START_MSG 			0
#define POLL_RECEIVE_TEXT		1
#define POLL_DATA_HANDLER		2
#define POLL_ACK_MSG			3
#define POLL_EOT_MSG			4
#define POLL_WAIT_15SEC			5
#define POLL_RECEIVE_EOT		6			

#define SEL_START_MSG 			0
#define SEL_RECEIVE_START_ACK	1
#define SEL_SEND_TEXT			2
#define SEL_RECEIVE_TEXT_ACK	3
#define SEL_SEND_EOT			4

/*******************************************************************/
void init_poll(void)
/*******************************************************************/
{
	g_PollEvents.fd = g_iUart2Fd; 
	/*
		#define POLLIN 0x0001 // 읽을 데이터가 있다. 
		#define POLLPRI 0x0002 // 긴급한 읽을 데이타가 있다. 
		#define POLLOUT 0x0004 // 쓰기가 봉쇄(block)가 아니다. 
		#define POLLERR 0x0008 // 에러발생 
		#define POLLHUP 0x0010 // 연결이 끊겼음 
		#define POLLNVAL 0x0020 // 파일지시자가 열리지 않은 것 같은, Invalid request (잘못된 요청) 	
	*/
	g_PollEvents.events  = POLLIN | POLLERR;
	g_PollEvents.revents = 0;

						
}

/*******************************************************************/
int OnWriteOut(unsigned char *p, int length)
/*******************************************************************/
{
	write(g_iUart2Fd, p, length);
	return 0;
}

/*******************************************************************/
int OnReadIn(void)
/*******************************************************************/
{
	int iRet = 0;;
	int iBufWp = 0;
	int iLoopCnt = 0;
	
	memset(rxbuf, 0x00, sizeof(rxbuf));
	
	while(1) {
		g_iPollState = poll(				// poll()을 호출하여 event 발생 여부 확인     
			(struct pollfd*)&g_PollEvents,	// event 등록 변수
			1,  							// 체크할 pollfd 개수
			30);  							// time out 시간 (ms)	

		if ( 0 < g_iPollState) {                            // 발생한 event 가 있음
			
			if ( g_PollEvents.revents & POLLIN) {           // event 가 자료 수신?
				iRet = read(g_iUart2Fd, &rxbuf[iBufWp], 32);
				iBufWp += iRet;
				iLoopCnt = 0;
			}
			else if ( g_PollEvents.revents & POLLERR) {     // event 가 에러?
				if ( g_dbgShow )
					printf( "Event is Error. Terminal Broken.\n");
				return -1;
			}
		}
		else {
			if ( 5 < iLoopCnt++ ) {
				if ( g_dbgShow )
					printf("Time out %d\n", iBufWp);
				return iBufWp;
			}
		}
	}	
}


/****************************************************************/
static void clear_tx_buf(void)
/****************************************************************/
{
	memset(txbuf, 0x00, sizeof(txbuf));
}

/****************************************************************/
static void clear_rx_buf(void)
/****************************************************************/
{
	memset(rxbuf, 0x00, sizeof(rxbuf));
}

/****************************************************************/
void Write_Point(int point_number, int pno, int val)
/****************************************************************/
{
	float value = (float)val;
	
	switch(point_number) {
		case 1:		//On Off			
			if (pGet(GHP_ERROR_PCM, pno) > 0 || pGet(GHP_ERROR_NUM_PCM, pno) > 0) {
				pSet(GHP_ONOFF_PCM, pno, 0); 
				pSet(GHP_ONOFF_FAN_PCM, pno, 0); 
				prePtbl[GHP_ONOFF_PCM][pno] = 0;
				prePtbl[GHP_ONOFF_FAN_PCM][pno] = 0;				
				return;
			}
			
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {				
				pSet(GHP_ONOFF_PCM, pno, value); 
				pSet(GHP_ONOFF_FAN_PCM, pno, value); 
				prePtbl[GHP_ONOFF_PCM][pno] = value;
				prePtbl[GHP_ONOFF_FAN_PCM][pno] = value;
				break;
			}
			break;

		case 4:		//Mode
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_MODE_PCM, pno, value); 
				prePtbl[GHP_MODE_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Mode(%d %d %f)\n", GHP_MODE_PCM, pno, value); 
				break;
			}
		case 2:		//Set temperature
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_SET_TEMP_PCM, pno, value); 
				prePtbl[GHP_SET_TEMP_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset SetTemp(%d %d %f)\n ", GHP_SET_TEMP_PCM,  pno, value); 
				break;
			}
			
		case 5:		//Room temperature
			pSet(GHP_TEMP_PCM, pno, value); 
			prePtbl[GHP_TEMP_PCM][pno] = value;
			if ( g_dbgShow ) {
				printf("Pset Temp(%d %d %f)\n ", GHP_TEMP_PCM,  pno, value); 
		
				printf("Pset OnOff(%d %d %f)\n", GHP_ONOFF_PCM,  pno, value);
				printf("Pset OnOff(%d %d %f)\n", GHP_ONOFF_FAN_PCM,  pno, value);
			}
			
			pSet(GHP_ERROR_PCM, pno, 0); 
			pSet(GHP_ERROR_NUM_PCM, pno, 0); 
			
			prePtbl[GHP_ERROR_PCM][pno] = 0;
			prePtbl[GHP_ERROR_NUM_PCM][pno] = 0;					
			
			break;

		case 30:	//Wind Speed
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_WINDSPEED_PCM, pno, value); 
				prePtbl[GHP_WINDSPEED_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Speed(%d %d %f)\n ", GHP_WINDSPEED_PCM,  pno, value); 
				break;
			}

		case 31:	//Wind Direction
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				// 풍향인 경우에만 값에 1을 빼서 값을 쓴다.
				value = value - 1;
				
				pSet(GHP_WINDDIRECTION_PCM, pno, value);
				prePtbl[GHP_WINDDIRECTION_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Direction(%d %d %f)\n ", GHP_WINDDIRECTION_PCM,  pno, value);  
				break;
			}
		
		#ifdef DAEGU_GHP	
		// Remote mode 1 
		// 실내기 리모콘의 운전/정지 버튼을 제어합니다. 
		case 6:	
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_REMOTE_1_PCM, pno, value);
				prePtbl[GHP_REMOTE_1_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Remote 1 (%d %d %f)\n ", GHP_REMOTE_1_PCM,  pno, value);  
				break;
			}
			break;

		// Remote mode 2 
		// 실내기 리모콘의 온도설정 버튼을 제어합니다. 
		case 7:		
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_REMOTE_2_PCM, pno, value);
				prePtbl[GHP_REMOTE_2_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Remote 2 (%d %d %f)\n ", GHP_REMOTE_2_PCM,  pno, value);  
				break;
			}
			break;

		// Remote mode 3 
		// 실내기 리모콘의 냉난방 버튼을 제어합니다. 
		case 8:		
			if (prePtbl[GHP_ONOFF_PCM][pno] != g_fExPtbl[GHP_ONOFF_PCM][pno]) {
				if ( g_dbgShow )
					printf("blocking >>>>>>>>>>>>>>>>>>>>\n");
				break;	
			}
			else {			
				pSet(GHP_REMOTE_3_PCM, pno, value);
				prePtbl[GHP_REMOTE_3_PCM][pno] = value;
				if ( g_dbgShow )
					printf("Pset Remote 3 (%d %d %f)\n ", GHP_REMOTE_3_PCM,  pno, value);  
				break;
			}
			break;
		#endif

		default:
			pno = 254;	
			value = 0;
			break;
	}
	//printf("Pno (%d), Val (%f)\n", pno, value);  	
}

/****************************************************************/
void Write_Point_Table(int block_number, int point_number, int val)
/****************************************************************/
{
	int pno = 0;
	
	for( pno = 0; pno < g_unitCnt; pno++) {
		if(block_number == g_UnitData[pno].block) {
			Write_Point(point_number, pno, val);
			continue;
		}
	}	
}


/****************************************************************/
void Write_Error_Table(int block_number, int point_number, int val)
/****************************************************************/
{
	int pno = 0;
	
	for( pno = 0; pno < g_unitCnt; pno++) {
		if(block_number == g_UnitData[pno].block) {

			if ( g_dbgShow ) {
				printf("Pset Error(%d %d 1)\n", GHP_ERROR_PCM,  pno);
				printf("Pset Error(%d %d 1)\n", GHP_ERROR_NUM_PCM,  pno);
			}

			pSet(GHP_ERROR_PCM, pno, 1); 
			pSet(GHP_ERROR_NUM_PCM, pno, 1); 
			
			prePtbl[GHP_ERROR_PCM][pno] = 1;
			prePtbl[GHP_ERROR_NUM_PCM][pno] = 1;	

			continue;
		}
	}	
}


/****************************************************************/
int Get_Block_Number(unsigned char *p)
/****************************************************************/
{
	int result = 0;
	
	result = ((p[0] - 0x30) * 100);
	result += ((p[1] - 0x30) * 10);
	result += (p[2] - 0x30);
	
	return result;	
}

/****************************************************************/
int Get_Point_Number(unsigned char *p)
/****************************************************************/
{
	int result = 0;
	
	result += ((p[0] - 0x30) * 10);
	result += (p[1] - 0x30);
	
	return result;	
}

/****************************************************************/
int RecvDataParsing(unsigned char *p, int length)
/****************************************************************/
{
	int i = 0;
	int block_number = 0;
	int point_number = 0;
	int value = 0;
	//int checkValue = 0;
	
	for(i = 0; i < length - 1; i++)
	{
		if (p[i] =='#' && p[i + 1] =='C')
		{
			block_number = Get_Block_Number(&p[i + 4]);	
			point_number = Get_Point_Number(&p[i + 7]);
			switch(p[i + 2])
			{
				case '0':
					value = 0;	
					value = ((p[i + 9] - 0x30) * 10);
					value += (p[i + 10] - 0x30);
					// Find Error.
					if (p[i + 11] == '?') {
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 11]);
						Write_Error_Table(block_number, point_number, 1);
					}
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}					
					break;	
								
				case '3':
					value = 0;	
					value = ((p[i + 12] - 0x30) * 100);
					value = ((p[i + 13] - 0x30) * 10);
					value += (p[i + 14] - 0x30);
					// Find Error.
					if (p[i + 9] == '?') {
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 9]);
						Write_Error_Table(block_number, point_number, 1);
					}					
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}					
					break;

				case '4':
					value = 0;	
					value = ((p[i + 12] - 0x30) * 100);
					value = ((p[i + 13] - 0x30) * 10);
					value += (p[i + 14] - 0x30);
					// Find Error.
					if (p[i + 9] == '?') {
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 9]);
						Write_Error_Table(block_number, point_number, 1);
					}
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}						
					break;											
			}
			//printf(">>>> %d, %d, %d\n",block_number, point_number, value);
			//Write_Point_Table(block_number, point_number, value);
		}
		else if (p[i] =='#' && p[i + 1] =='A')
		{
			block_number = Get_Block_Number(&p[i + 4]);	
			point_number = Get_Point_Number(&p[i + 7]);
			switch(p[i + 2])
			{
				case '0':
					value = 0;	
					value = ((p[i + 9] - 0x30) * 10);
					value += (p[i + 10] - 0x30);
					// Find Error.
					if (p[i + 11] == '?')
					{
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 11]);
						Write_Error_Table(block_number, point_number, 1);
					}
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}					
					break;	
								
				case '3':
					value = 0;	
					value = ((p[i + 12] - 0x30) * 100);
					value = ((p[i + 13] - 0x30) * 10);
					value += (p[i + 14] - 0x30);
					// Find Error.
					if (p[i + 9] == '?')
					{
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 9]);
						Write_Error_Table(block_number, point_number, 1);
					}
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}										
					break;

				case '4':
					value = 0;	
					value = ((p[i + 12] - 0x30) * 100);
					value = ((p[i + 13] - 0x30) * 10);
					value += (p[i + 14] - 0x30);
					// Find Error.
					if (p[i + 9] == '?')
					{
						value = 0;
						if ( g_dbgShow )
							printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 9]);
						Write_Error_Table(block_number, point_number, 1);
					}					
					else {
						if ( g_dbgShow )
							printf(">>>> %d, %d, %d\n",block_number, point_number, value);
						Write_Point_Table(block_number, point_number, value);						
					}					
					break;											
			}
			//printf(">>>> %d, %d, %d\n",block_number, point_number, value);
			//Write_Point_Table(block_number, point_number, value);
		}
		else if (p[i] =='#' && p[i + 1] =='D')
		{
			block_number = Get_Block_Number(&p[i + 4]);	
			point_number = Get_Point_Number(&p[i + 7]);
			switch(p[i + 2])
			{
				case '0':
					value = 0;
					if ( g_dbgShow )
						printf("!!!!!!!!!!!!!!!!!!!! find Error %c\n", p[i + 11]);
					Write_Error_Table(block_number, point_number, 1);
					break;	
			}
			//printf(">>>> %d, %d, %d\n",block_number, point_number, value);
			//Write_Point_Table(block_number, point_number, value);
		}
		else if (p[i] =='#' && p[i + 1] =='J')
		{
			continue;
		}		
		else if (p[i] =='#' && p[i + 1] =='K')
		{
			return POLL_REQUIRE_STARTUP;	
		}
		else if (p[i] =='#' && p[i + 1] =='L')
		{
			return POLL_ERROR;	
		}		
		else
			continue;
	}
	return POLL_OK;
}


/****************************************************************/
static void comm_start_msg(int sel)
/****************************************************************/
{
	unsigned char buf[4];

	if (sel == TYPE_POLL) {		// polling.
		buf[0] = 0x30;	
		buf[1] = 0x40;	
		buf[2] = 0x05;	
		//SendUart2(buf, 3);  
		OnWriteOut(buf, 3);
	}
	else {	// selecting.
		buf[0] = 0x30;	
		buf[1] = 0x20;	
		buf[2] = 0x05;	
		//SendUart2(buf, 3);  
		OnWriteOut(buf, 3);
	}		
}

#if 0
/****************************************************************/
static void comm_nak_msg(void)
/****************************************************************/
{
    unsigned char buf;
    
	buf = 0x15;
    //SendUart2(&buf, 1); 
    OnWriteOut(&buf, 1);
}
#endif

/****************************************************************/
static void comm_ack_msg(void)
/****************************************************************/
{
    unsigned char buf;
    
	buf = 0x06;
    //SendUart2(&buf, 1); 
    OnWriteOut(&buf, 1);
}

/****************************************************************/
static void comm_eot_msg(void)
/****************************************************************/
{
    unsigned char buf;
           
	buf = 0x04;
    //SendUart2(&buf, 1);
    OnWriteOut(&buf, 1);
}

/****************************************************************/
static int Send_Polling(void)
/****************************************************************/
{
	int st;
	int result = 0;
	int length = 0;
	int i = 0;
	//unsigned char text[256];
	struct timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = 500000000; //0.5sec
	
	st = POLL_START_MSG;
	if(g_dbgShow) 
		printf(" -P-  POLLLING\n");	
	for(;;)
	{
		switch (st)
		{
			case POLL_START_MSG:
				//if(g_dbgShow) printf(" -P-  POLL_START_MSG\n");
				clear_rx_buf();
				comm_start_msg(TYPE_POLL);		// polling protocol
				st = POLL_RECEIVE_TEXT;
				break;
	
			case POLL_RECEIVE_TEXT:
				//if(g_dbgShow) printf(" -P-  POLL_RECEIVE_TEXT\n");
				//sleep(1);
				//nanosleep(&ts, NULL);
				
				// jong2ry 2011_0303
				//IfaceGhpSelectSleep(0, 500);
				IfaceGhpSelectSleep(0, 100);

				//length = RecvUart2(rxbuf, sizeof(rxbuf));
				length = OnReadIn();
				
				if ( g_dbgShow ) {
					printf(" -P-  length = %d\n", length);	
					for (i = 0; i < length; i++)
						printf("%c ",rxbuf[i]);
					printf("\n======================\n");
				}
				
				if (length == -1) {
					//st = POLL_EOT_MSG;	
					comm_eot_msg();
					return POLL_EMPTY_DATA;
				}
				else if (rxbuf[0] == 0x04)
				{
					if ( g_dbgShow )
						printf("return...\n");
					return POLL_OK;
				}
				else
					st = POLL_DATA_HANDLER;
				break;
	
			case POLL_DATA_HANDLER:
				//if(g_dbgShow) printf(" -P-  POLL_DATA_HANDLER\n");
				//printf("Check >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
				//printf("result = %d\n", result);					
				result = RecvDataParsing(rxbuf, length);
				//printf(" -P-  result = %d\n", length);					
				st = POLL_ACK_MSG;
				break;
	
			case POLL_ACK_MSG:
				//if(g_dbgShow) printf(" -P-  POLL_ACK_MSG\n");
				comm_ack_msg();
				st = POLL_RECEIVE_EOT;
				break;
	
			case POLL_RECEIVE_EOT:
				if(g_dbgShow) 
					printf(" -P-  POLL_RECEIVE_EOT\n");
				clear_rx_buf();
				//length = RecvUart2(text, sizeof(text));
				length = OnReadIn();
				if ( g_dbgShow )
					printf(" -P-  length = %d, %x\n", length, rxbuf[0]);					
				//if(length == 1 && text[0] == 0x04)
				if(length == 1 && rxbuf[0] == 0x04)
				{	
					if (result == POLL_REQUIRE_STARTUP)
						return POLL_REQUIRE_STARTUP;
					else if (result == POLL_ERROR)
						return POLL_ERROR;
					else 
						return POLL_OK;
				}
				else
					st = POLL_EOT_MSG;	
					
			case POLL_EOT_MSG:
				if(g_dbgShow) 
					printf(" -P-  POLL_EOT_MSG\n");
				comm_eot_msg();
				if (result == POLL_REQUIRE_STARTUP)
					return POLL_REQUIRE_STARTUP;
				else if (result == POLL_ERROR)
					return POLL_ERROR;					
				else 
					return POLL_OK;
	
			case POLL_WAIT_15SEC:			
				if(g_dbgShow) 
					printf(" -P-  POLL_WAIT_15SEC\n");
				if (result == POLL_REQUIRE_STARTUP)
					return POLL_REQUIRE_STARTUP;
				else 
					return POLL_NG;
		}
	}
}



/****************************************************************/
static int Send_Selecting(unsigned char *p, int length)
/****************************************************************/
{
	int i = 0;
	int st;
	int result = 0;
	//unsigned char text[32];
	int sendRetry = 0;
	struct timespec ts;

	ts.tv_sec = 0;
	ts.tv_nsec = 500000000; //0.5sec
	
	if(g_dbgShow) 
		printf(" -S-  SELECTING\n");
		
	st = SEL_START_MSG;
	for(;;)
	{
		switch (st)
		{
			case SEL_START_MSG:
				//if(g_dbgShow) printf(" -S-  SEL_START_MSG\n");
				clear_rx_buf();
				comm_start_msg(TYPE_SEL);		// selecting protocol
				st = SEL_RECEIVE_START_ACK;
				break;
	
			case SEL_RECEIVE_START_ACK:
				//if(g_dbgShow) printf(" -S-  SEL_RECEIVE_START_ACK\n");
				//sleep(1);
				//nanosleep(&ts, NULL);
				//IfaceGhpSelectSleep(0, 50);

				// jong2ry 2011_0303
				//IfaceGhpSelectSleep(0, 500);
				IfaceGhpSelectSleep(0, 100);

				
				clear_rx_buf();
				//result = RecvUart2(rxbuf, sizeof(rxbuf));
				result = OnReadIn();
				if ( g_dbgShow )
					printf(" -S- Get Ack result = %d, %x\n", result, rxbuf[0]);		
				//for (i = 0; i < result; i++)
				//	printf("%c ",rxbuf[i]);
				//printf("\n############################\n");
							
				if (result == 1 && rxbuf[0] == 0x06)
				//if (result = 1 && rxbuf[0] == 0x06)
				//if (rxbuf[result - 1] == 0x06)
				{
					st = SEL_SEND_TEXT;
					break;				
				}
				else
				{
					if (sendRetry > 5)
					{
						comm_eot_msg();
						return SEL_RETRY;
					}
					else
					{
						st = SEL_START_MSG;
						sendRetry++;
						//sleep(1);
						IfaceGhpSelectSleep(1, 0);
					}
					break;				
				}
				break;
				
			case SEL_SEND_TEXT:
				if(g_dbgShow) 
					printf(" -S-  SEL_SEND_TEXT\n");
				//SendUart2(p, length);					
				OnWriteOut(p, length);
				if ( g_dbgShow ) {
					for (i = 0; i < length; i++)
						printf("%c ",p[i]);
					printf("\n");
				}
				st = SEL_RECEIVE_TEXT_ACK;
				break;
	
			case SEL_RECEIVE_TEXT_ACK:
				//if(g_dbgShow) printf(" -S-  SEL_RECEIVE_TEXT_ACK\n");
				//sleep(1);
				//nanosleep(&ts, NULL);
				
				// jong2ry 2011_0303
				//IfaceGhpSelectSleep(0, 500);
				IfaceGhpSelectSleep(0, 100);

				
				clear_rx_buf();
				//result = RecvUart2(rxbuf, sizeof(rxbuf));
				result = OnReadIn();
				//for (i = 0; i < result; i++)
				//	printf("%x ",rxbuf[i]);
				//printf("\n");
				st = SEL_SEND_EOT;
				break;
	
			case SEL_SEND_EOT:
				if(g_dbgShow) 
					printf(" -S-  SEL_SEND_EOT\n");
				comm_eot_msg();
				return SEL_OK;
		}
	}
}

/****************************************************************/
void Startup_SDDC(void)
/****************************************************************/
{
	int i = 0;
	int blockNum = 0;
	int textLength = 0;
	int sendResult = 0;
	unsigned char text[64];
	
	for ( blockNum=0; blockNum<255; blockNum++ )
	{
		memset( text, 0x00, sizeof(text) );
		textLength = Make_Startup_Msg ( text, blockNum );	
		printf("[Startup Tx]\n");
		for ( i=0; i < textLength; i++)
			printf("%x ", text[i]);
		printf("\n");
		sendResult = Send_Selecting ( text, textLength );	
		printf("Startup Block Number %d\n", blockNum);
		
		if(sendResult == SEL_RETRY)
		{
			sendResult = Send_Selecting ( text, textLength );	
			printf("Return Retry Startup Block Number %d\n", blockNum);
		}
							
		if (Send_Polling() == POLL_ERROR)
		{
			sendResult = Send_Selecting ( text, textLength );	
			printf("Polling Error Startup Block Number %d\n", blockNum);
			Send_Polling();
		}
	}
	
	printf("[Startup End] ##########################################\n"); 
	memset( text, 0x00, sizeof(text) );
	textLength = Make_Initial_Msg ( text );	
	sendResult = Send_Selecting ( text, textLength );	
	Send_Polling();
	printf("[Startup End] ##########################################\n"); 
	//sleep(1);
	IfaceGhpSelectSleep(1, 0);
}



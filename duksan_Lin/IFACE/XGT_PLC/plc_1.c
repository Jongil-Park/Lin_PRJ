char plc_1_ip[] = "125.7.234.169";

int g_nCountPlc1 = 3;

struct XGT_PLC_COUNT plc_1_count[] = {
	{XGT_AIO_TYPE, 0},	
	{XGT_AIO_TYPE, 200},
	{XGT_DIO_TYPE, 0},
};

struct XGT_PLC_POINT plc_1_point[] = {
	{ XGT_AIO_TYPE,000,000,0,50,0.1,0,1,0},
	{ XGT_AIO_TYPE,001,001,0,50,0.1,0,1,1},
	{ XGT_AIO_TYPE,002,002,0,50,0.1,0,1,2},
	{ XGT_AIO_TYPE,003,003,0,50,0.1,0,1,3},
	{ XGT_AIO_TYPE,004,004,0,50,0.1,0,1,4},
	{ XGT_AIO_TYPE,005,005,0,50,0.1,0,1,5},
	{ XGT_AIO_TYPE,006,006,0,50,0.1,0,1,6},
	{ XGT_AIO_TYPE,007,007,0,50,0.1,0,1,7},
	{ XGT_AIO_TYPE,010,010,0,50,0.1,0,1,8},
	{ XGT_AIO_TYPE,011,011,0,50,0.1,0,1,9},
	{ XGT_AIO_TYPE,200,200,-30,130,0.1,0,1,10},
	{ XGT_AIO_TYPE,201,201,-30,130,0.1,0,1,11},
	{ XGT_AIO_TYPE,202,202,-30,130,0.1,0,1,12},
	{ XGT_AIO_TYPE,203,203,-30,130,0.1,0,1,13},
	{ XGT_AIO_TYPE,204,204,-30,130,0.1,0,1,14},
	{ XGT_AIO_TYPE,205,205,-30,130,0.1,0,1,15},
	{ XGT_AIO_TYPE,206,206,-30,130,0.1,0,1,16},
	{ XGT_AIO_TYPE,207,207,-10,70,0.1,0,1,17},
	{ XGT_AIO_TYPE,210,210,0,100,0.1,0,1,18},
	{ XGT_AIO_TYPE,211,211,0,100,0.1,0,1,19},
	{ XGT_AIO_TYPE,212,212,0,100,0.1,0,1,20},
	{ XGT_AIO_TYPE,213,213,0,100,0.1,0,1,21},
	{ XGT_AIO_TYPE,214,214,0,100,0.1,0,1,22},
	{ XGT_AIO_TYPE,215,215,0,100,0.1,0,1,23},
	{ XGT_AIO_TYPE,216,216,0,100,0.1,0,1,24},
	{ XGT_AIO_TYPE,217,217,0,100,0.1,0,1,25},
	{ XGT_AIO_TYPE,220,220,0,100,0.1,0,1,26},
	{ XGT_AIO_TYPE,221,221,0,100,0.1,0,1,27},
	{ XGT_AIO_TYPE,222,222,0,100,0.1,0,1,28},
	{ XGT_AIO_TYPE,223,223,0,100,0.1,0,1,29},
	{ XGT_AIO_TYPE,224,224,0,100,0.1,0,1,30},
	{ XGT_AIO_TYPE,225,225,0,100,0.1,0,1,31},
	{ XGT_AIO_TYPE,226,226,0,100,0.1,0,1,32},
	{ XGT_AIO_TYPE,230,230,0,50,0.1,0,1,33},
	{ XGT_AIO_TYPE,231,231,0,100,0.1,0,1,34},
	{ XGT_AIO_TYPE,232,232,0,50,0.1,0,1,35},
	{ XGT_AIO_TYPE,233,233,0,100,0.1,0,1,36},
	{ XGT_AIO_TYPE,234,234,0,50,0.1,0,1,37},
	{ XGT_AIO_TYPE,235,235,0,100,0.1,0,1,38},
	{ XGT_AIO_TYPE,236,236,0,50,0.1,0,1,39},
	{ XGT_AIO_TYPE,237,237,0,100,0.1,0,1,40},
	{ XGT_AIO_TYPE,238,238,0,50,0.1,0,1,41},
	{ XGT_AIO_TYPE,239,239,0,100,0.1,0,1,42},
	{ XGT_AIO_TYPE,208,208,0,50,0.1,0,1,43},
	{ XGT_AIO_TYPE,218,218,0,100,0.1,0,1,44},
	{ XGT_DIO_TYPE,00,0,0,0,0,0,4,0},
	{ XGT_DIO_TYPE,00,1,0,0,0,0,4,1},
	{ XGT_DIO_TYPE,00,2,0,0,0,0,4,2},
	{ XGT_DIO_TYPE,01,0,0,0,0,0,4,3},
	{ XGT_DIO_TYPE,01,1,0,0,0,0,4,4},
	{ XGT_DIO_TYPE,01,2,0,0,0,0,4,5},
	{ XGT_DIO_TYPE,10,0,0,0,0,0,4,6},
	{ XGT_DIO_TYPE,10,1,0,0,0,0,4,7},
	{ XGT_DIO_TYPE,10,2,0,0,0,0,4,8},
	{ XGT_DIO_TYPE,10,3,0,0,0,0,4,9},
	{ XGT_DIO_TYPE,10,4,0,0,0,0,4,10},
	{ XGT_DIO_TYPE,10,5,0,0,0,0,4,11},
	{ XGT_DIO_TYPE,10,6,0,0,0,0,4,12},
	{ XGT_DIO_TYPE,10,7,0,0,0,0,4,13},
	{ XGT_DIO_TYPE,10,8,0,0,0,0,4,14},
	{ XGT_DIO_TYPE,10,9,0,0,0,0,4,15},
	{ XGT_DIO_TYPE,10,10,0,0,0,0,4,16},
	{ XGT_DIO_TYPE,10,11,0,0,0,0,4,17},
	{ XGT_DIO_TYPE,10,12,0,0,0,0,4,18},
	{ XGT_DIO_TYPE,10,13,0,0,0,0,4,19},
	{ XGT_DIO_TYPE,10,14,0,0,0,0,4,20},
	{ XGT_DIO_TYPE,10,15,0,0,0,0,4,21},
	{ XGT_DIO_TYPE,11,0,0,0,0,0,4,22},
	{ XGT_DIO_TYPE,11,1,0,0,0,0,4,23},
	{ XGT_DIO_TYPE,11,2,0,0,0,0,4,24},
	{ XGT_DIO_TYPE,11,3,0,0,0,0,4,25},
	{ XGT_DIO_TYPE,11,4,0,0,0,0,4,26},
	{ XGT_DIO_TYPE,11,5,0,0,0,0,4,27},
	{ XGT_DIO_TYPE,11,6,0,0,0,0,4,28},
	{ XGT_DIO_TYPE,11,7,0,0,0,0,4,29},
	{ XGT_DIO_TYPE,11,8,0,0,0,0,4,30},
	{ XGT_DIO_TYPE,11,9,0,0,0,0,4,31},
	{ XGT_DIO_TYPE,11,10,0,0,0,0,4,32},
	{ XGT_DIO_TYPE,11,11,0,0,0,0,4,33},
	{ XGT_DIO_TYPE,11,12,0,0,0,0,4,34},
	{ XGT_DIO_TYPE,11,13,0,0,0,0,4,35},
	{ XGT_DIO_TYPE,11,14,0,0,0,0,4,36},
	{ XGT_DIO_TYPE,14,0,0,0,0,0,4,37},
	{ XGT_DIO_TYPE,14,1,0,0,0,0,4,38},
	{ XGT_DIO_TYPE,14,2,0,0,0,0,4,39},
	{ XGT_DIO_TYPE,14,3,0,0,0,0,4,40},
	{ XGT_DIO_TYPE,14,4,0,0,0,0,4,41},
	{ XGT_DIO_TYPE,14,5,0,0,0,0,4,42},	
};



//-----------------------------------------------------------------------------
void xgt_parsing_aio_plc_1 ( 
	int nSocket, 
	unsigned int nAddr, 
	struct XGT_PLC_POINT *pPoint, 
	int nCount ) 
//-----------------------------------------------------------------------------
{
	int i = 0;
	int nIndex = 0;
	int nRecvByte = 0;
	
	nRecvByte = recv(nSocket,  g_chRxMsg, XGT_BUFFER_SIZE, 0);
	
	printf("AIO Parsing nRecvByte = %d nAddr = %d\n", nRecvByte, nAddr);

	for ( nIndex = 0; nIndex < 50; nIndex++ ) {	
		for ( i = 0; i < nCount; i++ ) {
			if ( pPoint[i].paddr == nAddr+nIndex && pPoint[i].type == XGT_AIO_TYPE) {
				printf("pPoint[%d]->pcm = %d  ", i, pPoint[i].pcm);	
				printf("pPoint[%d]->pno = %d\n", i, pPoint[i].pno);	
			}
		}
	}
}


//-----------------------------------------------------------------------------
void xgt_parsing_dio_plc_1 (
	int nSocket, 
	unsigned int nAddr, 
	struct XGT_PLC_POINT *pPoint, 
	int nCount ) 
//-----------------------------------------------------------------------------
{
	int i = 0;
	int nIndex = 0;
	int nRecvByte = 0;
	
	nRecvByte = recv(nSocket,  g_chRxMsg, XGT_BUFFER_SIZE, 0);
	
	printf("DIO Parsing nRecvByte = %d nAddr = %d\n", nRecvByte, nAddr);

	for ( nIndex = 0; nIndex < 50; nIndex++ ) {	
		for ( i = 0; i < nCount; i++ ) {
			if ( pPoint[i].paddr == nAddr+nIndex && pPoint[i].type == XGT_DIO_TYPE) {
				printf("pPoint[%d]->pcm = %d  ", i, pPoint[i].pcm);	
				printf("pPoint[%d]->pno = %d\n", i, pPoint[i].pno);	
			}
		}
	}
}


//-----------------------------------------------------------------------------
void xgt_plc_1_process(void)
//-----------------------------------------------------------------------------
{
	int i = 0;
	int nCount = 0;
	int nPlcSelect = 1;
	int nSocket = 0;
	struct timespec ts;
	
	printf("\n\nxgt plc %d\n", nPlcSelect);
	
	// connect plc 1
	nSocket = xgt_connect_plc(plc_1_ip);
	if ( nSocket < 0 )
		return;
	else
		printf("connecting xgt plc socket %d\n", nSocket);		

	// get point list count
	nCount = sizeof(plc_1_point) / sizeof(struct XGT_PLC_POINT);
	printf("nCount = %d\n", nCount);	
	
	for ( i = 0; i < g_nCountPlc1; i++ ) {
		if ( plc_1_count[i].type == XGT_AIO_TYPE ) {
			
			// read xgt data (aio)
			xgt_read_aio(nSocket,  plc_1_count[i].addr);
			
			// 500msec delay
			ts.tv_sec = 0;
			ts.tv_nsec = 500 * 1000000;			
			nanosleep(&ts, NULL);	
						
			// parsing xgt data
			xgt_parsing_aio_plc_1(nSocket,  plc_1_count[i].addr, &plc_1_point[0], nCount);
		}
		else if  ( plc_1_count[i].type == XGT_DIO_TYPE ) {
			// read xgt data (dio)
			xgt_read_dio(nSocket, plc_1_count[i].addr);

			// 500msec delay
			ts.tv_sec = 0;
			ts.tv_nsec = 500 * 1000000;			
			nanosleep(&ts, NULL);	
						
			// parsing xgt data
			xgt_parsing_dio_plc_1(nSocket,  plc_1_count[i].addr, &plc_1_point[0], nCount);			
		}
		sleep(1);
	}
	
	// close plc 1
	xgt_close_plc(nSocket);	
}


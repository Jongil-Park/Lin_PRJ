typedef struct group_class_name  
{
	char *name;
}Group_Class_Name;


// Fix Point
// 3iS에서 호실이 변경되면 아래의 list에서도 변경시켜줘야 한다. 
Group_Class_Name a_group_list[] =
{
{"A101"},{"A102"},{"A103"},{"A104"},{"A105"},
{"A106"},{"A108"},{"A109"},{"A112"},{"A213"},
{"A201"},{"A202"},{"A203"},{"A204"},{"A205"},
{"A206"},{"A207"},{"A208"},{"A209"},{"A210"},
};

Group_Class_Name b_group_list[] =
{
{"B101"},{"B102"},{"B103"},{"B104"},{"B105"},
{"B105"},{"B107"},{"B108"},{"B109"},{"B112"},
{"B113"},{"B201"},{"B202"},{"B203"},{"B204"},
{"B205"},{"B206"},{"B207"},{"B208"},{"B209"},
{"B210"},{"B211"},{"B214"},{"B301"},{"B302"},
{"B303"},{"B304"},{"B305"},{"B306"},{"B307"},
{"B308"},{"B309"},{"B310"},{"B311"},{"B312"},
{"B315"},{"B316"},{"B401"},{"B402"},{"B403"},
{"B404"},{"B405"},{"B406"},{"B407"},{"B408"},
{"B409"},{"B410"},{"B413"},{"B414"},
};

Group_Class_Name c_group_list[] =
{
{"C101"},{"C103"},{"C106"},{"C109"},{"C206"},
{"C205"},{"C204"},{"C203"},{"C202"},{"C201"},
{"C207"},{"C207"},{"C208"},{"C208"},{"C209"},
{"C210"},{"C211"},{"C303"},{"C302"},{"C301"},
{"C308"},{"C308"},{"C307"},{"C306"},{"C305"},
{"C304"},{"C309"},{"C401"},{"C402"},{"C403"},
{"C404"},{"C405"},{"C406"},{"C406"},{"C406"},
{"C407"},{"C407"},{"C407"},{"C411"},{"C412"},
{"C412"},{"C412"},{"C501"},{"C502"},{"C503"},
{"C504"},{"C505"},{"C506"},{"C507"},{"C507"},
{"C507"},{"C508"},{"C508"},{"C508"},{"C509"},
{"C509"},{"C509"},{"D111"},{"D110"},{"D112"},
{"D213"},{"D215"},{"D214"},{"D210"},{"D311"},
{"D311"},{"D311"},{"D310"},{"D314"},{"C211"},
};

typedef struct __attribute__ ( (__packed__))  
{
	unsigned char class_name[14];
	unsigned char sTime;
	unsigned char eTime;
	int year;
	unsigned char mon;
	unsigned char mday;    
	unsigned char hour;
	unsigned char min;
} TX_DATA_STRUCT;

TX_DATA_STRUCT rx_rcv_msg[1024];


void cnue_init_data(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);
	memset(&g_UnitData, 0x00, sizeof(g_UnitData));
}




void cnue_set_pcm_number(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);


        //GHP_ONOFF_PCM = 0;
        //GHP_ONOFF_FAN_PCM = 1;
        //GHP_SET_TEMP_PCM = 2;
        //GHP_TEMP_PCM = 8;
        //GHP_WINDSPEED_PCM = 22;
        //GHP_WINDDIRECTION_PCM = 23;        
        //GHP_ERROR_PCM = 21;
        GHP_ERROR_NUM_PCM = 30;
        //GHP_MODE_PCM = 6;
        //GHP_MODE_STATUS_PCM = 30;        
        //GHP_GROUP_PCM = 20;
        //GHP_GROUP_PCM = 10;
        //GHP_GROUP_MODE_PCM = 5;
        GHP_GROUP_STATUS_PCM = 30;
        GHP_GROUP_DATA_WEEK_PCM = 30;
        GHP_GROUP_DATA1_PCM = 30;
        GHP_GROUP_DATA2_PCM = 30;
        GHP_RUNTIME_PCM = 30;      


	GHP_ONOFF_PCM = 0;
	GHP_ONOFF_FAN_PCM = 1;
	GHP_SET_TEMP_PCM = 2;
	GHP_TEMP_PCM = 3;
	GHP_WINDSPEED_PCM = 5;
	GHP_WINDDIRECTION_PCM = 4;        
	GHP_ERROR_PCM = 7;
	GHP_GROUP_PCM = 8;
	GHP_MODE_PCM = 9;	
	GHP_GROUP_MODE_PCM = 10;
	
	GHP_MODE_STATUS_PCM = 31;
}


// ghpCfg.dat    (A , 1) , (B , 2) , (C , 3)
void cnue_get_unit_cnt(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);

	switch (g_sddcNum) {
		case 1 :
	        g_unitCnt = 20;
			break;
			
		case 2 : 
	        g_unitCnt = 49; 
			break;
			
		case 3 :
	        g_unitCnt = 70;
			break;	

		default:
			exit(0);
			break;
	}

	fprintf(stderr, "Sddc = %d, UnitCount = %d\n", g_sddcNum, g_unitCnt);
	fflush(stderr);

}


void cnue_get_ghp_data(void)
{
	int i = 0;
	FILE *fp;
	char filename[32];
	int datasize = 0;
	
	fprintf(stdout, "%s()\n", __FUNCTION__);
	fflush(stdout);
		
	g_pDongInfo = malloc( (sizeof(g_GHP_Info) * 256) );
	memset(g_pDongInfo, 0x00, (sizeof(g_GHP_Info) * 256));

	// get filename	
	memset(filename, 0x00, sizeof(filename));
	switch (g_sddcNum) {
		case 1 :
			memcpy(filename, "cnue_class_a.dat", sizeof("cnue_class_a.dat"));
			break;

		case 2 :
			memcpy(filename, "cnue_class_b.dat", sizeof("cnue_class_b.dat"));
			break;

		case 3 :
			memcpy(filename, "cnue_class_c.dat", sizeof("cnue_class_c.dat"));
			break;
					
		default :
			return;
	}
	
	
	if( (fp = fopen(filename, "r") ) == NULL) {
		printf("%s open fail.\n", filename);
		return;			
	}

	fprintf(stdout, "%s() Read File :: %s \n", __FUNCTION__, filename);
	fflush(stdout);
	
	datasize = sizeof(g_GHP_Info);
	for( i = 0; i < 256; i++ ) 
		fread((g_pDongInfo + i), datasize, 1, fp);
	
	fclose(fp);

	for (i = 0; i < g_unitCnt; i++) {
		g_UnitData[i].pno = (g_pDongInfo + i)->nPno;
		g_UnitData[i].block = (g_pDongInfo + i)->nBno;
		g_UnitData[i].chkTouch = 1;
		/*
		fprintf(stderr, "[%s] %d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		b_group_list[i],
		(g_pDongInfo + i)->nPno,
		(g_pDongInfo + i)->nBno,
		(g_pDongInfo + i)->nDDC,
		(g_pDongInfo + i)->nDDR,
		(g_pDongInfo + i)->nOutUnit,
		(g_pDongInfo + i)->nInUnit,
		(g_pDongInfo + i)->nType,
		(g_pDongInfo + i)->nFloor,
		(g_pDongInfo + i)->nSection);
		*/

		g_UnitData[i].on_off = (int)pGet(GHP_ONOFF_PCM, g_UnitData[i].pno);
		g_UnitData[i].r_temp = (int)pGet(GHP_TEMP_PCM, g_UnitData[i].pno);
		g_UnitData[i].s_temp = (int)pGet(GHP_SET_TEMP_PCM, g_UnitData[i].pno);
		g_UnitData[i].aamode = (int)pGet(GHP_MODE_PCM, g_UnitData[i].pno);
		g_UnitData[i].wind_s = (int)pGet(GHP_WINDSPEED_PCM, g_UnitData[i].pno);
		g_UnitData[i].wind_d = (int)pGet(GHP_WINDDIRECTION_PCM, g_UnitData[i].pno);


		fprintf(stderr, ">> g_unitCnt %d,%d,%d %d %d,%d %d %d,%d\n",
		g_UnitData[i].pno,
		g_UnitData[i].block,
		g_UnitData[i].chkTouch,
		g_UnitData[i].on_off,
		g_UnitData[i].r_temp,
		g_UnitData[i].s_temp,
		g_UnitData[i].aamode,
		g_UnitData[i].wind_s,
		g_UnitData[i].wind_d );
		fflush(stderr);
	}
	return;
}


void cnue_get_grpnum(int n_idx) 
{
	int n_pno = g_UnitData[n_idx].pno;

	g_UnitData[n_idx].group = pGet(GHP_GROUP_PCM, n_pno);

	// 학사모드는 모드가 항상 "시간"으로 되어야 한다.
	if (g_UnitData[n_idx].group == 0) {
		pSet(GHP_GROUP_MODE_PCM, g_UnitData[n_idx].pno, 1);
		g_UnitData[n_idx].group_mode = 1;
		return;
	}

	if ( g_UnitData[n_idx].group > 10 ) {
		pSet(GHP_GROUP_PCM, n_pno, 10);
		g_UnitData[n_idx].group = pGet(GHP_GROUP_PCM, n_pno);
	}
	

}


void cnue_get_grpstatus(int n_idx)
{
	int n_pno = 0;

	// 학사모드는 모드가 항상 "시간"으로 되어야 한다.
	if (g_UnitData[n_idx].group == 0) {
		pSet(GHP_GROUP_MODE_PCM, g_UnitData[n_idx].pno, 1);
		g_UnitData[n_idx].group_mode = 1;
		return;
	}

	if ( g_UnitData[n_idx].group > 0 ) {
		n_pno = 199 + g_UnitData[n_idx].group;
		g_UnitData[n_idx].group_mode = pGet(GHP_MODE_STATUS_PCM, n_pno);
		pSet(GHP_GROUP_MODE_PCM, g_UnitData[n_idx].pno, g_UnitData[n_idx].group_mode);
		//fprintf(stdout, "n_pno = %d, group = %d, mode = %d \n", g_UnitData[n_idx].pno, g_UnitData[n_idx].group, g_UnitData[n_idx].group_mode);
		//fflush(stdout);
	}
}


void cnue_get_grptime(int n_idx)
{
	int i = 0;
	int n_pno = 0;
	int n_grp = g_UnitData[n_idx].group;

	// 학사모드는 이미 시간값을 설정했으므로 바로 리턴한다
	if (g_UnitData[n_idx].group == 0) {
		//pSet(GHP_GROUP_MODE_PCM, g_UnitData[n_idx].pno, 1);
		//g_UnitData[n_idx].group_mode = 1;
		return;
	}

	if ( g_UnitData[n_idx].group > 0 ) {
		n_pno = (n_grp - 1) * 5;
		for (i = 0; i < 5; i++ ) {

			g_UnitData[n_idx].startT[i] = pGet(GHP_MODE_STATUS_PCM, n_pno + i);
			g_UnitData[n_idx].stopT[i] = pGet(GHP_MODE_STATUS_PCM, n_pno + i + 50);

			//3isheet에 잘못 기입되어 있기 때문에 아래와 같이 9그룹 이후에 종료시간을 수정한다. 
			//3isheet를 확인할 것.
			if ( n_grp >= 9 ) {
				g_UnitData[n_idx].stopT[i] = pGet(GHP_MODE_STATUS_PCM, n_pno + i + 80);
			}
			
			//if ( g_UnitData[n_idx].group == 10 ) {
			//	printf("pno (%d %d) (%d %d)\n", n_pno + i, n_pno + i + 50, g_UnitData[n_idx].startT[i], g_UnitData[n_idx].stopT[i] );	
			//}
			
		}
	}
}


void cnue_schedule(int n_idx) 
{
	int i = 0;
	int n_pno = g_UnitData[n_idx].pno;
	int n_grp = g_UnitData[n_idx].group;
	int n_mode = g_UnitData[n_idx].group_mode;
	time_t  the_time;
	struct tm *tm_ptr;
	int myTime = 0;
	int check = 0;
	int n_value = 0;

    // get now time
	the_time = time(NULL); 
    tm_ptr= localtime(&the_time);
    myTime = (tm_ptr->tm_hour * 100) + tm_ptr->tm_min;

	//if ( g_UnitData[n_idx].group > 0 ) {

		// 시간데이터와 비교한다. 
		for ( i = 0 ; i < 5; i++) {
			if ( g_UnitData[n_idx].startT[i] == 0 )
				continue;
	
			if (myTime >= g_UnitData[n_idx].startT[i]	
					&& myTime < g_UnitData[n_idx].stopT[i]) {
			check++;
			
			if ( n_grp == 10 ) {
				printf("check = %d, myTime = %d  (%d %d)\n", check, myTime, g_UnitData[n_idx].startT[i], g_UnitData[n_idx].stopT[i] );	
			}
			
			}
			
			
		}


		switch (n_mode) {
			// 프리모드 
			case 0:
				break;

			// 시간모드 
			// 지정된 시간에 무조건 On, 그외 시간엔 무조건 OFF
			case 1:
				if (check > 0) {
					printf("Always On grp = %d, pno = %d\n", g_UnitData[n_idx].group , g_UnitData[n_idx].pno);
					pSet(GHP_ONOFF_PCM, n_pno, 1);	
				}		
				else {
					pSet(GHP_ONOFF_PCM, n_pno, 0);	
				}	
				break;

			// 권한모드 
			// 지정된 시간에는 사용자 마음대로, 그 외 시간에는 제한시간 적용.
			case 2:
				if (check == 0) {
					//printf("Always On grp = %d, pno = %d\n", g_UnitData[n_idx].group , g_UnitData[n_idx].pno);
					// Timeout이 최초로 발생하면 GHP를 OFF 시킨다.
					if (g_UnitData[n_idx].chkTouch == 0) {
						g_UnitData[n_idx].chkTouch = 1;
						pSet(GHP_ONOFF_PCM, n_pno, 0);			
					}
					else {
						g_limitTime =  pGet(GHP_MODE_STATUS_PCM, 255);
						n_value = pGet(GHP_ONOFF_PCM, n_pno);
						if (n_value == 1) {
							if( CheckLimitTime(n_idx) == SUCCESS ) {
								pSet(GHP_ONOFF_PCM, n_pno, 0);			
								g_UnitData[n_idx].minCount = 0;
							}
						}		
					}					
				}
				else {
					g_UnitData[n_idx].chkTouch = 0;
					g_UnitData[n_idx].minCount = 0;
					g_UnitData[n_idx].preMinute = 0;
				}
				break;

			default:
				break;
		}
	//}
}


static void cnue_convertor_class_c(int datacnt)
{
    int i = 0, j = 0;
    //int buf_cnt = 0;
    //int stime_val = 0;
    //int etime_val = 0;
    int timecnt = 0;
    int n_pno = 0;
    int n_pcm = 0;

    for(i = 0; i < g_unitCnt; i++)
    {
        timecnt = 0;
        for(j = 0; j < datacnt; j++)
        { 
            if(strncasecmp(rx_rcv_msg[j].class_name, c_group_list[i].name, strlen(c_group_list[i].name)) == 0)
            {
                printf("DEBUG: Find  Unit = %d, Class = %s \n", i, rx_rcv_msg[j].class_name);
			
				n_pno = 89 + (int)rx_rcv_msg[j].sTime;					
				g_UnitData[i].startT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
				n_pno = 104 + (int)rx_rcv_msg[j].eTime;					
				g_UnitData[i].stopT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
                printf("DEBUG : Pno = %d, sTime = %d, eTime = %d\n"
                    , g_UnitData[i].pno, g_UnitData[i].startT[timecnt], g_UnitData[i].stopT[timecnt]);				
				
				/*
				n_pcm = timecnt + 11;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].startT[timecnt]);
				
				n_pcm = timecnt + 12;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].stopT[timecnt]);
				*/
				timecnt++;
            }
        }
    }
    
	for ( i = 0; i < g_unitCnt; i++ ) {
		for ( j = 0; j < 5; j++ ) {
			n_pcm = (j * 2) + 11;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].startT[j]);
			
			n_pcm = (j * 2) + 12;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].stopT[j]);
		}
	}    
}


static void cnue_convertor_class_b(int datacnt)
{
    int i = 0, j = 0;
    //int buf_cnt = 0;
    //int stime_val = 0;
    //int etime_val = 0;
    int timecnt = 0;
    int n_pno = 0;
    int n_pcm = 0;

    for(i = 0; i < g_unitCnt; i++)
    {
        timecnt = 0;
        for(j = 0; j < datacnt; j++)
        { 
            if(strncasecmp(rx_rcv_msg[j].class_name, b_group_list[i].name, strlen(b_group_list[i].name)) == 0)
            {
                printf("DEBUG: Find  Unit = %d, Class = %s \n", i, rx_rcv_msg[j].class_name);
			
				n_pno = 89 + (int)rx_rcv_msg[j].sTime;					
				g_UnitData[i].startT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
				n_pno = 104 + (int)rx_rcv_msg[j].eTime;					
				g_UnitData[i].stopT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
                printf("DEBUG : Pno = %d, sTime = %d, eTime = %d\n"
                    , g_UnitData[i].pno, g_UnitData[i].startT[timecnt], g_UnitData[i].stopT[timecnt]);				
				
				/*
				n_pcm = timecnt + 11;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].startT[timecnt]);
				
				n_pcm = timecnt + 12;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].stopT[timecnt]);
				*/
				timecnt++;
            }
        }
    }
    
	for ( i = 0; i < g_unitCnt; i++ ) {
		for ( j = 0; j < 5; j++ ) {
			n_pcm = (j * 2) + 11;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].startT[j]);
			
			n_pcm = (j * 2) + 12;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].stopT[j]);
		}
	}    
}


static void cnue_convertor_class_a(int datacnt)
{
    int i = 0, j = 0;
    //int buf_cnt = 0;
    //int stime_val = 0;
    //int etime_val = 0;
    int timecnt = 0;
    int n_pno = 0;
    int n_pcm = 0;

    for(i = 0; i < g_unitCnt; i++)
    {
        timecnt = 0;
        for(j = 0; j < datacnt; j++)
        { 
            if(strncasecmp(rx_rcv_msg[j].class_name, a_group_list[i].name, strlen(a_group_list[i].name)) == 0)
            {
                printf("DEBUG: Find  Unit = %d, Class = %s \n", i, rx_rcv_msg[j].class_name);
			
				n_pno = 89 + (int)rx_rcv_msg[j].sTime;					
				g_UnitData[i].startT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
				n_pno = 104 + (int)rx_rcv_msg[j].eTime;					
				g_UnitData[i].stopT[timecnt] = pGet(GHP_MODE_STATUS_PCM, n_pno);
				
                printf("DEBUG : Pno = %d, sTime = %d, eTime = %d\n"
                    , g_UnitData[i].pno, g_UnitData[i].startT[timecnt], g_UnitData[i].stopT[timecnt]);				
				
				/*
				n_pcm = timecnt + 11;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].startT[timecnt]);
				
				n_pcm = timecnt + 12;
				n_pno = g_UnitData[i].pno;
				pSet(n_pcm, n_pno, g_UnitData[i].stopT[timecnt]);
				*/
				timecnt++;
            }
        }
    }
    
	for ( i = 0; i < g_unitCnt; i++ ) {
		for ( j = 0; j < 5; j++ ) {
			n_pcm = (j * 2) + 11;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].startT[j]);
			
			n_pcm = (j * 2) + 12;
			n_pno = g_UnitData[i].pno;
			pSet(n_pcm, n_pno, g_UnitData[i].stopT[j]);
		}
	}    
}


void cnue_ReadScheduleFile(void)
{
	int i = 0;
	int j = 0;
	int k = 0;
	FILE *fp = NULL;
	unsigned char *p;
	int filesize = 0;
	SEND_DATA *pRx;
	int localDbg = 0;
	int datacnt = 0;
	//TX_DATA_STRUCT *p;
	

	pthread_mutex_lock(&schedule_mutex);
	if((fp = fopen("SchedGHP.dat", "r")) == NULL) {
		if (fp != NULL)  fclose(fp);
		printf("[ERROR] File Open with Option 'r'\n");		
		pthread_mutex_unlock(&schedule_mutex);
		return;
	}	
	
	fseek(fp, 0L, SEEK_END); 
	filesize = ftell( fp );
	memset(&g_schedFileData, 0x00, sizeof(g_schedFileData));
	fseek(fp, 0L, SEEK_SET); 
	
	//if (filesize != sizeof(g_schedFileData)) {
	//	fclose(fp);
	//	printf(">>>>>>>>>>>>>>>>>>>>> filesize error\n");
	//	pthread_mutex_unlock(&schedule_mutex);
	//	return;	
	//}
	printf("file size = %d\n", filesize);

	p = (unsigned char *)&g_schedFileData;
	for ( i = 0; i < filesize; i++ )
		//fread(&p[i], 1, sizeof(unsigned char), fp);
		fread(&p[i], 1, sizeof(g_schedFileData), fp);

	if (fp != NULL)  fclose(fp);
		
	//schedule_mutex
	pthread_mutex_unlock(&schedule_mutex);
	
	datacnt = filesize/sizeof(TX_DATA_STRUCT);
	//p = (TX_DATA_STRUCT *)&g_schedFileData;
	memcpy((unsigned char*)&rx_rcv_msg, (unsigned char*)&g_schedFileData, filesize);
	
	/*
	for ( i = 0; i < datacnt; i++) {
		printf("[%s] %d %d\n",
			rx_rcv_msg[i].class_name,
			rx_rcv_msg[i].sTime,
			rx_rcv_msg[i].eTime);
	}
	*/

	switch (g_sddcNum) {
		case 1 :
	        cnue_convertor_class_a(datacnt);	
			break;
			
		case 2 : 
	        cnue_convertor_class_b(datacnt);	
			break;
			
		case 3 :
	        cnue_convertor_class_c(datacnt);	
			break;	
	}
		
	
	return;
	//////////////////////////////////////////////////////////////////////////////////
	
	pRx = (SEND_DATA *)&g_schedFileData;
	if(CheckFileChksum(pRx) != SUCCESS)
		return;

	//Set ghpUnit limit time
	g_limitTime =  pRx->group[0].limitT;	

	if (localDbg)
	{
		for ( i=0; i<MAX_GROUP_COUNT; i++ )
		{	
			printf("Text->group[%d].group = %d\n", i, pRx->group[i].group); 	
			printf("Text->group[%d].method = %d\n", i, pRx->group[i].method); 	
			printf("Text->group[%d].mode = %d\n", i, pRx->group[i].mode); 	
			printf("Text->group[%d].limitT = %d\n", i, pRx->group[i].limitT); 	
		
			for ( j=0; j<DAY_COUNT; j++ )
			{
				for ( k=0; k<MAX_SCHEDULE_COUNT; k++ )
				{
				printf("Text->group[%d].startT[%d][%d] = %d\n", i, j, k
					,pRx->group[i].startT[j][k]); 	
				printf("Text->group[%d].stopT[%d][%d] = %d\n", i, j, k
					,pRx->group[i].stopT[j][k]); 	
				}
			}	
		}
	
		for ( i=0; i<MAX_TIMETABLE_COUNT; i++ )
		{
			printf("Text->time.startT[%d] = %d\n", i ,pRx->time.startT[i]); 	
			printf("Text->time.stopT[%d] = %d\n", i ,pRx->time.stopT[i]); 	
		}
	}	
	return;
}


void cnue_while(void)
{
	int i = 0;
	int j = 0;
	int st = 0;												// Fsm status
	int result = 0;
	int n_val = 0;											// value
	int n_tempval = 0;										// Temp value		
	int n_chkval = 0;										// check value
	int now_time = 0 ;
	int set_time = 0;
	int n_loopcnt = 0;
	struct timeval diff_time;
	//time_t     tm_st;
	time_t     tm_nd;
	//double     d_diff;
	//struct tm  user_stime;
	unsigned int n_temp_minute = 0;
	struct tm *tm_ptr;
	int nContolLoopCount = 0;

	
	st = ST_GET_POLLING;

	while (1) {
		//IfaceGhpSelectSleep(1, 0);
		//continue;

		// GHP 값을 읽어오거나 제어한다. 
		switch (st) {
			case ST_GET_POLLING:
				result = Send_Polling();
				switch(result)
				{
					case POLL_REQUIRE_STARTUP:	
						st = ST_STARTUP;	
						break;
					
					case POLL_EMPTY_DATA:
					case POLL_OK:		
						if (Send_Polling() == POLL_REQUIRE_STARTUP) {
							st = ST_STARTUP;	
							break;
						}
						else {
							st = ST_CHK_USER_CONTROL;
							break;
						}
					
					default: 
						st = ST_CHK_USER_CONTROL;
						break;
				}
				break;

			case ST_CHK_USER_CONTROL:
				CheckUserControl();
				st = ST_GET_USER_CONTROL;
				break;

			case ST_GET_USER_CONTROL:
				nContolLoopCount = 0;
				while(Do_User_Control()) {
					nContolLoopCount++;
					if (Send_Polling() == POLL_REQUIRE_STARTUP) {
						st = ST_STARTUP;	
						break;
					}
					
					// 제어루프가 계속 돌지 않도록 합니다.
					if ( nContolLoopCount > 5 ) {
						st = ST_GET_GHP_UNIT;
						break;
					}
				}
				st = ST_GET_GHP_UNIT;
				break;

			case ST_GET_GHP_UNIT:
				Get_Ghp_Unit();
				if (Send_Polling() == POLL_REQUIRE_STARTUP) {
					st = ST_STARTUP;	
					break;
				}   
				st = ST_GET_POLLING;
				break;

			case ST_STARTUP:
				Startup_SDDC();
				st = ST_GET_POLLING;
				break;

			default:
				st = ST_GET_POLLING;
				break;
		}


		// 모든 스케쥴을 초기화 한다.
		for ( i = 0; i < g_unitCnt; i++ ) {
			for ( j = 0; j < 5; j++ ) {
				g_UnitData[i].startT[j] = 0;
				g_UnitData[i].stopT[j] = 0;
			}
		}
		
		// 학사스케쥴을 적용한다. 
		cnue_ReadScheduleFile();
		
		// 스케쥴에 관련된 Data를 설정한다.
		for ( i = 0; i < g_unitCnt; i++ ) {

			cnue_get_grpnum(i);
			cnue_get_grpstatus(i);
			cnue_get_grptime(i);

			// 스케쥴 작업을 수행한다. 
			cnue_schedule(i);
		}		
		
		
		// 모든 GHP의 상태를 제어한다. 
		n_chkval = pGet(GHP_MODE_STATUS_PCM, 254);
		if ( n_chkval == 1 ) {
			fprintf(stdout, "[CNUE] All Stop \n" );
			fflush(stdout);			
			for ( i = 0; i < g_unitCnt; i++) {
				n_val = pGet(GHP_ONOFF_PCM, i);
				if ( n_val > 0 ) {
					pSet(GHP_ONOFF_PCM, i, 0);
					fprintf(stdout, " >> Stop %d, %d -> %d  \n", i, n_val, n_chkval);
					fflush(stdout);			
				}
			}
		}

		// 냉난방모드를 설정한다. 
		n_chkval = pGet(GHP_MODE_STATUS_PCM, 253);
		for ( i = 0; i < g_unitCnt; i++) {
			n_val = pGet(GHP_MODE_PCM, i);
			if ( n_val != n_chkval ) {
				pSet(GHP_MODE_PCM, i, n_chkval);
				fprintf(stdout, " >> Md %d, %d -> %d  \n", i, n_val, n_chkval);
				fflush(stdout);			
			}
		}

		if ( n_chkval > 0 ) {
			fprintf(stdout, "[CNUE] Cool Mode \n" );
			fflush(stdout);			
			
			// 온도제한을 설정한다. 냉방인 경우 제한온도보다 높도록 제어한다. 
			n_chkval = pGet(GHP_MODE_STATUS_PCM, 252);
			fprintf(stdout, "[CNUE] Limit Set Temperature \n" );
			fflush(stdout);			
			for ( i = 0; i < g_unitCnt; i++) {
				n_val = pGet(GHP_SET_TEMP_PCM, i);
				if ( n_chkval > n_val ) {
					pSet(GHP_SET_TEMP_PCM, i, n_chkval);
					fprintf(stdout, " >> LST %d, %d -> %d  \n", i, n_val, n_chkval);
					fflush(stdout);			
				}
			}			
		}
		else {
			fprintf(stdout, "[CNUE] Heat Mode \n" );
			fflush(stdout);			
			
			// 온도제한을 설정한다.  난방인 경우 제한온도보다 낮도록 제어한다.
			n_chkval = pGet(GHP_MODE_STATUS_PCM, 252);
			fprintf(stdout, "[CNUE] Limit Set Temperature \n" );
			fflush(stdout);			
			for ( i = 0; i < g_unitCnt; i++) {
				n_val = pGet(GHP_SET_TEMP_PCM, i);
				if ( n_chkval < n_val ) {
					pSet(GHP_SET_TEMP_PCM, i, n_chkval);
					fprintf(stdout, " >> LST %d, %d -> %d  \n", i, n_val, n_chkval);
					fflush(stdout);			
				}
			}			
		}

		if ( n_loopcnt++ > 60 ) {
			n_loopcnt = 0;
			system( "hwclock --hctosys" );
		}

		// 1분마다 시간을 Point에 기록한다.		
		time(&tm_nd);
		tm_ptr = localtime(&tm_nd);
		fprintf(stdout, " >> TIME  %d, %d \n", tm_ptr->tm_hour, tm_ptr->tm_min);
		fflush(stdout);	
		now_time = (tm_ptr->tm_hour * 100) +  tm_ptr->tm_min; 
		n_tempval = pGet(GHP_MODE_STATUS_PCM, 250);
		if ( n_tempval != now_time ) {
			pSet(GHP_MODE_STATUS_PCM, 250, now_time);
			pSet(GHP_MODE_STATUS_PCM, 251, now_time);
			set_time = now_time;
		}
		
		// 사용자의 시간설정을 처리한다. 
		n_tempval = pGet(GHP_MODE_STATUS_PCM, 251);
		if ( n_tempval != set_time ) {
			fprintf(stdout, " >> SET TIME  %d \n", n_tempval);
			fflush(stdout);	

			if ( n_tempval > set_time ) {
				n_temp_minute = set_time % 100;
				
				set_time = set_time - n_temp_minute;
								
				n_val = ((n_tempval - set_time) / 100) * 3600;
				fprintf(stdout, " >> hour  %d \n", n_val);
				n_val += ((n_tempval - set_time) % 100) * 60;
				fprintf(stdout, " >> minute  %d \n", n_val);

				gettimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
				diff_time.tv_sec = diff_time.tv_sec + n_val - (n_temp_minute * 60);
				settimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> ++ diff_time.tv_sec  %d \n", diff_time.tv_sec);
				fflush(stdout);
			}
			
			if ( n_tempval < set_time ) {
				n_temp_minute = set_time % 100;
				
				set_time = set_time - n_temp_minute;
				
				n_val = ((set_time - n_tempval) / 100) * 3600;
				fprintf(stdout, " >> hour  %d \n", n_val);
				n_val += ((set_time - n_tempval) % 100) * 60;
				fprintf(stdout, " >> minute  %d \n", n_val);
				
				
				if ( ((set_time - n_tempval) % 100) > 60 ) {
					n_val -= 40 * 60;
				}

				gettimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
				diff_time.tv_sec = diff_time.tv_sec - n_val - (n_temp_minute * 60);
				settimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> -- diff_time.tv_sec  %d \n", diff_time.tv_sec);
				fflush(stdout);
			}

			pSet(GHP_MODE_STATUS_PCM, 251, n_tempval);
			set_time = n_tempval;

			// 현장에서 테스트 할것..
			system( "hwclock --systohc" );
		}

	}
}


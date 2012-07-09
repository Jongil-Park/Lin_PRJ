

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

	GHP_ONOFF_PCM = 0;
	GHP_ONOFF_FAN_PCM = 1;
	GHP_SET_TEMP_PCM = 2;
	GHP_TEMP_PCM = 3;
	GHP_WINDSPEED_PCM = 4;
	GHP_WINDDIRECTION_PCM = 5;        
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

		fprintf(stderr, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		(g_pDongInfo + i)->nPno,
		(g_pDongInfo + i)->nBno,
		(g_pDongInfo + i)->nDDC,
		(g_pDongInfo + i)->nDDR,
		(g_pDongInfo + i)->nOutUnit,
		(g_pDongInfo + i)->nInUnit,
		(g_pDongInfo + i)->nType,
		(g_pDongInfo + i)->nFloor,
		(g_pDongInfo + i)->nSection);

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
}


void cnue_get_grpstatus(int n_idx)
{
	int n_pno = 0;

	if ( g_UnitData[n_idx].group > 0 ) {
		n_pno = 199 + g_UnitData[n_idx].group;
		g_UnitData[n_idx].group_mode = pGet(GHP_MODE_STATUS_PCM, n_pno);
		pSet(GHP_GROUP_MODE_PCM, g_UnitData[n_idx].pno, g_UnitData[n_idx].group_mode);
		//fprintf(stdout, "pcm = %d n_pno = %d, mode = %d \n", GHP_GROUP_MODE_PCM, n_pno, g_UnitData[n_idx].group_mode);
		//fflush(stdout);
	}
}


void cnue_get_grptime(int n_idx)
{
	int i = 0;
	int n_pno = 0;
	int n_grp = g_UnitData[n_idx].group;

	if ( g_UnitData[n_idx].group > 0 ) {
		n_pno = (1 - n_grp) * 5;
		for (i = 0; i < 5; i++ ) {
			g_UnitData[n_idx].startT[i] = pGet(GHP_MODE_STATUS_PCM, n_pno + i);
			g_UnitData[n_idx].stopT[i] = pGet(GHP_MODE_STATUS_PCM, n_pno + i + 50);
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

	if ( g_UnitData[n_idx].group > 0 ) {

		// 시간데이터와 비교한다. 
		for ( i = 0 ; i < 5; i++) {
			if ( g_UnitData[n_idx].startT[i] == 0 )
				continue;
	
			if (myTime >= g_UnitData[n_idx].startT[i]	
					&& myTime < g_UnitData[n_idx].stopT[i]) {
			check++;
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
	}
}



void cnue_while(void)
{
	int i = 0;
	int st = 0;												// Fsm status
	int result = 0;
	int n_val = 0;											// value
	int n_tempval = 0;										// Temp value		
	int n_chkval = 0;										// check value
	int now_time = 0 ;
	int set_time = 0;

	struct timeval diff_time;
	time_t     tm_st;
	time_t     tm_nd;
	double     d_diff;
	struct tm  user_stime;
	struct tm *tm_ptr;


	g_CntHaksaInit = 0;

	/* Test Code. 반드시 현장에서 지워야 한다. */
	/*
	while( 1 ) {
		IfaceGhpSelectSleep(3, 0);
	}
	*/

	while (1) {
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
		}
		else {
			fprintf(stdout, "[CNUE] Heat Mode \n" );
			fflush(stdout);			
		}

		// 온도제한을 설정한다. 
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
				n_val = ((n_tempval - set_time) / 100) * 3600;
				//fprintf(stdout, " >> n_val  %d \n", n_val);
				n_val += ((n_tempval - set_time) % 100) * 60;
				//fprintf(stdout, " >> n_val  %d \n", n_val);

				gettimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
				diff_time.tv_sec = diff_time.tv_sec + n_val;
				settimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
			}
			
			if ( n_tempval < set_time ) {
				n_val = ((set_time - n_tempval) / 100) * 3600;
				//fprintf(stdout, " >> n_val  %d \n", n_val);
				n_val += ((set_time - n_tempval) % 100) * 60;
				//fprintf(stdout, " >> n_val  %d \n", n_val);

				gettimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
				diff_time.tv_sec = diff_time.tv_sec - n_val;
				settimeofday( &diff_time, NULL );
				//fprintf(stdout, " >> diff_time.tv_sec  %d \n", diff_time.tv_sec);
			}

			pSet(GHP_MODE_STATUS_PCM, 251, n_tempval);

			// 현장에서 테스트 할것..
			//system( "hwclock --systohc" );
		}
		
		// 스케쥴에 관련된 Data를 설정한다.
		for ( i = 0; i < g_unitCnt; i++ ) {
			cnue_get_grpnum(i);
			cnue_get_grpstatus(i);
			cnue_get_grptime(i);

			// 스케쥴 작업을 수행한다. 
			cnue_schedule(i);
		}

		IfaceGhpSelectSleep(1, 0);
		continue;
			
		// GHP 값을 읽어오거나 제어한다. 
		st = ST_GET_POLLING;
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
				while(Do_User_Control()) {

					if (Send_Polling() == POLL_REQUIRE_STARTUP) {
						st = ST_STARTUP;	
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
	}
}


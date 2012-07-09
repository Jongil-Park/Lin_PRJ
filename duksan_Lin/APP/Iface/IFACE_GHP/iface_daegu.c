typedef struct _Tag_GHP_PCM_Point
{
	int nGroupNumber[256];

    int nSchedRunSt[30];
    int nSchedStopSt[30];
    int nSchedRun[256][30];
    int nSchedStop[256][30];

    int nSchedWeekSt[30];
    int nSchedWeekRunSt[30];
    int nSchedWeekStopSt[30];
    int nSchedWeekRun[256][30];
    int nSchedWeekStop[256][30];

    int nChkTouch[256];
    int nRunMinute[256];
} g_GHP_PCM_Point;

g_GHP_PCM_Point GHP_PCM_Data;


void daegu_init_data(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);
	memset(&g_UnitData, 0x00, sizeof(g_UnitData));
}




void daegu_set_pcm_number(void)
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


// ghpCfg.dat    (A , 1)
void daegu_get_unit_cnt(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);

	switch (g_sddcNum) {
		case 1 :
	        g_unitCnt = 20;
			break;

		default:
			exit(0);
			break;
	}

	fprintf(stderr, "Sddc = %d, UnitCount = %d\n", g_sddcNum, g_unitCnt);
	fflush(stderr);

}


void daegu_get_ghp_data(void)
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
			memcpy(filename, "daegu_class_a.dat", sizeof("daegu_class_a.dat"));
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


void daegu_get_grpnum(int n_idx) 
{

	int n_pno = g_UnitData[n_idx].pno;

	g_UnitData[n_idx].group = pGet(GHP_GROUP_PCM, n_pno);

	// �л���� ��尡 �׻� "�ð�"���� �Ǿ�� �Ѵ�.
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



/***************************************************************** 
 * GhP_TimeSched_Make(int nUnitCnt) 
 *  �� �ð��� üũ�Ѵ�. 1�д� �ý��� �ð��� ����Ѵ�.  
 *  �ź��� �ٲ� ������ g_MainMinuteFlag ���� 1�� �����Ѵ�. 
 */
void daegu_TimeSched_Make(int nUnitCnt)
{
	int i = 0;
    int j = 0;
    int nTempPno1 = 0;
    int nTempPno2 = 0;
    int nTempPno3 = 0;  
    int nTempPno4 = 0;    
   
    // Get group setting variable from ddc point table.
    nTempPno1 = 0;
    nTempPno2 = 0;
    nTempPno3 = 0;
    nTempPno4 = 0;
    
    // �׷��� ���� 10��, �׷츶�� �ð������� ������ 12���� �����մϴ�. 
    // �뱸�뿡�� ������ �ٲ���� ������ Hardcoding�մϴ�. 21=> �׷���۽ð� / 22=> �׷�����ð� / 23=> �׷���. 
	// �� 10���� �׷��� ���.
	for (i = 0 ; i < 10; i++)  {
		// �� �׷츶�� �ִ� 20���� �ð��� ���.
		for (j = 0; j < 20; j++) {
            GHP_PCM_Data.nSchedRun[i][j] = pGet(GHP_GROUP_DAY_START_PCM, nTempPno1);
            GHP_PCM_Data.nSchedStop[i][j] = pGet(GHP_GROUP_DAY_STOP_PCM, nTempPno2);
    
            GHP_PCM_Data.nSchedWeekRun[i][j] = pGet(GHP_GROUP_WEEK_START_PCM, nTempPno1++);
            GHP_PCM_Data.nSchedWeekStop[i][j] = pGet(GHP_GROUP_WEEK_STOP_PCM, nTempPno2++);
        }
    }    
    
    // �׷쿡�� ���۽ð��� ����ð��� �������θ� üũ�մϴ�. 
	// �� 10���� �׷�.
	for (i = 0; i < 10; i++) {
        GHP_PCM_Data.nSchedRunSt[i] = pGet(GHP_GROUP_ST_PCM, nTempPno3++);
        GHP_PCM_Data.nSchedStopSt[i] = pGet(GHP_GROUP_ST_PCM, nTempPno3++);
        
        GHP_PCM_Data.nSchedWeekSt[i] = pGet(GHP_GROUP_ST_PCM, i + 20);
        GHP_PCM_Data.nSchedWeekRunSt[i] = pGet(GHP_GROUP_ST_PCM, 30 + nTempPno4++);
        GHP_PCM_Data.nSchedWeekStopSt[i] = pGet(GHP_GROUP_ST_PCM, 30 + nTempPno4++);
    }
    
	// ?? �̺κ��� ����ϴ°���???����????????????????????????????
	/* 	  
    // �� GHP �ǳ����� �׷��ȣ�� Ȯ���մϴ�. 
    for (i = 0; i < nUnitCnt; i++) {
        nGrp = pGet(GHP_GROUP_PCM, i);
        GhP_Block_Set_GrpNum(GHP_PCM_Data.nBno[i], nGrp, g_nGhpCnt);
        GHP_PCM_Data.nGroupNumber[i] = nGrp;       
    } 
	*/   
}

/***************************************************************** 
 * GhP_ChkGrpTime(int nGrpNo, int nTime, int nTimeType, int nWday) 
 *  �׷�ð��� üũ�Ѵ�. �׷�ð��� �ý��� �ð���
 *  �����ϰ� �Ǹ� nRtnVal�� 1�� �Ͽ� �����Ѵ�. 
 *  �׷��� ���� ��쿡�� nRtnVal�� 0���� �Ͽ� �����Ѵ�. 
 */                                   
int daegu_ChkGrpTime(int nGrpNo, int nTime, int nTimeType, int nWday)
{
	int i = 0;
    int nRtnVal = -1;
    int TimeCnt = 0;
    int TimeVal = 0;
    //int nTimeStatus = 0;
    //int nPno = 0;

    // ���Ͽ� �ָ������� ON�̸�, ���Ͻð�ǥ ����    
    // ���Ͽ� �ָ������� OFF�̸�, ���Ͻð�ǥ ����
    // �ָ��� �ָ������� ON�̸�, �ָ��ð�ǥ ����
    // �ָ��� �ָ������� OFF�̸�, ���Ͻð�ǥ ����
    if ( nWday == 1 && GHP_PCM_Data.nSchedWeekSt[nGrpNo] == 1) {
        if ( nTimeType == 0 
			 && GHP_PCM_Data.nSchedWeekRunSt[nGrpNo] == 0 ) {
            nRtnVal = -1;
            return nRtnVal;
        }
        else if( nTimeType == 1 
				 && GHP_PCM_Data.nSchedWeekStopSt[nGrpNo] == 0) {
            nRtnVal = -1;
            return nRtnVal;
        }
    }
    else {
        if ( nTimeType == 0 
			 && GHP_PCM_Data.nSchedRunSt[nGrpNo] == 0 )	{
            nRtnVal = -1;
            return nRtnVal;
        }
        else if ( nTimeType == 1
				  && GHP_PCM_Data.nSchedStopSt[nGrpNo] == 0 ) {
            nRtnVal = -1;
            return nRtnVal;
        }
    }

    TimeCnt = 12;
    for( i = 0; i < TimeCnt; i++ ) {
        // ���Ͽ� �ָ������� ON�̸�, ���Ͻð�ǥ ����    
        // ���Ͽ� �ָ������� OFF�̸�, ���Ͻð�ǥ ����
        // �ָ��� �ָ������� ON�̸�, �ָ��ð�ǥ ����
        // �ָ��� �ָ������� OFF�̸�, ���Ͻð�ǥ ����
        if ( nWday == 1  && GHP_PCM_Data.nSchedWeekSt[nGrpNo] == 1 ) {
            // It check TimeType.
            if ( nTimeType == 0 ) // StartTime 
                TimeVal = GHP_PCM_Data.nSchedWeekRun[nGrpNo][i];
            else                // StopTime    
                TimeVal = GHP_PCM_Data.nSchedWeekStop[nGrpNo][i];
        }
        else {
            // It check TimeType.
            if ( nTimeType == 0 ) // StartTime 
                TimeVal = GHP_PCM_Data.nSchedRun[nGrpNo][i];
            else                // StopTime    
                TimeVal = GHP_PCM_Data.nSchedStop[nGrpNo][i];
        }

        if ( TimeVal != 0 ) {
            if ( TimeVal == nTime ) {
                nRtnVal = 1;
                printf(" - Type(%d) = Time Value(%d), nTime = %d\n", nTimeType, TimeVal, nTime);
                return nRtnVal;
            }
        }
        else
        {}      
    }
    
    nRtnVal = -1;
    return nRtnVal;
}


/***************************************************************** 
 * Ghp_GrpExecution(int nGrpNo, int nUnitCnt, int nSetVal) 
 *  �׷��ȣ�� �Ѱܹ޾Ƽ�, �� �׷��� �ش��ϴ� �ǳ��⸦ �����Ѵ�.
 *  g_MainMinuteFlag ���� �� ���� �ٲ�� �Ǹ� �ѹ� 1�� �����ȴ�. 
 *  �׷��� ������ �� ���� Ȯ���ؼ� 1�п� �ѹ� ��� �Ѵ�.
 *  ������ ��� ���� �� �ִ� ��츦 �����ؼ� 1�п� 2�� ���� �ϱ� ����
 *  g_MainMinuteFlag�� ������Ű�� 1���� ũ�� �Ǹ� �������� �ʴ´�.     
 */
void daegu_GrpExecution(int nGrpNo, int nUnitCnt, int nSetVal)
{
	//int i = 0;
    //int nVal = 0;    
    
	/*
    nUnitCnt = g_nGhpCnt;
    for ( i = 0; i < nUnitCnt; i++ ) {
        if (GHP_PCM_Data.nGroupNumber[i] == nGrpNo
                && g_MainMinuteFlag != 0) {
            printf("nGrpNo(%d):: nPno = %d, Val = %d\n", nGrpNo, GHP_PCM_Data.nPno[i], nSetVal);  
            GHP_PCM_Data.nChkTouch[i] == 1;
            nVal = pGet(0, GHP_PCM_Data.nPno[i]);
            if (nVal == nSetVal)
                continue;
            else
                pSet(0, GHP_PCM_Data.nPno[i], nSetVal);             
        } 
    }
	

    if (g_MainMinuteFlag > 1)
        g_MainMinuteFlag = 0;
    else
	 	g_MainMinuteFlag++;
	*/
}


/***************************************************************** 
 * GhP_TimeSched_Execution(int nUnitCnt) 
 *  �׷캰�� �ð��� üũ�Ѵ�. �׸��� ���� �Ѿ��� �ð��̸� �Ѱ�, ���� �� �ð��̸� ����. 
 *  ������ �ٸ� ���� �������� �ǳ��� ��θ� üũ�Ͽ� ó���Ͽ�����,
 *  ������ �׷캰�� �ð��� üũ�Ͽ� ó���Ѵ�. 
 */
void daegu_TimeSched_Execution(int nUnitCnt)
{
	int 		i = 0;
    int 		nGrpCnt = 0;
    int 		nTime = 0;    
    int 		nDayCheck = 0;
	time_t 		tm_nd;
	struct tm 	*tm_ptr;    
	
	    
	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	// ���� ���ϻ��¸� Ȯ���Ѵ�. 
	// ������ = 1, ȭ���� = 2, ������ = 3, ����� = 4
	// �ݿ��� = 5, ����� = 6, �Ͽ��� = 0 
	nDayCheck = tm_ptr->tm_wday;
    if (nDayCheck == 0 || nDayCheck == 6)
        nDayCheck = 1;
    else
        nDayCheck = 0;

	printf("%d\n", nDayCheck);
    
	// ����ð�
	nTime = (tm_ptr->tm_hour * 100) +  tm_ptr->tm_min; 

    nGrpCnt = 10;
    for ( i = 0; i < nGrpCnt; i++ ) {
		// It check StartTime. flag is '0'
        if (daegu_ChkGrpTime(i, nTime, 0, nDayCheck) == 1)
            daegu_GrpExecution(i, nUnitCnt, 1);
        
        // It check StopTime. flag is '1'
        if (daegu_ChkGrpTime(i, nTime, 1, nDayCheck) == 1)
			daegu_GrpExecution(i, nUnitCnt, 0); 
    } 
}




void daegu_while(void)
{
	int i = 0;
	//int j = 0;
	int st = 0;												// Fsm status
	int result = 0;
	int n_val = 0;											// value
	int n_tempval = 0;										// Temp value		
	//int n_chkval = 0;										// check value
	int now_time = 0 ;
	int set_time = 0;
	int n_loopcnt = 0;
	struct timeval diff_time;
	time_t     tm_nd;
	unsigned int n_temp_minute = 0;
	struct tm *tm_ptr;

	
	st = ST_GET_POLLING;

	while (1) {
		//IfaceGhpSelectSleep(1, 0);
		//continue;

		// �뱸���б� ������
		for (i = 0; i < g_unitCnt; i++) {
			daegu_TimeSched_Make(i);
			daegu_TimeSched_Execution(i);
		}
		
		// 1�и��� �ð��� Point�� ����Ѵ�.		
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


		// GHP ���� �о���ų� �����Ѵ�. 
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


		if ( n_loopcnt++ > 60 ) {
			n_loopcnt = 0;
			system( "hwclock --hctosys" );
		}
		
		// ������� �ð������� ó���Ѵ�. 
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

			// ���忡�� �׽�Ʈ �Ұ�..
			system( "hwclock --systohc" );
		}
	}
}


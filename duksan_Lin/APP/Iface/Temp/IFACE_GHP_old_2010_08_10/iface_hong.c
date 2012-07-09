

void hong_init_data(void)
{
	fprintf(stderr, "%s()\n", __FUNCTION__);
	fflush(stderr);
	memset(&g_UnitData, 0x00, sizeof(g_UnitData));
	memset(&Dong_Data, 0x00, sizeof(Dong_Data));
	memset(&Dong_Outdoor_Data, 0x00, sizeof(Dong_Outdoor_Data));
	memset(&Kisuksa_Data, 0x00, sizeof(Kisuksa_Data));
}




void hong_set_pcm_number(void)
{
    // Dormitory
    if (g_sddcNum == SDDC_KISUKSA)     
    {
        GHP_ONOFF_PCM = 0;
        GHP_ONOFF_FAN_PCM = 1;
        GHP_SET_TEMP_PCM = 2;
        GHP_TEMP_PCM = 8;
        GHP_WINDSPEED_PCM = 22;
        GHP_WINDDIRECTION_PCM = 23;        
        GHP_ERROR_PCM = 21;
        GHP_ERROR_NUM_PCM = 24;
        GHP_MODE_PCM = 6;
        GHP_MODE_STATUS_PCM = 30;        
        //GHP_GROUP_PCM = 20;
        GHP_GROUP_PCM = 10;
        GHP_GROUP_MODE_PCM = 5;
        GHP_GROUP_STATUS_PCM = 25;
        GHP_GROUP_DATA_WEEK_PCM = 26;
        GHP_GROUP_DATA1_PCM = 27;
        GHP_GROUP_DATA2_PCM = 28;
        GHP_RUNTIME_PCM = 31;      
    }
    else
    {
        GHP_ONOFF_PCM = 0;
        GHP_ONOFF_FAN_PCM = 1;
        GHP_SET_TEMP_PCM = 2;
        GHP_MODE_PCM = 6;
        GHP_TEMP_PCM = 8;
        GHP_GROUP_PCM = 20;
        GHP_GROUP_MODE_PCM = 5;
        GHP_WINDSPEED_PCM = 22;
        GHP_WINDDIRECTION_PCM = 23;        
        GHP_ERROR_PCM = 21;
        GHP_ERROR_NUM_PCM = 24;
        GHP_GROUP_STATUS_PCM = 25;
        GHP_GROUP_DATA_WEEK_PCM = 26;
        GHP_GROUP_DATA1_PCM = 27;
        GHP_GROUP_DATA2_PCM = 28;
        GHP_MODE_STATUS_PCM = 30;
        GHP_RUNTIME_PCM = 31;
    }
}



void hong_get_unit_cnt(void)
{
	switch(g_sddcNum)
	{
		case SDDC_AC_DONG :
	        g_unitCnt = CNT_A_MAX_POINT + CNT_C_MAX_POINT;
	        if(g_dbgShow)	printf("%s : A Dong, C Dong SDDC_Num(%d)\n", 
	        	__FUNCTION__, g_sddcNum);  		
			break;
			
		case SDDC_B_DONG : 
	        g_unitCnt = CNT_B_MAX_POINT; 
	        if(g_dbgShow)	printf("%s : B Dong SDDC_Num(%d) g_unitCnt = %d\n", 
	        	__FUNCTION__, g_sddcNum, g_unitCnt);
			break;
			
		case SDDC_D_DONG :
	        g_unitCnt = CNT_D_MAX_POINT;
	        if(g_dbgShow)	printf("%s : D Dong SDDC_Num(%d)\n", 
	        	__FUNCTION__, g_sddcNum);
			break;
			
		case SDDC_KISUKSA :
        	g_unitCnt = CNT_D_KISUKSA_MAX_POINT;
        	if(g_dbgShow)	printf("%s : Duruam Kisuksa SDDC_Num(%d)\n", 
        		__FUNCTION__, g_sddcNum);
			break;
			
		case SDDC_E_DONG : 
	        g_unitCnt = CNT_E_MAX_POINT;
	        if(g_dbgShow)	printf("%s : E Dong SDDC_Num(%d)\n", 
	        	__FUNCTION__, g_sddcNum);     		
			break;
			
		case SDDC_F_DONG : 
	        g_unitCnt = CNT_F_MAX_POINT;
	        if(g_dbgShow)	printf("%s : F Dong SDDC_Num(%d)\n", 
	        	__FUNCTION__, g_sddcNum);
			break;
			
		case SDDC_G_DONG : 
	        g_unitCnt = CNT_G_MAX_POINT;
	        if(g_dbgShow)	printf("%s : G Dong SDDC_Num(%d)\n", 
	        	__FUNCTION__, g_sddcNum);
			break;
			
		default :
	        if(g_dbgShow)	printf("%s : Unknown g_sddcNum(%d). set 0.\n", 
	        	__FUNCTION__, g_sddcNum);
	        g_unitCnt = 0;				
			break;
	}
}


void hong_get_ghp_data(void)
{
	int i = 0;
	FILE *fp;
	//char bufName[32];
	//char bufId[32];
	char filename[32];
	//unsigned char *pFile;
	//g_GHP_Info *p;
	//int filesize = 0;
	int datasize = 0;
	
	g_pDongInfo = malloc( (sizeof(g_GHP_Info) * 256) );
	memset(g_pDongInfo, 0x00, (sizeof(g_GHP_Info) * 256));

	// get filename	
	memset(filename, 0x00, sizeof(filename));
	switch(g_sddcNum)
	{
		case SDDC_AC_DONG :
			memcpy(filename, "ClassAC.dat", sizeof("ClassAC.dat"));
			break;
			
		case SDDC_B_DONG : 
			memcpy(filename, "ClassB.dat", sizeof("ClassB.dat"));
			break;
			
		case SDDC_D_DONG :
			memcpy(filename, "ClassD.dat", sizeof("ClassD.dat"));
			break;
			
		case SDDC_E_DONG : 
			memcpy(filename, "ClassE.dat", sizeof("ClassE.dat"));
			break;
			
		case SDDC_F_DONG : 
			memcpy(filename, "ClassF.dat", sizeof("ClassF.dat"));
			break;
			
		case SDDC_G_DONG : 
			memcpy(filename, "ClassG.dat", sizeof("ClassG.dat"));
			break;
			
		case SDDC_KISUKSA :
			memcpy(filename, "ClassK.dat", sizeof("ClassK.dat"));
			break;	
					
		default :
			return;
	}
	
	
	if((fp = fopen(filename, "r")) == NULL)	{
		printf("%s open fail.\n", filename);
		return;			
	}

	//printf("%s() Read File :: %s \n", __FUNCTION__, filename);
	
	datasize = sizeof(g_GHP_Info);
	for( i = 0; i < 256; i++) 
		fread((g_pDongInfo + i), datasize, 1, fp);
	
	fclose(fp);
	//printf("%s() Close File :: %s \n", __FUNCTION__, filename);
	//fprintf(stderr, "[GHP] %s() Close File :: %s \n", __FUNCTION__, filename);
	//fflush(stderr);

	//printf("pno = %d\n", g_pDongInfo->nPno);
	//printf("pno = %d\n", (g_pDongInfo+1)->nPno);

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

/*
        GHP_ONOFF_PCM = 0;
        GHP_ONOFF_FAN_PCM = 1;
        GHP_SET_TEMP_PCM = 2;
        GHP_MODE_PCM = 6;
        GHP_TEMP_PCM = 8;
        GHP_GROUP_PCM = 20;
        GHP_GROUP_MODE_PCM = 5;
        GHP_WINDSPEED_PCM = 22;
        GHP_WINDDIRECTION_PCM = 23;        
        GHP_ERROR_PCM = 21;
        GHP_ERROR_NUM_PCM = 24;
        GHP_GROUP_STATUS_PCM = 25;
        GHP_GROUP_DATA_WEEK_PCM = 26;
        GHP_GROUP_DATA1_PCM = 27;
        GHP_GROUP_DATA2_PCM = 28;
        GHP_MODE_STATUS_PCM = 30;
        GHP_RUNTIME_PCM = 31; 
 
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


void hong_while(void)
{
	int st = 0;
	int result = 0;

	g_CntHaksaInit = 0;

	/*
	 Test Code.
	 반드시 현장에서 지워야 한다. 
	*/
	while( 1 ) {
		IfaceGhpSelectSleep(3, 0);
	}

	/*
	uart2 통신에 의한 Delay가 발생하는 상황을 재현하고자 한다.
	Test가 끝나면 반드시 주석으로 막아 실행되지 않도록 한다.
	*/
#if 0	
	printf("Test Code\n");
	memset(txbuf, 0, sizeof(txbuf));
	for (i = 0; i < 256; i++) {
		txbuf[i] = tempVal++;
	}

	printf("init poll\n");
	init_poll();
	while(1)
	{
		result = OnReadIn();
		if ( 0 < result ) {
			printf("Rx (%d)\n", result);
			/*
			for (i = 0; i < result; i++) {
			printf("%x ", rxbuf[i]);
			}
			printf("\n\n");
			*/
			printf("Write\n");
			OnWriteOut(rxbuf, result);
			printf("scehdule\n");
			Check_Schedule(g_unitCnt);
		}
		
	}
#endif	
	
#if 0
	while( 1 ) {
		IfaceGhpSelectSleep(1, 0);
		Chk_Group_Control();

		// 전체 정지 모드 
		Set_AllStopMode(g_unitCnt);

		// 실내기 온도 제한 
		Set_Temperature(g_unitCnt);
	}
#endif

 	//super loop
	st = ST_GET_POLLING;
 	do{

		// 전체 정지 모드 
		Set_AllStopMode(g_unitCnt);
		
		// 실내기 온도 제한 
		Set_Temperature(g_unitCnt);
		
		Get_RunUnit(g_unitCnt);
				
		// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
		Chk_Group_Control();	
			
		// jong2ry test.
		// It is scheduler function.
		Check_Schedule(g_unitCnt);
		//sleep(1);
		//continue;
		
		
		Set_AllHaksaMode(g_unitCnt);

		switch(st)
		{
			case ST_GET_POLLING:
				//if(g_dbgShow) printf("ST_GET_POLLING\n");

				result = Send_Polling();
				//printf("result = %d\n", result);
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
						//if(g_dbgShow) printf(">>> POLLING ERROR\n");
						st = ST_CHK_USER_CONTROL;
						break;
				}
				break;

			case ST_CHK_USER_CONTROL:
				CheckUserControl();
				st = ST_GET_USER_CONTROL;
				break;

			case ST_GET_USER_CONTROL:
				//if(g_dbgShow) printf("ST_GET_USER_CONTROL\n");

				while(Do_User_Control()) {

					if (Send_Polling() == POLL_REQUIRE_STARTUP) {
						st = ST_STARTUP;	
						break;
					}
	
					// 전체 정지 모드 
					Set_AllStopMode(g_unitCnt);
					
					// 실내기 온도 제한 
					Set_Temperature(g_unitCnt);
					
					Get_RunUnit(g_unitCnt);
							
					// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
					Chk_Group_Control();						
				}


				st = ST_GET_GHP_UNIT;
				break;

			case ST_GET_GHP_UNIT:
				//if(g_dbgShow) printf("ST_GET_GHP_UNIT\n");
				Get_Ghp_Unit();

				if (Send_Polling() == POLL_REQUIRE_STARTUP) {
					st = ST_STARTUP;	
					break;
				}   

				//if(g_dbgShow) printf("\n\n");	
				st = ST_GET_POLLING;
				break;

			case ST_STARTUP:
				//if(g_dbgShow) printf("ST_STARTUP\n");
				Startup_SDDC();
				st = ST_GET_POLLING;
				break;

			default:
				//if(g_dbgShow) printf("default\n");
				st = ST_GET_POLLING;
				break;
		}
	}while(1);
	
	//exit(1);
	system("killall duksan");
}


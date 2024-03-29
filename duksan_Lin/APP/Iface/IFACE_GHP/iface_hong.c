



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
		GHP_UNIT_RUNTIME_PCM = 29;			// 실내기가 현재가동한 시간.
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
		GHP_UNIT_RUNTIME_PCM = 29;			// 실내기가 현재가동한 시간.
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

		// 스포츠센터
		case SDDC_I_DONG : 
	        g_unitCnt = CNT_I_MAX_POINT;
	        if(g_dbgShow)	printf("%s : I Dong(Sport Complex) SDDC_Num(%d)\n", 
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

		case SDDC_I_DONG : 
			memcpy(filename, "ClassI.dat", sizeof("ClassI.dat"));
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


		fprintf(stderr, ">> g_unitCnt [%s] %d,%d,%d %d %d,%d %d %d,%d\n",
		g_UnitData[i].name,
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


void hong_read_name(void)
{
	FILE *fp;
	char filename[32];
	char bufPno[12], bufBno[12], bufName[12];
	int cnt = 0;

	// get filename	
	memset(filename, 0x00, sizeof(filename));
	switch(g_sddcNum)
	{
		case SDDC_AC_DONG :
			memcpy(filename, "nameAC.txt", sizeof("nameAC.txt") );
			break;
			
		case SDDC_B_DONG : 
			memcpy(filename, "nameB.txt", sizeof("nameB.txt") );
			break;
			
		case SDDC_D_DONG :
			memcpy(filename, "nameD.txt", sizeof("nameD.txt") );
			break;
			
		case SDDC_E_DONG : 
			memcpy(filename, "nameE.txt", sizeof("nameE.txt") );
			break;
			
		case SDDC_F_DONG : 
			memcpy(filename, "nameF.txt", sizeof("nameF.txt") );
			break;
			
		case SDDC_G_DONG : 
			memcpy(filename, "nameG.txt", sizeof("nameG.txt") );
			break;

		case SDDC_I_DONG : 
			memcpy(filename, "nameI.txt", sizeof("nameI.txt") );
			break;
			
		case SDDC_KISUKSA :
			memcpy(filename, "nameKisuksa.txt", sizeof("nameKisuksa.txt") );
			break;	
					
		default :
			return;
	}
	
	fp = fopen(filename, "r");
	if( fp == NULL ) {
		printf("%s open fail.\n", filename);
		return;			
	}

	printf("%s() Read File :: %s \n", __FUNCTION__, filename);

	cnt = 0;
	while(!feof(fp)) {
		fscanf(fp, "%s %s %s ", bufPno, bufBno, bufName);	
		fprintf(stdout, "Read File :: %s, %s, %s\n", bufPno, bufBno, bufName);	
		fflush(stdout);
		
		memcpy(&g_UnitData[cnt++].name, bufName, 12 );
	}	
	
	fclose(fp);
}


void Get_RunUnit(int iUnitCnt)
{
    int i = 0;
    int iCnt = 0;
  
	switch(g_sddcNum) {
    case SDDC_AC_DONG :
        for (i = 0; i < iUnitCnt; i++) {
        	if ( i >= 105 && i <= 229 )
        		continue;
        	
            if ( pGet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno) > 0 )
                iCnt++;
        }
        if(g_dbgShow)
        	printf("Count A = %d\n", iCnt);	
        	
        pSet( GHP_MODE_STATUS_PCM, 235, iCnt );

		iCnt = 0;
        for (i = 0; i < iUnitCnt; i++) {
        	if ( i >= 105 && i <= 229 ) {
	            if ( pGet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno) > 0 )
	                iCnt++;
            }
        }
        if(g_dbgShow)
        	printf("Count C = %d\n", iCnt);	
        pSet( GHP_MODE_STATUS_PCM, 236, iCnt );
        return;
    }

	for (i = 0; i < iUnitCnt; i++) {
	    if ( pGet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno) > 0 )
	    	iCnt++;
	}
	if(g_dbgShow)
		printf("Count = %d\n", iCnt);	
	pSet( GHP_MODE_STATUS_PCM, 236, iCnt );
}


void Set_AllStopMode(int iUnitCnt)
{
    int i = 0;
  
    if( pGet(GHP_MODE_STATUS_PCM, PNO_ALL_STOP_MODE) > 0) {
        for (i = 0; i < iUnitCnt; i++) {
            pSet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno, 0);	
        }
    }
}


void Log_Scehudule(int iUnitCnt)
// ----------------------------------------------------------------------------
// WRITE LOG TEXT
// Description : log를 기록하여 Debugging에 사용한다. 
// Arguments   : type		log에 기록되는 text의 type
// Returns     : none
{
	int i = 0, j = 0;
	FILE *fp = NULL;
	//FILE *fp_log = NULL;
	//FILE *fp_org = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	//int size = 0;
	//unsigned char buff[1028];
	unsigned int myTime = 0;
	unsigned int runtime = 0;
	unsigned int limit_time = 0;
	unsigned int chk_onoff = 0;
	unsigned int chk_err = 0;
	//unsigned int name_size = 0;
	char br_code[32] = {
		0x3c, 0x68, 0x72, 0x20, 
		0x63, 0x6f, 0x6c, 0x6f, 
		0x72, 0x20, 0x3d, 0x22, 
		0x73, 0x69, 0x6c, 0x76, 
		0x65, 0x72, 0x22, 0x20, 
		0x73, 0x69, 0x7a, 0x65, 
		0x3d, 0x22, 0x31, 0x22, 
		0x2f, 0x3e
	};
	
	//char chPer = 0x25;
	char chChk = 0x22;
		
	/*
	char tab_code[24] = {
		0x26, 0x6e, 0x62, 0x73, 0x70, 0x3b, 
		0x26, 0x6e, 0x62, 0x73, 0x70, 0x3b, 
		0x26, 0x6e, 0x62, 0x73, 0x70, 0x3b, 
		0x26, 0x6e, 0x62, 0x73, 0x70, 0x3b
	};
	*/

	/*
	char space_code[6] = {
		0x26, 0x6e, 0x62, 0x73, 0x70, 0x3b
	};
	*/

	/*
	char name_tab_code[5] = {
		0x26, 0x23, 0x30, 0x39, 0x3b
	};
	*/

	fp = fopen("/httpd/s.html", "w");
	if( fp == NULL ) {
		return;		
	}
	/*
	fp_org = fopen("/httpd/ss.html", "r");
	if( fp_org == NULL ) {
		fclose(fp);
		return;		
	}
	else {
		fseek(fp, 0L, SEEK_SET); 
		memset( buff, 0x00, sizeof(buff) );
		while( 0 < (size = fread( buff, 1, 1024, fp_org))) {
			fwrite( buff, 1, size, fp);
			memset( buff, 0x00, sizeof(buff) );
		}  
	}	
	
	fclose(fp_org);	
	*/
	
	
	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);

	fprintf(fp, "<head>");
	fprintf(fp, "<title>");
	fprintf(fp, "[CCMS]");
	fprintf(fp, "Hong Schedule Control");
	fprintf(fp, "</title>");
	
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=utf-8%c /> ",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<meta http-equiv=%ccache-control%c content=%cno-cache%c />  ",
			chChk, chChk, chChk, chChk);	
			
	fprintf(fp, "<header manifest=%cduksan.manifest%c>",chChk, chChk);	
	
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");	
	
	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);
	myTime = (tm_ptr->tm_hour * 100) + tm_ptr->tm_min;

	fprintf(fp,	"Update Time :  %d/%d %d:%d\n\n", 
			tm_ptr->tm_mon + 1, 
			tm_ptr->tm_mday, 
			tm_ptr->tm_hour, 
			tm_ptr->tm_min);

	fwrite( br_code, 1, 32, fp);

	limit_time = (g_limitTime/100) * 60;
	limit_time += g_limitTime%100;


	fprintf(fp, "<TABLE border=0 bgcolor=ddffdd>");
	fprintf(fp, "<tr bgcolor=ddffdd>");

	// name
	fprintf(fp, "<td width=100 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp, "호실");
	fprintf(fp, "</td>");

	// onoff
	fprintf(fp, "<td width=50 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"상태");	
	fprintf(fp, "</td>");

	
	// schedule check
	fprintf(fp, "<td width=60 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"스케쥴");	
	fprintf(fp, "</td>");

	// pno
	fprintf(fp, "<td width=80 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"PNO");	
	fprintf(fp, "</td>");
	
	// group
	fprintf(fp, "<td width=60 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"그룹");	
	fprintf(fp, "</td>");

	
	// mode
	fprintf(fp, "<td width=40 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"모드");	
	fprintf(fp, "</td>");

	// method
	fprintf(fp, "<td width=40 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"동작");	
	fprintf(fp, "</td>");

	// time-schedule
	fprintf(fp, "<td width=450 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"스케쥴 적용 시간");
	fprintf(fp, "</td>");

	// limit-time
	fprintf(fp, "<td width=140 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	" 제한시간");
	fprintf(fp, "</td>");
	
	// run-time 
	fprintf(fp, "<td width=200 ALIGN=CENTER bgcolor=ddffdd>");
	fprintf(fp,	"기동시간");
	fprintf(fp, "</td>");

	fprintf(fp, "</tr>");
	fprintf(fp, "</table>");

	fwrite( br_code, 1, 32, fp);

	for ( i = 0; i < iUnitCnt; i++ ) {
		
		//fwrite( "<TABLE border=0>", 1, sizeof("<TABLE border=0>"), fp);	
		//fwrite( "<tr>", 1, 4, fp);	

		fprintf(fp, "<TABLE border=0 >");
		fprintf(fp, "<tr>");

		// name
		fprintf(fp, "<td width=100 ALIGN=LEFT >");
		fprintf(fp, " + %s ", g_UnitData[i].name);
		fprintf(fp, "</td>");

		// onoff
		fprintf(fp, "<td width=50 ALIGN=CENTER >");
		chk_onoff = pGet(GHP_ONOFF_FAN_PCM, g_UnitData[i].pno);
		if ( chk_onoff > 0 ) 
			fprintf(fp,	"[On]");
		//else 
		//	fprintf(fp,	"[--]");
		fprintf(fp, "</td>");

		
		// schedule check
		fprintf(fp, "<td width=60 ALIGN=CENTER >");
		chk_err = 0;
		for ( j = 0; j < 5; j++ )	{
			if ( g_UnitData[i].startT[j] <= myTime && g_UnitData[i].stopT[j] >= myTime )  {
				chk_err++;
			}
		}
		if ( chk_err > 0 ) 
			fprintf(fp,	"[S]");
		//else 
		//	fprintf(fp,	"[-]");
		fprintf(fp, "</td>");

		// pno
		fprintf(fp, "<td width=80 ALIGN=CENTER >");
		fprintf(fp,	"Pno(%3d)",g_UnitData[i].pno);
		fprintf(fp, "</td>");
		
		// group
		fprintf(fp, "<td width=60 ALIGN=CENTER >");
		if ( g_UnitData[i].group == 0) 
			fprintf(fp,	"학사");
		else 
			fprintf(fp,	"%02d그룹",g_UnitData[i].group);
		fprintf(fp, "</td>");

		
		// mode
		fprintf(fp, "<td width=40 ALIGN=CENTER >");
		if ( g_UnitData[i].group_method == 0 ) 
			fprintf(fp,	"주간");
		else if ( g_UnitData[i].group_method == 1 ) 
			fprintf(fp,	"요일");
		fprintf(fp, "</td>");

		// method
		fprintf(fp, "<td width=40 ALIGN=CENTER >");
		if ( g_UnitData[i].group_mode == 0 ) 
			fprintf(fp,	"가동");
		else if ( g_UnitData[i].group_mode == 1 ) 
			fprintf(fp,	"권한");
		else if ( g_UnitData[i].group_mode == 2 ) 
			fprintf(fp,	"정지");
		fprintf(fp, "</td>");

		// time-schedule
		for ( j = 0; j < 5; j++ )	{
			fprintf(fp, "<td width=90 ALIGN=CENTER >");
			if ( g_UnitData[i].startT[j] != 0 || g_UnitData[i].stopT[j] != 0 ) {
				fprintf(fp,	"[%04d - %04d]  ", 
					g_UnitData[i].startT[j], 
					g_UnitData[i].stopT[j]);
			}
			fprintf(fp, "</td>");
		}

		// limit-time
		fprintf(fp, "<td width=140 ALIGN=CENTER >");
		fprintf(fp,	"( %d / %d )분", g_UnitData[i].minCount, limit_time);
		fprintf(fp, "</td>");
		
		// run-time 
		fprintf(fp, "<td width=200 ALIGN=CENTER >");
		runtime = pGet(GHP_UNIT_RUNTIME_PCM, g_UnitData[i].pno);
		fprintf(fp,	"%d시간 %d분(%d)", runtime/60, runtime%60, runtime);
		fprintf(fp, "</td>");

		fprintf(fp, "</tr>");
		fprintf(fp, "</table>");
		
		fwrite( br_code, 1, 32, fp);
	}


	fwrite( "</font>", 1, 7, fp);
	fwrite( "</body>", 1, 7, fp);
	fwrite( "</html>", 1, 7, fp);

	fclose(fp);	
	return;
}


void Get_IndoorUnitRunTime(int iUnitCnt)
{
    int i = 0;
	float f_runtime = 0;

	for (i = 0; i < iUnitCnt; i++) {
	    if ( pGet(GHP_ONOFF_FAN_PCM, g_pDongInfo[i].nPno) > 0 ) {
			f_runtime = pGet(GHP_UNIT_RUNTIME_PCM, g_pDongInfo[i].nPno);
			f_runtime++;
			pSet(GHP_UNIT_RUNTIME_PCM, g_pDongInfo[i].nPno, f_runtime);
		}
	}
}


void Auto_OutUnitCtrl(void)
{
	float chk_mode = 0;
	float chk_temp = 0;
	float chk_cool_hi = 0;
	float chk_cool_lo = 0;
	float chk_heat_hi = 0;
	float chk_heat_lo = 0;
	float chk_vent_hi = 0;
	float chk_vent_lo = 0;

	// point define
	// 242 : now outdoor temperature
	// 243 : mode on/off
	// 244 : heat hi value
	// 245 : heat low value
	// 246 : cool hi value
	// 247 : cool low value
	// 248 : vent hi value
	// 249 : vent low value
	chk_temp = pGet(GHP_MODE_STATUS_PCM, 242);;
	chk_mode = pGet(GHP_MODE_STATUS_PCM, 243);
	chk_heat_hi = pGet(GHP_MODE_STATUS_PCM, 244);
	chk_heat_lo = pGet(GHP_MODE_STATUS_PCM, 245);
	chk_cool_hi = pGet(GHP_MODE_STATUS_PCM, 246);
	chk_cool_lo = pGet(GHP_MODE_STATUS_PCM, 247);
	chk_vent_hi = pGet(GHP_MODE_STATUS_PCM, 248);
	chk_vent_lo = pGet(GHP_MODE_STATUS_PCM, 249);

	if(g_dbgShow) {
		printf("(%3.0f - %3.0f) C-[%3.0f-%3.0f] H-[%3.0f-%3.0f] V-[%3.0f-%3.0f]\n",
			   chk_temp, chk_mode,
			   chk_cool_hi, chk_cool_lo,
			   chk_heat_hi, chk_heat_lo,
			   chk_vent_hi, chk_vent_lo );
	}

	if ( chk_mode == 0 ) {
		return;
	}

	// Auto vent mode
	if ( chk_vent_hi >= chk_temp && chk_vent_lo <= chk_temp ) {
		pSet(GHP_MODE_STATUS_PCM, 253, 2);
		return;
	}
		
	// Auto cool mode
	if ( chk_cool_hi >= chk_temp && chk_cool_lo <= chk_temp ) {
		pSet(GHP_MODE_STATUS_PCM, 253, 1);
		return;
	}

	// Auto heat mode
	if ( chk_heat_hi >= chk_temp && chk_heat_lo <= chk_temp ) {
		pSet(GHP_MODE_STATUS_PCM, 253, 0);
		return;
	}
}




void Auto_OutUnitCtrl_A(void)
{
	int i = 0;
	float chk_mode = 0;
	float chk_temp = 0;
	float chk_cool_hi = 0;
	float chk_cool_lo = 0;
	float chk_heat_hi = 0;
	float chk_heat_lo = 0;
	float chk_vent_hi = 0;
	float chk_vent_lo = 0;

	// point define
	// 225 : A동 실외기 모드 상태
	// 242 : now outdoor temperature
	// 243 : mode on/off
	// 244 : heat hi value
	// 245 : heat low value
	// 246 : cool hi value
	// 247 : cool low value
	// 248 : vent hi value
	// 249 : vent low value
	chk_temp = pGet(GHP_MODE_STATUS_PCM, 242);;
	chk_mode = pGet(GHP_MODE_STATUS_PCM, 243);
	chk_heat_hi = pGet(GHP_MODE_STATUS_PCM, 244);
	chk_heat_lo = pGet(GHP_MODE_STATUS_PCM, 245);
	chk_cool_hi = pGet(GHP_MODE_STATUS_PCM, 246);
	chk_cool_lo = pGet(GHP_MODE_STATUS_PCM, 247);
	chk_vent_hi = pGet(GHP_MODE_STATUS_PCM, 248);
	chk_vent_lo = pGet(GHP_MODE_STATUS_PCM, 249);

	if(g_dbgShow) {
		printf("(%3.0f - %3.0f) C-[%3.0f-%3.0f] H-[%3.0f-%3.0f] V-[%3.0f-%3.0f]\n",
			   chk_temp, chk_mode,
			   chk_cool_hi, chk_cool_lo,
			   chk_heat_hi, chk_heat_lo,
			   chk_vent_hi, chk_vent_lo );
	}

	if ( chk_mode == 0 ) {
		return;
	}

	// Auto vent mode
	if ( chk_vent_hi >= chk_temp && chk_vent_lo <= chk_temp ) {
		// A동 실외기
		for( i = 101; i <= 111; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 2);
		}
		pSet(GHP_MODE_STATUS_PCM, 225, 2);;
		return;
	}
		
	// Auto cool mode
	if ( chk_cool_hi >= chk_temp && chk_cool_lo <= chk_temp ) {
		// A동 실외기
		for( i = 101; i <= 111; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 1);
		}
		pSet(GHP_MODE_STATUS_PCM, 225, 1);;
		return;
	}

	// Auto heat mode
	if ( chk_heat_hi >= chk_temp && chk_heat_lo <= chk_temp ) {
		// A동 실외기
		for( i = 101; i <= 111; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 0);
		}
		pSet(GHP_MODE_STATUS_PCM, 225, 0);;
		return;
	}
}


void Auto_OutUnitCtrl_C(void)
{
	int i = 0;
	float chk_mode = 0;
	float chk_temp = 0;
	float chk_cool_hi = 0;
	float chk_cool_lo = 0;
	float chk_heat_hi = 0;
	float chk_heat_lo = 0;
	float chk_vent_hi = 0;
	float chk_vent_lo = 0;

	// point define
	// 226 : C동 실외기 모드 상태
	// 227 : now outdoor temperature
	// 228 : mode on/off
	// 229 : heat hi value
	// 230 : heat low value
	// 231 : cool hi value
	// 232 : cool low value
	// 233 : vent hi value
	// 234 : vent low value
	chk_temp = pGet(GHP_MODE_STATUS_PCM, 227);;
	chk_mode = pGet(GHP_MODE_STATUS_PCM, 228);
	chk_heat_hi = pGet(GHP_MODE_STATUS_PCM, 229);
	chk_heat_lo = pGet(GHP_MODE_STATUS_PCM, 230);
	chk_cool_hi = pGet(GHP_MODE_STATUS_PCM, 231);
	chk_cool_lo = pGet(GHP_MODE_STATUS_PCM, 232);
	chk_vent_hi = pGet(GHP_MODE_STATUS_PCM, 233);
	chk_vent_lo = pGet(GHP_MODE_STATUS_PCM, 234);

	if(g_dbgShow) {
		printf("(%3.0f - %3.0f) C-[%3.0f-%3.0f] H-[%3.0f-%3.0f] V-[%3.0f-%3.0f]\n",
			   chk_temp, chk_mode,
			   chk_cool_hi, chk_cool_lo,
			   chk_heat_hi, chk_heat_lo,
			   chk_vent_hi, chk_vent_lo );
	}

	if ( chk_mode == 0 ) {
		return;
	}

	// Auto vent mode
	if ( chk_vent_hi >= chk_temp && chk_vent_lo <= chk_temp ) {
		// C동 실외기
		for( i = 112; i <= 126; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 2);
		}
		pSet(GHP_MODE_STATUS_PCM, 226, 2);;
		return;
	}
		
	// Auto cool mode
	if ( chk_cool_hi >= chk_temp && chk_cool_lo <= chk_temp ) {
		// C동 실외기
		for( i = 112; i <= 126; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 1);
		}
		pSet(GHP_MODE_STATUS_PCM, 226, 1);;
		return;
	}

	// Auto heat mode
	if ( chk_heat_hi >= chk_temp && chk_heat_lo <= chk_temp ) {
		// C동 실외기
		for( i = 112; i <= 126; i++ ) {
			pSet(GHP_MODE_STATUS_PCM, i, 0);
		}
		pSet(GHP_MODE_STATUS_PCM, 226, 0);;
		return;
	}
}


void Set_Temperature(int iUnitCnt)
{
    int i = 0;
    int nVal = 0;
    int nBno = 0;
    int nPno = 0;
    int nType = 0;
    int iFh_Temp_Min = (int)pGet(30, 238);
    int iFh_Temp_Max = (int)pGet(30, 239);
    int iSh_Temp_Min = (int)pGet(30, 240);
    int iSh_Temp_Max = (int)pGet(30, 241);

    //printf("fh %d - %d\n", iFh_Temp_Min, iFh_Temp_Max);
    //printf("sh %d - %d\n", iSh_Temp_Min, iSh_Temp_Max);

    for(i = 0; i < g_unitCnt; i++) {
        Get_Bno_Pno_Type(i, &nBno, &nPno, &nType);

        if (nType == SH_INDOOR) {
            nVal = (int)pGet(GHP_SET_TEMP_PCM, nPno);
            //printf("%d - %d\n", nPno, nVal);
            if (nVal < iSh_Temp_Min) {
                pSet(GHP_SET_TEMP_PCM, nPno, iSh_Temp_Min);
                continue;
            }

            if (nVal > iSh_Temp_Max) {
                pSet(GHP_SET_TEMP_PCM, nPno, iSh_Temp_Max);
                continue;
            }                        
        }
        else if (nType == FH_INDOOR) {
            nVal = (int)pGet(GHP_SET_TEMP_PCM, nPno);

            //printf("%d - %d\n", nPno, nVal);
            if (nVal < iFh_Temp_Min) {
                pSet(GHP_SET_TEMP_PCM, nPno, iFh_Temp_Min);
                continue;
            }

            if (nVal > iFh_Temp_Max) {
                pSet(GHP_SET_TEMP_PCM, nPno, iFh_Temp_Max);
                continue;
            }  
        }

    }
}


void Set_AllHaksaMode(int iUnitCnt)
{
    int i = 0;
    float value = 0;
  
  	value = g_fExPtbl[GHP_MODE_STATUS_PCM][237] ;
  	
	if ( prePtbl[GHP_MODE_STATUS_PCM][237] != value ) {
		for (i = 0; i < iUnitCnt; i++) {
		    if ( pGet(GHP_GROUP_PCM, g_pDongInfo[i].nPno) == 0 )
		    	pSet( GHP_GROUP_MODE_PCM, g_pDongInfo[i].nPno, value);
		}
		prePtbl[GHP_MODE_STATUS_PCM][237] = g_fExPtbl[GHP_MODE_STATUS_PCM][237];
	}
}




void hong_while(void)
{
	int st = 0;
	int result = 0;
	time_t     tm_nd;
	struct tm *tm_ptr;
	int pre_minute = 0;
	int now_minute = 0;
	int nCountMinute = 0;
	int nContolLoopCount = 0;

	g_CntHaksaInit = 0;


 	//super loop
	st = ST_GET_POLLING;

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);
	pre_minute = 0;
	now_minute = tm_ptr->tm_min;

 	do{
 		// jong2ry 2011.03.21 
 		// watchdog으로 감시하기 위하여 설정하였다. 
 		g_pList->nLoogCnt[0]++;
 		
		IfaceGhpSelectSleep(0,100);		 		
#if 1
		fflush(stdout);

		// 전체 정지 모드 
		// test code...
		Set_AllStopMode(g_unitCnt);
printf("\na");
		
		// 실내기 온도 제한 
		if ( g_sddcNum != SDDC_KISUKSA )
			Set_Temperature(g_unitCnt);
printf("b");
		
		// 현재 가동중인 실내기 파악.
		Get_RunUnit(g_unitCnt);
printf("c");
				
		// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
		Chk_Group_Control();	
printf("d");			

		// 홍대 스케쥴.
		Check_Schedule(g_unitCnt);
printf("e");
		
		// 학사모드의 모든 실내기의 모드변환.
		Set_AllHaksaMode(g_unitCnt);
printf("f");		

		time(&tm_nd);
		tm_ptr = localtime(&tm_nd);
		now_minute = tm_ptr->tm_min;
		if(g_dbgShow)
			printf("minute ( %d , %d )\n",now_minute, pre_minute); 
		if ( pre_minute != now_minute ) {
			pre_minute = now_minute;

			// 실내기 가동시간 확인(Minute 단위)
			Get_IndoorUnitRunTime(g_unitCnt);

			// jong2ry 2011_0321 disable
			// 스케쥴 모드 확인.
			//Log_Scehudule(g_unitCnt);

			// 1시간마다 hwclock과 동기화
			if ( nCountMinute++ > 60 ) {
				nCountMinute = 0;
				system("hwclock -s");
			}
		}
printf("g");		

		// 외기온도에 따른 실외기 모드전환.
		if ( g_sddcNum != SDDC_AC_DONG ) {
			Auto_OutUnitCtrl();
		}
		else { 
			Auto_OutUnitCtrl_A();
			Auto_OutUnitCtrl_C();
		}
printf("h");
		
		// test code.
	//	IfaceGhpSelectSleep(5,0);
	//	continue;
#endif
		switch(st)
		{
			case ST_GET_POLLING:
				//if(g_dbgShow) printf("ST_GET_POLLING\n");
printf("i");
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
printf("j");				
				CheckUserControl();
				st = ST_GET_USER_CONTROL;
				break;

			case ST_GET_USER_CONTROL:
printf("k");								
				//if(g_dbgShow) printf("ST_GET_USER_CONTROL\n");
				nContolLoopCount = 0;
				while(Do_User_Control()) {
					if (Send_Polling() == POLL_REQUIRE_STARTUP) {
						st = ST_STARTUP;	
						break;
					}
#if 1
					// 전체 정지 모드 
					Set_AllStopMode(g_unitCnt);
					
					// 실내기 온도 제한 
					if ( g_sddcNum != SDDC_KISUKSA )
						Set_Temperature(g_unitCnt);
					
					Get_RunUnit(g_unitCnt);
							
					// 각 동의 층별제어 및 실내기 타입별 온도제어를 한다.
					Chk_Group_Control();			
#endif									
					// 제어루프가 계속 돌지 않도록 합니다.
					nContolLoopCount++;					
					if ( nContolLoopCount > 5 ) {
						st = ST_GET_GHP_UNIT;
						break;
					}

				}

				st = ST_GET_GHP_UNIT;
				break;

			case ST_GET_GHP_UNIT:
printf("l");								
				//if(g_dbgShow) printf("ST_GET_GHP_UNIT\n");
				Get_Ghp_Unit();

				if (Send_Polling() == POLL_REQUIRE_STARTUP) {
					st = ST_STARTUP;	
					break;
				}   

				st = ST_GET_POLLING;
				break;

			case ST_STARTUP:
printf("m");								
				//if(g_dbgShow) printf("ST_STARTUP\n");
				Startup_SDDC();
				st = ST_GET_POLLING;
				break;

			default:
printf("n");												
				//if(g_dbgShow) printf("default\n");
				st = ST_GET_POLLING;
				break;
		}
	}while(1);
	//exit(1);
	system("killall duksan");
}


// Test Code...!!!
/*
#if 0
	 //Test Code.
	 //반드시 현장에서 지워야 한다. 
#if 0	
	while( 1 ) {
		IfaceGhpSelectSleep(3, 0);
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
 
#endif
*/

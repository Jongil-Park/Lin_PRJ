///******************************************************************/
// file : ghp_schedule.h
// date : 2010.02.23.
// author : jong2ry
///******************************************************************/

int g_limitTime = 0;

/***********************************************************************/
// if group-number is 's' , group-name is 'Haksa".
// Otherwise group-name is "GROUP"
// but group-name have small 10
void ReadScheduleGroup(int index)
/***********************************************************************/
{
	int result = 0;
	int localDbg = 0;
	int pno = 0;
	
	pno = g_UnitData[index].pno;
	
	if (localDbg)
		printf("GHP_GROUP_PCM = %d, pno = %d\n", GHP_GROUP_PCM, pno);

	result = (int)pGet(GHP_GROUP_PCM, pno);	
	if( result > 10 || result < 0 ) {
		// When it's wrong value, value initialize '0'. 
		// '0' is HAKSA MODE.
		result = 0;		
		if (point_table[GHP_GROUP_PCM][pno] != 0)
			pSet(GHP_GROUP_PCM, pno, 0);	
	}

    g_UnitData[index].group = result; 

	if (localDbg)
		printf("%s :: g_UnitData[%d].group, %d, %d\n", __FUNCTION__, index, pno, g_UnitData[index].group);    	
}
//
//
/***********************************************************************/
void ReadScheduleMode(int index)
/***********************************************************************/
{
	int result = 0;
	int localDbg = 0;
	int pno = 0;
	
	pno = g_UnitData[index].pno;	

	result = (int)pGet(GHP_GROUP_MODE_PCM, pno);	
	if( result > 2 || result < 0  ) {
		result = 1;		// 설정이 잘못되어 있으면 초기값은 권한모드
		if (point_table[GHP_GROUP_MODE_PCM][pno] != 1)
			pSet(GHP_GROUP_MODE_PCM, pno, 1);			
		//DEL pSet(GHP_GROUP_MODE_PCM, g_UnitData[index].pno, 1);	
	}	
	
	g_UnitData[index].group_mode = result; 

	if (localDbg)
		printf("%s :: g_UnitData[%d].group_mode, %d\n", 
			__FUNCTION__, 
			index, 
			g_UnitData[index].group_mode);
}
//
//
/***********************************************************************/
void ReadScheduleMethod(int index)
/***********************************************************************/
{
	int groupIndex = 0;
	int method = 0;
	int localDbg = 0;
	
	if ( g_UnitData[index].group == SCHED_HAKSA_MODE ) {
		g_UnitData[index].group_method = 0; 
	}
	else {
		groupIndex = g_UnitData[index].group - 1;		//!! Be careful.
		method = g_schedFileData.group[groupIndex].method;
		g_UnitData[index].group_method = method; 
	}

	if (localDbg)
		printf("%s :: g_UnitData[%d].group_method, %d\n", 
			__FUNCTION__, 
			index, 
			g_UnitData[index].group_method);	
}
//
//
/***********************************************************************/
void ReadGroupScheduleTime(int index, int tm_wday)
/***********************************************************************/
{
	int i = 0;
	int j = 0;
	int group = 0;
	int groupIndex = 0;
	int day = 0;
	SEND_DATA *p;
	int localDbg = 0;
	int isTime = 0;

	group = g_UnitData[index].group;
	memset(&g_UnitData[index].startT, 0x00, sizeof(g_UnitData[index].startT));
	memset(&g_UnitData[index].stopT, 0x00, sizeof(g_UnitData[index].stopT));

	 /*
	 	printf("g_UnitData[%d].group_method  = %d, tm_wday = %d\n", 
		index,
		g_UnitData[index].group_method, 
		tm_wday);
	 */

	//printf("g_CntHaksaInit = %d\n", g_CntHaksaInit);
	if(group == SCHED_HAKSA_MODE) {
		p = (SEND_DATA *)&g_schedFileData;

		if ( g_CntHaksaInit > 300) {
			for (i = 0; i < 5; i++) {
				pSet( (SCHED_START_PCM_NO + (i * 2)), g_UnitData[index].pno, 0 );
				pSet( (SCHED_END_PCM_NO + (i * 2)),  g_UnitData[index].pno, 0 );
			}
		}

		for (i = 0; i < CLASS_DATA_COUNT; i++) {
			//printf("p->classData[i].classNum = %d, g_UnitData[index].pno = %d\n", p->classData[i].classNum, g_UnitData[index].pno);
			
			isTime = p->classData[i].classNum + p->classData[i].startT + p->classData[i].stopT;
			if (isTime == 0)
				continue;
				 
			if (p->classData[i].classNum == g_UnitData[index].pno) {
				g_UnitData[index].startT[j] = p->classData[i].startT;
				g_UnitData[index].stopT[j] = p->classData[i].stopT;
	
				pSet( (SCHED_START_PCM_NO + (j * 2)), g_UnitData[index].pno, g_UnitData[index].startT[j] );
				pSet( (SCHED_END_PCM_NO + (j * 2)),  g_UnitData[index].pno, g_UnitData[index].stopT[j] );
				
				/*
				printf("%03d-%03d (%04d),  %03d-%03d (%04d)\n", 
						 (SCHED_START_PCM_NO + (j * 2)),
						 g_UnitData[index].pno, 
						 g_UnitData[index].startT[j],
						 (SCHED_END_PCM_NO + (j * 2)),
						 g_UnitData[index].pno, 
						 g_UnitData[index].stopT[j] );
				*/
				j++;
				if (j >= 5)
					return;
			}
		}	
	}
	else {
		groupIndex = group - 1;
		//주간모드일 경우 Day값은 7이 된다. 
		//요일모드인경우 월요일은 (0) ~ 일요일(6)까지 계산된다. 
		
		if (g_UnitData[index].group_method == 0)
			day = 7;
		else {
			if (tm_wday == 0)
				day = 6;	
			else
				day = tm_wday - 1;	
		}
	
		for (i = 0; i < 5; i++) {
			g_UnitData[index].startT[i] = g_schedFileData.group[groupIndex].startT[day][i];
			g_UnitData[index].stopT[i] = g_schedFileData.group[groupIndex].stopT[day][i];
			
			pSet( (SCHED_START_PCM_NO + (i * 2)), g_UnitData[index].pno, g_UnitData[index].startT[i]);
			pSet( (SCHED_END_PCM_NO + (i * 2)),  g_UnitData[index].pno, g_UnitData[index].stopT[i]);
			/*
			printf("%03d-%03d (%04d),  %03d-%03d (%04d)\n", 
					 (SCHED_START_PCM_NO + (i * 2)),
					 g_UnitData[index].pno, 
					 g_UnitData[index].startT[i],
					 (SCHED_END_PCM_NO + (i * 2)),
					 g_UnitData[index].pno, 
					 g_UnitData[index].stopT[i] );
			*/
		}
	}
}
//
//
#if 0	//not used.
/***********************************************************************/
void gettime(time_t org_time)
/***********************************************************************/
{
    struct tm *tm_ptr;

    tm_ptr = localtime(&org_time);

	g_time.hour = tm_ptr->tm_hour;
	g_time.min = tm_ptr->tm_min;
	g_time.sec = tm_ptr->tm_sec;
}
#endif
//
//
/***********************************************************************/
int CheckLimitTime(int index)
/***********************************************************************/
{
	//DEL int retval = 0;
	time_t  the_time;
	struct tm *tm_ptr;
	int limitTime = 0;
	
	the_time = time(NULL); 
	tm_ptr= localtime(&the_time);	
	
	if( tm_ptr->tm_min != g_UnitData[index].preMinute ) {
		g_UnitData[index].minCount++;
		g_UnitData[index].preMinute = tm_ptr->tm_min;
	}
	
	limitTime = (g_limitTime/100) * 60;
	limitTime += g_limitTime%100;
	if (g_UnitData[index].minCount > limitTime)
		return SUCCESS;
	else
		return FAIL;	
}
//
//
/***********************************************************************/
void Sched_FreeMode(int index)
/***********************************************************************/
{
	int pno = 0;
	float value = 0;
	int localDbg = 1;
	
	pno = g_UnitData[index].pno;

	// Timeout이 최초로 발생하면 GHP를 OFF 시킨다.
	if (g_UnitData[index].chkTouch == 0) {
		g_UnitData[index].chkTouch = 1;

		if (point_table[GHP_ONOFF_PCM][pno] != 0)
			pSet(GHP_ONOFF_PCM, pno, 0);			
		//DEL pSet(GHP_ONOFF_PCM, pno, 0);		// GHP Unit off
		if (localDbg)
			printf("[Freemode] First Touch >> Off\n"); 
	}
	else {
		value = pGet(GHP_ONOFF_PCM, pno);
		if (value == 1) {
			if( CheckLimitTime(index) == SUCCESS ) {
				if (point_table[GHP_ONOFF_PCM][pno] != 0)
					pSet(GHP_ONOFF_PCM, pno, 0);	
				//DEL pSet(GHP_ONOFF_PCM, pno, 0);		// GHP Unit off
				g_UnitData[index].minCount = 0;
				if (localDbg)
					printf("[Freemode] Timeout >> Offf\n"); 
			}
		}		
	}
}
//
//
/***********************************************************************/
void Sched_ForceMode(int index)
/***********************************************************************/
{
	int pno = 0;
	float value = 0;
	int localDbg = 1;
	
	pno = g_UnitData[index].pno;
	
	// Timeout이 최초로 발생하면 GHP를 ON 시킨다.
	if (g_UnitData[index].chkTouch == 0) {
		g_UnitData[index].chkTouch = 1;
		
		if (point_table[GHP_ONOFF_PCM][pno] != 1)
			pSet(GHP_ONOFF_PCM, pno, 1);	

		if (localDbg)
			printf("[ForceMode] First Touch >> On\n"); 
	}
	/*
	else {
		
		value = pGet(GHP_ONOFF_PCM, pno);
		if (value == 1) {
			if( CheckLimitTime(index) == SUCCESS ) {
				if (point_table[GHP_ONOFF_PCM][pno] != 0)
					pSet(GHP_ONOFF_PCM, pno, 0);					
				//DEL pSet(GHP_ONOFF_PCM, pno, 0);		// GHP Unit off
				g_UnitData[index].minCount = 0;
				if (localDbg)
					printf("[ForceMode] Timeout >> Off\n"); 
			}
		}
	}	
	*/			
}
//	
//
/***********************************************************************/
void RunSchduleControl(int index, int myTime)
/***********************************************************************/
{
	int i = 0;
	int mode = 0;
	int check = 0;
	int localDbg = 1;
	
	check = 0;
	mode = g_UnitData[index].group_mode;
	for (i = 0 ; i < MAX_SCHEDULE_COUNT; i++)
	{
		if (g_UnitData[index].startT[i] == 0)
			continue;

		if (myTime >= g_UnitData[index].startT[i]	
				&& myTime < g_UnitData[index].stopT[i]) {
		check++;
		}
	}//for

	switch(mode) {
		// Force Mode
        // 가동모드일 경우 현재시간이 스케쥴시간에 포함이 된다면 한번 GHP를 On 시킨다. 
        // 하지만 스케줄시간에 포함되지 않는다면 GHP를 Off시킨다.
        // 현재시간이 스케쥴시간에 포함되지 않을 때 사용자가 'On'하게 되면 설정시간만큼 동작하고 Off된다. 
		case 0:
			if (check > 0) {
				Sched_ForceMode(index);
			}		
			else {
				g_UnitData[index].chkTouch = 0;
				g_UnitData[index].minCount = 0;
				g_UnitData[index].preMinute = 0;

				if (point_table[GHP_ONOFF_PCM][g_UnitData[index].pno] != 0) {
					pSet(GHP_ONOFF_PCM, g_UnitData[index].pno, 0);	
					if (localDbg)
						printf("[ForceMode] Time >> Off (%d)\n", g_UnitData[index].pno); 
				}
			}	
			break;
			
		// Free Mode
        // 권한모드일 경우 현재시간이 스케줄시간에 포함이 되어도 GHP를 On 시키지 않는다.   
        // 하지만 스케줄시간에 포함되지 않을 때 GHP를 Off시킨다.
        // 현재시간이 스케쥴시간에 포함되지 않을 때 사용자가 'On'하게 되면 설정시간만큼 동작하고 Off된다.         
		case 1:
			if (check > 0) {
				g_UnitData[index].chkTouch = 0;
				g_UnitData[index].minCount = 0;
				g_UnitData[index].preMinute = 0;
			}
			else {
				Sched_FreeMode(index);
			}
			break;
			
		// Stop Mode
		case 2:
			if (point_table[GHP_ONOFF_PCM][g_UnitData[index].pno] != 0)
				pSet(GHP_ONOFF_PCM, g_UnitData[index].pno, 0);				
			//DEL pSet(GHP_ONOFF_PCM, g_UnitData[index].pno, 0);	
			//printf("[STOP Mode] %d >> Offf\n", g_UnitData[index].pno); 
			break;
	}//end of switch(mode)
}
//
//
/***********************************************************************/
int CheckFileChksum(SEND_DATA *pRx)
/***********************************************************************/
{
	unsigned int i = 0;
	unsigned char *p;
	unsigned short chksum = 0;
	
	//printf("length = %d\n", pRx->length);	
	//printf("chksum = %d\n", pRx->chksum);		
	
	p = (unsigned char *)pRx;
	for ( i=0; i<pRx->length-2; i++)
		chksum += p[i];
	
	//printf(">> Cal = %d\n", chksum);
	if(chksum == pRx->chksum)
		return SUCCESS;	
	else		
		return FAIL;	
}
//
//
/***********************************************************************/
void ReadScheduleFile(void)
/***********************************************************************/
{
	int i = 0;
	int j = 0;
	int k = 0;
	FILE *fp = NULL;
	unsigned char *p;
	int filesize = 0;
	SEND_DATA *pRx;
	int localDbg = 0;

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
	
	if (filesize != sizeof(g_schedFileData)) {
		fclose(fp);
		printf(">>>>>>>>>>>>>>>>>>>>> filesize error\n");
		pthread_mutex_unlock(&schedule_mutex);
		return;	
	}

	p = (unsigned char *)&g_schedFileData;
	for ( i = 0; i < filesize; i++ )
		//fread(&p[i], 1, sizeof(unsigned char), fp);
		fread(&p[i], 1, sizeof(g_schedFileData), fp);

	if (fp != NULL)  fclose(fp);
		
	//schedule_mutex
	pthread_mutex_unlock(&schedule_mutex);
	
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
//
//
/***********************************************************************/
void SetScheduleData(int unitCount)
/***********************************************************************/
{
	int i = 0;
	int groupIndex = 0;
	int mode = 0;
	int localDbg = 0;
	
	for (i = 0; i < unitCount; i++)
	{
		if (g_UnitData[i].group != SCHED_HAKSA_MODE)
		{
			groupIndex = g_UnitData[i].group - 1;		//!! Be careful.
			mode = g_schedFileData.group[groupIndex].mode;
			if (point_table[GHP_GROUP_MODE_PCM][g_UnitData[i].pno] != (float)mode)
				pSet(GHP_GROUP_MODE_PCM, g_UnitData[i].pno, (float)mode);				
			//DEL pSet(GHP_GROUP_MODE_PCM, g_UnitData[i].pno, (float)mode);	
			
			if (localDbg)
				printf(">> (%d) - %d - %d\n", 
					g_UnitData[i].pno, 
					groupIndex, 
					mode);
		}
		if (localDbg)
			printf(">> (%d) - %d - %d\n", 
				g_UnitData[i].pno, 
				groupIndex, 
				mode);		
	}
}
//
//
/***********************************************************************/
void Check_Schedule(int unitCount)
/***********************************************************************/
{
	int i = 0;
	int j = 0;
	//DEL int mode = 0;
	//DEL int group = 0;
	//DEL int method = 0;
	//DEL int result = 0;
	time_t  the_time;
	struct tm *tm_ptr;
	//DEL char buffer[32];
	int myTime = 0;

    // get now time
	the_time = time(NULL); 
    tm_ptr= localtime(&the_time);
    myTime = (tm_ptr->tm_hour * 100) + tm_ptr->tm_min;
	
	// read schedule data from 'SchedGHP.dat'
	ReadScheduleFile();			
	
	// if it's group-mode, set mode value. (GHP_GROUP_MODE_PCM)
	SetScheduleData(unitCount);			
	
	
	if ( g_CntHaksaInit++ > 301 )
		g_CntHaksaInit = 0;
	
	printf("g_CntHaksaInit = %d\n", g_CntHaksaInit);
	
	for (i = 0; i < unitCount; i++)
	{
		//IfaceGhpSelectSleep(0, 10);
		ReadScheduleGroup(i);			// read group of ghpUnit
		ReadScheduleMode(i);			// read groupMode of ghpUnit	(force, free, stop)
		ReadScheduleMethod(i);			// read groupMethod of ghpUnit (wday, week)
		ReadGroupScheduleTime(i, tm_ptr->tm_wday);	// read schedule-time of ghpUnit

		RunSchduleControl(i, myTime);	//schdule
#if 0
		if (localDbg)
		{
			printf("myTime[%d] (%d) - %d - %d - %d ", 
				myTime,
				g_UnitData[i].pno, 
				g_UnitData[i].group, 
				g_UnitData[i].group_method, 
				g_UnitData[i].group_mode);

			for (j = 0; j < 5; j++)
			{
				printf("(%d - %d) ", 
					g_UnitData[i].startT[j], 
					g_UnitData[i].stopT[j]);
			}
			printf("\n");
		}
#endif
	}	
}


/* file : iface_ccms_mgr.c
 *
 * CCMS SERVER가 동작할 때, server의 접속상태를 표시하는 페이지를 생성한다. 
 * 또한 Control Web page를  생성해서 제어가 가능하도록 한다. 
 *
 *
 * User가 접속상태를 확인할 때 
 * |-------|                         |----------------|
 * | USER  | == Web(HTTP로 접근) ==> | ccms_info.html |
 * |-------|                         |----------------|
 *
 *
 * User가 제어할 때
 * 페이지 링크를 통해서 mini_httpd server로 페이지 이동을 시키면
 * mini_httpd server가 web-page의 이름을 분석해서 제어를 실행하고,
 * 1초~3초 사이의 Wait Time 후에 페이지를 갱신하도록 한다. 
 * |-------|                         |----------------|                        |-------------------|
 * | USER  | == Web(HTTP로 접근) ==> | ccms_ctrl.html |  == move link_page ==> | mini_httpd server |
 * |-------|                         |----------------|                        |-------------------|
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>										// use poll event

#include "TYPE.h"
#include "define.h"
#include "PORT.h"
#include "STRUCTURE.h"
#include "FUNCTION.h"

#include "iface_ccms_mgr.h"									// interface ccms


extern CCMS_T				*pCcms;							// from iface_ccms_client.c
extern CCMS_MODEM_T			*pCcmsModem;					// from iface_modem.c
extern CCMS_PTBL_LIST_T		*g_pPtbl;
extern unsigned char 		g_cNode[32];					// from main.c
extern float 				g_fExPtbl[MAX_NET32_NUMBER][MAX_POINT_NUMBER];	// from main.c

// '/duksan/FILE/WebCfg' file structure.
// 최대 2048개의 포인트가 가능하다. 
typedef struct {
	char table[4];
	char index[4];
	char rd_name[32];	
	char rd_pcm[4];	
	char rd_pno[4];	
	char wr_name[32];	
	char wr_pcm[4];	
	char wr_pno[4];		
} CCMS_DATA_INFO_T;
CCMS_DATA_INFO_T g_ccmsData[2048];
unsigned int g_nControlPointCount = 0;

float ccms_pre_ptbl[MAX_NET32_NUMBER][64];	// from main.c

// '/duksan/FILE/point_log,txt' file structure.
// 최대 2048개의 포인트가 가능하다. 
typedef struct {
	unsigned short 	wDate;
	unsigned short 	wTime;
	unsigned short 	wPcm;
	unsigned short 	wPno;
	float			fValue;
} CCMS_POINT_LOG_T;
CCMS_POINT_LOG_T g_ccmsPointLog[1024];
CCMS_POINT_LOG_T g_ccmsPrevPointLog[1024];
unsigned int g_nLogPointCount = 0;


void cfile_make_link(FILE *pFp) 
{
	char chPer = 0x25;
	char chChk = 0x22;
	
	fprintf(pFp, "<table width=100%c height=30 border=0 bgcolor=lightskyblue  cellpadding=0 cellspacing=0>", chPer);
	fprintf(pFp, "<tr bgcolor=lightskyblue >");

	fprintf(pFp, "<td width=4%c ALIGN=CENTER>",chPer);
	fprintf(pFp,	" <font color=%c#FFFFFF%c>|</font> ",chChk,chChk);	
	fprintf(pFp, "</td>");

	// Status
	fprintf(pFp, "<td width=20%c ALIGN=CENTER>", chPer);
	fprintf(pFp, "<a href=%cccms_info.html%c target=%c_top%c style=%ctext-decoration: none;color:black;%c/>",
		chChk, chChk, chChk, chChk, chChk, chChk);		
	fprintf(pFp, " STATUS ");
	fprintf(pFp, "</a>" );	
	fprintf(pFp, "</td>");

	fprintf(pFp, "<td width=4%c ALIGN=CENTER>",chPer);
	fprintf(pFp,	" <font color=%c#FFFFFF%c>|</font> ",chChk,chChk);	
	fprintf(pFp, "</td>");
	
	// Control
	fprintf(pFp, "<td width=20%c ALIGN=CENTER>",chPer);
	fprintf(pFp, "<a href=%cccms_ctrl.html%c target=%c_top%c style=%ctext-decoration: none;color:black;%c/>",
		chChk, chChk, chChk, chChk, chChk, chChk);			
	fprintf(pFp,	" CONTROL ");	
	fprintf(pFp, "</a>" );	
	fprintf(pFp, "</td>");

	fprintf(pFp, "<td width=4%c ALIGN=CENTER>",chPer);
	fprintf(pFp,	" <font color=%c#FFFFFF%c>|</font> ",chChk,chChk);	
	fprintf(pFp, "</td>");

	// Loger
	fprintf(pFp, "<td width=20%c ALIGN=CENTER>",chPer);
	fprintf(pFp, "<a href=%cccms_log.html%c target=%c_top%c style=%ctext-decoration: none;color:black;%c/>",
		chChk, chChk, chChk, chChk, chChk, chChk);			
	fprintf(pFp,	" LOG ");	
	fprintf(pFp, "</a>" );	
	fprintf(pFp, "</td>");

	fprintf(pFp, "<td width=4%c ALIGN=CENTER>",chPer);
	fprintf(pFp,	" <font color=%c#FFFFFF%c>|</font> ",chChk,chChk);	
	fprintf(pFp, "</td>");
		
	// Update
	fprintf(pFp, "<td width=20%c ALIGN=CENTER>",chPer);
	fprintf(pFp, "<a href=%cAutoUpload.exe%c target=%c_top%c style=%ctext-decoration: none;color:black;%c/>",
		chChk, chChk, chChk, chChk, chChk, chChk);			
	fprintf(pFp,	" UPDATE ");	
	fprintf(pFp, "</a>" );	
	fprintf(pFp, "</td>");

	fprintf(pFp, "<td width=4%c ALIGN=CENTER>",chPer);
	fprintf(pFp,	" <font color=%c#FFFFFF%c>|</font> ",chChk,chChk);	
	fprintf(pFp, "</td>");
	
	fprintf(pFp, "</tr>");
	fprintf(pFp, "</table>");
	
}


void cfile_read_control_point(void)
{
	int nCnt = 0;
	FILE *fpData = NULL;

	if((fpData = fopen("/duksan/FILE/WebCfg", "r")) == NULL) {
		if (fpData != NULL)  
			fclose(fpData);
		printf("[ERROR] WebCfg File Open with Option 'r'\n");		
	}	

	// search file.
	nCnt = 0;
	while( !feof(fpData) ) {
		fscanf(fpData, "%s %s %s %s %s %s %s %s\n", 
			(char *)&g_ccmsData[nCnt].table, 
			(char *)&g_ccmsData[nCnt].index, 
			(char *)&g_ccmsData[nCnt].rd_name, 
			(char *)&g_ccmsData[nCnt].rd_pcm, 
			(char *)&g_ccmsData[nCnt].rd_pno,
			(char *)&g_ccmsData[nCnt].wr_name, 
			(char *)&g_ccmsData[nCnt].wr_pcm, 
			(char *)&g_ccmsData[nCnt].wr_pno );	
		nCnt++;
	}	
	
	g_nControlPointCount = nCnt;
	printf("g_nControlPointCount = %d\n", g_nControlPointCount);
		
	if (fpData != NULL)  
		fclose(fpData);		
}

void cfile_makefile_control(void)
// ----------------------------------------------------------------------------
// MAKE COTROL FILE
// Description : It make file that contorl-page.
// Arguments   : none
// Returns     : none
{
	FILE *fp;
	char chPer = 0x25;
	char chChk = 0x22;
	int nCnt = 0;
	int nRdPcm = 0; 
	int nRdPno = 0; 
	int nWrPcm = 0; 
	int nWrPno = 0; 	
	time_t     tm_nd;
	struct tm *tm_ptr;

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	fp = fopen("/httpd/ccms_ctrl.html", "w");
	if( fp == NULL ) {
		return;		
	}

	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);

	fprintf(fp, "<head>");
	fprintf(fp, "<title>");
	fprintf(fp, "[CCMS]");
	fprintf(fp, "Client Control");
	fprintf(fp, "</title>");
	
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=utf-8%c /> ",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<meta http-equiv=%ccache-control%c content=%cno-cache%c />  ",
			chChk, chChk, chChk, chChk);	
			
	fprintf(fp, "<header manifest=%cduksan.manifest%c>",chChk, chChk);	
	
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");

	fprintf(fp, "<font face=%cArial Black%c>", chChk, chChk);
	fprintf(fp, "<h1><b>");
	fprintf(fp, "CCMS Client Control");
	fprintf(fp, "</b></h1>");

	//////////////////////////////////////////////////////////////////
	// Link
	//////////////////////////////////////////////////////////////////
	cfile_make_link(fp);
	
	fprintf(fp, "<br>");
		
	//fprintf(fp, "<b>Update :: 2010-11-10 Test Time</b>", 
	fprintf(fp, "<b>Update :: %d-%d-%d %d:%d</b>", 
		tm_ptr->tm_year+1900,
		tm_ptr->tm_mon + 1, 
		tm_ptr->tm_mday, 
		tm_ptr->tm_hour, 
		tm_ptr->tm_min);
	
	fprintf(fp, "<hr color =silver size=2>");
/*
	//////////////////////////////////////////////////////////////////
	// Index
	//////////////////////////////////////////////////////////////////
	fprintf(fp, "<table width=100%c height=30 border=1 bgcolor=deepskyblue cellpadding=0 cellspacing=0>", chPer);
	fprintf(fp, "<tr bgcolor=deepskyblue>");

	// index number
	fprintf(fp, "<td width=10%c ALIGN=CENTER>", chPer);
	fprintf(fp, " INDEX ");
	fprintf(fp, "</td>");

	// name
	fprintf(fp, "<td width=40%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" NAME ");	
	fprintf(fp, "</td>");
	
	// status
	fprintf(fp, "<td width=25%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" STATUS ");	
	fprintf(fp, "</td>");

	// control
	fprintf(fp, "<td width=25%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" CONTROL ");	
	fprintf(fp, "</td>");

	fprintf(fp, "</tr>");
	fprintf(fp, "</table>");
*/
	//////////////////////////////////////////////////////////////////
	// Control
	//////////////////////////////////////////////////////////////////
	fprintf(fp, "<table width=100%c height=50 border=1 cellpadding=0 cellspacing=0>", chPer);
	
	// search file.
	for ( nCnt = 0; nCnt < g_nControlPointCount; nCnt++) {
		nRdPcm = atoi(g_ccmsData[nCnt].rd_pcm); 
		nRdPno = atoi(g_ccmsData[nCnt].rd_pno); 
	 	nWrPcm = atoi(g_ccmsData[nCnt].wr_pcm); 
		nWrPno = atoi(g_ccmsData[nCnt].wr_pno); 
		
		fprintf(fp, "<tr>");
		// index number
		fprintf(fp, "<td width=10%c height=50 ALIGN=CENTER>", chPer);
		fprintf(fp, " %d ", nCnt + 1);
		fprintf(fp, "</td>");
	
		// name
		fprintf(fp, "<td width=40%c  height=50 ALIGN=CENTER>",chPer);
		fprintf(fp,	" %s ", g_ccmsData[nCnt].rd_name);	
		fprintf(fp, "</td>");
		
		if (strncmp((char *)&g_ccmsData[nCnt].wr_name , "light", 5) == 0) {
			// status
			fprintf(fp, "<td width=25%c  height=50 ALIGN=CENTER>",chPer);
			if ( g_fExPtbl[nRdPcm][nRdPno] > 0 ) {
				fprintf(fp, "<img src=%c/img/ctrl/Light_On.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);		
			}
			else {
				fprintf(fp, "<img src=%c/img/ctrl/Light_Off.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);					
			}
			fprintf(fp, "</td>");
		}
		else {
			// status
			fprintf(fp, "<td width=25%c  height=50 ALIGN=CENTER>",chPer);
			if ( g_fExPtbl[nRdPcm][nRdPno] > 0 ) {
				fprintf(fp, "<img src=%c/img/ctrl/Fan_On.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);		
			}
			else {
				fprintf(fp, "<img src=%c/img/ctrl/Fan_Off.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);					
			}
			fprintf(fp, "</td>");			
		}

	
		// control
		fprintf(fp, "<td width=25%c height=50 ALIGN=CENTER>",chPer);

		if ( g_fExPtbl[nWrPcm][nWrPno] > 0 ) {
			fprintf(fp, "<a href=%ccctrl_%02d%03d_0000.html%c target=%c_top%c style=%ctext-decoration: none;%c>",
				chChk, nWrPcm, nWrPno, chChk, chChk, chChk, chChk, chChk);		
			
			fprintf(fp, "<img src=%c/img/ctrl/On.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
				chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);		
				
			fprintf(fp, "</a>");				
		}
		else {
			fprintf(fp, "<a href=%ccctrl_%02d%03d_0001.html%c target=%c_top%c style=%ctext-decoration: none;%c>",
				chChk, nWrPcm, nWrPno, chChk, chChk, chChk, chChk, chChk);	
							
			fprintf(fp, "<img src=%c/img/ctrl/Off.gif%c alt=%c%c border=%c0%calign=%cabsbottom%c /></a>",
				chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);					
				
			fprintf(fp, "</a>");				
		}
	
		
		fprintf(fp, "</td>");
		fprintf(fp, "</tr>");
	}	
	

	fprintf(fp, "</table>");	
	
	fprintf(fp, "</table>");
	
	fprintf(fp, "</font>");
	fprintf(fp, "</body>");
	
	fclose(fp);
}



void cfile_makefile_status(CCMS_T *pTcp, CCMS_MODEM_T *pModem)
// ----------------------------------------------------------------------------
// MAKE INFORMATION FILE
// Description : It make file that ccms-server-information.
// Arguments   : pTcp		Is a pointer of CCMS_T structure.
//				 pModem		Is a pointer of CCMS_MODEM_T structure.
// Returns     : none
{
	FILE *fp;
	char chPer = 0x25;
	char chChk = 0x22;
	int nIndex = 0; 
	int nCcmsId = 0;
	int nModemId = 0;
	time_t     tm_nd;
	struct tm *tm_ptr;

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	fp = fopen("/httpd/ccms_info.html", "w");
	if( fp == NULL ) {
		return;		
	}

	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);

	fprintf(fp, "<head>");
	fprintf(fp, "<title>");
	fprintf(fp, "CCMS Information");
	fprintf(fp, "</title>");
	
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=utf-8%c /> ",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<meta http-equiv=%ccache-control%c content=%cno-cache%c />  ",
			chChk, chChk, chChk, chChk);	
			
	fprintf(fp, "<header manifest=%cduksan.manifest%c>",chChk, chChk);		
	
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");
	
	fprintf(fp, "<font face=%cArial Black%c>", chChk, chChk);
	fprintf(fp, "<h1><b>");
	fprintf(fp, "CCMS Client Information");
	fprintf(fp, "</b></h1>");

	//////////////////////////////////////////////////////////////////
	// Link
	//////////////////////////////////////////////////////////////////
	cfile_make_link(fp);
	
	fprintf(fp, "<br>");
		
	
	fprintf(fp, "<b>Update :: %d-%d-%d %d:%d</b>", 
		tm_ptr->tm_year+1900,
		tm_ptr->tm_mon + 1, 
		tm_ptr->tm_mday, 
		tm_ptr->tm_hour, 
		tm_ptr->tm_min);
			
			
	fprintf(fp, "<hr color =silver size=2>");
/*
	//////////////////////////////////////////////////////////////////
	// Index
	//////////////////////////////////////////////////////////////////
	fprintf(fp, "<table width=100%c height=30 border=1 bgcolor=deepskyblue cellpadding=0 cellspacing=0>", chPer);
	fprintf(fp, "<tr bgcolor=deepskyblue>");

	// Client index
	fprintf(fp, "<td width=30%c ALIGN=CENTER>", chPer);
	fprintf(fp, " CLIENT ");
	fprintf(fp, "</td>");

	// onoff
	fprintf(fp, "<td width=10%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" ID ");	
	fprintf(fp, "</td>");

	
	// schedule check
	fprintf(fp, "<td width=60%c ALIGN=CENTER>",chPer);
	fprintf(fp,	" CONNECTION TYPE ");	
	fprintf(fp, "</td>");

	fprintf(fp, "</tr>");
	fprintf(fp, "</table>");
*/
	fprintf(fp, "<TABLE width=100%c height=35 border=1 cellpadding=0 cellspacing=0>",chPer);
	for ( nIndex = 0; nIndex < 32; nIndex++) {
		if ( g_cNode[nIndex] != 0 ) {
			fprintf(fp, "<tr>");
			// index
			fprintf(fp, "<td width=30%c height=35 ALIGN=CENTER>",chPer);
			if ( nIndex == 0 )
				fprintf( fp, "<b>Master</b>");
			else
				fprintf( fp, "Client [%02d]",	nIndex);
			fprintf(fp, "</td>");
			// id
			fprintf(fp, "<td width=10%c height=35 ALIGN=CENTER>",chPer);
			fprintf( fp, "Id [%d]",	nIndex);
			fprintf(fp, "</td>");
			//status
			fprintf(fp, "<td width=60%c height=35 ALIGN=CENTER>",chPer);
			
			fprintf(fp, "<table width=60%c height=25 border=0 cellpadding=0 cellspacing=0 ALIGN=CENTER>",chPer);
			fprintf(fp, "<tr>");

			fprintf(fp, "<td width=30%c  ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=red CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=yellow ALIGN=CENTER>");
			fprintf(fp, "<b>NET32</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);			
			fprintf(fp, "</td>");
			
			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>TCP</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);		
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>MODEM</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");
									
			fprintf(fp, "</tr>");			
			
			fprintf(fp, "</table>");

			fprintf(fp, "</td>");
			fprintf(fp, "</tr>");			
			continue;
		}
	}

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( pTcp->nClientStatus[nIndex] != 0 ) {
						
			nCcmsId = pTcp->nClientId[nIndex];	
			
			if ( g_cNode[nCcmsId] > 0 )
				continue;			
						
			fprintf(fp, "<tr>");
			// index
			fprintf(fp, "<td width=30%c height=35 ALIGN=CENTER>",chPer); 
			fprintf( fp, "Client [%02d]",	nCcmsId);
			fprintf(fp, "</td>");
			// id
			fprintf(fp, "<td width=10%c height=35 ALIGN=CENTER>",chPer);
			fprintf( fp, "Id [%d]",	nCcmsId);
			fprintf(fp, "</td>");
			//status
			fprintf(fp, "<td width=60%c height=35 ALIGN=CENTER>",chPer);

			fprintf(fp, "<table width=60%c height=25 border=0 cellpadding=0 cellspacing=0 ALIGN=CENTER> ",chPer);
			fprintf(fp, "<tr>");

			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);	
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>NET32</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);			
			fprintf(fp, "</td>");
			
			fprintf(fp, "<td width=30%c  ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=red CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=yellow ALIGN=CENTER>");
			fprintf(fp, "<b>TCP</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);		
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>MODEM</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");
									
			fprintf(fp, "</tr>");			
			fprintf(fp, "</table>");

			fprintf(fp, "</td>");
			fprintf(fp, "</tr>");			
			continue;					
		}	
	}

	for ( nIndex = 0; nIndex < CCMS_MAX_CLIENT; nIndex++) {
		if ( pModem->nClientStatus[nIndex] != 0 ) {
						
			nModemId = pModem->nClientId[nIndex];	
			
			if ( nModemId == 0 )
				continue;						
			
			if ( g_cNode[nModemId] > 0 )
				continue;		
						
			fprintf(fp, "<tr>");
			// index
			fprintf(fp, "<td width=30%c height=35 ALIGN=CENTER>",chPer); 
			fprintf( fp, "Client [%02d]", nModemId);
			fprintf(fp, "</td>");
			// id
			fprintf(fp, "<td width=10%c height=35 ALIGN=CENTER>",chPer);
			fprintf( fp, "Id [%d]",	nModemId);
			fprintf(fp, "</td>");
			//status
			fprintf(fp, "<td width=60%c height=35 ALIGN=CENTER>",chPer);

			fprintf(fp, "<table width=60%c height=25 border=0 cellpadding=0 cellspacing=0 ALIGN=CENTER> ",chPer);
			fprintf(fp, "<tr>");

			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);	
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>NET32</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);			
			fprintf(fp, "</td>");
			
			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);	
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>TCP</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);		
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=30%c  ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=red CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=yellow ALIGN=CENTER>");
			fprintf(fp, "<b>MODEM</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");
									
			fprintf(fp, "</tr>");			
			fprintf(fp, "</table>");

			fprintf(fp, "</td>");
			fprintf(fp, "</tr>");			
			continue;					
		}	
	}	
		
	fprintf(fp, "</table>");	
	fprintf(fp, "</font>");
	fprintf(fp, "</body>");
	fclose(fp);	
}


void cfile_makefile_local_status(void)
// ----------------------------------------------------------------------------
// MAKE INFORMATION FILE
// Description : It make file that ccms-server-information.
// Arguments   : pTcp		Is a pointer of CCMS_T structure.
//				 pModem		Is a pointer of CCMS_MODEM_T structure.
// Returns     : none
{
	FILE *fp;
	char chPer = 0x25;
	char chChk = 0x22;
	int nIndex = 0; 
	time_t     tm_nd;
	struct tm *tm_ptr;

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	fp = fopen("/httpd/ccms_info.html", "w");
	if( fp == NULL ) {
		return;		
	}

	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);

	fprintf(fp, "<head>");
	fprintf(fp, "<title>");
	fprintf(fp, "CCMS Information");
	fprintf(fp, "</title>");
	
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=utf-8%c /> ",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<meta http-equiv=%ccache-control%c content=%cno-cache%c />  ",
			chChk, chChk, chChk, chChk);	
			
	fprintf(fp, "<header manifest=%cduksan.manifest%c>",chChk, chChk);		
	
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");
	
	fprintf(fp, "<font face=%cArial Black%c>", chChk, chChk);
	fprintf(fp, "<h1><b>");
	fprintf(fp, "CCMS Client Information");
	fprintf(fp, "</b></h1>");

	//////////////////////////////////////////////////////////////////
	// Link
	//////////////////////////////////////////////////////////////////
	cfile_make_link(fp);
	
	fprintf(fp, "<br>");
		
	
	fprintf(fp, "<b>Update :: %d-%d-%d %d:%d</b>", 
		tm_ptr->tm_year+1900,
		tm_ptr->tm_mon + 1, 
		tm_ptr->tm_mday, 
		tm_ptr->tm_hour, 
		tm_ptr->tm_min);
			
			
	fprintf(fp, "<hr color =silver size=2>");

	fprintf(fp, "<TABLE width=100%c height=35 border=1 cellpadding=0 cellspacing=0>",chPer);
	for ( nIndex = 0; nIndex < 32; nIndex++) {
		if ( g_cNode[nIndex] != 0 ) {
			fprintf(fp, "<tr>");
			// index
			fprintf(fp, "<td width=30%c height=35 ALIGN=CENTER>",chPer);
			if ( nIndex == 0 )
				fprintf( fp, "<b>Master</b>");
			else
				fprintf( fp, "Client [%02d]",	nIndex);
			fprintf(fp, "</td>");
			// id
			fprintf(fp, "<td width=10%c height=35 ALIGN=CENTER>",chPer);
			fprintf( fp, "Id [%d]",	nIndex);
			fprintf(fp, "</td>");
			//status
			fprintf(fp, "<td width=60%c height=35 ALIGN=CENTER>",chPer);
			
			fprintf(fp, "<table width=60%c height=25 border=0 cellpadding=0 cellspacing=0 ALIGN=CENTER>",chPer);
			fprintf(fp, "<tr>");

			fprintf(fp, "<td width=30%c  ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=red CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=yellow ALIGN=CENTER>");
			fprintf(fp, "<b>NET32</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);			
			fprintf(fp, "</td>");
			
			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>TCP</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=5%c>",chPer);		
			fprintf(fp, "</td>");

			fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);			
			fprintf(fp, "<TABLE BGCOLOR=gray  CELLPADDING==3 CELLSPACING=0 WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td>");
			fprintf(fp, "<TABLE WIDTH=100%c>", chPer);
			fprintf(fp, "<tr><td BGCOLOR=gainsboro  ALIGN=CENTER>");
			fprintf(fp, "<b>MODEM</b>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td></tr>");
			fprintf(fp, "</table>");
			fprintf(fp, "</td>");
									
			fprintf(fp, "</tr>");			
			
			fprintf(fp, "</table>");

			fprintf(fp, "</td>");
			fprintf(fp, "</tr>");			
			continue;
		}
	}
		
	fprintf(fp, "</table>");	
	fprintf(fp, "</font>");
	fprintf(fp, "</body>");
	fclose(fp);	
}


void cfile_makefile_log(void)
// ----------------------------------------------------------------------------
// MAKE LOG FILE
// Description : It make file that ccms-log-information
// Arguments   : none
// Returns     : none
{
	FILE *fp;
	FILE *fpData = NULL;
	char chPer = 0x25;
	char chChk = 0x22;
	time_t     tm_nd;
	struct tm *tm_ptr;
	//char chDate[32];
 	//char chPcm[4];
	//char chPno[4];
	//char chValue[32];
	int nCnt = 0;
				

	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);

	fp = fopen("/httpd/ccms_log.html", "w");
	if( fp == NULL ) {
		return;		
	}

	if((fpData = fopen("/duksan/FILE/point_log.txt", "w+")) == NULL) {
		if (fpData != NULL)  
			fclose(fpData);
		printf("[ERROR] point_log.txt File Open with Option 'w+'\n");		
	}		


	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);

	fprintf(fp, "<head>");
	fprintf(fp, "<title>");
	fprintf(fp, "CCMS Loger");
	fprintf(fp, "</title>");
	
	fprintf(fp, "<meta http-equiv=%cContent-Type%c content=%ctext/html; charset=utf-8%c /> ",
			chChk, chChk, chChk, chChk);

	fprintf(fp, "<meta http-equiv=%ccache-control%c content=%cno-cache%c />  ",
			chChk, chChk, chChk, chChk);	
			
	fprintf(fp, "<header manifest=%cduksan.manifest%c>",chChk, chChk);		
	
	fprintf(fp, "</head>");
	fprintf(fp, "<body>");
	
	fprintf(fp, "<font face=%cArial Black%c>", chChk, chChk);
	fprintf(fp, "<h1><b>");
	fprintf(fp, "CCMS Loger");
	fprintf(fp, "</b></h1>");

	//////////////////////////////////////////////////////////////////
	// Link
	//////////////////////////////////////////////////////////////////
	cfile_make_link(fp);
	
	fprintf(fp, "<br>");
		
	
	fprintf(fp, "<b>Update :: %d-%d-%d %d:%d</b>", 
		tm_ptr->tm_year+1900,
		tm_ptr->tm_mon + 1, 
		tm_ptr->tm_mday, 
		tm_ptr->tm_hour, 
		tm_ptr->tm_min);
			
			
	fprintf(fp, "<hr color =silver size=2>");

	
	//////////////////////////////////////////////////////////////////
	// Log Page
	//////////////////////////////////////////////////////////////////
	fprintf(fp, "<table width=100%c height=50 border=1 cellpadding=0 cellspacing=0>", chPer);
	
	// search file.
	for ( nCnt = 0; nCnt < g_nLogPointCount; nCnt++ ) {
		fprintf(fp, "<tr>");
		// index number
		fprintf(fp, "<td width=10%c height=50 ALIGN=CENTER>", chPer);
		fprintf(fp, " %d ", nCnt + 1);
		fprintf(fp, "</td>");
	
		// date
		fprintf(fp, "<td width=30%c  height=50 ALIGN=CENTER>",chPer);
		fprintf(fp,	" %d/%d ", g_ccmsPointLog[nCnt].wDate/100, g_ccmsPointLog[nCnt].wDate%100);	
		fprintf(fp,	" %d:%d ", g_ccmsPointLog[nCnt].wTime/100, g_ccmsPointLog[nCnt].wTime%100);	
		fprintf(fp, "</td>");
		
		fprintf(fpData,	" %d/%d ", g_ccmsPointLog[nCnt].wDate/100, g_ccmsPointLog[nCnt].wDate%100);	
		fprintf(fpData,	" %d:%d ", g_ccmsPointLog[nCnt].wTime/100, g_ccmsPointLog[nCnt].wTime%100);			

		// Address
		fprintf(fp, "<td width=20%c  height=50 ALIGN=CENTER>",chPer);
		fprintf(fp,	" %d - %d ", g_ccmsPointLog[nCnt].wPcm, g_ccmsPointLog[nCnt].wPno);	
		fprintf(fp, "</td>");
		
		fprintf(fpData,	" %d %d ", g_ccmsPointLog[nCnt].wPcm, g_ccmsPointLog[nCnt].wPno);	

		// value
		fprintf(fp, "<td width=40%c  height=50 ALIGN=CENTER>",chPer);
		fprintf(fp,	" %0.2f ", g_ccmsPointLog[nCnt].fValue);	
		fprintf(fp, "</td>");
				
		fprintf(fpData,	" %0.2f \r\n", g_ccmsPointLog[nCnt].fValue);					
		
		fprintf(fp, "</tr>");
		
		//nCnt++;
	}	
	fprintf(fp, "</table>");	
	

	fprintf(fp, "</font>");
	fprintf(fp, "</body>");
	fclose(fp);	
	fclose(fpData);	
}


void ccms_mgr_sleep(int sec, int msec) 
// ----------------------------------------------------------------------------
// WAIT TIMER
// Description : use select function for timer.
// Arguments   : sec		Is a second value.
//				 msec		Is a micro-second value. 
// Returns     : none
{
    fd_set reads, temps;
    struct timeval tv;
    
    FD_ZERO(&reads);
    FD_SET(0, &reads);
    
    tv.tv_sec = sec;
    tv.tv_usec = 1000 * msec;
    temps = reads;
    
    select( 0, &temps, 0, 0, &tv );
    
	return;
}


int fileCopy(const char* src, const char* dst) 
{
	FILE *in, *out;
	char* buf;
	size_t len;
	
	//if (!strcmpi(src, dst)) return 4; // 원본과 사본 파일이 동일하면 에러
	
	if ((in  = fopen(src, "rb")) == NULL) return 1; // 원본 파일 열기
	if ((out = fopen(dst, "wb")) == NULL) { fclose(in); return 2; } // 대상 파일 만들기
	
	if ((buf = (char *) malloc(1024)) == NULL) { fclose(in); fclose(out); return 10; } // 버퍼 메모리 할당
	
	while ( (len = fread(buf, sizeof(char), sizeof(buf), in)) != '\0' )
	if (fwrite(buf, sizeof(char), len, out) == 0) {
	  fclose(in); fclose(out);
	  free(buf);
	  unlink(dst); // 에러난 파일 지우고 종료
	  return 3;
	}
	
	fclose(in); fclose(out);
	free(buf); // 메모리 할당 해제
	
	return 0;
}


void cfile_log(point_info *pPnt)
// ----------------------------------------------------------------------------
// MAKE LOG FILE
// Description : It make log file
// Arguments   : pPnt			Is a pointer point-table
// Returns     : none
{
	FILE *fp = NULL;
	FILE *fp_bak = NULL;
	//FILE *fp_log = NULL;
	time_t     tm_nd;
	struct tm *tm_ptr;
	int filesize = 0;
	//int size = 0;
	//unsigned char buff[1028];
	char chDate[32];
 	char chPcm[4];
	char chPno[4];
	char chValue[32];
	int nCount = 0;
	int e = 0;
	
	//printf("%s() \n", __FUNCTION__);	
	
	
	fp = fopen("/duksan/FILE/point_log.txt", "w+");
	if( fp == NULL ) {
		fprintf( stdout, "PointLog 1 : point_log.txt file open error\n" );
		fflush( stdout );
		ccms_mgr_sleep(5, 0);
		system("reboot 1000");
		return;		
	}
	
	fp_bak = fopen("/duksan/FILE/point_log.bak", "a+");
	if( fp_bak == NULL ) {
		fprintf( stdout, "PointLog 2 : point_log.bak file open error\n" );
		fflush( stdout );
		ccms_mgr_sleep(5, 0);
		system("reboot 1000");
		return;		
	}	

	fseek(fp_bak, 0L, SEEK_END); 
	filesize = ftell( fp_bak );
	
	//printf("bak filesize = %d\n", filesize);

	fseek(fp, 0L, SEEK_SET); 
	fseek(fp_bak, 0L, SEEK_SET); 
	
	// 가장 최근의 Data를 먼저 넣는다.	
	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);
	fprintf(fp,	"%d/%d_%d:%d %d %d %0.2f\n", 
			tm_ptr->tm_mon + 1, 
			tm_ptr->tm_mday, 
			tm_ptr->tm_hour, 
			tm_ptr->tm_min,
			pPnt->pcm,
			pPnt->pno,
			pPnt->value	);	
	
	nCount = 0;
	if (filesize != 0) {
		while( !feof(fp_bak) ) {
			fscanf(fp_bak, "%s %s %s %s\n", 
				chDate, 
				chPcm, 
				chPno, 
				chValue );	

			fprintf(fp,	"%s %s %s %s\n", 
					chDate,
					chPcm,
					chPno,
					chValue);
					
			// 최대 100개의 Data만 저장한다. 
			if ( nCount++ > 200 )
				break;
		}
	}

	fclose(fp);	
	fclose(fp_bak);	
	
	e = fileCopy("/duksan/FILE/point_log.txt", "/duksan/FILE/point_log.bak");

	/*	
	if (e == 0)
		printf("Copy Complete.\n");
	else {
		switch (e) {
		case  1 : fputs("can't open point_log.txt\n", stderr); break;
		case  2 : fputs("can't open point_log.bak\n", stderr); break;
		case  3 : fputs("write error\n", stderr); break;
		case 10 : fputs("memory error\n", stderr); break;
		default : fputs("unknown error\n", stderr); break;
		}
	}
	*/
}


void cfile_put_log(point_info *pPoint)
{
	unsigned int nCopySize = 0;
	time_t     tm_nd;
	struct tm *tm_ptr;

	// 가장 최근의 Data를 먼저 넣는다.	
	time(&tm_nd);
	tm_ptr = localtime(&tm_nd);
	
	nCopySize = g_nLogPointCount * sizeof(CCMS_POINT_LOG_T);
	
	if ( nCopySize != 0 ) {
		//memcpy(&g_ccmsPointLog[1] , &g_ccmsPointLog[0], nCopySize);	
		memcpy(&g_ccmsPrevPointLog[0] , &g_ccmsPointLog[0], nCopySize);	
		memcpy(&g_ccmsPointLog[1] , &g_ccmsPrevPointLog[0], nCopySize);	
	}
	
	//printf("nCopySize = %d\n", nCopySize);
	
	//printf(">> nPcm = %d, nPno = %d, value = %0.2f\n", pPoint->pcm, pPoint->pno, pPoint->value);	
	
	g_ccmsPointLog[0].wDate = ((tm_ptr->tm_mon + 1) * 100) + tm_ptr->tm_mday;
	g_ccmsPointLog[0].wTime = (tm_ptr->tm_hour * 100) + tm_ptr->tm_min;
	g_ccmsPointLog[0].wPcm = pPoint->pcm;
	g_ccmsPointLog[0].wPno = pPoint->pno;
	g_ccmsPointLog[0].fValue = pPoint->value;
	
	if ( g_nLogPointCount < 100 )
		g_nLogPointCount++;	
		
	//printf("g_nLogPointCount = %d\n", g_nLogPointCount);
}



int cfile_value_check(void) 
// ----------------------------------------------------------------------------
// CHECK POINT TABLE
// Description : It check point-table to find changing value.
// Arguments   : none
// Returns     : nChangeFlag		Is a flag that changing value
{
	int nPcm = 0;
	int nPno = 0;
	int nChangeFlag = 0;
	float fDiffValue = 0;
	point_info point;
	
	for ( nPcm = 0; nPcm < MAX_NET32_NUMBER; nPcm++) {
		for ( nPno = 0; nPno < 64; nPno++) {
			if ( g_fExPtbl[nPcm][nPno] != ccms_pre_ptbl[nPcm][nPno] ) {
				
				if ( g_fExPtbl[nPcm][nPno] > ccms_pre_ptbl[nPcm][nPno] ) {
					fDiffValue = g_fExPtbl[nPcm][nPno] - ccms_pre_ptbl[nPcm][nPno];
				}
				else {
					fDiffValue = ccms_pre_ptbl[nPcm][nPno] - g_fExPtbl[nPcm][nPno];
				}
				
				// 값의 차이가 1이상 나는 포인트만 처리한다. 
				if ( fDiffValue >= 1 ) { 
					nChangeFlag++;
					//printf("\n\n Prevalue = %0.2f value = %0.2f\n", ccms_pre_ptbl[nPcm][nPno], g_fExPtbl[nPcm][nPno]);
					
					ccms_pre_ptbl[nPcm][nPno] = g_fExPtbl[nPcm][nPno];
					point.pcm = nPcm;
					point.pno = nPno;
					point.value = ccms_pre_ptbl[nPcm][nPno];
					cfile_put_log(&point);
					
					//cfile_log(&point);
					//printf("CCMS Mgr Write nPcm = %d, nPno = %d, value = %0.2f\n", nPcm, nPno, ccms_pre_ptbl[nPcm][nPno]);
				}
			}
		}		
	}
	
	return nChangeFlag;
}


void cfile_restore_value (void ) 
{
	int nPcm = 0;
	int nPno = 0;
		
	for ( nPcm = 0; nPcm < MAX_NET32_NUMBER; nPcm++) {
		for ( nPno = 0; nPno < 64; nPno++) {
			ccms_pre_ptbl[nPcm][nPno] = g_fExPtbl[nPcm][nPno];
		}	
	}
}


void *ccms_mgr_main(void* arg)
// ----------------------------------------------------------------------------
// CCMS FILE MANAGER
// Description : It make file(html) for ccms-server.
// Arguments   : arg				Thread Argument
{
	//time_t		tm_nd;
	//struct 		tm *tm_ptr;
	//int 		nMin = 0;
	//int 		nSec = 0;

	// Ignore broken_pipe signal
	signal(SIGPIPE, SIG_IGN);				
	
	// '/duksan/FILE/WebCfg' 파일의 DATA 구조체를 초기화 한다. 	
	memset(&g_ccmsData, 0x00, sizeof(g_ccmsData));
	
	// '/duksan/FILE/point_log.txt' 파일의 DATA 구조체를 초기화 한다. 	
	memset(&g_ccmsPointLog, 0x00, sizeof(g_ccmsPointLog));
	g_nLogPointCount = 0;
	
	// point-table의 값을 비교하기 위한 이전값을 저장하는
	// 배열을 초기화한다. 
	memset(&ccms_pre_ptbl, 0x00, sizeof(ccms_pre_ptbl));
		
	ccms_mgr_sleep(3,0);		
	cfile_restore_value();
	cfile_value_check();
	//cfile_makefile_log();		
	
	cfile_read_control_point();
	
	while (1) {
		
		ccms_mgr_sleep(1,0);

		// 매 초마다 File을 생성한다. 
		cfile_makefile_control();
		if ( pCcms != NULL || pCcmsModem != NULL ) {
			cfile_makefile_status(pCcms, pCcmsModem);			
		}
		else {
			cfile_makefile_local_status();
		}
			
		if ( cfile_value_check() > 0 ) {
			cfile_makefile_log();
		}


		// 현재 접속된 상태를 확인하는 Debug Code.
		/*
		printf("NODE = ");
		for (i = 0; i < CCMS_MAX_CLIENT; i++ ) {
			if ( g_cNode[i] != 0 ) {
				printf("%d, ", i);
			}
		}
		printf("\n");
		
		printf("TCP = ");
		for (i = 0; i < CCMS_MAX_CLIENT; i++ ) {
			if ( pCcms->nClientStatus[i]  != 0 ) {
				if ( pCcms->nClientId[i] == 0 )
					continue;
				printf("%d, ", pCcms->nClientId[i]);
			}
		}
		printf("\n");		
		
		printf("MODEM = ");
		for (i = 0; i < CCMS_MAX_CLIENT; i++ ) {
			if ( pCcmsModem->nClientStatus[i]  != 0 ) {
				if ( pCcmsModem->nClientId[i] == 0 )
					continue;				
				printf("%d, ", pCcmsModem->nClientId[i]);
			}
		}
		printf("\n");	
		*/	
	} // while(1)
	syslog_record(SYSLOG_DESTROY_CCMS_MANAGER);	
}




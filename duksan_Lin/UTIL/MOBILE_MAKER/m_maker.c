
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
#include <sys/poll.h>		// use poll event

#define	MAX_NET32_NUMBER		32
#define	MAX_POINT_NUMBER		256

typedef struct {
	char 	chName[32];
	int 	nPcm;
	int 	nPno;
} _MOBILE_IFACE_DATA_T;

_MOBILE_IFACE_DATA_T  g_data[32];

float point_table[MAX_NET32_NUMBER][MAX_POINT_NUMBER];

void cmd_make_iface_file(void)
{
	FILE 	*fp;
	char 	chPer = 0x25;		// %
	char 	chChk = 0x22;		// "
	char 	chCr = 0x0d;		// 엔터
	int 	nIndex = 0;
	char 	chFileName[32];
	int 	nPcm = 0;
	int 	nPno = 0;

	printf("%s() \n", __FUNCTION__);
	
	memset(chFileName, 0, sizeof(chFileName));
	sprintf(chFileName, "/httpd/m_iface.html");

	fp = fopen(chFileName, "w");
	if( fp == NULL ) {
		return;		
	}
	
	// Mobiel setting
	fprintf(fp, "<!DOCTYPE HTML PUBLIC %c-//WAPFORUM//DTD XHTML Mobile 1.2//EN%c %chttp://www.wapforum.org/DTD/xhtml-mobile12.dtd%c>", 
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<html xmlns=%chttp://www.w3.org/1999/xhtml%c lang=%cko%c xml:lang=%cko%c>", 
			chChk, chChk, chChk, chChk, chChk, chChk);


	fprintf(fp, "%c", chCr);
	fprintf(fp, "<html>");
	fprintf(fp, "%c", chCr);

	fprintf(fp, "<header manifest=%cm_iface.manifest%c>", chChk, chChk);
	fprintf(fp, "%c", chCr);

	// 항상 서버로부터 갱신되도록 하는 코드
	fprintf(fp, "<meta http-equiv=%cExpires%c content=%c0%c/> ",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<meta http-equiv=%cPragma%c content=%cno-cache%c/>",
			chChk, chChk, chChk, chChk);
	fprintf(fp, "%c", chCr);


	fprintf(fp, "</header>");
	fprintf(fp, "%c", chCr);
	////////////////////////////////////////////  body start
	fprintf(fp, "<body >");
	fprintf(fp, "%c", chCr);

	fprintf(fp, "<h1><b>");
	fprintf(fp, "Mobile Interface" );
	fprintf(fp, "</b></h1>");

	// 중간 라인
	fprintf(fp, "<hr color =silver size=3>");
	fprintf(fp, "%c", chCr);

	// Table 시작
	fprintf(fp, "<TABLE width=100%c height=30  height=30 border=1 cellpadding=0 cellspacing=0>",chPer);
	fprintf(fp, "%c", chCr);
	
	// name
	fprintf(fp, "<b>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<tr>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "<td bgcolor=skyblue width=30%c height=40 ALIGN=CENTER>", chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp, " 이름 ");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);

	// status
	fprintf(fp, "<td bgcolor=skyblue width=30%c ALIGN=CENTER>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp,	" 상태 ");	
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);

	// control Check
	fprintf(fp, "<td bgcolor=skyblue width=40%c ALIGN=CENTER>",chPer);
	fprintf(fp, "%c", chCr);
	fprintf(fp,	" 제어값 ");
	fprintf(fp, "%c", chCr);	
	fprintf(fp, "</td>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</tr>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</b>");
	fprintf(fp, "%c", chCr);

	for ( nIndex = 0; nIndex < 5; nIndex++) {
		fprintf(fp, "<tr>");
		fprintf(fp, "%c", chCr);
		// name
		fprintf(fp, "<td width=30%c ALIGN=CENTER>", chPer);
		fprintf(fp, "%c", chCr);
		fprintf(fp, " %s", g_data[nIndex].chName );
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</td>");
		fprintf(fp, "%c", chCr);

		// value
		nPcm = g_data[nIndex].nPcm;
		nPno = g_data[nIndex].nPno;
		fprintf(fp, "<td width=30%c ALIGN=CENTER>",chPer);
		fprintf(fp, "%c", chCr);
		if ( point_table[nPcm][nPno] > 0 ) {
			fprintf(fp,	"<img src=%cimg/bt_iface_on.jpg%c height=20 alt=%c%c border=%c0%c align=%cabsbottom%c />", 
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
			fprintf(fp, "%c", chCr);
		}
		else {
			fprintf(fp,	"<img src=%cimg/bt_iface_off.jpg%c height=20 alt=%c%c border=%c0%c align=%cabsbottom%c />", 
					chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
			fprintf(fp, "%c", chCr);
		}
		fprintf(fp, "</td>");
		fprintf(fp, "%c", chCr);

		// control
		fprintf(fp, "<td width=60%c ALIGN=CENTER>",chPer);
		fprintf(fp, "%c", chCr);

		fprintf(fp, "<form action=%cm_wait.html%c method=%cpost%c name=%cst_form%c id=%cst_form%c>", 
				chChk, chChk, chChk, chChk, chChk, chChk, chChk, chChk);
		fprintf(fp, "%c", chCr);

		fprintf(fp, "<SELECT NAME=%cpCtrlVal_%d_%d%c onchange=%cthis.form.submit()%c> ",
				chChk, g_data[nIndex].nPcm, g_data[nIndex].nPno, chChk, chChk, chChk);
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<OPTION > 선택 ");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<OPTION value=1> ON ");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "<OPTION value=0> OFF ");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</SELECT>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</form>");
		fprintf(fp, "%c", chCr);

		fprintf(fp, "</td>");
		fprintf(fp, "%c", chCr);
		fprintf(fp, "</tr>");
		fprintf(fp, "%c", chCr);
	}
	
	fprintf(fp, "</table>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</body>");
	fprintf(fp, "%c", chCr);
	fprintf(fp, "</html>");
	////////////////////////////////////////////  body start

	fclose(fp);
}


void point_file_check(void) 
{
	FILE *pFp;
	int nPcm = 0;
	int nPno = 0;
	char chFileName[32];
	float fValue;

	memset(chFileName, 0, sizeof(chFileName));
	strncpy(chFileName, "/httpd/data.dat\0", sizeof(chFileName));
	
	//if there's no file named "data.dat", make one.
	if((pFp = fopen(chFileName, "r")) == NULL) {
		return;
	}

	fseek( pFp, 0, SEEK_SET);

	// call back value.		
	for ( nPcm = 0; nPcm < MAX_NET32_NUMBER;  nPcm++ ) {
		for ( nPno = 0; nPno < MAX_POINT_NUMBER;  nPno++ ) {
			// call back value
			if ( fread(&fValue, sizeof(float), 1, pFp) == 1 ) 
				point_table[nPcm][nPno] = fValue;
		}
	}

	fclose(pFp);
}

/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	memset(&g_data, 0, sizeof(g_data));

	memcpy(g_data[0].chName, "관리부", sizeof("연구소 1"));
	g_data[0].nPcm = 2;
	g_data[0].nPno = 12;

	memcpy(g_data[1].chName, "공사부", sizeof("연구소 2"));
	g_data[1].nPcm = 2;
	g_data[1].nPno = 5;

	memcpy(g_data[2].chName, "영업부", sizeof("연구소 2"));
	g_data[2].nPcm = 2;
	g_data[2].nPno = 15;

	memcpy(g_data[3].chName, "연구소 1", sizeof("연구소 2"));
	g_data[3].nPcm = 2;
	g_data[3].nPno = 21;

	memcpy(g_data[4].chName, "연구소 2", sizeof("연구소 2"));
	g_data[4].nPcm = 2;
	g_data[4].nPno = 23;

	point_file_check();
	cmd_make_iface_file();

	exit(1);
}




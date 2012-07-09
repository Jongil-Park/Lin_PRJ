///******************************************************************/
// file : ghp_app.h
// date : 2010.01.18.
// author : jong2ry
// job :
//		jong2ry. 2010-01-18 3:35 PM
///******************************************************************/
#ifndef		IFACE_GHP_H_
#define		IFACE_GHP_H_

/** define ************************************s*********************/
// SDDC_ID Definition
#define SDDC_AC_DONG              		1       // AC_Dong ID 
#define SDDC_B_DONG               		3       // B_Dong ID
#define SDDC_D_DONG               		2       // D_Dong ID
#define SDDC_KISUKSA              		4       // Kisuksa ID
#define SDDC_E_DONG               		5       // E_Dong ID
#define SDDC_F_DONG               		6       // F_Dong ID
#define SDDC_G_DONG               		7       // G_Dong ID

// Max Point Count Definition
#define CNT_A_MAX_POINT                 108     // A Dong Indoor GHP Count  // 3개가 더 추가됨.
#define CNT_B_MAX_POINT                 239     // B Dong Indoor GHP Count
#define CNT_C_MAX_POINT                 125     // C Dong Indoor GHP Count
#define CNT_D_MAX_POINT                 192     // D Dong Indoor GHP Count
#define CNT_E_MAX_POINT                 212     // E Dong Indoor GHP Count
#define CNT_F_MAX_POINT                 204     // F Dong Indoor GHP Count
#define CNT_G_MAX_POINT                 97      // G Dong Indoor GHP Count
#define CNT_D_KISUKSA_MAX_POINT			129     // Duruam Kisuksa Indoor GHP Count

#define GHP_UNIT_MAX					256		// GHP Maximun count.

#define ST_LOOP_HANDLER					0
#define ST_GET_POLLING					1
#define ST_CHK_USER_CONTROL				2
#define ST_GET_USER_CONTROL				3
#define ST_GET_GHP_UNIT					4
#define ST_CHK_CTRL_PTBL				5
#define ST_SET_GHP_UNIT					6
#define ST_STARTUP						7

#define SUCCESS							1
#define ERROR							-1
#define FAIL							-1

#define POLL_OK							1
#define POLL_EMPTY_DATA					2
#define POLL_REQUIRE_STARTUP			-1
#define POLL_NG							-1
#define POLL_ERROR						-2
#define SEL_OK							1
#define SEL_RETRY						-1
#define SEL_NG							-1

#define TYPE_POLL						1
#define TYPE_SEL						2

#define USER_CONTROL_ONOFF				1
#define USER_CONTROL_MODE				2
#define USER_CONTROL_SETTEMP			3
#define USER_CONTROL_SPEED				4
#define USER_CONTROL_DIRECTION			5

//#define MAX_QUEUE_SIZE					1024
//#define QUEUE_FULL						2
//#define QUEUE_EMPTY						3

#define START_HEX_CODE_ADDR				1
#define BCG_BIT_CNT						7
#define BCG_MASK                    	0x01

#define INIT_PROCESS				1
#define SELECT_PROCESS				2
#define CONNECTION_REQUESTED		3
#define HANDLE_COMMAND				4

#define SUCCESS						1
#define FAIL						-1

#define MAX_BUF_LENGTH				4906
#define MESSAGE_LENGTH				9812

//#define SERVER_PORT					9000

#define		MON_LIST				0
#define		TUE_LIST				1
#define		WED_LIST				2
#define		THU_LIST				3
#define		FRI_LIST				4
#define		SAT_LIST				5
#define		SUN_LIST				6
#define		WEEK_LIST				7
#define		TIMETABLE_LIST			8

#define		MAX_TIMETABLE_COUNT		15
#define		MAX_GROUP_COUNT			10
#define		MAX_SCHEDULE_COUNT		5
#define		DAY_COUNT				8
#define		FILE_DATA_COUNT			4

#define 	CLASS_DATA_COUNT		200

#define 	SCHED_HAKSA_MODE		0

/*
typedef struct 
{
	short pcm;
	short pno;
	float value;	
}point_info;

typedef struct 
{
	int front;
	int rear;
	point_info data[MAX_QUEUE_SIZE];	
}point_queue;
point_queue ghp_message_queue;
*/


// struct
typedef struct __attribute__ ( (__packed__)) {
	// Basic Information
	int	    pno;            // Point number in LSU            
	int	    block;          // Block number in GHP
	// GHP Status Variable
	int	    on_off;
	int     r_temp;         // Room temperature.
	int     s_temp;         // Setting temperature.
	int	    aamode;         // aamode : heating mode, cooling mode.
	int	    wind_s;         // Wind Speed.     
	int	    wind_d;         // Wind Direction.
	int	    filter;    
	// GHP schedule time
	int		group;          		// Indoor group.(학, 1 ~ 10)
	int		group_mode;     		// Group Mode : 가동모드(0), 권한모드(1), 정지모드(2).	
	int		group_method;   		// Group Method : 주간(0), 요일(1).
	int		startT[12];
	int		stopT[12];
	unsigned char   minCount;       // 1분마다 증가하는 Timer Count.
	unsigned char   preMinute;      // 1분마다 증가하는 Timer Count.
	unsigned char   chkTouch;      	// 가동모드 또는 권한모드에서 사용되는 Flag. 함수에의 Set_On이 되면 1이 되고 타임아웃에 의해  Set_Off되면 0이 된다. 	
	
/*    
    // University Schedule Variable
    int     ghp_class;
    int     chk_mode;    
    int     chk_group;
    int     start_class[GHP_HEAD_MAX_SCHED];    
    int     end_class[GHP_HEAD_MAX_SCHED];        
    unsigned char   chk_min;        // 1분마다 증가하는 Timer Count.
    unsigned char   chk_touch;      // 가동모드 또는 권한모드에서 사용되는 Flag. 함수에의 Set_On이 되면 1이 되고 타임아웃에 의해  Set_Off되면 0이 된다. 
*/
    // Error Check
    int     error;    
    int     error_num;
}	GHP_DATA;
GHP_DATA g_UnitData[GHP_UNIT_MAX];           // GHP Status Queue

typedef struct __attribute__ ( (__packed__)) 
{
    int nPno;
    int nBno;
    char *p_cRoomNum;
    char *p_cRoomName;
    int nDDC;
    int nDDR;
    int nOutUnit;
    int nInUnit;
    int nType;
    int nFloor;
    int nSection;
} g_GHP_Info;

typedef struct __attribute__ ( (__packed__))  {
	int group;
	int method;
	int mode;
	int startT[8][12];	
	int stopT[8][12];
} g_schedule_info;
g_schedule_info g_schedule[12];

typedef struct __attribute__ ( (__packed__))  {
	int hour;
	int min;
	int sec;
} g_time_info;
g_time_info g_time;

typedef struct __attribute__ ( (__packed__)) 
{
	unsigned short group;
	unsigned short method;
	unsigned short mode;
	unsigned short startT[DAY_COUNT][MAX_SCHEDULE_COUNT];
	unsigned short stopT[DAY_COUNT][MAX_SCHEDULE_COUNT];
	unsigned short limitT;
}GROUP_DATA;

typedef struct __attribute__ ( (__packed__)) 
{
	unsigned short startT[MAX_TIMETABLE_COUNT];
	unsigned short stopT[MAX_TIMETABLE_COUNT];
}TIME_DATA;

typedef struct __attribute__ ( (__packed__)) 
{
	unsigned short		index;
	unsigned short		classNum;
	unsigned short		startT;
	unsigned short		stopT;
}CLASS_DATA;

typedef struct __attribute__ ( (__packed__)) 
{
	unsigned short		length;
	TIME_DATA			time; 
	GROUP_DATA			group[MAX_GROUP_COUNT];
	CLASS_DATA			classData[CLASS_DATA_COUNT];
	unsigned short		year;
	unsigned short		month;
	unsigned short		day;
	unsigned short		hour;
	unsigned short		min;
	unsigned short		sec;
	unsigned short		chksum;
}SEND_DATA;

typedef struct __attribute__ ( (__packed__)) 
{
    int  temperature_sh;
    int  temperature_fh;
    int  onoff;
    int  onoff_B3;
    int  onoff_B2;
    int  onoff_B1;
    int  onoff_1;
    int  onoff_2;
    int  onoff_3;
    int  onoff_4;
    int  onoff_5;
    int  onoff_6;
    int  onoff_7;
    int  onoff_B3_A;
    int  onoff_B3_B;
    int  onoff_B3_C;
    int  onoff_B3_D;
    int  onoff_B2_A;
    int  onoff_B2_B;
    int  onoff_B2_C;
    int  onoff_B2_D;
    int  onoff_B1_A;
    int  onoff_B1_B;
    int  onoff_B1_C;
    int  onoff_B1_D;
    int  onoff_1_A;
    int  onoff_1_B;
    int  onoff_1_C;
    int  onoff_1_D;
    int  onoff_2_A;
    int  onoff_2_B;
    int  onoff_2_C;
    int  onoff_2_D;    
    int  onoff_3_A;
    int  onoff_3_B;
    int  onoff_3_C;
    int  onoff_3_D;
    int  onoff_4_A;
    int  onoff_4_B;
    int  onoff_4_C;
    int  onoff_4_D;
    int  onoff_5_A;
    int  onoff_5_B;
    int  onoff_5_C;
    int  onoff_5_D;
    int  onoff_6_A;
    int  onoff_6_B;
    int  onoff_6_C;
    int  onoff_6_D;
    int  onoff_7_A;
    int  onoff_7_B;
    int  onoff_7_C;
    int  onoff_7_D;
} g_Dong_Floor;

typedef struct __attribute__ ( (__packed__)) 
{
	int  outdoor_unit_all;
	int  outdoor_unit_01;
    int  outdoor_unit_02;
    int  outdoor_unit_03;
    int  outdoor_unit_04;
    int  outdoor_unit_05;
    int  outdoor_unit_06;
    int  outdoor_unit_07;
    int  outdoor_unit_08;
    int  outdoor_unit_09;
    int  outdoor_unit_10;
    int  outdoor_unit_11;
    int  outdoor_unit_12;
    int  outdoor_unit_13;
    int  outdoor_unit_14;
    int  outdoor_unit_15;
    int  outdoor_unit_16;
    int  outdoor_unit_17;
    int  outdoor_unit_18;
    int  outdoor_unit_19;
    int  outdoor_unit_20;
    int  outdoor_unit_21;
    int  outdoor_unit_22;
    int  outdoor_unit_23;
    int  outdoor_unit_24;
    int  outdoor_unit_25;
    int  outdoor_unit_26;
    int  outdoor_unit_27;
    int  outdoor_unit_28;
    int  outdoor_unit_29;
    int  outdoor_unit_30;
    int  outdoor_unit_31;
    int  outdoor_unit_32;
    int  outdoor_unit_33;
    int  outdoor_unit_34;
    int  outdoor_unit_35;
    int  outdoor_unit_36;
    int  outdoor_unit_37;
    int  outdoor_unit_38;
    int  outdoor_unit_39;
    int  outdoor_unit_40;        
} g_Dong_Outdoor;

typedef struct __attribute__ ( (__packed__)) 
{
    int  temp_man_b1;
    int  temp_man_1;
    int  temp_man_2;
    int  temp_man_3;
    int  temp_man_4;
    int  temp_man_5;
    int  temp_woman_1;
    int  temp_woman_2;
    int  temp_woman_3;
    int  temp_woman;
    int  temp_man;
    int  temp;
    int  onoff_man;
    int  onoff_woman; 
    int  onoff;   
} g_Kisuksa_Floor;

typedef struct __attribute__ ( (__packed__)) 
{
    unsigned char stx;
    unsigned short length;
    unsigned short cmd;
    unsigned short pcm;
    unsigned short pno;
    unsigned short cnt;
    unsigned short chksum;
    unsigned char etx;
}_REQ_GET_CNT_DATA;

typedef struct __attribute__ ( (__packed__)) 
{
    unsigned char stx;
    unsigned short length;
    unsigned short cmd;
    unsigned short pcm;
    unsigned short pno;
    unsigned short cnt;
} _RES_GET_CNT_DATA_HEADER;


int iface_ghp_main(void);

#endif

#define SECTION_A                   1
#define SECTION_B                   2
#define SECTION_C                   3
#define SECTION_D                   4
#define DONG_ALL                    0
#define DONG_B3                     103
#define DONG_B2                     102
#define DONG_B1                     101
#define DONG_1                      1
#define DONG_2                      2
#define DONG_3                      3
#define DONG_4                      4
#define DONG_5                      5
#define DONG_6                      6
#define DONG_7                      7

#define PNO_ALL_ON_OFF              252
#define PNO_ALL_DEFINE_START        141
#define PNO_ALL_DEFINE_END          185
#define PNO_B3_ON_OFF               141
#define PNO_B3_A_ON_OFF             142
#define PNO_B3_B_ON_OFF             143
#define PNO_B3_C_ON_OFF             144
#define PNO_B3_D_ON_OFF             145
#define PNO_B2_ON_OFF               146
#define PNO_B2_A_ON_OFF             147
#define PNO_B2_B_ON_OFF             148
#define PNO_B2_C_ON_OFF             149
#define PNO_B2_D_ON_OFF             150
#define PNO_B1_ON_OFF               151
#define PNO_B1_A_ON_OFF             152
#define PNO_B1_B_ON_OFF             153
#define PNO_B1_C_ON_OFF             154
#define PNO_B1_D_ON_OFF             155
#define PNO_1_ON_OFF                156
#define PNO_1_A_ON_OFF              157
#define PNO_1_B_ON_OFF              158
#define PNO_1_C_ON_OFF              159
#define PNO_1_D_ON_OFF              160
#define PNO_2_ON_OFF                161
#define PNO_2_A_ON_OFF              162
#define PNO_2_B_ON_OFF              163
#define PNO_2_C_ON_OFF              164
#define PNO_2_D_ON_OFF              165
#define PNO_3_ON_OFF                166
#define PNO_3_A_ON_OFF              167
#define PNO_3_B_ON_OFF              168
#define PNO_3_C_ON_OFF              169
#define PNO_3_D_ON_OFF              170
#define PNO_4_ON_OFF                171
#define PNO_4_A_ON_OFF              172
#define PNO_4_B_ON_OFF              173
#define PNO_4_C_ON_OFF              174
#define PNO_4_D_ON_OFF              175
#define PNO_5_ON_OFF                176
#define PNO_5_A_ON_OFF              177
#define PNO_5_B_ON_OFF              178
#define PNO_5_C_ON_OFF              179
#define PNO_5_D_ON_OFF              180
#define PNO_6_ON_OFF                181
#define PNO_6_A_ON_OFF              182
#define PNO_6_B_ON_OFF              183
#define PNO_6_C_ON_OFF              184
#define PNO_6_D_ON_OFF              185
#define PNO_7_ON_OFF                186
#define PNO_7_A_ON_OFF              187
#define PNO_7_B_ON_OFF              188
#define PNO_7_C_ON_OFF              189
#define PNO_7_D_ON_OFF              190

// Outdoor group Pno in GHP_MODE_STATUS_PCM(30)
#define PNO_OUT_UNIT_01             101
#define PNO_OUT_UNIT_02             102
#define PNO_OUT_UNIT_03             103
#define PNO_OUT_UNIT_04             104
#define PNO_OUT_UNIT_05             105
#define PNO_OUT_UNIT_06             106
#define PNO_OUT_UNIT_07             107
#define PNO_OUT_UNIT_08             108
#define PNO_OUT_UNIT_09             109
#define PNO_OUT_UNIT_10             110
#define PNO_OUT_UNIT_11             111
#define PNO_OUT_UNIT_12             112
#define PNO_OUT_UNIT_13             113
#define PNO_OUT_UNIT_14             114
#define PNO_OUT_UNIT_15             115
#define PNO_OUT_UNIT_16             116
#define PNO_OUT_UNIT_17             117
#define PNO_OUT_UNIT_18             118
#define PNO_OUT_UNIT_19             119
#define PNO_OUT_UNIT_20             120
#define PNO_OUT_UNIT_21             121
#define PNO_OUT_UNIT_22             122
#define PNO_OUT_UNIT_23             123
#define PNO_OUT_UNIT_24             124
#define PNO_OUT_UNIT_25             125
#define PNO_OUT_UNIT_26             126
#define PNO_OUT_UNIT_27             127
#define PNO_OUT_UNIT_28             128
#define PNO_OUT_UNIT_29             129
#define PNO_OUT_UNIT_30             130
#define PNO_OUT_UNIT_31             131
#define PNO_OUT_UNIT_32             132
#define PNO_OUT_UNIT_33             133
#define PNO_OUT_UNIT_34             134
#define PNO_OUT_UNIT_35             135
#define PNO_OUT_UNIT_36             136
#define PNO_OUT_UNIT_37             137
#define PNO_OUT_UNIT_38             138
#define PNO_OUT_UNIT_39             139
#define PNO_OUT_UNIT_40             140

#define PNO_ALL_TEMPERATURE_SH      	250
#define PNO_ALL_TEMPERATURE_FH      	251

#define PNO_ALL_OUT_UNIT      			253
#define PNO_ALL_STOP_MODE     	 		255

#define PNO_KISUKSA_MAN_ON_OFF          1
#define PNO_KISUKSA_WOMAN_ON_OFF        2
#define PNO_KISUKSA_ALL_ON_OFF          3
#define PNO_KISUKSA_ALL_TEMPERATURE     4
#define PNO_KISUKSA_MAN_TEMPERATURE     5
#define PNO_KISUKSA_WOMAN_TEMPERATURE   6
#define PNO_KISUKSA_MAN_B1_TEMPERATURE  7
#define PNO_KISUKSA_MAN_1_TEMPERATURE   8
#define PNO_KISUKSA_MAN_2_TEMPERATURE   9
#define PNO_KISUKSA_MAN_3_TEMPERATURE   10
#define PNO_KISUKSA_MAN_4_TEMPERATURE   11
#define PNO_KISUKSA_MAN_5_TEMPERATURE   12
#define PNO_KISUKSA_WOMAN_1_TEMPERATURE 13
#define PNO_KISUKSA_WOMAN_2_TEMPERATURE 14
#define PNO_KISUKSA_WOMAN_3_TEMPERATURE 15

// schdule pcm
#define SCHED_START_PCM_NO          	9
#define SCHED_END_PCM_NO            	10

// Check Indoor Type Definition. And Limit Temperature.
#define SH_INDOOR                   1
#define FH_INDOOR                   0

//g_GHP_Point GHP_Point;
g_Dong_Floor Dong_Data;
g_Dong_Outdoor Dong_Outdoor_Data;
g_Kisuksa_Floor Kisuksa_Data;




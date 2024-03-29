#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>


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

/* �����.
g_GHP_Info Info[256] = 
{
0,1,   "C101","name",7,19,2,1,1,1,2,       
1,2,   "C103","name",7,19,2,2,1,1,2,       
2,3,   "C106","name",7,19,2,3,1,1,2,       
3,4,   "C109","name",7,19,2,4,1,1,2,       
4,5,   "C206","name",7,19,2,5,0,1,2,       

5,6,   "C205","name",7,19,2,6,0,1,2,       
6,7,   "C204","name",7,19,1,1,1,1,1,       
7,8,   "C203","name",7,19,1,2,1,1,1,       
8,9,   "C202","name",7,19,1,3,1,1,1,       
9,69,   "C201","name",7,19,1,4,1,1,1,       

10,10,   "C207","name",7,19,1,5,1,1,1,       
11,11,   "C207","name",7,19,1,6,1,1,1,       
12,12,   "C208","name",7,19,1,7,1,1,1,       
13,13,   "C208","name",7,19,1,8,1,1,1,       
14,14,   "C209","name",7,19,1,9,1,1,1,       

15,15,   "C210","name",7,19,1,10,1,1,1,      
16,16,   "C211","name",7,19,1,11,1,1,1,      
17,19,   "C303","name",7,19,1,12,1,1,1,      
18,18,   "C302","name",7,19,1,13,1,1,1,      
19,17,   "C301","name",7,19,1,14,1,1,1,      

20,20,   "C308","name",7,19,1,15,1,1,1,      
21,21,   "C308","name",7,19,1,16,1,1,1,      
22,22,   "C307","name",7,19,1,17,1,1,1,      
23,23,   "C306","name",7,20,4,1,1,2,2,       
24,24,   "C305","name",7,20,4,2,1,2,2,       

25,25,   "C304","name",7,20,4,3,1,2,2,       
26,26,   "C309","name",7,20,4,4,1,2,2,       
27,27,   "C401","name",7,19,3,1,1,2,2,       
28,28,   "C402","name",7,19,3,2,1,2,2,       
29,29,   "C403","name",7,19,2,7,1,2,2,       

30,30,   "C404","name",7,19,2,8,1,2,1,       
31,31,   "C405","name",7,19,2,9,1,2,1,       
32,32,   "C406","name",7,19,2,10,1,2,1,      
33,33,   "C406","name",7,19,2,11,1,2,1,      
34,34,   "C406","name",7,19,2,12,1,2,1,      

35,35,   "C407","name",7,19,2,13,1,2,1,      
36,36,   "C407","name",7,19,2,14,0,2,1,      
37,37,   "C407","name",7,19,2,15,0,2,1,      
38,38,   "C411","name",7,19,2,16,1,2,1,      
39,39,   "C412","name",7,19,2,17,1,2,1,      

40,40,   "C412","name",7,19,3,3,1,2,1,       
41,41,   "C412","name",7,19,3,4,1,2,1,       
42,42,   "C501","name",7,19,3,5,1,2,1,       
43,43,   "C502","name",7,19,3,6,1,2,1,       
44,44,   "C503","name",7,19,3,7,1,2,1,       

45,45,   "C504","name",7,20,4,5,1,3,2,            
46,46,   "C505","name",7,20,4,6,1,3,2,                
47,47,   "C506","name",7,20,4,7,1,3,2,                 
48,48,   "C507","name",7,20,4,8,1,3,2,              
49,49,   "C507","name",7,20,4,9,1,3,2,        

50,50,   "C507","name",7,20,5,1,1,3,1,           
51,51,   "C508","name",7,20,5,2,1,3,1,           
52,52,   "C508","name",7,20,5,3,1,3,1,                
53,53,   "C508","name",7,20,5,4,1,3,1,              
54,54,   "C509","name",7,20,5,5,1,3,1,                

55,55,   "C509","name",7,20,5,6,1,3,1,              
56,56,   "C509","name",7,20,5,7,1,3,1,                
57,57,   "D111","name",7,20,5,8,1,3,1,             
58,58,   "D110","name",7,20,5,9,1,3,1,             
59,59,   "D112","name",7,20,6,1,1,4,2,        

60,60,   "D213","name",7,20,6,2,1,4,2,              
61,61,   "D215","name",7,20,6,3,1,4,2,                
62,62,   "D214","name",7,20,6,4,1,4,2,                
63,63,   "D210","name",7,20,6,5,1,4,2,                
64,64,   "D311","name",7,20,6,6,1,4,2,              

65,65,   "D311","name",7,20,7,1,1,4,2,            
66,66,   "D311","name",7,20,7,2,1,4,2,          
67,67,   "D310","name",7,20,7,3,1,4,1,              
68,68,   "D314","name",7,20,7,4,1,4,1, 
69,70,   "C211","name",7,20,7,4,1,4,1            
};
*/


g_GHP_Info Info[256] = 
{
0,1,   "B101","name",7,19,2,1,1,1,2,       
1,2,   "B102","name",7,19,2,2,1,1,2,       
2,3,   "B103","name",7,19,2,3,1,1,2,       
3,4,   "B104","name",7,19,2,4,1,1,2,       
4,5,   "B105","name",7,19,2,5,0,1,2,       

5,6,   "B105","name",7,19,2,6,0,1,2,       
6,7,   "B107","name",7,19,1,1,1,1,1,       
7,8,   "B108","name",7,19,1,2,1,1,1,       
8,9,   "B109","name",7,19,1,3,1,1,1,       
9,10,   "B112","name",7,19,1,4,1,1,1,       

10,49,   "B113","name",7,19,1,5,1,1,1,       
11,11,   "B201","name",7,19,1,6,1,1,1,       
12,12,   "B202","name",7,19,1,7,1,1,1,       
13,13,   "B203","name",7,19,1,8,1,1,1,       
14,14,   "B204","name",7,19,1,9,1,1,1,       

15,15,   "B205","name",7,19,1,10,1,1,1,      
16,16,   "B206","name",7,19,1,11,1,1,1,      
17,17,   "B207","name",7,19,1,12,1,1,1,      
18,18,   "B208","name",7,19,1,13,1,1,1,      
19,19,   "B209","name",7,19,1,14,1,1,1,      

20,20,   "B210","name",7,19,1,15,1,1,1,      
21,21,   "B211","name",7,19,1,16,1,1,1,      
22,22,   "B214","name",7,19,1,17,1,1,1,      
23,23,   "B301","name",7,20,4,1,1,2,2,       
24,24,   "B302","name",7,20,4,2,1,2,2,       

25,25,   "B303","name",7,20,4,3,1,2,2,       
26,26,   "B304","name",7,20,4,4,1,2,2,       
27,27,   "B305","name",7,19,3,1,1,2,2,       
28,28,   "B306","name",7,19,3,2,1,2,2,       
29,29,   "B307","name",7,19,2,7,1,2,2,       

30,30,   "B308","name",7,19,2,8,1,2,1,       
31,31,   "B309","name",7,19,2,9,1,2,1,       
32,32,   "B310","name",7,19,2,10,1,2,1,      
33,33,   "B311","name",7,19,2,11,1,2,1,      
34,34,   "B312","name",7,19,2,12,1,2,1,      

35,35,   "B315","name",7,19,2,13,1,2,1,      
36,36,   "B316","name",7,19,2,14,0,2,1,      
37,37,   "B401","name",7,19,2,15,0,2,1,      
38,38,   "B402","name",7,19,2,16,1,2,1,      
39,39,   "B403","name",7,19,2,17,1,2,1,      

40,40,   "B404","name",7,19,3,3,1,2,1,       
41,41,   "B405","name",7,19,3,4,1,2,1,       
42,42,   "B406","name",7,19,3,5,1,2,1,       
43,43,   "B407","name",7,19,3,6,1,2,1,       
44,44,   "B408","name",7,19,3,7,1,2,1,       

45,45,   "B409","name",7,20,4,5,1,3,2,            
46,46,   "B410","name",7,20,4,6,1,3,2,                
47,47,   "B413","name",7,20,4,7,1,3,2,                 
48,48,   "B414","name",7,20,4,8,1,3,2          
};

int main()
{
	FILE *fp;
	char filename[32];

	fprintf(stdout, "Size = %d\n", sizeof(Info));
	fflush(stdout);

	memcpy(filename, "cnue_class_b.dat", sizeof("cnue_class_b.dat"));
	if ( (fp = fopen(filename, "w")) == NULL ) {
		printf("%s open fail.\n", filename);
		return;			
	}
	
	fwrite( &Info ,  sizeof(Info), 1, fp);
	
	fclose(fp);

}

///******************************************************************/
// file 	: ghp_message.h
// date 	: 2010.02.03
// author 	: jong2ry
// job 		: jong2ry. 2010-02-03 5:44¿ÀÈÄ
///******************************************************************/

/****************************************************************/
char Get_Bcc(unsigned char *data, int data_length)
/****************************************************************/
{
    //int debug_uart2_tx = 0;

    // Define Variable
    int i = 0, j = 0;
    char bcg_value = 0;
    int bcg_value_hi = 0;
    int bcg_value_lo = 0;
    unsigned int bcg_bit[BCG_BIT_CNT];
    unsigned char bcg_buf[256];

    // Step 1 : Data Variable Change.
    for(i = START_HEX_CODE_ADDR; i < data_length ; i++)
    {
        bcg_buf[i] = data[i];
    }
/*
        printf("DEBUG : BCG Buf Data =  ");
        for(i = START_HEX_CODE_ADDR; i < data_length ; i++)
            printf("%02x ", bcg_buf[i]); 
        printf("\n");
*/        
    // Step 2 : Data Calculation.
    for(i = 0; i < BCG_BIT_CNT; i++)
    {
        bcg_bit[i] = 0;
         
        for(j = START_HEX_CODE_ADDR; j < data_length ; j++)
        {
            bcg_bit[i] = bcg_bit[i] + (bcg_buf[j] >> i & BCG_MASK);
        }
        
        bcg_bit[i] = bcg_bit[i] & BCG_MASK;
    }    

    /*
        printf("DEBUG : BCG Bit Data =  ");
        for(i = 0; i < BCG_BIT_CNT; i++)
            printf("%02d ", bcg_bit[i]); 
        printf("\n");
	*/
	
    // Step 3 : BCG Value Calculation.
    bcg_value_lo = bcg_bit[0] + (bcg_bit[1] * 2) + (bcg_bit[2] * 4) + (bcg_bit[3] * 8);
    bcg_value_hi = bcg_bit[4] + (bcg_bit[5] * 2) + (bcg_bit[6] * 4);
    
    bcg_value = bcg_value_lo + (bcg_value_hi << 4 & 0xf0); 

  	//printf("DEBUG : BCG VALUE = %02x \n\n", bcg_value);
    return bcg_value;
}
//
//
/****************************************************************/
char itoa(int value)
/****************************************************************/
{
	char result = 0x30;
	
	if (value > 9)
		return result;
	
	result += value;
	return result;
}
//
//
/****************************************************************/
int Start_Text(unsigned char *p, int type, int textNumber)
/****************************************************************/
{
	int length = 0;
	int temp = 0;

	if (text_number >= 255)
		text_number = 0;
	else
		text_number++;

	if (type == TYPE_POLL)
	{
		p[length++] = 0x02;
		p[length++] = 0x30;
		p[length++] = 0x40;
		p[length++] = 0x0F;		
	}

	if (type == TYPE_SEL)
	{
		p[length++] = 0x02;
		p[length++] = 0x30;
		p[length++] = 0x20;
		p[length++] = 0x0F;		
	}
	
	//Make Text number.
	temp = text_number % 100;
    p[length++] = itoa(text_number/100);
	p[length++] = itoa(temp/10);
    temp = temp % 10;               
	p[length++] = itoa(temp); 	
		
	return length;
}
//
//
/****************************************************************/
int End_Text(unsigned char *p, int length)
/****************************************************************/
{
	p[length++] = 0x03; 			
	p[length] = Get_Bcc(p, length);		//Make BCC
	length++;	
	return length;
}
//
//
/****************************************************************/
int Make_Digital_Set_Msg(unsigned char *p, int point_number, point_info point)
/****************************************************************/
{
	int length = 0;
	int block_number = 0;
	int temp = 0;
    int value = 0;
    
    value = (int)point.value;
    block_number = g_UnitData[point.pno].block;

	//Make Message Code
    p[length++] = '$';       // '$'
	p[length++] = 'C';       // 'C'               
	p[length++] = '0';       // '0' is Zero
    
    //Make Block Number in PA Code
    p[length++] = '1';      // '1' Fixed
	temp = block_number % 100;
    p[length++] = itoa(block_number/100);
	p[length++] = itoa(temp/10);
    temp = temp % 10;               
	p[length++] = itoa(temp);   
    //Make Point Number in PA Code
	p[length++] = itoa(point_number/10);
	temp = point_number % 10;               
	p[length++] = itoa(temp);  

    //Make PC (1 Byte Command)
    p[length++] = itoa(value);

	return length;	
}
//
//
/****************************************************************/
int Make_Analog_Set_Msg(unsigned char *p, int point_number, point_info point)
/****************************************************************/
{
	int length = 0;
	int block_number = 0;
	int temp = 0;
    int value = 0;

    value = (int)point.value;
    block_number = g_UnitData[point.pno].block;

	//Make Message Code
    p[length++] = '$';       // '$'
	p[length++] = 'C';       // 'C'               
	p[length++] = '4';       // '4'
    
    //Make Block Number in PA Code
    p[length++] = '1';      // '1' Fixed
	temp = block_number % 100;
    p[length++] = itoa(block_number/100);
	p[length++] = itoa(temp/10);
    temp = temp % 10;               
	p[length++] = itoa(temp);   
    //Make Point Number in PA Code
	p[length++] = itoa(point_number/10);
	temp = point_number % 10;               
	p[length++] = itoa(temp);      

    //Make Indicates Code
    p[length++] = '+';      // Always '+'

    /* Step 7 : Make Set Value */
    value = value * 10;
    p[length++] = itoa(value / 1000);
	temp = value % 100;
    p[length++] = itoa(value / 100);
	p[length++] = itoa(temp / 10);
    temp = temp % 10;               
	p[length++] = itoa(temp);       

	return length;	
} 
//
//
/****************************************************************/
int Make_Read_Msg(unsigned char *p, int point_number, int unit)
/****************************************************************/
{
	int length = 0;
	int block_number = 0;
	int temp = 0;

    block_number = g_UnitData[unit].block;		//unit same pno.

	//Make Message Code
    p[length++] = '$';       // '$'
	p[length++] = 'D';       // 'D'               
	p[length++] = '0';       // '0'
    
    //Make Block Number in PA Code
    p[length++] = '1';      // '1' Fixed
	temp = block_number % 100;
    p[length++] = itoa(block_number/100);
	p[length++] = itoa(temp/10);
    temp = temp % 10;               
	p[length++] = itoa(temp);   
    //Make Point Number in PA Code
	p[length++] = itoa(point_number/10);
	temp = point_number % 10;               
	p[length++] = itoa(temp);      

	return length;	
} 
//
//
/****************************************************************/
int Make_Startup_Msg(unsigned char *p, int blockNumber)
/****************************************************************/
{
	//int i = 0;
	int length = 0;
	int temp = 0;

	printf("Startup blockNumber = %d\n", blockNumber);
	
	//Make Message Code
	length = Start_Text(p, TYPE_SEL, text_number);
	
	p[length++] = '$';       // '$'
	p[length++] = 'J';       // 'J'               
	p[length++] = '6';       // '6'
    
    //Make GROUP Number
	temp = blockNumber % 100;
	p[length++] = itoa ( blockNumber/100 );
	p[length++] = itoa ( temp/10 );
	temp = temp % 10;               
	p[length++] = itoa ( temp );   

    //Make BLOCK Number
	temp = blockNumber % 100;
	p[length++] = itoa ( blockNumber/100 );
	p[length++] = itoa ( temp/10 );
    temp = temp % 10;               
	p[length++] = itoa ( temp );   

	p[length++] = itoa ( 1 );
	
	length = End_Text(p, length);
	
	return length;	
} 
//
//
/****************************************************************/
int Make_Initial_Msg(unsigned char *p)
/****************************************************************/
{
	int length = 0;

	length = Start_Text(p, TYPE_SEL, text_number);
	
	//Make Message Code
    p[length++] = '$';       // '$'
	p[length++] = 'K';       // 'K'               
	p[length++] = '0';       // '0'
	p[length++] = '0';       // '0'
	p[length++] = '1';       // '1'

	length = End_Text(p, length);
	
	return length;	
} 
//
//
/****************************************************************/
int Add_Text(unsigned char *p, point_info point)
/****************************************************************/
{
	int result = 0;
	int cmd = 0;
	
	cmd = point.pcm;
	
	//command is 'on_off'
	if(cmd == GHP_ONOFF_PCM)
	{
		if (point.value < 2)
			result = Make_Digital_Set_Msg(p, 1, point);
		return result;
	}
	//command is 'mode'
	if(cmd == GHP_MODE_PCM)
	{
		result = Make_Digital_Set_Msg(p, 4, point);
		return result;
	}
	
	//command is 'set_temp'
	if(cmd == GHP_SET_TEMP_PCM)
	{
		result = Make_Analog_Set_Msg(p, 2, point);
		return result;
	}

	//command is 'wind_speed'
	if(cmd == GHP_WINDSPEED_PCM)
	{
		result = Make_Digital_Set_Msg(p, 30, point);
		return result;
	}

	//command is 'wind_direction'
	if(cmd == GHP_WINDDIRECTION_PCM)
	{
		result = Make_Digital_Set_Msg(p, 31, point);
		return result;
	}

	return result;	
}


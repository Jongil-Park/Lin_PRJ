///******************************************************************/
// file :	ghp_group.h
// date : 	2010.03.02.
// author : jong2ry
///******************************************************************/


void Get_Bno_Pno(int nIndex, int *npBno, int *npPno)
{

    *npPno = (g_pDongInfo + nIndex)->nPno;
    *npBno = (g_pDongInfo + nIndex)->nBno;

#if 0
    switch(g_sddcNum)
    {

        case SDDC_AC_DONG:
			*npPno = AC_Dong_Info[nIndex].nPno;
			*npBno = AC_Dong_Info[nIndex].nBno;
			break;
		
		case SDDC_B_DONG:
			*npPno = B_Dong_Info[nIndex].nPno;
			*npBno = B_Dong_Info[nIndex].nBno;
			break;
		
		case SDDC_D_DONG:
			*npPno = D_Dong_Info[nIndex].nPno;
			*npBno = D_Dong_Info[nIndex].nBno;
			break; 
		
		case SDDC_E_DONG:
			*npPno = E_Dong_Info[nIndex].nPno;
			*npBno = E_Dong_Info[nIndex].nBno;
			break;
		
		case SDDC_F_DONG:
			*npPno = F_Dong_Info[nIndex].nPno;
			*npBno = F_Dong_Info[nIndex].nBno;
			break;
		
		case SDDC_G_DONG:
			*npPno = G_Dong_Info[nIndex].nPno;
			*npBno = G_Dong_Info[nIndex].nBno;
			break;
		
		case SDDC_KISUKSA:
			*npPno = Duruarm_Kisuksa_Info[nIndex].nPno;
			*npBno = Duruarm_Kisuksa_Info[nIndex].nBno;
			break;

		default:
			printf("%s(%d) : Unknown g_sddcNum(%d)\n", __FUNCTION__, __LINE__, g_sddcNum);
			break; 
	}
#endif		
}

void Get_Bno_Pno_Outdoor(int nIndex, int *npBno, int *npPno, int *npOutdoor)
{

    *npPno = (g_pDongInfo + nIndex)->nPno;
    *npBno =(g_pDongInfo + nIndex)->nBno;
    *npOutdoor = (g_pDongInfo + nIndex)->nOutUnit;
    
#if 0
    if (g_sddcNum == SDDC_AC_DONG)
    {
        *npPno = AC_Dong_Info[nIndex].nPno;
        *npBno = AC_Dong_Info[nIndex].nBno;
        *npOutdoor = AC_Dong_Info[nIndex].nOutUnit;
    }   
    else if (g_sddcNum == SDDC_B_DONG)
    {
        *npPno = B_Dong_Info[nIndex].nPno;
        *npBno = B_Dong_Info[nIndex].nBno;
        *npOutdoor = B_Dong_Info[nIndex].nOutUnit;
    }   
    else if (g_sddcNum == SDDC_D_DONG)
    {
        *npPno = D_Dong_Info[nIndex].nPno;
        *npBno = D_Dong_Info[nIndex].nBno;
        *npOutdoor = D_Dong_Info[nIndex].nOutUnit;
    }   
    else if (g_sddcNum == SDDC_E_DONG)
    {
        *npPno = E_Dong_Info[nIndex].nPno;
        *npBno = E_Dong_Info[nIndex].nBno;
        *npOutdoor = E_Dong_Info[nIndex].nOutUnit;
    }                                                 
    else if (g_sddcNum == SDDC_F_DONG)
    {
        *npPno = F_Dong_Info[nIndex].nPno;
        *npBno = F_Dong_Info[nIndex].nBno;
        *npOutdoor = F_Dong_Info[nIndex].nOutUnit;
    }                                                 
    else if (g_sddcNum == SDDC_G_DONG)
    {                                         
        *npPno = G_Dong_Info[nIndex].nPno;
        *npBno = G_Dong_Info[nIndex].nBno;
        *npOutdoor = G_Dong_Info[nIndex].nOutUnit;
    }
    else if (g_sddcNum == SDDC_KISUKSA)
    { 
        *npPno = Duruarm_Kisuksa_Info[nIndex].nPno;
        *npBno = Duruarm_Kisuksa_Info[nIndex].nBno;
        *npOutdoor = Duruarm_Kisuksa_Info[nIndex].nOutUnit;
    }   
    else
    {
        printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum); 
    }
#endif
}

void Get_Bno_Pno_Type(int nIndex, int *npBno, int *npPno, int *npType)
{	
    *npPno = (g_pDongInfo + nIndex)->nPno;
    *npBno =(g_pDongInfo + nIndex)->nBno;
    *npType = (g_pDongInfo + nIndex)->nType;
#if 0
    if (g_sddcNum == SDDC_AC_DONG)
    {
        *npPno = AC_Dong_Info[nIndex].nPno;
        *npBno = AC_Dong_Info[nIndex].nBno;
        *npType = AC_Dong_Info[nIndex].nType;
    }   
    else if (g_sddcNum == SDDC_B_DONG)
    {
        *npPno = B_Dong_Info[nIndex].nPno;
        *npBno = B_Dong_Info[nIndex].nBno;
        *npType = B_Dong_Info[nIndex].nType;
    }   
    else if (g_sddcNum == SDDC_D_DONG)
    {
        *npPno = D_Dong_Info[nIndex].nPno;
        *npBno = D_Dong_Info[nIndex].nBno;
        *npType = D_Dong_Info[nIndex].nType;
    }   
    else if (g_sddcNum == SDDC_E_DONG)
    {
        *npPno = E_Dong_Info[nIndex].nPno;
        *npBno = E_Dong_Info[nIndex].nBno;
        *npType = E_Dong_Info[nIndex].nType;
    }                                                 
    else if (g_sddcNum == SDDC_F_DONG)
    {
        *npPno = F_Dong_Info[nIndex].nPno;
        *npBno = F_Dong_Info[nIndex].nBno;
        *npType = F_Dong_Info[nIndex].nType;
    }                                                 
    else if (g_sddcNum == SDDC_G_DONG)
    {                                         
        *npPno = G_Dong_Info[nIndex].nPno;
        *npBno = G_Dong_Info[nIndex].nBno;
        *npType = G_Dong_Info[nIndex].nType;
    }
    else if (g_sddcNum == SDDC_KISUKSA)
    { 
        *npPno = Duruarm_Kisuksa_Info[nIndex].nPno;
        *npBno = Duruarm_Kisuksa_Info[nIndex].nBno;
        *npType = Duruarm_Kisuksa_Info[nIndex].nType;
    } 
    else
    {
        printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum); 
    }
#endif
}

/****************************************************************/
// GHP_MODE_STATUS_PCM(30)
void Chk_All_Haksa(void)
/****************************************************************/
{
    //int nVal = 0;
    /*	
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_ALL_HAKSAMODE);

    if (GHP_Point.nAllHaksa != nVal)
    {
        GHP_Point.nAllHaksa = nVal;
        //GHP_Haksa_Mode_Change(nVal);
    }
    */

}

/****************************************************************/
// GHP_MODE_STATUS_PCM(30), SH_INDOOR(1), FH_INDOOR(0)
static void Dong_SetTemperature(void)
/****************************************************************/
{  
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    int nType = 0;
    int nVal = 0;
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_ALL_TEMPERATURE_SH);
    if(Dong_Data.temperature_sh != nVal)
    {
        for(i = 0; i < g_unitCnt; i++)
        {
            Get_Bno_Pno_Type(i, &nBno, &nPno, &nType);
            //printf("DEBUG : bno = %d, pno = %d, type = %d\n", nBno, nPno, nType);
                        
            if (nType == SH_INDOOR)
                pSet(GHP_SET_TEMP_PCM, nPno, nVal);
        } 
        Dong_Data.temperature_sh = nVal;
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_ALL_TEMPERATURE_FH);
    if(Dong_Data.temperature_fh != nVal)
    {
        for(i = 0; i < g_unitCnt; i++)
        {
            Get_Bno_Pno_Type(i, &nBno, &nPno, &nType);
            //printf("DEBUG : bno = %d, pno = %d, type = %d\n", nBno, nPno, nType);
                        
            if (nType == FH_INDOOR)
                pSet(GHP_SET_TEMP_PCM, nPno, nVal);
        } 
        Dong_Data.temperature_fh = nVal;
    }
}





/****************************************************************/
static void Function_Dong_Outdoor(int nPnoOutUnit, int value)
/****************************************************************/
{
    int i = 0;
    int nBno = 0;
    int nPno = 0;
    int nOutdoor = 0;
    int nOutdoorNum = 0;
#if 0
    if (value > 2)
    {
        printf("%s(%d) : Unknown Mode(%d)\n", __FUNCTION__, __LINE__, value);
        return;
    }
#endif
    
    for(i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno_Outdoor(i, &nBno, &nPno, &nOutdoor);
        nOutdoorNum = nPnoOutUnit - 100;

        if (nOutdoorNum == nOutdoor)
        {
            pSet(GHP_MODE_PCM, nPno, value);
#if 0
            if (value == 0)
                printf("Change OutMode(난방) : %d, %d\n", nBno, nPno);
            else if (value == 1)
                printf("Change OutMode(냉방) : %d, %d\n", nBno, nPno);
            else if (value == 2)
                printf("Change OutMode(송풍) : %d, %d\n", nBno, nPno);
            else
            {
                printf("%s(%d) : Unknown Mode(%d)\n", __FUNCTION__, __LINE__, value);
                return;
            }
#endif
        }     
    }
}

/****************************************************************/
static void Dong_Outdoor(void)
/****************************************************************/
{
    int i = 0;
    int value = 0;

    value = (int)pGet(GHP_MODE_STATUS_PCM, 253);
    if (Dong_Outdoor_Data.outdoor_unit_all != value)
    {
        Dong_Outdoor_Data.outdoor_unit_all = value;
        for ( i = 101; i <= 140; i++ ) {
        	pSet(GHP_MODE_STATUS_PCM, i, value);
        }
    }     

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_01);
    //if (Dong_Outdoor_Data.outdoor_unit_01 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_01, value);
        Dong_Outdoor_Data.outdoor_unit_01 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_02);
    //if (Dong_Outdoor_Data.outdoor_unit_02 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_02, value);
        Dong_Outdoor_Data.outdoor_unit_02 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_03);
    //if (Dong_Outdoor_Data.outdoor_unit_03 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_03, value);
        Dong_Outdoor_Data.outdoor_unit_03 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_04);
    //if (Dong_Outdoor_Data.outdoor_unit_04 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_04, value);
        Dong_Outdoor_Data.outdoor_unit_04 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_05);
    //if (Dong_Outdoor_Data.outdoor_unit_05 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_05, value);
        Dong_Outdoor_Data.outdoor_unit_05 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_06);
    //if (Dong_Outdoor_Data.outdoor_unit_06 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_06, value);
        Dong_Outdoor_Data.outdoor_unit_06 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_07);
    //if (Dong_Outdoor_Data.outdoor_unit_07 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_07, value);
        Dong_Outdoor_Data.outdoor_unit_07 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_08);
    //if (Dong_Outdoor_Data.outdoor_unit_08 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_08, value);
        Dong_Outdoor_Data.outdoor_unit_08 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_09);
    //if (Dong_Outdoor_Data.outdoor_unit_09 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_09, value);
        Dong_Outdoor_Data.outdoor_unit_09 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_10);
    //if (Dong_Outdoor_Data.outdoor_unit_10 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_10, value);
        Dong_Outdoor_Data.outdoor_unit_10 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_11);
    //if (Dong_Outdoor_Data.outdoor_unit_11 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_11, value);
        Dong_Outdoor_Data.outdoor_unit_11 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_12);
    //if (Dong_Outdoor_Data.outdoor_unit_12 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_12, value);
        Dong_Outdoor_Data.outdoor_unit_12 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_13);
    //if (Dong_Outdoor_Data.outdoor_unit_13 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_13, value);
        Dong_Outdoor_Data.outdoor_unit_13 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_14);
    //if (Dong_Outdoor_Data.outdoor_unit_14 != value)    
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_14, value);
        Dong_Outdoor_Data.outdoor_unit_14 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_15);
    //if (Dong_Outdoor_Data.outdoor_unit_15 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_15, value);
        Dong_Outdoor_Data.outdoor_unit_15 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_16);
    //if (Dong_Outdoor_Data.outdoor_unit_16 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_16, value);
        Dong_Outdoor_Data.outdoor_unit_16 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_17);
    //if (Dong_Outdoor_Data.outdoor_unit_17 != value)
   // {
        Function_Dong_Outdoor(PNO_OUT_UNIT_17, value);
        Dong_Outdoor_Data.outdoor_unit_17 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_18);
    //if (Dong_Outdoor_Data.outdoor_unit_18 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_18, value);
        Dong_Outdoor_Data.outdoor_unit_18 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_19);
    //if (Dong_Outdoor_Data.outdoor_unit_19 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_19, value);
        Dong_Outdoor_Data.outdoor_unit_19 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_20);
    //if (Dong_Outdoor_Data.outdoor_unit_20 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_20, value);
        Dong_Outdoor_Data.outdoor_unit_20 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_21);
    //if (Dong_Outdoor_Data.outdoor_unit_21 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_21, value);
        Dong_Outdoor_Data.outdoor_unit_21 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_22);
    //if (Dong_Outdoor_Data.outdoor_unit_22 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_22, value);
        Dong_Outdoor_Data.outdoor_unit_22 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_23);
    //if (Dong_Outdoor_Data.outdoor_unit_23 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_23, value);
        Dong_Outdoor_Data.outdoor_unit_23 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_24);
    //if (Dong_Outdoor_Data.outdoor_unit_24 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_24, value);
        Dong_Outdoor_Data.outdoor_unit_24 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_25);
    //if (Dong_Outdoor_Data.outdoor_unit_25 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_25, value);
        Dong_Outdoor_Data.outdoor_unit_25 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_26);
    //if (Dong_Outdoor_Data.outdoor_unit_26 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_26, value);
        Dong_Outdoor_Data.outdoor_unit_26 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_27);
    //if (Dong_Outdoor_Data.outdoor_unit_27 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_27, value);
        Dong_Outdoor_Data.outdoor_unit_27 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_28);
    //if (Dong_Outdoor_Data.outdoor_unit_28 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_28, value);
        Dong_Outdoor_Data.outdoor_unit_28 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_29);
    //if (Dong_Outdoor_Data.outdoor_unit_29 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_29, value);
        Dong_Outdoor_Data.outdoor_unit_29 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_30);
    //if (Dong_Outdoor_Data.outdoor_unit_30 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_30, value);
        Dong_Outdoor_Data.outdoor_unit_30 = value;
    //}     
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_31);
    //if (Dong_Outdoor_Data.outdoor_unit_31 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_31, value);
        Dong_Outdoor_Data.outdoor_unit_31 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_32);
    //if (Dong_Outdoor_Data.outdoor_unit_32 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_32, value);
        Dong_Outdoor_Data.outdoor_unit_32 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_33);
    //if (Dong_Outdoor_Data.outdoor_unit_33 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_33, value);
        Dong_Outdoor_Data.outdoor_unit_33 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_34);
    //if (Dong_Outdoor_Data.outdoor_unit_34 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_34, value);
        Dong_Outdoor_Data.outdoor_unit_34 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_35);
    //if (Dong_Outdoor_Data.outdoor_unit_35 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_35, value);
        Dong_Outdoor_Data.outdoor_unit_35 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_36);
    //if (Dong_Outdoor_Data.outdoor_unit_36 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_36, value);
        Dong_Outdoor_Data.outdoor_unit_36 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_37);
    //if (Dong_Outdoor_Data.outdoor_unit_37 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_37, value);
        Dong_Outdoor_Data.outdoor_unit_37 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_38);
    //if (Dong_Outdoor_Data.outdoor_unit_38 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_38, value);
        Dong_Outdoor_Data.outdoor_unit_38 = value;
    //} 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_39);
    //if (Dong_Outdoor_Data.outdoor_unit_39 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_39, value);
        Dong_Outdoor_Data.outdoor_unit_39 = value;
   // } 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_OUT_UNIT_40);
    //if (Dong_Outdoor_Data.outdoor_unit_40 != value)
    //{
        Function_Dong_Outdoor(PNO_OUT_UNIT_40, value);
        Dong_Outdoor_Data.outdoor_unit_40 = value;
    //}    
}















/****************************************************************/
static void A_Section_OnOff(int nFloor, int value)
/****************************************************************/
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        
      	//printf(">>>>>>>>> nFloor %d  %d\n", (g_pDongInfo + i)->nFloor,   nFloor);
      	//printf(">>>>>>>>> nSection %d  %d\n", (g_pDongInfo + i)->nSection,   SECTION_A);
        if ((g_pDongInfo + i)->nFloor == nFloor 
                && (g_pDongInfo + i)->nSection == SECTION_A) {
            pSet(GHP_ONOFF_PCM, nPno, value);
            //printf(">>>>>>>>> GROUP %d \n",  nPno);
        }
#if 0
        switch(g_sddcNum)
        {
            case SDDC_AC_DONG:
                if (AC_Dong_Info[i].nFloor == nFloor && AC_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_B_DONG:
                if (B_Dong_Info[i].nFloor == nFloor && B_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_D_DONG:
                if (D_Dong_Info[i].nFloor == nFloor && D_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break; 
                              
            case SDDC_E_DONG:
                if (E_Dong_Info[i].nFloor == nFloor && E_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_F_DONG:
                if (F_Dong_Info[i].nFloor == nFloor && F_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_G_DONG:
                if (G_Dong_Info[i].nFloor == nFloor && G_Dong_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
                
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
                
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}

/****************************************************************/
static void B_Section_OnOff(int nFloor, int value)
/****************************************************************/
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        if ((g_pDongInfo + i)->nFloor == nFloor 
                && (g_pDongInfo + i)->nSection == SECTION_B) {
            pSet(GHP_ONOFF_PCM, nPno, value);
        }
#if 0
        switch(g_sddcNum)
        {
            case SDDC_AC_DONG:
                if (AC_Dong_Info[i].nFloor == nFloor && AC_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_B_DONG:
                if (B_Dong_Info[i].nFloor == nFloor && B_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_D_DONG:
                if (D_Dong_Info[i].nFloor == nFloor && D_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;  
                        
            case SDDC_E_DONG:
                if (E_Dong_Info[i].nFloor == nFloor && E_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_F_DONG:
                if (F_Dong_Info[i].nFloor == nFloor && F_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_G_DONG:
                if (G_Dong_Info[i].nFloor == nFloor && G_Dong_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
            
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
            
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}

/****************************************************************/
static void C_Section_OnOff(int nFloor, int value)
/****************************************************************/
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        if ((g_pDongInfo + i)->nFloor == nFloor 
                && (g_pDongInfo + i)->nSection == SECTION_C) {
            pSet(GHP_ONOFF_PCM, nPno, value);
        }
#if 0
        switch(g_sddcNum)
        {
            case SDDC_AC_DONG:
                if (AC_Dong_Info[i].nFloor == nFloor && AC_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_B_DONG:
                if (B_Dong_Info[i].nFloor == nFloor && B_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_D_DONG:
                if (D_Dong_Info[i].nFloor == nFloor && D_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break; 
                        
            case SDDC_E_DONG:
                if (E_Dong_Info[i].nFloor == nFloor && E_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_F_DONG:
                if (F_Dong_Info[i].nFloor == nFloor && F_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_G_DONG:
                if (G_Dong_Info[i].nFloor == nFloor && G_Dong_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
            
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
                
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}

/****************************************************************/
static void D_Section_OnOff(int nFloor, int value)
/****************************************************************/
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        if ((g_pDongInfo + i)->nFloor == nFloor 
                && (g_pDongInfo + i)->nSection == SECTION_D) {
            pSet(GHP_ONOFF_PCM, nPno, value);
        }
   
#if 0
        switch(g_sddcNum)
        {
            case SDDC_AC_DONG:
                if (AC_Dong_Info[i].nFloor == nFloor && AC_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_B_DONG:
                if (B_Dong_Info[i].nFloor == nFloor && B_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_D_DONG:
                if (D_Dong_Info[i].nFloor == nFloor && D_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break; 
                           
            case SDDC_E_DONG:
                if (E_Dong_Info[i].nFloor == nFloor && E_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_F_DONG:
                if (F_Dong_Info[i].nFloor == nFloor && F_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;

            case SDDC_G_DONG:
                if (G_Dong_Info[i].nFloor == nFloor && G_Dong_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
            
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_D)
                {
                    pSet(GHP_ONOFF_PCM, nPno, value);
                }
                break;
                
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}




// GHP_MODE_STATUS_PCM(30)
/****************************************************************/
static void Dong_OnOff(void)
/****************************************************************/
{
    int i = 0;
    int value = 0;

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_ALL_ON_OFF);
    if (Dong_Data.onoff != value)
    {
        for (i = PNO_ALL_DEFINE_START; i < PNO_ALL_DEFINE_END; i++)
        {
            pSet(GHP_MODE_STATUS_PCM, i, value);         
        }  
        Dong_Data.onoff = value;        
    } 

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_ON_OFF);
    if (Dong_Data.onoff_B3 != value)
    {      
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_D_ON_OFF, value);
        Dong_Data.onoff_B3 = value;            
    } 

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_ON_OFF);
    if (Dong_Data.onoff_B2 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_D_ON_OFF, value);
        Dong_Data.onoff_B2 = value;        
    } 
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_ON_OFF);
    if (Dong_Data.onoff_B1 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_D_ON_OFF, value);
        Dong_Data.onoff_B1 = value;        
    } 

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_ON_OFF);
    if (Dong_Data.onoff_1 != value)
    {                                
        pSet(GHP_MODE_STATUS_PCM, PNO_1_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_D_ON_OFF, value);
        Dong_Data.onoff_1 = value;        
    }
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_ON_OFF);
    if (Dong_Data.onoff_2 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_2_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_D_ON_OFF, value);
        Dong_Data.onoff_2 = value;        
    }
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_ON_OFF);
    if (Dong_Data.onoff_3 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_3_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_D_ON_OFF, value);
        Dong_Data.onoff_3 = value;        
    }
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_ON_OFF);
    if (Dong_Data.onoff_4 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_4_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_D_ON_OFF, value);
        Dong_Data.onoff_4 = value;        
    }
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_ON_OFF);
    if (Dong_Data.onoff_5 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_5_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_D_ON_OFF, value);
        Dong_Data.onoff_5 = value;        
    }
    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_ON_OFF);
    if (Dong_Data.onoff_6 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_D_ON_OFF, value);
        Dong_Data.onoff_6 = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_ON_OFF);
    if (Dong_Data.onoff_7 != value)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_7_A_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_B_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_C_ON_OFF, value);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_D_ON_OFF, value);
        Dong_Data.onoff_7 = value;        
    }
    
    // Section B3 A,B,C,D. 
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_A_ON_OFF);
    if (Dong_Data.onoff_B3_A != value)
    {
        A_Section_OnOff(DONG_B3, value);
        Dong_Data.onoff_B3_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_B_ON_OFF);
    if (Dong_Data.onoff_B3_B != value)
    {
        B_Section_OnOff(DONG_B3, value);
        Dong_Data.onoff_B3_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_C_ON_OFF);
    if (Dong_Data.onoff_B3_C != value)
    {
        C_Section_OnOff(DONG_B3, value);
        Dong_Data.onoff_B3_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_D_ON_OFF);
    if (Dong_Data.onoff_B3_D!= value)
    {
        D_Section_OnOff(DONG_B3, value);
        Dong_Data.onoff_B3_D = value;        
    }
    
    // Section B2 A,B,C,D.   
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_A_ON_OFF);
    if (Dong_Data.onoff_B2_A != value)
    {
        A_Section_OnOff(DONG_B2, value);
        Dong_Data.onoff_B2_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_B_ON_OFF);
    if (Dong_Data.onoff_B2_B != value)
    {
        B_Section_OnOff(DONG_B2, value);
        Dong_Data.onoff_B2_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_C_ON_OFF);
    if (Dong_Data.onoff_B2_C != value)
    {
        C_Section_OnOff(DONG_B2, value);
        Dong_Data.onoff_B2_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_D_ON_OFF);
    if (Dong_Data.onoff_B2_D!= value)
    {
        D_Section_OnOff(DONG_B2, value);
        Dong_Data.onoff_B2_D = value;        
    }
    
    // Section B1 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_A_ON_OFF);
    if (Dong_Data.onoff_B1_A != value)
    {
        A_Section_OnOff(DONG_B1, value);
        Dong_Data.onoff_B1_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_B_ON_OFF);
    if (Dong_Data.onoff_B1_B != value)
    {
        B_Section_OnOff(DONG_B1, value);
        Dong_Data.onoff_B1_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_C_ON_OFF);
    if (Dong_Data.onoff_B1_C != value)
    {
        C_Section_OnOff(DONG_B1, value);
        Dong_Data.onoff_B1_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_D_ON_OFF);
    if (Dong_Data.onoff_B1_D!= value)
    {
        D_Section_OnOff(DONG_B1, value);
        Dong_Data.onoff_B1_D = value;        
    }
    
    // Section 1 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_A_ON_OFF);
    if (Dong_Data.onoff_1_A != value)
    {
        A_Section_OnOff(DONG_1, value);
        Dong_Data.onoff_1_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_B_ON_OFF);
    if (Dong_Data.onoff_1_B != value)
    {
        B_Section_OnOff(DONG_1, value);
        Dong_Data.onoff_1_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_C_ON_OFF);
    if (Dong_Data.onoff_1_C != value)
    {
        C_Section_OnOff(DONG_1, value);
        Dong_Data.onoff_1_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_D_ON_OFF);
    if (Dong_Data.onoff_1_D!= value)
    {
        D_Section_OnOff(DONG_1, value);
        Dong_Data.onoff_1_D = value;        
    }
        
    // Section 2 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_A_ON_OFF);
    if (Dong_Data.onoff_2_A != value)
    {
        A_Section_OnOff(DONG_2, value);
        Dong_Data.onoff_2_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_B_ON_OFF);
    if (Dong_Data.onoff_2_B != value)
    {
        B_Section_OnOff(DONG_2, value);
        Dong_Data.onoff_2_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_C_ON_OFF);
    if (Dong_Data.onoff_2_C != value)
    {
        C_Section_OnOff(DONG_2, value);
        Dong_Data.onoff_2_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_D_ON_OFF);
    if (Dong_Data.onoff_2_D!= value)
    {
        D_Section_OnOff(DONG_2, value);
        Dong_Data.onoff_2_D = value;        
    }
    
    // Section 3 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_A_ON_OFF);
    if (Dong_Data.onoff_3_A != value)
    {
        A_Section_OnOff(DONG_3, value);
        Dong_Data.onoff_3_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_B_ON_OFF);
    if (Dong_Data.onoff_3_B != value)
    {
        B_Section_OnOff(DONG_3, value);
        Dong_Data.onoff_3_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_C_ON_OFF);
    if (Dong_Data.onoff_3_C != value)
    {
        C_Section_OnOff(DONG_3, value);
        Dong_Data.onoff_3_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_D_ON_OFF);
    if (Dong_Data.onoff_3_D!= value)
    {
        D_Section_OnOff(DONG_3, value);
        Dong_Data.onoff_3_D = value;        
    }

    // Section 4 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_A_ON_OFF);
    if (Dong_Data.onoff_4_A != value)
    {
        A_Section_OnOff(DONG_4, value);
        Dong_Data.onoff_4_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_B_ON_OFF);
    if (Dong_Data.onoff_4_B != value)
    {
        B_Section_OnOff(DONG_4, value);
        Dong_Data.onoff_4_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_C_ON_OFF);
    if (Dong_Data.onoff_4_C != value)
    {
        C_Section_OnOff(DONG_4, value);
        Dong_Data.onoff_4_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_D_ON_OFF);
    if (Dong_Data.onoff_4_D!= value)
    {
        D_Section_OnOff(DONG_4, value);
        Dong_Data.onoff_4_D = value;        
    }
    
    // Section 5 A,B,C,D.
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_A_ON_OFF);
    if (Dong_Data.onoff_5_A != value)
    {
        A_Section_OnOff(DONG_5, value);
        Dong_Data.onoff_5_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_B_ON_OFF);
    if (Dong_Data.onoff_5_B != value)
    {
        B_Section_OnOff(DONG_5, value);
        Dong_Data.onoff_5_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_C_ON_OFF);
    if (Dong_Data.onoff_5_C != value)
    {
        C_Section_OnOff(DONG_5, value);
        Dong_Data.onoff_5_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_D_ON_OFF);
    if (Dong_Data.onoff_5_D!= value)
    {
        D_Section_OnOff(DONG_5, value);
        Dong_Data.onoff_5_D = value;        
    }
    
    // Section 6 A,B,C,D.    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF);
    if (Dong_Data.onoff_6_A != value)
    {
        A_Section_OnOff(DONG_6, value);
        Dong_Data.onoff_6_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF);
    if (Dong_Data.onoff_6_B != value)
    {
        B_Section_OnOff(DONG_6, value);
        Dong_Data.onoff_6_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF);
    if (Dong_Data.onoff_6_C != value)
    {
        C_Section_OnOff(DONG_6, value);
        Dong_Data.onoff_6_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_D_ON_OFF);
    if (Dong_Data.onoff_6_D!= value)
    {
        D_Section_OnOff(DONG_6, value);
        Dong_Data.onoff_6_D = value;        
    }

    // Section 7 A,B,C,D.    
    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_A_ON_OFF);
    if (Dong_Data.onoff_7_A != value)
    {
        A_Section_OnOff(DONG_7, value);
        Dong_Data.onoff_7_A = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_B_ON_OFF);
    if (Dong_Data.onoff_7_B != value)
    {
        B_Section_OnOff(DONG_7, value);
        Dong_Data.onoff_7_B = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_C_ON_OFF);
    if (Dong_Data.onoff_7_C != value)
    {
        C_Section_OnOff(DONG_7, value);
        Dong_Data.onoff_7_C = value;        
    }

    value = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_D_ON_OFF);
    if (Dong_Data.onoff_7_D!= value)
    {
        D_Section_OnOff(DONG_7, value);
        Dong_Data.onoff_7_D = value;        
    }
}

static void A_Section_Temperature(int nFloor, int nVal)
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);

        if (g_pDongInfo[i].nFloor == nFloor 
                && g_pDongInfo[i].nSection == SECTION_A) {
            pSet(GHP_SET_TEMP_PCM, nPno, nVal);
        }

#if 0
        switch(g_sddcNum)
        {
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_A)
                {
                    pSet(GHP_SET_TEMP_PCM, nPno, nVal);
                }
                break;
        }
#endif
    }
}

static void B_Section_Temperature(int nFloor, int nVal)
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        if (g_pDongInfo[i].nFloor == nFloor 
                && g_pDongInfo[i].nSection == SECTION_B) {
            pSet(GHP_SET_TEMP_PCM, nPno, nVal);
        }
#if 0
        switch(g_sddcNum)
        {
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_B)
                {
                    pSet(GHP_SET_TEMP_PCM, nPno, nVal);
                }
                break;
                
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}

static void C_Section_Temperature(int nFloor, int nVal)
{
    int i = 0;
    int nPno = 0;
    int nBno = 0;
    
    for (i = 0; i < g_unitCnt; i++)
    {
        Get_Bno_Pno(i, &nBno, &nPno);
        if (g_pDongInfo[i].nFloor == nFloor 
                && g_pDongInfo[i].nSection == SECTION_C) {
            pSet(GHP_SET_TEMP_PCM, nPno, nVal);
        }
#if 0
        switch(g_sddcNum)
        {            
            case SDDC_KISUKSA:
                if (Duruarm_Kisuksa_Info[i].nFloor == nFloor && Duruarm_Kisuksa_Info[i].nSection == SECTION_C)
                {
                    pSet(GHP_SET_TEMP_PCM, nPno, nVal);
                }
                break;
                
            default:
                printf("%s(%d) : Unknown g_sddcNum = %d\n", __FUNCTION__, __LINE__, g_sddcNum);
                break; 
        }
#endif
    }
}

/****************************************************************/
// GHP_MODE_STATUS_PCM(30)
static void Kisuksa_Grp_Chk(void)
/****************************************************************/
{
    int nVal = 0;

    // 기숙사 전체 제어 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_ALL_ON_OFF);
    //printf("Kisuksa_Data.onoff = %d, value = %d\n", Kisuksa_Data.onoff, nVal);
    if (Kisuksa_Data.onoff != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_ALL_ON_OFF, nVal);
        Kisuksa_Data.onoff = nVal;
    } 

    // 남자기숙사 전체 제어 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_ON_OFF);
    //printf("Kisuksa_Data.onoff_man = %d, value = %d\n", Kisuksa_Data.onoff_man, nVal);
    if (Kisuksa_Data.onoff_man != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_ON_OFF, nVal);
        Kisuksa_Data.onoff_man = nVal;                 
    } 

    // 여자기숙사 전체 제어 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_ON_OFF);
    //printf("Kisuksa_Data.onoff_woman = %d, value = %d\n", Kisuksa_Data.onoff_woman, nVal);
    if (Kisuksa_Data.onoff_woman != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF, nVal);   
        pSet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_ON_OFF, nVal);
        Kisuksa_Data.onoff_woman = nVal;
    }

    // 기숙사 전체 설정온도 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_ALL_TEMPERATURE);
    if (Kisuksa_Data.temp != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_ALL_TEMPERATURE, nVal);
        Kisuksa_Data.temp = nVal;
    } 

    // 남자기숙사 전체 설정온도 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_TEMPERATURE);
    if (Kisuksa_Data.temp_man != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_B1_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_1_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_2_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_3_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_4_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_5_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_TEMPERATURE, nVal);
        Kisuksa_Data.temp_man = nVal;                 
    } 

    // 여자기숙사 전체 설정온도 (추가된 층별제어 포인트 이용) 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_TEMPERATURE);
    if (Kisuksa_Data.temp_woman != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_1_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_2_TEMPERATURE, nVal);   
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_3_TEMPERATURE, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_TEMPERATURE, nVal);
        Kisuksa_Data.temp_woman = nVal;
    }  

    // 남자기숙사 지하1층 식당 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_ON_OFF);
    if (Dong_Data.onoff_B1 != nVal)
    {
        A_Section_OnOff(DONG_B1, nVal);
        B_Section_OnOff(DONG_B1, nVal);
        C_Section_OnOff(DONG_B1, nVal);
        D_Section_OnOff(DONG_B1, nVal);
        Dong_Data.onoff_B1 = nVal;        
    } 

    // 남자기숙사 1층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_ON_OFF);
    if (Dong_Data.onoff_1 != nVal)
    {
        A_Section_OnOff(DONG_1, nVal);
        B_Section_OnOff(DONG_1, nVal);
        C_Section_OnOff(DONG_1, nVal);
        D_Section_OnOff(DONG_1, nVal);
        Dong_Data.onoff_1 = nVal;        
    }
    
    // 남자기숙사 2층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_ON_OFF);
    if (Dong_Data.onoff_2 != nVal)
    {
        A_Section_OnOff(DONG_2, nVal);
        B_Section_OnOff(DONG_2, nVal);
        C_Section_OnOff(DONG_2, nVal);
        D_Section_OnOff(DONG_2, nVal);
        Dong_Data.onoff_2 = nVal;        
    }
    
    // 남자기숙사 3층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_ON_OFF);
    if (Dong_Data.onoff_3 != nVal)
    {
        A_Section_OnOff(DONG_3, nVal);
        B_Section_OnOff(DONG_3, nVal);
        C_Section_OnOff(DONG_3, nVal);
        D_Section_OnOff(DONG_3, nVal);
        Dong_Data.onoff_3 = nVal;        
    }
    
    // 남자기숙사 4층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_ON_OFF);
    if (Dong_Data.onoff_4 != nVal)
    {
        A_Section_OnOff(DONG_4, nVal);
        B_Section_OnOff(DONG_4, nVal);
        C_Section_OnOff(DONG_4, nVal);
        D_Section_OnOff(DONG_4, nVal);
        Dong_Data.onoff_4 = nVal;        
    }
    
    // 남자기숙사 5층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_ON_OFF);
    if (Dong_Data.onoff_5 != nVal)
    {
        A_Section_OnOff(DONG_5, nVal);
        B_Section_OnOff(DONG_5, nVal);
        C_Section_OnOff(DONG_5, nVal);
        D_Section_OnOff(DONG_5, nVal);
        Dong_Data.onoff_5 = nVal;        
    }
    
    // 여자기숙사 1층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF);
    if (Dong_Data.onoff_6_A != nVal)
    {
        A_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_A = nVal;        
    }

    // 여자기숙사 2층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF);
    if (Dong_Data.onoff_6_B != nVal)
    {
        B_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_B = nVal;        
    }

    // 여자기숙사 3층 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF);
    if (Dong_Data.onoff_6_C != nVal)
    {
        C_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_C = nVal;        
    }

    // 체육관 운전정지 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_D_ON_OFF);
    if (Dong_Data.onoff_6_D!= nVal)
    {
        D_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_D = nVal;        
    }  

    // 남자기숙사 지하 1층 식당 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_B1_TEMPERATURE);
    if (Kisuksa_Data.temp_man_b1 != nVal)
    {
        A_Section_Temperature(DONG_B1, nVal);
        Kisuksa_Data.temp_man_b1 = nVal;        
    }

    // 남자기숙사 1층 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_1_TEMPERATURE);
    if (Kisuksa_Data.temp_man_1 != nVal)
    {
        A_Section_Temperature(DONG_1, nVal);
        Kisuksa_Data.temp_man_1 = nVal;        
    }

    // 남자기숙사 1층 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_2_TEMPERATURE);
    if (Kisuksa_Data.temp_man_2 != nVal)
    {
        A_Section_Temperature(DONG_2, nVal);
        Kisuksa_Data.temp_man_2 = nVal;        
    }

    // 남자기숙사 1층 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_3_TEMPERATURE);
    if (Kisuksa_Data.temp_man_3 != nVal)
    {
        A_Section_Temperature(DONG_3, nVal);
        Kisuksa_Data.temp_man_3 = nVal;        
    }

    // 남자기숙사 1층 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_4_TEMPERATURE);
    if (Kisuksa_Data.temp_man_4 != nVal)
    {
        A_Section_Temperature(DONG_4, nVal);
        Kisuksa_Data.temp_man_4 = nVal;        
    }

    // 남자기숙사 1층 설정온도 (추가된 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_MAN_5_TEMPERATURE);
    if (Kisuksa_Data.temp_man_5 != nVal)
    {
        A_Section_Temperature(DONG_5, nVal);
        Kisuksa_Data.temp_man_5 = nVal;        
    }


    // 여자기숙사 1층 설정온도 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_1_TEMPERATURE);
    if (Kisuksa_Data.temp_woman_1 != nVal)
    {
        A_Section_Temperature(DONG_6, nVal);
        Kisuksa_Data.temp_woman_1 = nVal;        
    }

    // 여자기숙사 2층 설정온도 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_2_TEMPERATURE);
    if (Kisuksa_Data.temp_woman_2 != nVal)
    {
        B_Section_Temperature(DONG_6, nVal);
        Kisuksa_Data.temp_woman_2 = nVal;        
    }

    // 여자기숙사 3층 설정온도 (기존 층별제어 포인트 이용)
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_KISUKSA_WOMAN_3_TEMPERATURE);
    if (Kisuksa_Data.temp_woman_3 != nVal)
    {
        C_Section_Temperature(DONG_6, nVal);
        Kisuksa_Data.temp_woman_3 = nVal;        
    } 
}

/****************************************************************/
static void AC_Dong_OnOff(void)
/****************************************************************/
{
    int i = 0;
    int nVal = 0;

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_ALL_ON_OFF);
    if (Dong_Data.onoff != nVal)
    {
        for (i = PNO_ALL_DEFINE_START; i < PNO_ALL_DEFINE_END; i++)
        {
            pSet(GHP_MODE_STATUS_PCM, i, nVal);         
        }  
        Dong_Data.onoff = nVal;        
    } 

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_ON_OFF);
    if (Dong_Data.onoff_B3 != nVal)
    {      
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B3_D_ON_OFF, nVal);
        Dong_Data.onoff_B3 = nVal;            
    } 

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_ON_OFF);
    if (Dong_Data.onoff_B2 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B2_D_ON_OFF, nVal);
        Dong_Data.onoff_B2 = nVal;        
    } 
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_ON_OFF);
    if (Dong_Data.onoff_B1 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_B1_D_ON_OFF, nVal);
        Dong_Data.onoff_B1 = nVal;        
    } 

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_ON_OFF);
    if (Dong_Data.onoff_1 != nVal)
    {                                
        pSet(GHP_MODE_STATUS_PCM, PNO_1_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_1_D_ON_OFF, nVal);
        Dong_Data.onoff_1 = nVal;        
    }
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_ON_OFF);
    if (Dong_Data.onoff_2 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_2_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_2_D_ON_OFF, nVal);
        Dong_Data.onoff_2 = nVal;        
    }
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_ON_OFF);
    if (Dong_Data.onoff_3 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_3_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_3_D_ON_OFF, nVal);
        Dong_Data.onoff_3 = nVal;        
    }
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_ON_OFF);
    if (Dong_Data.onoff_4 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_4_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_4_D_ON_OFF, nVal);
        Dong_Data.onoff_4 = nVal;        
    }
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_ON_OFF);
    if (Dong_Data.onoff_5 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_5_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_5_D_ON_OFF, nVal);
        Dong_Data.onoff_5 = nVal;        
    }
    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_ON_OFF);
    if (Dong_Data.onoff_6 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_6_D_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_D_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_ON_OFF, nVal);
        Dong_Data.onoff_6 = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_7_ON_OFF);
    if (Dong_Data.onoff_7 != nVal)
    {
        pSet(GHP_MODE_STATUS_PCM, PNO_7_A_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_B_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_C_ON_OFF, nVal);
        pSet(GHP_MODE_STATUS_PCM, PNO_7_D_ON_OFF, nVal);
        Dong_Data.onoff_7 = nVal;        
    }
    
    // Section B3 A,B,C,D. 
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_A_ON_OFF);
    if (Dong_Data.onoff_B3_A != nVal)
    {
        A_Section_OnOff(DONG_B3, nVal);
        Dong_Data.onoff_B3_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_B_ON_OFF);
    if (Dong_Data.onoff_B3_B != nVal)
    {
        B_Section_OnOff(DONG_B3, nVal);
        Dong_Data.onoff_B3_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_C_ON_OFF);
    if (Dong_Data.onoff_B3_C != nVal)
    {
        C_Section_OnOff(DONG_B3, nVal);
        Dong_Data.onoff_B3_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B3_D_ON_OFF);
    if (Dong_Data.onoff_B3_D!= nVal)
    {
        D_Section_OnOff(DONG_B3, nVal);
        Dong_Data.onoff_B3_D = nVal;        
    }
    
    // Section B2 A,B,C,D.   
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_A_ON_OFF);
    if (Dong_Data.onoff_B2_A != nVal)
    {
        A_Section_OnOff(DONG_B2, nVal);
        Dong_Data.onoff_B2_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_B_ON_OFF);
    if (Dong_Data.onoff_B2_B != nVal)
    {
        B_Section_OnOff(DONG_B2, nVal);
        Dong_Data.onoff_B2_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_C_ON_OFF);
    if (Dong_Data.onoff_B2_C != nVal)
    {
        C_Section_OnOff(DONG_B2, nVal);
        Dong_Data.onoff_B2_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B2_D_ON_OFF);
    if (Dong_Data.onoff_B2_D!= nVal)
    {
        D_Section_OnOff(DONG_B2, nVal);
        Dong_Data.onoff_B2_D = nVal;        
    }
    
    // Section B1 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_A_ON_OFF);
    if (Dong_Data.onoff_B1_A != nVal)
    {
        A_Section_OnOff(DONG_B1, nVal);
        Dong_Data.onoff_B1_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_B_ON_OFF);
    if (Dong_Data.onoff_B1_B != nVal)
    {
        B_Section_OnOff(DONG_B1, nVal);
        Dong_Data.onoff_B1_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_C_ON_OFF);
    if (Dong_Data.onoff_B1_C != nVal)
    {
        C_Section_OnOff(DONG_B1, nVal);
        Dong_Data.onoff_B1_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_B1_D_ON_OFF);
    if (Dong_Data.onoff_B1_D!= nVal)
    {
        D_Section_OnOff(DONG_B1, nVal);
        Dong_Data.onoff_B1_D = nVal;        
    }
    
    // Section 1 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_A_ON_OFF);
    if (Dong_Data.onoff_1_A != nVal)
    {
        A_Section_OnOff(DONG_1, nVal);
        Dong_Data.onoff_1_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_B_ON_OFF);
    if (Dong_Data.onoff_1_B != nVal)
    {
        B_Section_OnOff(DONG_1, nVal);
        Dong_Data.onoff_1_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_C_ON_OFF);
    if (Dong_Data.onoff_1_C != nVal)
    {
        C_Section_OnOff(DONG_1, nVal);
        Dong_Data.onoff_1_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_1_D_ON_OFF);
    if (Dong_Data.onoff_1_D!= nVal)
    {
        D_Section_OnOff(DONG_1, nVal);
        Dong_Data.onoff_1_D = nVal;        
    }
        
    // Section 2 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_A_ON_OFF);
    if (Dong_Data.onoff_2_A != nVal)
    {
        A_Section_OnOff(DONG_2, nVal);
        Dong_Data.onoff_2_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_B_ON_OFF);
    if (Dong_Data.onoff_2_B != nVal)
    {
        B_Section_OnOff(DONG_2, nVal);
        Dong_Data.onoff_2_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_C_ON_OFF);
    if (Dong_Data.onoff_2_C != nVal)
    {
        C_Section_OnOff(DONG_2, nVal);
        Dong_Data.onoff_2_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_2_D_ON_OFF);
    if (Dong_Data.onoff_2_D!= nVal)
    {
        D_Section_OnOff(DONG_2, nVal);
        Dong_Data.onoff_2_D = nVal;        
    }
    
    // Section 3 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_A_ON_OFF);
    if (Dong_Data.onoff_3_A != nVal)
    {
        A_Section_OnOff(DONG_3, nVal);
        Dong_Data.onoff_3_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_B_ON_OFF);
    if (Dong_Data.onoff_3_B != nVal)
    {
        B_Section_OnOff(DONG_3, nVal);
        Dong_Data.onoff_3_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_C_ON_OFF);
    if (Dong_Data.onoff_3_C != nVal)
    {
        C_Section_OnOff(DONG_3, nVal);
        Dong_Data.onoff_3_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_3_D_ON_OFF);
    if (Dong_Data.onoff_3_D!= nVal)
    {
        D_Section_OnOff(DONG_3, nVal);
        Dong_Data.onoff_3_D = nVal;        
    }

    // Section 4 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_A_ON_OFF);
    if (Dong_Data.onoff_4_A != nVal)
    {
        A_Section_OnOff(DONG_4, nVal);
        Dong_Data.onoff_4_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_B_ON_OFF);
    if (Dong_Data.onoff_4_B != nVal)
    {
        B_Section_OnOff(DONG_4, nVal);
        Dong_Data.onoff_4_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_C_ON_OFF);
    if (Dong_Data.onoff_4_C != nVal)
    {
        C_Section_OnOff(DONG_4, nVal);
        Dong_Data.onoff_4_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_4_D_ON_OFF);
    if (Dong_Data.onoff_4_D!= nVal)
    {
        D_Section_OnOff(DONG_4, nVal);
        Dong_Data.onoff_4_D = nVal;        
    }
    
    // Section 5 A,B,C,D.
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_A_ON_OFF);
    if (Dong_Data.onoff_5_A != nVal)
    {
        A_Section_OnOff(DONG_5, nVal);
        Dong_Data.onoff_5_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_B_ON_OFF);
    if (Dong_Data.onoff_5_B != nVal)
    {
        B_Section_OnOff(DONG_5, nVal);
        Dong_Data.onoff_5_B = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_C_ON_OFF);
    if (Dong_Data.onoff_5_C != nVal)
    {
        C_Section_OnOff(DONG_5, nVal);
        Dong_Data.onoff_5_C = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_5_D_ON_OFF);
    if (Dong_Data.onoff_5_D!= nVal)
    {
        D_Section_OnOff(DONG_5, nVal);
        Dong_Data.onoff_5_D = nVal;        
    }
    
    // Section 6 A,B,C,D.    
    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_A_ON_OFF);
    if (Dong_Data.onoff_6_A != nVal)
    {
        A_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_A = nVal;        
    }

    nVal = (int)pGet(GHP_MODE_STATUS_PCM, PNO_6_B_ON_OFF);
    if (Dong_Data.onoff_6_B != nVal)
    {
        B_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_B = nVal;        
    }

    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_6_C_ON_OFF);
    if (Dong_Data.onoff_6_C != nVal)
    {
        C_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_C = nVal;        
    }

    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_6_D_ON_OFF);
    if (Dong_Data.onoff_6_D!= nVal)
    {
        D_Section_OnOff(DONG_6, nVal);
        Dong_Data.onoff_6_D = nVal;        
    }

    // Section 7 A,B,C,D.    
    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_7_A_ON_OFF);
    if (Dong_Data.onoff_7_A != nVal)
    {
        A_Section_OnOff(DONG_7, nVal);
        Dong_Data.onoff_7_A = nVal;        
    }

    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_7_B_ON_OFF);
    if (Dong_Data.onoff_7_B != nVal)
    {
        B_Section_OnOff(DONG_7, nVal);
        Dong_Data.onoff_7_B = nVal;        
    }

    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_7_C_ON_OFF);
    if (Dong_Data.onoff_7_C != nVal)
    {
        C_Section_OnOff(DONG_7, nVal);
        Dong_Data.onoff_7_C = nVal;        
    }

    nVal = (int)(int)pGet(GHP_MODE_STATUS_PCM, PNO_7_D_ON_OFF);
    if (Dong_Data.onoff_7_D!= nVal)
    {
        D_Section_OnOff(DONG_7, nVal);
        Dong_Data.onoff_7_D = nVal;        
    }
}


/****************************************************************/
void Chk_Group_Control(void)
/****************************************************************/
{

    //printf("g_sddcNum = %d\n", g_sddcNum);
    switch(g_sddcNum)
    {
		case SDDC_AC_DONG:
			AC_Dong_OnOff();
			Dong_Outdoor();
			Chk_All_Haksa();
			Dong_SetTemperature();
			break;
		
		case SDDC_B_DONG:
		case SDDC_D_DONG:
		case SDDC_E_DONG:
		case SDDC_F_DONG:
		case SDDC_G_DONG:
	        Dong_OnOff();
	        Dong_Outdoor();
	        Chk_All_Haksa();
	        Dong_SetTemperature();
			break;
		
		case SDDC_KISUKSA:
			//printf("KISUKSA GROUP CONTROL\n");
			Kisuksa_Grp_Chk();
			Dong_Outdoor();
			break;
		
		default:
			printf("%s(%d) : Unknown g_sddcNum(%d)\n", __FUNCTION__, __LINE__, g_sddcNum);
			break; 
	}
}


void Set_AllStopMode(int iUnitCnt)
{
    int i = 0;
  
    if( pGet(GHP_MODE_STATUS_PCM, PNO_ALL_STOP_MODE) > 0) {
        for (i = 0; i < iUnitCnt; i++) {
            pSet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno, 0);	
            printf("%d - %d - off\n", GHP_ONOFF_PCM, g_pDongInfo[i].nPno);
        }
        printf("\n");
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

void Get_RunUnit(int iUnitCnt)
{
    int i = 0;
    int iCnt = 0;
  
	for (i = 0; i < iUnitCnt; i++) {
	    if ( pGet(GHP_ONOFF_PCM, g_pDongInfo[i].nPno) > 0 )
	    	iCnt++;
	}
	printf("Count = %d\n", iCnt);	
	pSet( GHP_MODE_STATUS_PCM, 236, iCnt );
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



#include "file_h_all.h"


uint8 err = 0;
float Circle_Temp_Dif = 0;
uint8 Circle_Temp_Num = 0;
uint8 L_Circle_State = 0;
void L_Circle_Process(void)
{
	uint8 i = 0;
    
    uint8 State_1_Flag = 0;
    uint8 State_2_Flag = 0;
    uint8 Break_Flag = 0;
	buzzer_beep();

    Image_Count = 0;

    Image_Count_Flag = 1;
	L_Circle_State = 1;
	
	while(1)
	{
		Element_State = L_Circle;
		switch(L_Circle_State)
		{
			case 1: 
			{
				uint8 Temp_Num = 0;
                Process_Image();

                Get_L_Right_Turn_Up_Point_1(2);
                Get_L_Arc_Turn_Point();			

                Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 3, R_Unilateral_Patrol);
				//Get_Straight_Deviation_Value(Element_State, Adaptive_Y_Meet , Image_Y - 3 , C_Line , L_Circle_State);
				Get_Straight_Deviation_Value(0, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);

				speed_control();
				//tft180_show_int16(0, 100, err + 1);
				switch(State_1_Flag)
                {
                    case 0:
                    {
							
                        for(i = 57; i > 40; i--)
                        {
                            if(L_Border[i] == 3)
                            {
                                Temp_Num ++;
                            }
                        }
						
                        if(Temp_Num >= 14 )
                        {
                            State_1_Flag = 1;
                        }
                        break;
                    }
                    case 1:
                    {
                        for( i = 57 ; i > 37 ; i-- )
                        {
                            if(L_Border[i] != 3)
                            {
                                Temp_Num ++;
                            }
                        }
                        if( L_Arc_Turn_Point_Flag == 1 )
                        {
                            if(Temp_Num >= 8 && L_Right_Turn_Up_Point_Flag_1 == 1 /*&& L_Right_Turn_Up_Point_1[1] >= 8 && L_Arc_Turn_Point[0][1] >= 30 && R_Straight_Flag*/)
                            {

                               buzzer_beep();
                                L_Circle_State = 2;
//                                Circle_Temp_Num_2 = 0;
                                Image_Count = 0;
                            }
                        }
					else
					{
						if(Temp_Num >= 8 && L_Right_Turn_Up_Point_Flag_1 == 1 && L_Right_Turn_Up_Point_1[1] >= 8)
						{
							
						 buzzer_beep();
							L_Circle_State = 2;
//                                Circle_Temp_Num_2 = 0;
							Image_Count = 0;
						}
					}
                        if(L_Right_Turn_Up_Point_Flag_1 == 1)
                        {
                            Temp_Num = 0;
                            for(i = L_Right_Turn_Up_Point_1[1]; i < Adaptive_L_Start_Point[1]; i++)
                            {
                                if(L_Border[i] == 3)
                                {
                                    Temp_Num ++;
                                }
                            }
                            if(L_Right_Turn_Up_Point_1[1] >= 15 && (((float)Temp_Num / (float)(Adaptive_L_Start_Point[0] - L_Right_Turn_Up_Point_1[1])) >= 0.85f))
                            {
								
                              buzzer_beep();
                                L_Circle_State = 2;
//                                Circle_Temp_Num_2 = 0;
                                Image_Count = 0;
                            }
                        }
                        break;
                    }
                }
				break;
			}
			case 2:
			{
				uint8 Num = 0;
				
				IMU_JF_Flag = 1;
				Process_Image();
				
				Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 3, L_Arc_Unilateral_Patrol_2);		
				
				Get_Straight_Deviation_Value(Element_State , Adaptive_Y_Meet  , Image_Y - 3 , C_Line , L_Circle_State);
				Deviation_Value += 5;
				speed_control();
				//tft180_show_int16(0, 100, err + 2);
                for(i = 57; i > 0; i--)
                {
                    if(Adaptive_R_Line[i][0] == X_Border_Max - 3)
                    {
                        Num ++;
                    }
                }
								//tft180_show_int16(60, 120, Num);
                if(Num >= 20)
                {
                    State_2_Flag = 1;
                    break;
                }
                if(State_2_Flag == 1)
                {
                    if(Num <= 16)
                    {
					buzzer_beep();
                        L_Circle_State = 3;
                        Image_Count = 0;
                        break;
                    }
                }
//                if(Image_Count >= 75)
//                {
//					buzzer_beep();
//                    L_Circle_State = 3;
//                    Image_Count = 0;
//                }
                break;
			}
			case 3:
			{
				Process_Image();

                Get_R_Up_Turn_Right_Point_1(2);

               // Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 4, L_Arc_Unilateral_Patrol_2);
                Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 4, L_Arc_Unilateral_Patrol_4);
                
                Get_Straight_Deviation_Value(0, Adaptive_Y_Meet , Image_Y - 3 , C_Line ,0);
                //Get_Straight_Deviation_Value(Element_State, Adaptive_Y_Meet , Image_Y - 3 , C_Line , L_Circle_State);

				speed_control();
				//tft180_show_int16(0, 100, err + 3);
//				tft180_show_int16(0, 120, R_Up_Turn_Right_Point_Flag_1);		
//				tft180_show_int16(60, 120, Adaptive_Y_Meet);		
                if(R_Up_Turn_Right_Point_Flag_1 == 1 && Adaptive_Y_Meet >= 15)
                {

					
                   buzzer_beep();
                    L_Circle_State = 4;

                    Image_Count = 0;
                }
				if(My_ABS_F(Z_Yaw) >= 300)
                {
                    buzzer_beep();
                    L_Circle_State = 5;
                    Image_Count = 0;
                }
                break;
			}
			case 4:
            {
                Process_Image();

                Get_L_Right_Turn_Up_Point_1(3);
                Get_R_Intercept_And_Slope( Adaptive_R_Line[ Adaptive_R_Statics ] + 1, Image_Y - 10);

                Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 3, L_Arc_Unilateral_Patrol_4);

                Get_Straight_Deviation_Value(0, Adaptive_Y_Meet , Image_Y - 3 , C_Line , 0);

				speed_control();
				//tft180_show_int16(0, 100, err + 4);
//				tft180_show_int16(60, 100, R_Straight_Flag);
//				tft180_show_int16(0, 120, L_Right_Turn_Up_Point_Flag_1);
//				tft180_show_int16(60, 120, Adaptive_Y_Meet);			//30~40
                if(R_Straight_Flag == 1 && L_Right_Turn_Up_Point_Flag_1 == 1 && Adaptive_Y_Meet <= 15)
                {
                    buzzer_beep();
                    L_Circle_State = 5;
                    Image_Count = 0;
                }
//                if(Image_Count >= 200)
//                {
//                   buzzer_beep();
//                    L_Circle_State = 5;
//                    Image_Count = 0;
//                }
                if(My_ABS_F(Z_Yaw) >= 300)
                {
                   buzzer_beep();
                    L_Circle_State = 5;
                    Image_Count = 0;
                }
                break;
            }
			case 5:
            {	
				uint8 Num = 0;
                
				IMU_JF_Flag = 0;
				Z_Yaw = 0 ; 
				
                
                Process_Image();

                Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 3, R_Unilateral_Patrol);
                
				//Get_Straight_Deviation_Value(Element_State, Adaptive_Y_Meet , Image_Y - 3 , C_Line , L_Circle_State);
				Get_Straight_Deviation_Value(0, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);

				speed_control();
			  //tft180_show_int16(0, 100, err + 5);
				
                for(i = 57; i > 34; i--)
                {
                    if(L_Border[i] == 3)
                    {
                        Num ++;
                    }
                }
								
                if(Num <= 4)
                {
                    buzzer_beep();
                    Break_Flag = 1;
                    Image_Count = 0;
                }
//								{
//										uint8 near_width = R_Border[55] - L_Border[55];
//										if(near_width >= 60)   
//										{
//												Break_Flag = 1;
//										}
//								}
//                if(Image_Count >= 250)
//                {
//                   buzzer_beep();
//                    Break_Flag = 1;
//                    Image_Count = 0;
//                }
								//tft180_show_int16(0, 140, R_Border[55] - L_Border[55]);
            }
    
		}
		if(Break_Flag == 1)
        {
            Image_Count_Flag = 0;
            Image_Count = 0;
            Circle_Temp_Num = 0;
//            L_Circle_Count ++ ;

			
			Element = 0;
 //           Zebra_Flag = 1;
            break;
        }
		
////		photo_send();
//		
//        tft180_show_gray_image(0, 0, 
//				(const uint8 *)Find_Line_Image, 
//               Image_X,      // ͼ��ʵ�ʿ���:80
//               Image_Y,      // ͼ��ʵ�ʸ߶�:60  
//                80,      // ͼ����ʾ����
//                60,      // ͼ����ʾ����
//                0);           //0:�����ֵ��
//			tft180_show_float(0,100, servo_pid.Error0 ,2,2);
//			tft180_show_float(60,100, servo_pid.servo_out ,2,2);	
////			
//			tft180_show_string(0,80, "Z_Yaw:");
//			tft180_show_int16(64, 80, Z_Yaw);
//////				
//		 Draw_Border_Points_1(Adaptive_L_Line, Adaptive_R_Line, C_Line, Adaptive_L_Statics, Adaptive_R_Statics);
	}
	
}

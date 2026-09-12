#include "file_h_all.h"

#define Derailment     1        //出赛道
#define Straightaway   2        //直道
#define L_Turn         3        //左转弯道
#define R_Turn         4        //右转弯道
#define L_Circle       5        //左环岛
#define R_Circle       6        //右环岛
#define Cross          7        //十字路口
#define Zebra          8        //斑马线
#define Ramp           9        //坡道
#define Barrier        11       //障碍
#define L_Oblique_Cross 16		//左斜入
#define R_Oblique_Cross 17		//右斜入
#define Small_S        18       //小S弯



uint8 L_C_cnt=0;
uint8 R_C_cnt=0;

uint8 Element = 0;

uint8 Straightaway_Flag = 1;       //直道             0：关闭        1：开启
uint8 L_Turn_Flag = 1;       //左转弯道         0：关闭        1：开启
uint8 R_Turn_Flag = 1;       //右转弯道         0：关闭        1：开启
uint8 L_Circle_Flag = 1;       //左环岛           0：关闭        1：单边巡线处理          
uint8 R_Circle_Flag = 1;       //右环岛           0：关闭        1：单边巡线处理           
uint8 Cross_Flag = 0;       //十字路口         0：关闭        1：不做特殊处理     
uint8 L_Oblique_Cross_Flag = 0;       //斜十字路口       0：关闭        1：开启（补线处理）
uint8 R_Oblique_Cross_Flag = 0;       //斜十字路口       0：关闭        1：开启（补线处理）
uint8 Zebra_Flag = 1;       //斑马线           0：关闭        1：开启
uint8 Ramp_Flag = 0;       //坡道             0：关闭        1：开启
uint8 Three_Bif_Flag = 0;       //三岔             0：关闭        1：开启
uint8 Barrier_Flag = 0;       //障碍             0：关闭        1：开启
uint8 Disconnection_Flag = 0;       //断路             0：关闭        1：开启
uint8 T_Way_Flag = 0;       //T路口            0：关闭        1：开启
uint8 L_Garage_Flag = 0;       //左车库           0：关闭        1：开启
uint8 R_Garage_Flag = 0;       //右车库           0：关闭        1：开启
uint8 Small_S_Flag = 1;       //小S弯            0:关闭         1:识别

// 出界检测相关变量
uint8 Line_Lost_Flag = 0;       // 丢线标志（0：正常，1：丢线）
uint8 Out_Bound_Cnt = 0;        // 出界计数器（连续异常帧计数）
uint8 Out_Bound_Flag = 0;       // 出界标志（0：正常，1：出界）

uint8 Zebra_Count_Flag = 0;

// uint8 shot_flag = 0;
uint8 shot_CD = 0 ;
uint8 shot_CD_flag = 0;
uint8 shot_CD_cnt = 0;

uint16 shot_center_x = 0;
uint16 shot_center_y = 0;
uint8  lamp = 0;


uint8 stop_flag = 0;

uint8 Element_Judgement(uint8(*image)[Image_X], uint16 l_total_num, uint16 r_total_num, int8* l_dir, int8* r_dir, uint8(*l_line)[2], uint8(*r_line)[2], uint8 Adaptive_X_Meet, uint8 Adaptive_Y_Meet,
    uint8* l_border, uint8* r_border)
{
    uint8 Element_t = 0;
    uint8 i = 0;
    uint8 r_bottom_lost;
    static uint8 r_lost_filtered = 0;
    //斑马线判断
     if(Zebra_Flag != 0)
     {
         uint8 Black_Num = 0;
         for(i = L_Border[41]; i < R_Border[41]; i ++)
         {
             if( ( Find_Line_Image[41][i] - Find_Line_Image[41][i+1] >= 15 || Find_Line_Image[41][i] - Find_Line_Image[41][i+1] <= -15 ) )
             {
                 Black_Num ++;
             }
         }
         for(i = L_Border[46]; i < R_Border[46]; i ++)
         {
             if( ( Find_Line_Image[46][i] - Find_Line_Image[46][i+1] >= 15 || Find_Line_Image[46][i] - Find_Line_Image[46][i+1] <= -15 ) )
             {
                 Black_Num ++;
             }
         }
         if(Black_Num >= 18)
         {
             //Element_t = Zebra;
			 if (stop_flag == 1) stop_flag = 2;
			 //zebra_found_flag = 1;
         }
     }

    //直线判断
    if (Element_t == 0 && Straightaway_Flag != 0)
    {
        if (L_Straight_Flag == 1 && R_Straight_Flag == 1)
        {

            if (Adaptive_Y_Meet <= 20 && Adaptive_X_Meet <= 55 && Adaptive_X_Meet >=0 )
            {
                Element_t = Straightaway;
            }
        }

        //        //坡道判断嵌入直线判断内
        //        if(Element_t == Straightaway && Ramp_Flag != 0 && Image_Num >= 10)
        //        {
        //            if(Barrier_Distance <= 700 && L_Border_Point_Num <= 8 && R_Border_Point_Num <= 8 && Adaptive_Y_Meet <= 4)
        //            {
        //                Element_t = Ramp;
        //            }
        //        }
    }

    //左弯道判断
    if (Element_t == 0 && L_Turn_Flag != 0)
    {
        if (Adaptive_X_Meet <= 30 && Adaptive_Y_Meet >= 4 && L_Border_Point_Num >= 5 && L_UP_Border_Point_Num == 0 && R_UP_Border_Point_Num == 0 && R_Border_Point_Num <= 1)
        {
            Element_t = L_Turn;
        }
    }
    //右弯道判断
    if (Element_t == 0 && R_Turn_Flag != 0)
    {
        if (Adaptive_X_Meet >= 50 && Adaptive_Y_Meet >= 4 && R_Border_Point_Num >= 5 && L_UP_Border_Point_Num == 0 && R_UP_Border_Point_Num == 0 && L_Border_Point_Num <= 1)
        {
            Element_t = R_Turn;
        }
    }

    //十字路口判断
        //正入判断
    if (Element_t == 0 && Cross_Flag != 0)
    {
        if ((L_Up_Turn_Left_Point_Flag_1 + R_Up_Turn_Right_Point_Flag_1 + L_Right_Turn_Up_Point_Flag_1 + R_Left_Turn_Up_Point_Flag_1) >= 3)
        {
            uint8 L_Count = 0;
            uint8 R_Count = 0;
            //判断左右各有大量横向生长的点
            for (i = 0; i < l_total_num; i++)
            {
                if (l_dir[i] == (-3) || l_dir[i] == 3)
                {
                    L_Count++;
                }
            }
            for (i = 0; i < r_total_num; i++)
            {
                if (r_dir[i] == (-3) || r_dir[i] == 3)
                {
                    R_Count++;
                }
            }
            if (L_Count >= 10 && R_Count >= 10 && L_Border_Point_Num >= 6 && R_Border_Point_Num >= 6)
            {
                if (Relative_Border_Point_Num >= 6)
                {
                    Element_t = Cross;
                }
            }
        }
    }
		
    //判断左环岛
        //一次远距判断
    if ( Element_t == 0 && L_Circle_Flag != 0)
    {
       if (L_Up_Turn_Left_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 0 && R_Left_Turn_Up_Point_Flag == 0 && R_Left_Turn_Up_Point_Flag_1 == 0 &&
		   (R_Straightaway_Lope_Rate_A > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre && 
(R_Straightaway_Lope_Rate_B > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre	   )
       {
           // 右侧中段有效性检查：中段大量落在黑框=刚恢复丢线=S弯，非环岛
           r_bottom_lost = 0;
           for (i = 5; i < 30; i++) {
               if (R_Border[i] >= 76) r_bottom_lost++;
           }
           r_lost_filtered += (r_bottom_lost - r_lost_filtered) >> 3;
					 tft180_show_int32(0, 120, r_lost_filtered, 2);
           if (r_lost_filtered < 6 && R_Border_Point_Num <=15 &&  L_Border_Point_Num >= 5 && L_Up_Turn_Left_Point_1[1] >= 14 /*&& R_UP_Border_Point_Num > 1*/ )
           {
               uint8 Temp_Num = 0;
               for (i = 0; i < Adaptive_R_Statics / 2; i++)
               {
                   if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
                   {
                       Temp_Num++;
                   }
               }
               if (Temp_Num <= 3)
               {
                   if (Relative_Border_Point_Num <= 3)
                   {
                       //buzzer_flag = 1;
                       Element_t = L_Circle;
					   L_Circle_State = 1;
					   //L_C_cnt++;
                   }
               }
           }
       }
    }
    //二次近距判断
    if (Element_t == 0 && L_Circle_Flag != 0)
    {
       //判断左存在拐点, 判断右侧为直线
       if (L_Up_Turn_Left_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 0  && L_Arc_Turn_Point_Flag == 1 && R_Left_Turn_Up_Point_Flag == 0 && R_Left_Turn_Up_Point_Flag_1 == 0 &&
(R_Straightaway_Lope_Rate_A > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre	&&
(R_Straightaway_Lope_Rate_B > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre	   )
       {
           // 右侧中段有效性检查：中段大量落在黑框=刚恢复丢线=S弯，非环岛
           r_bottom_lost = 0;
           for (i = 5; i < 30; i++) {
               if (R_Border[i] >= 76) r_bottom_lost++;
           }
           r_lost_filtered += (r_bottom_lost - r_lost_filtered) >> 3;
					 tft180_show_int32(60, 120, r_lost_filtered, 2);
           if (r_lost_filtered < 6 && L_Up_Turn_Left_Point_1[1] > 18  && (L_Up_Turn_Left_Point_1[1] - L_Arc_Turn_Point[0][1]) >= 10 /*&& R_UP_Border_Point_Num > 1*/ )// && (L_Up_Turn_Left_Point_1[1] - L_Arc_Turn_Point[0][1]) > 15)
           {
               //判断右侧直线不在边界上
               if (/*R_Border_Point_Num <= 3 && */L_Border_Point_Num >= 18)
               {
                   uint8 Temp_Num = 0;
                   for (i = 0; i < Adaptive_R_Statics / 2; i++)
                   {
                       if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
                       {
                           Temp_Num++;
                       }
                   }
                   if (Temp_Num <= 8)
                   {
                       if (Relative_Border_Point_Num <= 3)
                       {
                           //buzzer_flag = 1;
                           Element_t = L_Circle;
						   L_Circle_State = 1;
						   //L_C_cnt++;
                       }
                   }
               }
           }
       }
    }
	
	if (Element_t == 0 && L_Circle_Flag != 0)
    {
			//判断左存在拐点, 判断右侧为直线
       if (L_Right_Turn_Up_Point_Flag == 1 && R_Up_Turn_Right_Point_Flag_1 == 0  && L_Arc_Turn_Point_Flag == 1 && R_Left_Turn_Up_Point_Flag == 0 && R_Left_Turn_Up_Point_Flag_1 == 0 &&
(R_Straightaway_Lope_Rate_A > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre	&& 
(R_Straightaway_Lope_Rate_B > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre	   )
	   {
		   if (r_lost_filtered < 6 && L_Arc_Turn_Point[0][1] > 20  && (L_Arc_Turn_Point[0][1] - L_Right_Turn_Up_Point[1] ) >= 10 /*&& R_UP_Border_Point_Num > 1*/ )// && (L_Up_Turn_Left_Point_1[1] - L_Arc_Turn_Point[0][1]) > 15)
           {
               //判断右侧直线不在边界上
               if (/*R_Border_Point_Num <= 3 && */L_Border_Point_Num <= 25 && L_Border_Point_Num >= 7 )
               {
                   uint8 Temp_Num = 0;
                   for (i = 0; i < Adaptive_R_Statics / 2; i++)
                   {
                       if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
                       {
                           Temp_Num++;
                       }
                   }
                   if (Temp_Num <= 8)
                   {
                       if (Relative_Border_Point_Num <= 3)
                       {
                           //buzzer_flag = 1;
                           Element_t = L_Circle;
						   L_Circle_State = 2;
						   //L_C_cnt++;
                       }
                   }
               }
           }
		   
	   }	 
		
	}
	
    //三次斜入判断(偏向圆环一侧)
//    if (Element == 0 &&  L_Circle_Flag != 0)
//    {
//        if (Adaptive_Y_Meet >= 3 /*&&  Adaptive_X_Meet <= 30  */&& Adaptive_R_Start_Point[0] == 76)
//        {
//            uint8 Temp_Num = 0;
//            for (i = 57; i > 2; i--)
//            {
//                if (L_Border[i] == 3 && R_Border[i] != 76)
//                {
//                    Temp_Num++;
//                }
//                if (L_Border[i] != 3 && R_Border[i] == 76)
//                {
//                    Temp_Num++;
//                }
//            }
//            if (Temp_Num >= 20  && R_UP_Border_Point_Num >= 3  && L_Border_Point_Num >= 25 /*&& R_Border_Point_Num >= 13 */ )
//            {
//                uint8 Temp_Num_2 = 0;
//                for (i = 0; i < Adaptive_R_Statics / 2; i++)
//                {
//                    if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
//                    {
//                        Temp_Num_2++;
//                    }
//                }
////                if (Temp_Num_2 <= 4)
////                {
////                    if (Relative_Border_Point_Num <= 3)
////                    {
//                        //buzzer_flag = 1;
//                        Element_t = L_Circle;
//						//L_C_cnt++;
////                    }
////                }
//            }
//        }
//    }
//	if(L_C_cnt>=2){
//		Element_t = L_Circle;
//		L_C_cnt=0;
//	}

    //    //四次斜入判断（偏向直线一侧）
//    if (Element_t == 0 && L_Circle_Flag != 0)
//    {
//        if (Adaptive_L_Start_Point[0] <= 10 && Adaptive_R_Start_Point[0] <= 75 )
//        {
//            if (L_Border_Point_Num >= 20 && L_Arc_Turn_Point_Flag == 1  && L_Up_Turn_Left_Point_Flag_1 == 0 )
//            {
//                if (R_Straight_Flag == 1 && R_Border_Point_Num <= 8)
//                {
//                    if (Adaptive_Y_Meet >= 3  && Adaptive_X_Meet <= 40 )
//                    {
//                        //buzzer_flag = 1;
//                        Element_t = L_Circle;
//                    }
//                }
//            }
//        }
//    }

    // //判断右环岛
    //     //一次远距判断
    if (Element_t == 0 && R_Circle_Flag != 0)
    {
        if (L_Up_Turn_Left_Point_Flag_1 == 0 && R_Up_Turn_Right_Point_Flag_1 == 1  && 
			(L_Straightaway_Lope_Rate_A > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre && 
			(L_Straightaway_Lope_Rate_B > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre)
        {
            if ( /* L_Border_Point_Num <=3 && */   L_Border_Point_Num <=15 &&  R_Border_Point_Num >= 5  && R_Up_Turn_Right_Point_1[1] >= 18  && L_UP_Border_Point_Num > 1)
            {
                uint8 Temp_Num = 0;
                for (i = 0; i < Adaptive_L_Statics / 2; i++)
                {
                    if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                    {
                        Temp_Num++;
                    }
                }
                if (Temp_Num <= 3)
                {
                    if (Relative_Border_Point_Num <= 3)
                    {
                        //buzzer_flag = 1;
                        Element_t = R_Circle;
						R_Circle_State = 1;
						//R_C_cnt++;
                    }
                }
            }
        }
    }
    //二次近距判断
    if (Element_t == 0 && R_Circle_Flag != 0)
    {
        //判断右存在拐点, 判断左侧为直线
        if (R_Up_Turn_Right_Point_Flag_1 == 1 && L_Up_Turn_Left_Point_Flag_1 == 0  && R_Arc_Turn_Point_Flag == 1 && 
			(L_Straightaway_Lope_Rate_A > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre && 
			(L_Straightaway_Lope_Rate_B > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre )
        {
            if (R_Up_Turn_Right_Point_1[1] > 20 && (R_Up_Turn_Right_Point_1[1] - R_Arc_Turn_Point[0][1]) >= 10  && L_UP_Border_Point_Num > 1)//(R_Up_Turn_Right_Point_1[1] - R_Arc_Turn_Point[0][1]) > 15)
            {
                //判断左侧直线不在边界上
                if (/* L_Border_Point_Num <=3 && */ R_Border_Point_Num >= 18)
                {
                    uint8 Temp_Num = 0;
                    for (i = 0; i < Adaptive_L_Statics / 2; i++)
                    {
                        if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                        {
                            Temp_Num++;
                        }
                    }
                    if (Temp_Num <= 8)
                    {
                        if (Relative_Border_Point_Num <= 3)
                        {
                            //buzzer_flag = 1;
                            Element_t = R_Circle;
							R_Circle_State = 1;
							//R_C_cnt++;
                        }
                    }
                }
            }
        }
    }
		if (Element_t == 0 && R_Circle_Flag != 0)
    {
			//判断左存在拐点, 判断右侧为直线
       if (R_Left_Turn_Up_Point_Flag == 1 && L_Up_Turn_Left_Point_Flag_1 == 0  && R_Arc_Turn_Point_Flag == 1 && 
(L_Straightaway_Lope_Rate_A > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_A)  <= Min_Slope_Dif_Thre	&& 
(L_Straightaway_Lope_Rate_B > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_B)  <= Min_Slope_Dif_Thre	   )
	   {
		   if ( R_Arc_Turn_Point[0][1] > 20 /* && (R_Arc_Turn_Point[0][1] - R_Left_Turn_Up_Point[1] ) >= 10 */&& L_UP_Border_Point_Num > 1 )// && (R_Up_Turn_Reft_Point_1[1] - R_Arc_Turn_Point[0][1]) > 15)
           {
               //判断右侧直线不在边界上
               if (/*R_Border_Point_Num <= 3 && */R_Border_Point_Num <= 25 && R_Border_Point_Num>=7)
               {
                   uint8 Temp_Num = 0;
                   for (i = 0; i < Adaptive_L_Statics / 2; i++)
                   {
                       if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                       {
                           Temp_Num++;
                       }
                   }
                   if (Temp_Num <= 8)
                   {
                       if (Relative_Border_Point_Num <= 3)
                       {
                           //buzzer_flag = 1;
                           Element_t = R_Circle;
						   R_Circle_State = 2;
						   //L_C_cnt++;
                       }
                   }
               }
           }
		   
	   }
			 
		
   }
//    //三次斜入判断
//    if (Element == 0 && R_Circle_Flag != 0)
//    {
//        if (Adaptive_Y_Meet >= 3 && Adaptive_X_Meet >= 50 && Adaptive_L_Start_Point[0] == 3)
//        {
//            uint8 Temp_Num = 0;
//            for (i = 57; i > 2; i--)
//            {
//                if (L_Border[i] == 3 && R_Border[i] != 76)
//                {
//                    Temp_Num++;
//                }
//                if (L_Border[i] != 3 && R_Border[i] == 76)
//                {
//                    Temp_Num++;
//                }
//            }
//            if (Temp_Num >= 30 &&  L_UP_Border_Point_Num >= 3 &&  L_Border_Point_Num >= 13 &&  R_Border_Point_Num >= 16)
//            {
//                uint8 Temp_Num_2 = 0;
//                for (i = 0; i < Adaptive_R_Statics / 2; i++)
//                {
//                    if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
//                    {
//                        Temp_Num_2++;
//                    }
//                }
//                if (Temp_Num_2 <= 4)
//                {
////                    if (Relative_Border_Point_Num <= 3)
////                    {
//                        //buzzer_flag = 1;
//                        Element_t = R_Circle;
//						//R_C_cnt++;
////                    }
//                }
//            }
//        }
//    }
//	if(R_C_cnt>=2){
//		Element_t = R_Circle;
//		R_C_cnt=0;
//	}
    //    //四次斜入判断（偏向直线一侧）
//        if(Element_t == 0 && R_Circle_Flag != 0)
//        {
//            if( Adaptive_L_Start_Point[0] >= 5 && Adaptive_R_Start_Point[0] >= 70)
//            {
//                if(R_Border_Point_Num >= 20 && R_Arc_Turn_Point_Flag == 1  && R_Up_Turn_Right_Point_Flag_1 == 0 )
//                {
//                    if(L_Straight_Flag == 1 && L_Border_Point_Num <= 8)
//                    {
//                        if(Adaptive_Y_Meet >= 3  && Adaptive_X_Meet >= 40  )
//                        {
//							
//                            Element_t = R_Circle;
//                        }
//                    }
//                }
//            }
//        }
		
        //小S弯判断
       if(Element_t == 0 && Small_S_Flag != 0)
       {
           if(L_Arc_Turn_Point_Flag == 1 && R_Arc_Turn_Point_Flag == 1 && ((L_Arc_Turn_Point_Num + R_Arc_Turn_Point_Num) >= 3))
           {
               if(Adaptive_Y_Meet <= 5)
               {
                   Element_t = Small_S;
               }
           }
       }
   //    //左侧障碍物识别
   //    if((Element_t == 0 || Element_t == Straightaway) && Barrier_Flag != 0)
   //    {
   //        if(L_Border_Point_Num == 0)
   //        {
   //            if(L_Straight_Flag == 1 && R_Straight_Flag == 1)
   //            {
   //                uint8 Temp_Barrier_Flag = 0;
   //                for(i = 57; i > 2; i--)
   //                {
   //                    if(L_Border[i] - L_Border[i - 1] >= 5)
   //                    {
   //                        Temp_Barrier_Flag = 1;
   //                        break;
   //                    }
   //                }
   //                uint8 Temp_X = L_Border[i];
   //                uint8 Temp_Y = i;
   //                if(Temp_Barrier_Flag == 1)
   //                {
   //                    uint8 j = 0;
   //                    uint8 Temp_Num = 0;
   //                    for(i = Temp_Y - 5; i < Temp_Y; i++)
   //                    {
   //                        for(j = Temp_X + 1; j < Temp_X + 6; j ++)
   //                        {
   //                            if(Find_Line_Image[i][j] < (Adaptive_Thres_Average - 20))
   //                            {
   //                                Temp_Num ++;
   //                            }
   //                        }
   //                    }
   //                    if(Temp_Num >= 23)
   //                    {
   //                        Element_t = L_Barrier;
   //                    }
   //                }
   //            }
   //        }
   //    }

   //    //出赛道判断
   //    if(Adaptive_Y_Meet >= 40 && L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_B && L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_C
   //                    && R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_B && R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_C)
   //    {
   //        Element_t = Derailment;
   //    }
   
   if( Element_t == L_Circle || Element_t == R_Circle || Element_t == Cross ){
		if(stop_flag == 0)stop_flag = 1;
	}
   
    return Element_t;
}

// 纯整数 出界检测
void Bound_Lost_Detect(void)
{
	 Line_Lost_Flag = 0;
    // 1. 左右边线点数过少判定丢线
    if(Adaptive_L_Statics < LINE_LOST_MIN_NUM || Adaptive_R_Statics < LINE_LOST_MIN_NUM)
    {
        Line_Lost_Flag = 1;
    }
	
	if( Adaptive_R_Start_Point[0] - Adaptive_L_Start_Point[0]  <= 20){
		 Line_Lost_Flag = 1;
	}
//    if(Adaptive_R_Start_Point[0] -  Adaptive_L_Start_Point[0] <= 17)
//    {
//        Line_Lost_Flag = 1;
//    }
//	if( Adaptive_Y_Meet >= 40 && ( ( L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_B && L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_C) ||
//                       ( R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_B && R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_C ) ) )
//       {
//           Line_Lost_Flag = 1;
//       }
    // 连续多帧触发，判定真正出界
    if(Line_Lost_Flag)
    {
        Out_Bound_Cnt ++;
        if(Out_Bound_Cnt >= 3)   // 连续8帧异常=判定出界
        {
            Out_Bound_Flag = 1;
        }
    }
    else
    {
        Out_Bound_Cnt = 0;
        Out_Bound_Flag = 0;
    }
}

/**
* 函数功能：      十字路口边线整形处理
* 特殊说明：      当识别到十字路口时，对左右两侧的拐点进行连线处理
*                 将两个拐点之间的边线点统一设置为两点x坐标的平均值
* 形  参：        无
* 示例：          Cross_Border_Reshape();
* 返回值：        无
*/
void Cross_Border_Reshape(void)
{
    uint8 i = 0;
    uint8 start_y = 0;
    uint8 end_y = 0;
    uint8 avg_x = 0;
  //  if( !(R_Up_Turn_Right_Point_Flag_1 == 1 && R_Left_Turn_Up_Point_Flag_1 == 1 && L_Up_Turn_Left_Point_Flag_1 == 1 && L_Right_Turn_Up_Point_Flag_1 == 1) ) return;
    // ========== 处理左侧边线 ==========
    // 左侧两个拐点：L_Up_Turn_Left_Point_1 和 L_Right_Turn_Up_Point_1
    if(L_Up_Turn_Left_Point_Flag_1 == 1 && L_Right_Turn_Up_Point_Flag_1 == 1)
    {
        // 确定y坐标范围
        if(L_Up_Turn_Left_Point_1[1] < L_Right_Turn_Up_Point_1[1])
        {
            start_y = L_Up_Turn_Left_Point_1[1];
            end_y = L_Right_Turn_Up_Point_1[1];
        }
        else
        {
            start_y = L_Right_Turn_Up_Point_1[1];
            end_y = L_Up_Turn_Left_Point_1[1];
        }
        
        // 计算x坐标平均值
        avg_x = (L_Up_Turn_Left_Point_1[0] + L_Right_Turn_Up_Point_1[0]) / 2;
        
        // 将两点y值间的所有一维边线点都设置为这个x平均值
        for(i = start_y; i <= end_y; i++)
        {
            L_Border[i] = avg_x;
        }
    }
    
    // ========== 处理右侧边线 ==========
    // 右侧两个拐点：R_Up_Turn_Right_Point_1 和 R_Left_Turn_Up_Point_1
    if(R_Up_Turn_Right_Point_Flag_1 == 1 && R_Left_Turn_Up_Point_Flag_1 == 1)
    {
        // 确定y坐标范围
        if(R_Up_Turn_Right_Point_1[1] < R_Left_Turn_Up_Point_1[1])
        {
            start_y = R_Up_Turn_Right_Point_1[1];
            end_y = R_Left_Turn_Up_Point_1[1];
        }
        else
        {
            start_y = R_Left_Turn_Up_Point_1[1];
            end_y = R_Up_Turn_Right_Point_1[1];
        }
        
        // 计算x坐标平均值
        avg_x = (R_Up_Turn_Right_Point_1[0] + R_Left_Turn_Up_Point_1[0]) / 2;
        
        // 将两点y值间的所有一维边线点都设置为这个x平均值
        for(i = start_y; i <= end_y; i++)
        {
            R_Border[i] = avg_x;
        }
    }
}
static uint8 shot_debounce = 0;
//uint8 laser_fired = 0;   // 激光相关，已禁用
uint8 ring_shot_flag = 0;
uint8 cnt=0;
// 1. 将魔法数字提取为宏定义，方便后续根据摄像头和靶子调整
#define RING_CHECK_ROWS_CNT 16
#define RING_WIDTH_MIN      2
#define RING_WIDTH_MAX      25
#define RING_HEIGHT_MIN     2
#define RING_HEIGHT_MAX     25
#define RING_Y_MIN          24
#define RING_Y_MAX          47
#define RING_MIN_VALID_CNT  9

// 假设图像高度，请根据实际摄像头分辨率修改
#define IMAGE_HEIGHT        60

void ring_shot_judge(uint8(*image)[Image_X] , uint8 (*l_line)[2], uint8 (*r_line)[2], uint8 *l_border, uint8 *r_border)
{
    /* -------------------------------------------------------
     * C251 规范要求：所有变量定义必须放在函数的最前面
     * ------------------------------------------------------- */
    // 常量数组建议用 const 修饰，可能会存放在 ROM 中节省 RAM
    const uint8 check_rows[RING_CHECK_ROWS_CNT] = {
         24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54 };
    //};

    uint8 row_idx;
    uint8 i;                  // 用于内部循环
    uint8 row;                // 当前行号
    uint8 width;              // 特征宽度
    uint8 ring_height;        // 圆环高度
//    uint8 lamp;               // 灯号逻辑变量

    uint16 shot_center_x_sum = 0; // X坐标累加和 (使用uint16防止溢出)
    uint16 shot_center_y_sum = 0; // Y坐标累加和
    uint8 find_cnt = 0;       // 有效点数统计

    uint8 first_point;
    uint8 end_point;

//    shot_center_x;     // 最终计算出的中心X
//    shot_center_y;     // 最终计算出的中心Y

    uint8 up_point;
    uint8 down_point;
    uint8 start_point_up;
    uint8 start_point_down;
    
    uint8 ring_flag = 0;

    /* -------------------------------------------------------
     * 1. 冷却期处理
     * ------------------------------------------------------- */
    if (shot_CD)
    {
        // 冷却期间强制关闭所有输出，并直接返回，节省算力
//        gpio_set_level(IO_P67, 0);
//        gpio_set_level(IO_P05, 0);
//        gpio_set_level(IO_PA2, 0);
//        gpio_set_level(IO_PA4, 0);
//        gpio_set_level(IO_PA6, 0);
        return;
    }

    /* -------------------------------------------------------
     * 2. 单次扫描：提取特征并计算初步中心
     * ------------------------------------------------------- */
    for (row_idx = 0; row_idx < RING_CHECK_ROWS_CNT; row_idx++)
    {
        row = check_rows[row_idx];
        first_point = 0;
        end_point = 0;

        // 边框有效性检查
        if (l_border[row] > 3 && r_border[row] < 76 && r_border[row] - l_border[row] > 25)
        {
            // 寻找边缘突变点
            for (i = l_border[row] + 10; i < r_border[row] - 10; i++)
            {
                if (Compare_Num(image[row][i], image[row][i + 1], 10))
                {
                    if (!first_point) first_point = i;
                    end_point = i + 2; // 记录结束位置
                }
            }

            // 验证宽度并累加坐标
            // 注意：如果没找到点，first_point和end_point都是0，width为0，会被下面的条件过滤掉
            width = end_point - first_point;
            if (width >= RING_WIDTH_MIN && width <= RING_WIDTH_MAX && first_point > 12)
            {
                shot_center_x_sum += (first_point + end_point) / 2;
                shot_center_y_sum += row;
                find_cnt++;
            }
				}
			}
			 
    /* -------------------------------------------------------
     * 3. 计算平均值与防除零保护
     * ------------------------------------------------------- */
//	if (find_cnt >= 1 )
//	{
//		t = 90 ;
//	}
    if (find_cnt >=  2 )ring_flag = 1;
		if(ring_flag)
    {
 //       buzzer_beep();
//        // 防止除以0导致死机
        shot_center_x = (shot_center_x_sum / find_cnt) + 1 ; // +1 可能是为了补偿偏移，保留原逻辑
        shot_center_y = shot_center_y_sum / find_cnt ;
		
        /* -------------------------------------------------------
         * 4. 垂直方向寻找圆环上下边界 (带边界保护)
         * ------------------------------------------------------- */
        up_point = shot_center_y ;
        down_point = shot_center_y ;

        // 向上搜索
        start_point_up = shot_center_y - 1 ;
        
        while (start_point_up > 18)
        {
            if (Compare_Num(image[start_point_up][shot_center_x],
                            image[start_point_up - 1][shot_center_x], 12))
            {
                up_point = start_point_up;
                break;
            }
            start_point_up--;
        }

        // 向下搜索 (注意不要超过图像最大高度)
        start_point_down = shot_center_y + 1 ;
        
        while (start_point_down < IMAGE_HEIGHT - 3)
        {
            if (Compare_Num(image[start_point_down][shot_center_x],
                            image[start_point_down + 1][shot_center_x], 12))
            {
                down_point = start_point_down;
                break;
            }
            start_point_down++;
        }

//        
        ring_height = down_point - up_point ;

        if (ring_height >= RING_HEIGHT_MIN && ring_height <= RING_HEIGHT_MAX)
        {
            ring_shot_flag = 1;
            // 更新更精确的Y中心
            shot_center_y = ( down_point + up_point ) / 2;

            // 确认Y轴在有效射击区域内
            if ( shot_center_y >= RING_Y_MIN && shot_center_y <= RING_Y_MAX  && !shot_CD)
            {
                // 触发蜂鸣器并进入冷却
                buzzer_beep();
								shot_CD_flag = 1;
                //shot_CD_cnt = 0; 
				        //shot_CD = 15;

                // 根据X轴坐标确定区域 (1-5)
                lamp = 0;
                if (shot_center_x < 32) lamp = 1;
                else if (shot_center_x < 38) lamp = 2;
                else if (shot_center_x < 44) lamp = 3;
                else if (shot_center_x < 51) lamp = 4;
                else lamp = 5;

                /* -------------------------------------------------------
                 * 6. 执行动作 (仅设置电平，不重复初始化)
                 * ------------------------------------------------------- */
                //switch (lamp)
                //{
                //case 1:
                    //gpio_init(IO_P76, GPO, 1, GPO_PUSH_PULL);
										//gpio_set_level(IO_P76, 1);
//										system_delay_ms(50);
//										gpio_set_level(IO_P76, 0);
                    //break;
                //case 2:
                    //gpio_init(IO_P74, GPO, 1, GPO_PUSH_PULL);
										//gpio_set_level(IO_P74, 1);
//										system_delay_ms(50);
//										gpio_set_level(IO_P74, 0);
                    //break;
                //case 3:                   
										//gpio_init(IO_P77, GPO, 1, GPO_PUSH_PULL);
										//gpio_set_level(IO_P77, 1);
//										system_delay_ms(50);
//										gpio_set_level(IO_P77, 0);
                    //break;
                //case 4:
                    //gpio_init(IO_P75, GPO, 1, GPO_PUSH_PULL);
										//gpio_set_level(IO_P75, 1);
//										system_delay_ms(50);
//										gpio_set_level(IO_P75, 0);
                    //break;
                //case 5:
                    //gpio_init(IO_P67, GPO, 1, GPO_PUSH_PULL);
										//gpio_set_level(IO_P67, 1);
//										system_delay_ms(50);
//										gpio_set_level(IO_P67, 0);
                    //break;
                //default:
                    //break;
                //}
            }
        }
		//if(photo_flag == 0)
			if(1)
			{
//				tft180_show_string(0,64, "center_x:");			
//			tft180_show_int16(64, 64, shot_center_x);
//				tft180_show_string(0,80, "center_y:");			
//			tft180_show_int16(64, 80, shot_center_y);
//				tft180_show_string(0,96, "lamp:");			
//			tft180_show_int16(64, 96, lamp);
				//tft180_show_int16(0, 0, 2);
			}
        }
    else
    {
        ring_shot_flag = 0;
    }
}

void draw_target_center(void)
{
		int8 i;

		if (!ring_shot_flag)  return;

		for (i = -5; i <= 5; i++)
		{
				tft180_draw_point(shot_center_x + i, shot_center_y, RGB565_RED);
		}

		for (i = -5; i <= 5; i++)
		{
				tft180_draw_point(shot_center_x, shot_center_y + i, RGB565_RED);
		}
}

  
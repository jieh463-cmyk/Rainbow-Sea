#include "file_h_all.h"

//**********************图像压缩********************************
/**
* 函数功能：      复制并压缩图像，将 188 * 120 图像压缩为 80 * 60 大小
* 特殊说明：      总钻风使用手册中说明：图像分辨率为  752 * 480， 376 * 240， 188 * 120 这三种分辨率视野是一样的，三者呈整数倍关系
*                其他分辨率是通过裁减得到的(这个裁减包含比188 * 120小的任何分辨率，如 94 * 60)，如376 * 240 的视野反而比752 * 400 的视野广
*                此处将总钻风传回图像 188 * 120 压缩为 80 * 60， 所以将 j 乘系数 2.35（188 / 80）
*                经实际测试，当设置图像大小为 94 * 60 时，传回的图像视野是 188 * 120 的四分之一，虽然也和 752 * 480 呈整数倍关系，但和上述情况不同
*
*                注意复制是必须的，这样在处理复制图像时，原图像变量就可以正常接收摄像头数据
* 形  参：        无
* 示例：          Copy_Zip_Image();
* 返回值：        无
*/

// 定义全局变量，与头文件的extern声明对应
uint8 Inverse_Flag = 0;
uint8 far Find_Line_Image[Image_Y][Image_X];
uint8 Image_Num = 0;

void Copy_Zip_Image(void)               //*****
{
    uint8 i,j;

	if(Image_Count_Flag == 1)
        {
            Image_Count ++;
        }
	
    if(mt9v03x_finish_flag == 1 && Inverse_Flag == 0)       //mt9v03x_finish_flag：逐飞库定义摄像头采集标志位，采集完一帧图像会挂1
    //Inverse_Flag == 1                                     //Inverse_Flag：自定义逆透视标志位，挂1时会得到一张逆透视图像，根据自己需求而定
    {
        for(i = 0; i < Image_Y; i++)
        {
            for(j = 0; j < Image_X; j++)
            {
                Find_Line_Image[i][j] = mt9v03x_image[i * 2][(uint8)(j * 235 / 100 )];    //将188 * 120图像压缩为 80 * 60大小，X轴比为2.35，Y轴比为2
            }
        }
        if(Image_Count_Flag == 1)                 //Image_Count_Flag：自定义开启图像计数标志位，挂1时开启图像计数，即每采集一张图像计数+1；挂0时图像计数清零
        {
            Image_Count ++;
        }
        else if(Image_Count_Flag == 0)
        {
            Image_Count = 0;
        }
        Image_Num ++;
        mt9v03x_finish_flag = 0;    //注意清掉图像采集完成标志位
//																			(Copy_Zip_Image 内部已经清零了 mt9v03x_finish_flag，如果图像处理时间过长，可能会错过新一帧的数据，导致显示撕裂。)
    }

}
//****************************************************************

//**********************图像补黑框********************************
uint8 Black_Box_Value = 0;
//画黑框（必须为一个像素宽度，边界务必空出一格）
/**
* 函数功能：      图像补黑框
* 特殊说明：      注意黑框与边界间隔一格像素宽度
* 形  参：        uint8 black_box_value            黑框的灰度值
*                uint8(*image)[Image_X]            要补黑框的图像
*
* 示例：          Draw_Black_Box(Black_Box_Value, Find_Line_Image);
* 返回值：        无
*/
void Draw_Black_Box(uint8 black_box_value, uint8(*image)[Image_X])          
{
    uint8 i,j;

    for(i = 2; i < Image_Y; i++)
    {
        image[i][Image_X - 3] = black_box_value;
        image[i][2] = black_box_value;
    }
    for(j = 2; j < Image_X - 3; j++)
    {
        image[1][j] = black_box_value;
    }
}

//****************************************************************

//// 快速整数开平方（输入0~65535，输出0~255）
//uint8 Fast_Sqrt8(uint16 val)
//{
//    uint8 i;
//    uint8 res = 0;
//    uint8 b = 128;
//    for(i=0; i<8; i++)
//    {
//        uint8 temp = res | b;
//        if((uint16)temp * temp <= val)
//            res = temp;
//        b >>= 1;
//    }
//    return res;
//}

//uint8 Adaptive_L_Thres_Max = 0;     //左侧阈值最大值
//uint8 Adaptive_R_Thres_Max = 0;     //右侧阈值最大值
//uint8 Adaptive_L_Thres_Min = 0;     //左侧阈值最小值
//uint8 Adaptive_R_Thres_Min = 0;     //右侧阈值最小值
//uint8 Adaptive_Thres_Average = 0;   //阈值均值
//uint8 Last_Adaptive_Thres_Average = 0;  //用于阈值均值滤波
///**
//* 函数功能：      提取阈值中的最大最小值,并求出阈值均值
//* 特殊说明：      计算时间小于5us
//* 形  参：        无
//*
//* 示例：          Thres_Record_Process（）；
//* 返回值：        无
//*/
//void Thres_Record_Process(void)
//{
//    uint8 i = 0;
//    uint8 Left_Temp_Value_1 = 0;
//    uint16 Left_Temp_Value_2 = 0;
//    uint8 Right_Temp_Value_1 = 0;
//    uint16 Right_Temp_Value_2 = 0;
//    uint8 L_Average_Thres = 0;
//    uint8 R_Average_Thres = 0;
// 
//    Adaptive_L_Thres_Max = 0;
//    Adaptive_R_Thres_Max = 0;
//    Adaptive_L_Thres_Min = 0;
//    Adaptive_R_Thres_Min = 0;
// 
//    Adaptive_L_Thres_Max = L_Thres_Record[0];
//    Adaptive_L_Thres_Min = L_Thres_Record[0];
//    Adaptive_R_Thres_Max = R_Thres_Record[0];
//    Adaptive_R_Thres_Min = R_Thres_Record[0];
// 
//    for(i = 0; i < Adaptive_L_Statics; i += 2)      //间隔取值即可，减少计算量
//    {
//        if(Adaptive_L_Line[i][0] != 2 && Adaptive_L_Line[i][1] != 2)      //舍去位于黑框上的边线点阈值，“2”即左边线位于黑框上时的X坐标
//        {
//            if(L_Thres_Record[i] < Adaptive_L_Thres_Min)
//            {
//                Adaptive_L_Thres_Min = L_Thres_Record[i];
//            }
//            if(L_Thres_Record[i] > Adaptive_L_Thres_Max)
//            {
//                Adaptive_L_Thres_Max = L_Thres_Record[i];
//            }
//            Left_Temp_Value_1 ++;
//            Left_Temp_Value_2 += L_Thres_Record[i];
//        }
//    }
//	
//    for(i = 0; i < Adaptive_R_Statics; i += 2)
//    {
//        if(Adaptive_R_Line[i][0] != 77 && Adaptive_R_Line[i][1] != 2)       //与左侧同理
//        {
//            if(R_Thres_Record[i] < Adaptive_R_Thres_Min)
//            {
//                Adaptive_R_Thres_Min = R_Thres_Record[i];
//            }
//            if(R_Thres_Record[i] > Adaptive_R_Thres_Max)
//            {
//                Adaptive_R_Thres_Max = R_Thres_Record[i];
//            }
//            Right_Temp_Value_1 ++;
//            Right_Temp_Value_2 += R_Thres_Record[i];
//        }
//    }
// 
//    if(Left_Temp_Value_1 == 0)      //当左侧边线全部位于边界上时，直接将阈值均值取0
//    {
//        L_Average_Thres = 0;
//    }
//    else        //求出左侧阈值均值
//    {
//        L_Average_Thres = (uint8)(Left_Temp_Value_2 / Left_Temp_Value_1);
//    }
// 
//    if(Right_Temp_Value_1 == 0)     //与左侧同理
//    {
//        R_Average_Thres = 0;
//    }
//    else
//    {
//        R_Average_Thres = (uint8)(Right_Temp_Value_2 / Right_Temp_Value_1);
//    }
// 
//    if(Image_Num <= 1)      //前两张图像直接求均值
//    {
//        Last_Adaptive_Thres_Average = (uint8)((L_Average_Thres + R_Average_Thres) / 2);
//    }
//    else
//    {
//        if( My_ABS_uint8(L_Average_Thres - R_Average_Thres) >= 40)       //当两侧边线的阈值均值相差过大时，进一步处理
//        {
//            if( My_ABS_uint8(L_Average_Thres - Last_Adaptive_Thres_Average) <= My_ABS_uint8(R_Average_Thres - Last_Adaptive_Thres_Average))      //选取两侧阈值均值最接近上次图像阈值均值的值作为此次的值
//            {
//                Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + L_Average_Thres) / 2);
//                Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
//            }
//            else
//            {
//                Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + R_Average_Thres) / 2);
//                Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
//            }
//        }
//        else    //当两侧阈值均值相差不大时，直接求三者均值
//        {
//            Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + L_Average_Thres + R_Average_Thres) / 3);
//            Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
//        }
//    }
// 
//    //获得判断斑马线的阈值
//   // Zbra_Thres = Adaptive_Thres_Average - 10;
// 
//    //获得起始点差比和的阈值
//    if(Adaptive_Thres_Average >= 100 && Adaptive_Thres_Average <= 140)      //阈值均值落在100 - 140之间，说明图像亮度很合适，此时将差比和阈值设为20即可
//    {
//        Compare_Value = 20;
//    }
//    else if(Adaptive_Thres_Average < 100)
//    {
//        Compare_Value = 20 - (uint8)(((100 - Adaptive_Thres_Average) * 10) / 60 );      //当小于100时，适当拉低差比和阈值，使其在图像较暗的情况下更容易找出赛道边线
//    }
//    else if(Adaptive_Thres_Average > 140)
//    {
//        Compare_Value = 20 - (uint8)(((Adaptive_Thres_Average - 140) * 10 ) / 60 );      //当大于140时，适当拉低差比和阈值，使其在图像较亮的情况下更容易找出赛道边线
//    }
//    //求黑框灰度值
//    Black_Box_Value = (uint8)(9 * (uint8)sqrt(Adaptive_L_Thres_Min * Adaptive_L_Thres_Min + Adaptive_R_Thres_Min * Adaptive_R_Thres_Min)) / 20 + (uint8)(uint8)Black_Box_Value_1 / 10;      
//    //0.45f即0.9 * 0.5，0.9为权重，0.5可自己调节。Black_Box_Value_1为两侧爬线起点的灰度值均值（起点值灰度值为图像中赛道黑线的灰度值，不可为黑框灰度值，可以自己写算法处理下）
//    //或者Black_Box_Value_1直接丢20 ~ 50之间的值即可
//	
//}


uint8 Adaptive_L_Thres_Max = 0;     //左侧阈值最大值
uint8 Adaptive_R_Thres_Max = 0;     //右侧阈值最大值
uint8 Adaptive_L_Thres_Min = 0;     //左侧阈值最小值
uint8 Adaptive_R_Thres_Min = 0;     //右侧阈值最小值
uint8 Adaptive_Thres_Average = 0;   //阈值均值
uint8 Last_Adaptive_Thres_Average = 0;  //用于阈值均值滤波
/**
* 函数功能：      提取阈值中的最大最小值,并求出阈值均值
* 特殊说明：      计算时间小于5us
* 形  参：        无
*
* 示例：          Thres_Record_Process（）；
* 返回值：        无
*/
void Thres_Record_Process(void)
{
    uint8 i = 0;
    uint8 Left_Temp_Value_1 = 0;
    uint32 Left_Temp_Value_2 = 0;
    uint8 Right_Temp_Value_1 = 0;
    uint32 Right_Temp_Value_2 = 0;
    uint8 L_Average_Thres = 0;
    uint8 R_Average_Thres = 0;
 
    Adaptive_L_Thres_Max = 0;
    Adaptive_R_Thres_Max = 0;
    Adaptive_L_Thres_Min = 0;
    Adaptive_R_Thres_Min = 0;
 
    Adaptive_L_Thres_Max = L_Thres_Record[0];
    Adaptive_L_Thres_Min = L_Thres_Record[0];
    Adaptive_R_Thres_Max = R_Thres_Record[0];
    Adaptive_R_Thres_Min = R_Thres_Record[0];
 
    for(i = 0; i < Adaptive_L_Statics; i += 2)      //间隔取值即可，减少计算量
    {
        if(Adaptive_L_Line[i][0] != 2 && Adaptive_L_Line[i][1] != 2)      //舍去位于黑框上的边线点阈值，“2”即左边线位于黑框上时的X坐标
        {
            if(L_Thres_Record[i] < Adaptive_L_Thres_Min)
            {
                Adaptive_L_Thres_Min = L_Thres_Record[i];
            }
            if(L_Thres_Record[i] > Adaptive_L_Thres_Max)
            {
                Adaptive_L_Thres_Max = L_Thres_Record[i];
            }
            Left_Temp_Value_1 ++;
            Left_Temp_Value_2 += L_Thres_Record[i];
        }
    }
    for(i = 0; i < Adaptive_R_Statics; i += 2)
    {
        if(Adaptive_R_Line[i][0] != 77 && Adaptive_R_Line[i][1] != 2)       //与左侧同理
        {
            if(R_Thres_Record[i] < Adaptive_R_Thres_Min)
            {
                Adaptive_R_Thres_Min = R_Thres_Record[i];
            }
            if(R_Thres_Record[i] > Adaptive_R_Thres_Max)
            {
                Adaptive_R_Thres_Max = R_Thres_Record[i];
            }
            Right_Temp_Value_1 ++;
            Right_Temp_Value_2 += R_Thres_Record[i];
        }
    }
 
    if(Left_Temp_Value_1 == 0)      //当左侧边线全部位于边界上时，直接将阈值均值取0
    {
        L_Average_Thres = 0;
    }
    else        //求出左侧阈值均值
    {
        L_Average_Thres = (uint8)(Left_Temp_Value_2 / Left_Temp_Value_1);
    }
 
    if(Right_Temp_Value_1 == 0)     //与左侧同理
    {
        R_Average_Thres = 0;
    }
    else
    {
        R_Average_Thres = (uint8)(Right_Temp_Value_2 / Right_Temp_Value_1);
    }
 
    if(Image_Num <= 1)      //前两张图像直接求均值
    {
        Last_Adaptive_Thres_Average = (uint8)((L_Average_Thres + R_Average_Thres) / 2);
    }
    else
    {
        if( My_ABS_uint8(L_Average_Thres - R_Average_Thres) >= 40)       //当两侧边线的阈值均值相差过大时，进一步处理
        {
            if( My_ABS_uint8(L_Average_Thres - Last_Adaptive_Thres_Average) <= My_ABS_uint8(R_Average_Thres - Last_Adaptive_Thres_Average))      //选取两侧阈值均值最接近上次图像阈值均值的值作为此次的值
            {
                Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + L_Average_Thres) / 2);
                Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
            }
            else
            {
                Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + R_Average_Thres) / 2);
                Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
            }
        }
        else    //当两侧阈值均值相差不大时，直接求三者均值
        {
            Adaptive_Thres_Average = (uint8)((Last_Adaptive_Thres_Average + L_Average_Thres + R_Average_Thres) / 3);
            Last_Adaptive_Thres_Average = Adaptive_Thres_Average;
        }
    }
 
    //获得判断斑马线的阈值
   // Zbra_Thres = Adaptive_Thres_Average - 10;
 
    //获得起始点差比和的阈值
    if(Adaptive_Thres_Average >= 100 && Adaptive_Thres_Average <= 140)      //阈值均值落在100 - 140之间，说明图像亮度很合适，此时将差比和阈值设为20即可
    {
        Compare_Value = 15;   // 保持原始值，避免噪声导致边线提取异常
    }
    else if(Adaptive_Thres_Average < 100)
    {
        Compare_Value = 20 - (uint8)(((float)(100 - Adaptive_Thres_Average) / 60.0f) * 10.0f);      //当小于100时，适当拉低差比和阈值，使其在图像较暗的情况下更容易找出赛道边线
    }
    else if(Adaptive_Thres_Average > 140)
    {
        Compare_Value = 20 - (uint8)(((float)(Adaptive_Thres_Average - 140) / 60.0f) * 10.0f);      //当大于140时，适当拉低差比和阈值，使其在图像较亮的情况下更容易找出赛道边线
    }
    //求黑框灰度值
    //    Black_Box_Value = (uint8)(0.45f * (float)sqrt(Adaptive_L_Thres_Min * Adaptive_L_Thres_Min + Adaptive_R_Thres_Min * Adaptive_R_Thres_Min)) + (uint8)(0.1f * (float)Black_Box_Value_1);
    // Black_Box_Value = (uint8)(0.45f * (float)Fast_Sqrt16((uint16)(Adaptive_L_Thres_Min * Adaptive_L_Thres_Min + Adaptive_R_Thres_Min * Adaptive_R_Thres_Min))) + (uint8)(0.1f * (float)Black_Box_Value_1);      
    //0.45f即0.9 * 0.5，0.9为权重，0.5可自己调节。Black_Box_Value_1为两侧爬线起点的灰度值均值（起点值灰度值为图像中赛道黑线的灰度值，不可为黑框灰度值，可以自己写算法处理下）
    //或者Black_Box_Value_1直接丢20 ~ 50之间的值即可
}

void Process_Image(void)
{

    Adaptive_L_Statics = 0;
    Adaptive_R_Statics = 0;

    		Copy_Zip_Image( );  //ѹ��ͼ���ɣ�188��120��ѹ��Ϊ��80 ��60��
			
			Draw_Black_Box(Black_Box_Value, Find_Line_Image);  //ͼ���ⲿ���ڿ�����һ�б߽磩
			
			if(Get_Start_Point(Image_Y - 3, Find_Line_Image, Adaptive_L_Start_Point, Adaptive_R_Start_Point, 2, 77) ==1 )
			{
//				
				Dir_Labyrinth_5( (uint8)150, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
						Adaptive_L_Start_Point[0],Adaptive_L_Start_Point[1],Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], Clip_Value );
//										

//				
//				if( (( Adaptive_L_Statics <= 15 && Adaptive_R_Statics - Adaptive_L_Statics >= 20 ) || ( Adaptive_R_Statics <= 15 && Adaptive_L_Statics - Adaptive_R_Statics >= 20 )) )
//				{
//					Adaptive_L_Statics = 0;
//					Adaptive_R_Statics = 0;
//					if(Get_Start_Point(47, Find_Line_Image, Adaptive_L_Start_Point, Adaptive_R_Start_Point, 2, 77) == 1)
//					{

//						Dir_Labyrinth_5((uint8)150, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
//						          Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1], Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], Clip_Value);
////						
//					}
//				}
				
				if( (( Adaptive_L_Statics <= 15 && Adaptive_R_Statics - Adaptive_L_Statics >= 20 ) || ( Adaptive_R_Statics <= 15 && Adaptive_L_Statics - Adaptive_R_Statics >= 20 )) )
				{
						// ===== 保存第一次扫描的底部数据（Y >= 48 的点）=====
						uint16 saved_L_bottom = 0;
						uint16 saved_R_bottom = 0;
						uint16 idx;
						uint16 k;

						// 找出左侧 Y >= 48 的底部点个数
						// Adaptive_L_Line 从 index 0 开始是从下往上存储的（Y 递减）
						for(idx = 0; idx < Adaptive_L_Statics; idx++)
						{
								if(Adaptive_L_Line[idx][1] >= 48)
										saved_L_bottom = idx + 1;
								else
										break;
						}
						// 找出右侧 Y >= 48 的底部点个数
						for(idx = 0; idx < Adaptive_R_Statics; idx++)
						{
								if(Adaptive_R_Line[idx][1] >= 48)
										saved_R_bottom = idx + 1;
								else
										break;
						}

						// 尝试从 row 47 重扫
						if(Get_Start_Point(47, Find_Line_Image, Adaptive_L_Start_Point,
															 Adaptive_R_Start_Point, 2, 77) == 1)
						{
								// 把第一次扫描的底部数据暂存到临时数组
								uint8 temp_L_line[15][2];
								uint8 temp_R_line[15][2];
								int8  temp_L_dir[15];
								int8  temp_R_dir[15];

								for(k = 0; k < saved_L_bottom; k++)
								{
										temp_L_line[k][0] = Adaptive_L_Line[k][0];
										temp_L_line[k][1] = Adaptive_L_Line[k][1];
										temp_L_dir[k]     = Adaptive_L_Grow_Dir[k];
								}
								for(k = 0; k < saved_R_bottom; k++)
								{
										temp_R_line[k][0] = Adaptive_R_Line[k][0];
										temp_R_line[k][1] = Adaptive_R_Line[k][1];
										temp_R_dir[k]     = Adaptive_R_Grow_Dir[k];
								}

								// 清零并重扫（重扫覆盖 Adaptive_L_Line[0..M]，row 47→顶部）
								Adaptive_L_Statics = 0;
								Adaptive_R_Statics = 0;
								Dir_Labyrinth_5((uint8)150, Find_Line_Image, Adaptive_L_Line,
										Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir,
										&Adaptive_L_Statics, &Adaptive_R_Statics,
										&Adaptive_X_Meet, &Adaptive_Y_Meet,
										Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1],
										Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], Clip_Value);

								// 把保存的底部数据追加到重扫数据后面
								// Get_Border 按 Y 坐标自行整理每行的最佳值，追加顺序不影响结果
								for(k = 0; k < saved_L_bottom; k++)
								{
										Adaptive_L_Line[Adaptive_L_Statics][0] = temp_L_line[k][0];
										Adaptive_L_Line[Adaptive_L_Statics][1] = temp_L_line[k][1];
										Adaptive_L_Grow_Dir[Adaptive_L_Statics]  = temp_L_dir[k];
										Adaptive_L_Statics++;
								}
								for(k = 0; k < saved_R_bottom; k++)
								{
										Adaptive_R_Line[Adaptive_R_Statics][0] = temp_R_line[k][0];
										Adaptive_R_Line[Adaptive_R_Statics][1] = temp_R_line[k][1];
										Adaptive_R_Grow_Dir[Adaptive_R_Statics]  = temp_R_dir[k];
										Adaptive_R_Statics++;
								}
						}
						// 如果 Get_Start_Point(47) 失败 → 第一次扫描数据保留不变
						// （statics 没被清零，Adaptive_L_Line 没被覆盖）
				}

				
			}
					
//			Thres_Record_Process( );  //  &&&&&����&&&&&

			Get_Border(Adaptive_L_Statics, Adaptive_R_Statics, Image_Y - 3, 2, L_Border, R_Border, Adaptive_L_Line, Adaptive_R_Line);//��ȡһά����
}

void photo_send(void)
{
	uint8 j = 0;
	uint8 i_photo_send = 0; 
	
			memset(xy_x1_boundary, 0, BOUNDARY_NUM);
			memset(xy_y1_boundary, 0, BOUNDARY_NUM);
			memset(xy_x2_boundary, 0, BOUNDARY_NUM);
			memset(xy_y2_boundary, 0, BOUNDARY_NUM);
			memset(xy_x3_boundary, 0, BOUNDARY_NUM);
			memset(xy_y3_boundary, 0, BOUNDARY_NUM);
			
	
	
	for( i_photo_send = 0 ; i_photo_send < Adaptive_L_Statics ; i_photo_send++)
		 	{
		 		xy_x1_boundary[i_photo_send] = Adaptive_L_Line[i_photo_send][0];
		 		xy_y1_boundary[i_photo_send] = Adaptive_L_Line[i_photo_send][1];
		 	}
	
		 	//右线
		 	for( i_photo_send = 0 ; i_photo_send < Adaptive_R_Statics ; i_photo_send++)
		 	{
		 		xy_x2_boundary[i_photo_send] = Adaptive_R_Line[i_photo_send][0];
         		xy_y2_boundary[i_photo_send] = Adaptive_R_Line[i_photo_send][1];
		 	}
	
		 //中线
		 	for( i_photo_send = 2 ; i_photo_send < Image_Y - 5 ; i_photo_send++ )
		 	{
		 		xy_x3_boundary[j] = C_Line[i_photo_send];
		 		xy_y3_boundary[j] = i_photo_send;
		 		j++;
		 	}
		 	j=0;

              memcpy(image_copy[0], Find_Line_Image[0], Image_X * Image_Y);

            // // // 发送图像
              seekfree_assistant_camera_send();
}

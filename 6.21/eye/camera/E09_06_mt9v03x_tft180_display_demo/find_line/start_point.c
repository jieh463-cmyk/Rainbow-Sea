#include "file_h_all.h"

//*************************差比和*************************
/**
* 函数功能：      差比和
* 特殊说明：      用于爬线算法找起点
* 形  参：        int16 a                  数值较大的灰度值
*                int16 b                   数值较小的灰度值
*                uint8 compare_value       差比和阈值
*
* 示例：          Compare_Num(image[start_row][i + 5], image[start_row][i], Compare_Value)；
* 返回值：        大于阈值返回1，否则返回0.
*/
//int16 Compare_Num(int16 a, int16 b, uint8 compare_value)            
//{
//    if( ( ( (a - b) << 7) / (a + b)) > compare_value )
//    {
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
//}
//**************************************************************


//*************************找起点（差比和）*************************
/**
* 函数功能：      爬线算法找起点
* 特殊说明：      无
* 形  参：        uint8 start_row              找起点的图像行Y坐标
*                uint8(*image)[Image_X]        要处理的图像
*                uint8 *l_start_point          存储左侧起始点的数组（全局变量）
*                uint8 *r_start_point          存储右侧起始点的数组（全局变量）
*                uint8 l_border_x              向左找起点的截止点，最远找到这里就停止
*                uint8 r_border_x              向右找起点的截止点，最远找到这里就停止
*
* 示例：          Get_Start_Point(Image_Y - 3, Find_Line_Image, Adaptive_L_Start_Point, Adaptive_R_Start_Point, 1, 78)
* 返回值：        两边都找到返回1，否则返回0.
*/

uint8 Start_Flag = 0;
uint8 Compare_Value = 40;   // 原始值，Thres_Record_Process会动态覆写
uint8 Black_Box_Value_1 = 0;
uint8 Get_Start_Point(uint8 start_row, uint8(*image)[Image_X], uint8 *l_start_point, uint8 *r_start_point, uint8 l_border_x, uint8 r_border_x)//*****
{
    uint8 i = 0, j = 0;
    uint8 L_Is_Found = 0, R_Is_Found = 0;   //找到起点时挂出对应标志位
    uint8 Start_X  = 0;                     //起始X坐标，第一张图像取图像的行中点，后续图像用上一次图像左右两侧起始点的中间值
    uint8 Start_Row_0 = 0;                  //起始Y坐标
 
    Start_Row_0 = start_row;
    Start_X = Image_X / 2;
    //从中间往左边，先找起点
    for(j = 0; j < 10; j ++)        //指定的行没找到起点时，向上走一行继续找，最多找十行
    {
        l_start_point[1] = start_row;//y
        r_start_point[1] = start_row;//y
 
        if(Start_Flag == 0 )       //第一张图像和遇到斑马线时，起始X坐标选用图像的行中点
        {
            Start_X = Image_X / 2;
        }
        else
        {
            Start_X = (l_start_point[0] + r_start_point[0]) / 2;    //否则起始X坐标用上一次图像左右两侧起始点的中间值
        }
		if(!ring_shot_flag)
        {
            for (i = Start_X - 7; i > l_border_x - 1; i--)      //向左找起始点
            {
				if (Compare_Num(image[start_row][i+5], image[start_row][i], Compare_Value))
				{
						l_start_point[0] = i; //X指
						L_Is_Found = 1;
						break;
				}
            }
 
            for (i = Start_X + 7; i < r_border_x + 1; i++)      //向右找起始点
            {
				if (Compare_Num(image[start_row][i - 5], image[start_row][i], Compare_Value))
				{
						r_start_point[0] = i;
						R_Is_Found = 1;
						break;
				}
            }
            if(L_Is_Found && R_Is_Found)
            {
                Start_Flag = 1;    //是否为第一张图像标志位

				
                return 1;
            }
            else
            {
                start_row = start_row - 1;      //当此行有一侧没找到，就向上移动一行重新找
            }
        }
		else
		{
			for (i = Start_X - 25; i > l_border_x - 1; i--)      //向左找起始点
            {
				if (Compare_Num(image[start_row][i+5], image[start_row][i], Compare_Value))
				{
						l_start_point[0] = i; //X指
						L_Is_Found = 1;
						break;
				}
            }
 
            for (i = Start_X + 25; i < r_border_x + 1; i++)      //向右找起始点
            {
				if (Compare_Num(image[start_row][i - 5], image[start_row][i], Compare_Value))
				{
						r_start_point[0] = i;
						R_Is_Found = 1;
						break;
				}
            }
            if(L_Is_Found && R_Is_Found)
            {
                Start_Flag = 1;    //是否为第一张图像标志位

				
                return 1;
            }
            else
            {
                start_row = start_row - 1;      //当此行有一侧没找到，就向上移动一行重新找
            }
		}
    }

}


#include "file_h_all.h"

//*************************************************************************提取信息，判断元素**********************************************************************************
/**
* 函数功能：      最小二乘法计算斜率
* 特殊说明：      无
* 形  参：        uint8 begin                输入起点
*                 uint8 end                  输入终点
*                 uint8 *border              输入需要计算斜率的一维边线数组
* 示例：          Slope_Calculate(start, end, border);
* 返回值：        Result    计算出的斜率*100，int16
*/
int16 Slope_Calculate(uint8 begin, uint8 end, uint8 *border)    //注：begin 必须小于 end,一般 begin 位于图像上方， end 位于图像下方
{
    int32 X_Sum = 0, Y_Sum = 0, XY_Sum = 0, X2_Sum = 0;
    int16 i = 0;
    int16 Result = 0;
    static int16 Result_Last;
	int32 Denominator;
    if(begin >= end)
    {
        return Result_Last;
    }

    for(i = begin; i < end ; i++)
    {
        X_Sum += i;
        Y_Sum += border[i];
        XY_Sum += i * border[i];
        X2_Sum += i * i;
    }

     Denominator = (int32)(end - begin) * X2_Sum - X_Sum * X_Sum;
    if(Denominator != 0)      //防止为0的情况出现
    {
        int32 Numerator = (int32)(end - begin) * XY_Sum - X_Sum * Y_Sum;
        Result = (int16)( (Numerator * 100) / Denominator );
        Result_Last = Result;
    }
    else
    {
        Result = Result_Last;
    }
    return Result;
}

/**
* @brief 计算斜率截距
* @param uint8 start                输入起点
* @param uint8 end                  输入终点
* @param uint8 *border              输入需要计算斜率的边界
* @param int16 *slope_rate          输入斜率地址 *100
* @param int16 *intercept           输入截距地址
*  @see CTest       calculate_s_i(start, end, r_border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/

/**
* 函数功能：      计算斜率截距
* 特殊说明：      调用最小二乘法计算斜率
* 形  参：        uint8 start                输入起点
*                 uint8 end                  输入终点
*                 uint8 *border              输入需要计算斜率的一维边线数组
*                 int16 *slope_rate          存储斜率的变量地址 *100
*                 int16 *intercept           存储截距的变量地址
* 示例：          Calculate_Slope_Intercept(start, end, L_Border, &L_Straightaway_Lope_Rate_C, &L_Intercept);
* 返回值：        无
*/
void Calculate_Slope_Intercept(uint8 start, uint8 end, uint8 *border, int16 *slope_rate, int16 *intercept)
{
    uint16 i, Num = 0;
    int32 X_Sum = 0, Y_Sum = 0;
    int16 X_Average = 0, Y_Average = 0;

    for(i = start; i < end; i++)
    {
        X_Sum += i;
        Y_Sum += border[i];
        Num ++;
    }

    if(Num)
    {
        X_Average = (int16)(X_Sum / Num);
        Y_Average = (int16)(Y_Sum / Num);
    }

    *slope_rate = Slope_Calculate(start, end, border);
    *intercept = Y_Average - (int16)((*slope_rate * X_Average)/100);
}

uint8 Straight_Thres = 15;
uint8 Sml_Arc_Thres = 10;
uint8 Big_Arc_thres = 130;

//求方差
//uint8 Num = 26;
void Get_Variance(uint8 start, uint8 end, int16 *fitting_line, uint8 *actual_line, uint16 *variance, uint8 multiple)
{
    uint8 i = 0;
    uint32 S = 0;
    uint8 total_num = 0;
    int16 Temp = 0;

    if(start % 3 != 0)
    {
        for(i = 1; i < 3; i++)
        {
            start = start + i;
            if(start % 3 == 1)
            {
                break;
            }
        }
    }

    total_num = (uint8)((end - start) / multiple);

    for(i = 0; i < total_num; i++)
    {
        uint8 Temp_Value = 0;
        Temp_Value = i * multiple + start;
        Temp = (int16)actual_line[Temp_Value] - fitting_line[Temp_Value / 3];
        S += Temp * Temp;
    }
    if(total_num > 0)
    {
        *variance = (uint16)(S / total_num);
    }
    else
    {
        *variance = 0;
    }
}

/**
* 函数功能：      寻找上转左拐点
* 特殊说明：      分为三个严格等级，根据每个点的生长方向判断
*                 寻找方式为遍历二维数组边线的每个点
* 形  参：        uint8 Grade      选择判定严格等级，常用2，即中等等级
*
* 示例：          Get_L_Up_Turn_Left_Point_1(2);
* 返回值：        无
*/
uint8 L_Up_Turn_Left_Point_1[2] = {0};      //存储上转左拐点的坐标
uint8 L_Up_Turn_Left_Point_Flag_1 = 0;      //上转左拐点存在标志位，找到时置1
uint8 L_Up_Turn_Left_Point_Position_1 = 0;  //记录位置，即是二维数组中的第几个点
int16 L_Up_Turn_Left_Point_Angle_1 = 0;     //记录找到的拐点的角度（逆透视求取）
void Get_L_Up_Turn_Left_Point_1(uint8 Grade)
{
    uint8 i = 0;
    L_Up_Turn_Left_Point_Flag_1 = 0;
    L_Up_Turn_Left_Point_1[0] = 0;
    L_Up_Turn_Left_Point_1[1] = 0;
    L_Up_Turn_Left_Point_Position_1 = 0;
    L_Up_Turn_Left_Point_Angle_1 = 0;       //将各个参数清零

    if(Adaptive_L_Statics < 8)
    {
        return;
    }

    switch(Grade)
    {
        case 1:     //严格判断
        {
            for (i = 4; i < (Adaptive_L_Statics - 4); i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1) &&
                        (Adaptive_L_Grow_Dir[i + 2] == -3) && (Adaptive_L_Grow_Dir[i + 4] == -3) && (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3))
                    {
                        L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Up_Turn_Left_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 2:     //中等判断
        {
            for (i = 4; i < (Adaptive_L_Statics - 4); i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1) &&
                        (Adaptive_L_Grow_Dir[i + 2] == -3 || Adaptive_L_Grow_Dir[i + 2] == -4) && (Adaptive_L_Grow_Dir[i + 4] == -3 || Adaptive_L_Grow_Dir[i + 4] == -4) &&
                        (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == -4))
                    {
                        L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Up_Turn_Left_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 3:     //宽松判断
        {
            for (i = 4; i < (Adaptive_L_Statics - 4); i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1 || Adaptive_L_Grow_Dir[i - 2] == -2) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1 || Adaptive_L_Grow_Dir[i - 2] == -2) &&
                        (Adaptive_L_Grow_Dir[i + 2] == -3 || Adaptive_L_Grow_Dir[i + 2] == -4 || Adaptive_L_Grow_Dir[i + 2] == -2) && (Adaptive_L_Grow_Dir[i + 4] == -3 || Adaptive_L_Grow_Dir[i + 4] == -4 || Adaptive_L_Grow_Dir[i + 4] == -2) &&
                        (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == -4))
                    {
                        L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Up_Turn_Left_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
    }
//    if(L_Up_Turn_Left_Point_Flag_1 == 1)
//    {
//        L_Up_Turn_Left_Point_Position_1 = i;    //记录拐点的位置
//        L_Up_Turn_Left_Point_Angle_1 = Get_Turn_Point_Angle(Adaptive_L_Line[i - 5][0], Adaptive_L_Line[i - 5][1], Adaptive_L_Line[i][0], Adaptive_L_Line[i][1], Adaptive_L_Line[i + 5][0], Adaptive_L_Line[i + 5][1]);    //逆透视求取拐点角度，后续补充
//    }
}

/**
* 函数功能：      寻找上转左拐点
* 特殊说明：      通过检测X值的跳变判定拐点的存在
*                 寻找方式为遍历一维数组边线的每个点
* 形  参：        uint8 Region_Start      起始行
*                 uint8 Region_End        截止行
*
* 示例：          Get_L_Up_Turn_Left_Point_2(2, 57);
* 返回值：        无
*/
uint8 L_Up_Turn_Left_Point_2[2] = {0};
uint8 L_Up_Turn_Left_Point_Flag_2 = 0;
uint8 L_Up_Turn_Left_Point_Position_2 = 0;
void Get_L_Up_Turn_Left_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    L_Up_Turn_Left_Point_2[0] = 0;
    L_Up_Turn_Left_Point_2[1] = 0;
    L_Up_Turn_Left_Point_Position_2 = 0;
    if(Region_Start <= Region_End && Region_Start < 2)
    {
        Region_Start = 2;
    }
    if(Region_Start <= Region_End)
    {
        for(i = Region_Start; i <= Region_End; i++)
        {
            if(L_Border[i] - L_Border[i - 1] >= 6)
            {
                if(L_Border[i + 1] - L_Border[i - 2] >= 6)
                {
                    if(L_Border[i - 1] == 2 && L_Border[i - 2] == 2 && L_Border[i] != 2)
                    {
                        L_Up_Turn_Left_Point_2[0] = L_Border[i];
                        L_Up_Turn_Left_Point_2[1] = i;
                        L_Up_Turn_Left_Point_Flag_2 = 1;
                        L_Up_Turn_Left_Point_Position_2 = i;
                    }
                }
            }
        }
    }
}
uint8 L_Up_Turn_Left_Point[2] = {0};
uint8 L_Up_Turn_Left_Point_Flag = 0;
void Get_L_Up_Turn_Left_Point(void)
{
    L_Up_Turn_Left_Point_Flag = 0;
    Get_L_Up_Turn_Left_Point_1(2);
    if(L_Up_Turn_Left_Point_Flag_1 == 1 && L_Up_Turn_Left_Point_1[1] >= 7)
    {
        L_Up_Turn_Left_Point_Flag_1 = 0;
        Get_L_Up_Turn_Left_Point_2(L_Up_Turn_Left_Point_1[1] - 7, L_Up_Turn_Left_Point_1[1] + 7);
        {
            if(L_Up_Turn_Left_Point_Flag_2 == 1)
            {
                L_Up_Turn_Left_Point_Flag_2 = 0;
                L_Up_Turn_Left_Point_Flag = 1;
                L_Up_Turn_Left_Point[0] = L_Up_Turn_Left_Point_1[0];
                L_Up_Turn_Left_Point[1] = L_Up_Turn_Left_Point_1[1];
            }
        }
    }
}

/**
* 函数功能：      寻找右转上拐点
* 特殊说明：      分为三个严格等级，根据每个点的生长方向判断
*                 寻找方式为遍历二维数组边线的每个点
* 形  参：        uint8 Grade      选择判定严格等级，常用2，即中等等级
*
* 示例：          Get_L_Right_Turn_Up_Point_1(2);
* 返回值：        无
*/
uint8 L_Right_Turn_Up_Point_1[2] = {0};
uint8 L_Right_Turn_Up_Point_Flag_1 = 0;
uint8 L_Right_Turn_Up_Point_Position_1 = 0;
int16 L_Right_Turn_Up_Point_Angle_1 = 0;
void Get_L_Right_Turn_Up_Point_1(uint8 Grade)
{
    uint8 i = 0;
    L_Right_Turn_Up_Point_Flag_1 = 0;
    L_Right_Turn_Up_Point_1[0] = 0;
    L_Right_Turn_Up_Point_1[1] = 0;
    L_Right_Turn_Up_Point_Position_1 = 0;
    L_Right_Turn_Up_Point_Angle_1 = 0;

    if(Adaptive_L_Statics < 12)
    {
        return;
    }

    switch(Grade)
    {
        case 1:
        {
            for (i = 5; (i + 7) < Adaptive_L_Statics; i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1) && (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1) &&
                        (Adaptive_L_Grow_Dir[i - 2] == 3) && (Adaptive_L_Grow_Dir[i - 5] == 3) && (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                    {
                        L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Right_Turn_Up_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 2:
        {
            for (i = 5; (i + 7) < Adaptive_L_Statics; i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1) && (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1) &&
                        (Adaptive_L_Grow_Dir[i - 2] == 2 || Adaptive_L_Grow_Dir[i - 2] == 3) && (Adaptive_L_Grow_Dir[i - 5] == 2 || Adaptive_L_Grow_Dir[i - 5] == 3) &&
                        (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                    {
                        L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Right_Turn_Up_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 3:
        {
            for (i = 5; (i + 7) < Adaptive_L_Statics; i++)
            {
                if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1 || Adaptive_L_Grow_Dir[i + 2] == -2) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1 || Adaptive_L_Grow_Dir[i + 5] == -2) &&
                        (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1 || Adaptive_L_Grow_Dir[i + 7] == -2) &&
                        (Adaptive_L_Grow_Dir[i - 2] == 2 || Adaptive_L_Grow_Dir[i - 2] == 3 || Adaptive_L_Grow_Dir[i - 2] == 4) && (Adaptive_L_Grow_Dir[i - 5] == 2 || Adaptive_L_Grow_Dir[i - 5] == 3 || Adaptive_L_Grow_Dir[i - 5] == 4) &&
                        (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                    {
                        L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                        L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                        L_Right_Turn_Up_Point_Flag_1  = 1;
                        break;
                    }
                }
            }
            break;
        }
    }
//    if(L_Right_Turn_Up_Point_Flag_1 == 1)
//    {
//        L_Right_Turn_Up_Point_Position_1 = i;
//        L_Right_Turn_Up_Point_Angle_1 = Get_Turn_Point_Angle(Adaptive_L_Line[i - 5][0], Adaptive_L_Line[i - 5][1], Adaptive_L_Line[i][0], Adaptive_L_Line[i][1], Adaptive_L_Line[i + 5][0], Adaptive_L_Line[i + 5][1]);
//    }
}

/**
* 函数功能：      寻找右转上拐点
* 特殊说明：      通过检测X值的跳变判定拐点的存在
*                 寻找方式为遍历一维数组边线的每个点
* 形  参：        uint8 Region_Start      起始行
*                 uint8 Region_End        截止行
*
* 示例：          Get_L_Right_Turn_Up_Point_2(2, 57);
* 返回值：        无
*/
uint8 L_Right_Turn_Up_Point_2[2] = {0};
uint8 L_Right_Turn_Up_Point_Flag_2 = 0;
uint8 L_Right_Turn_Up_Point_Position_2 = 0;
void Get_L_Right_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    L_Right_Turn_Up_Point_2[0] = 0;
    L_Right_Turn_Up_Point_2[1] = 0;
    L_Right_Turn_Up_Point_Position_2 = 0;
    if(Region_Start <= Region_End && Region_Start < 2)
    {
        Region_Start = 2;
    }
    if(Region_Start <= Region_End)
    {
        for(i = Region_Start; i <= Region_End; i++)
        {
            if(L_Border[i] - L_Border[i + 1] >= 6)
            {
                if(L_Border[i - 1] - L_Border[i + 2] >= 6)
                {
                    if(L_Border[i + 1] == 2 && L_Border[i + 2] == 2 && L_Border[i] != 2)
                    {
                        L_Right_Turn_Up_Point_2[0] = L_Border[i];
                        L_Right_Turn_Up_Point_2[1] = i;
                        L_Right_Turn_Up_Point_Flag_2 = 1;
                        L_Right_Turn_Up_Point_Position_2 = i;
                    }
                }
            }
        }
    }
}
uint8 L_Right_Turn_Up_Point[2] = {0};
uint8 L_Right_Turn_Up_Point_Flag = 0;
void Get_L_Right_Turn_Up_Point(void)
{
    L_Right_Turn_Up_Point_Flag = 0;
    Get_L_Right_Turn_Up_Point_1(2);
    if(L_Right_Turn_Up_Point_Flag_1 == 1 && L_Right_Turn_Up_Point_1[1] >= 7)
    {
        L_Right_Turn_Up_Point_Flag_1 = 0;
        Get_L_Right_Turn_Up_Point_2(L_Right_Turn_Up_Point_1[1] - 7, L_Right_Turn_Up_Point_1[1] + 7);
        {
            if(L_Right_Turn_Up_Point_Flag_2 == 1)
            {
                L_Right_Turn_Up_Point_Flag_2 = 0;
                L_Right_Turn_Up_Point_Flag = 1;
                L_Right_Turn_Up_Point[0] = L_Right_Turn_Up_Point_1[0];
                L_Right_Turn_Up_Point[1] = L_Right_Turn_Up_Point_1[1];
            }
        }
    }
}

/**
* 函数功能：      寻找上转右拐点
* 特殊说明：     分为三个严格等级，根据每个点的生长方向判断
*                 寻找方式为遍历二维数组边线的每个点
* 形  参：        uint8 Grade      选择判定严格等级，常用2，即中等等级
*
* 示例：          Get_R_Up_Turn_Right_Point_1(2);
* 返回值：        无
*/
uint8 R_Up_Turn_Right_Point_1[2] = {0};
uint8 R_Up_Turn_Right_Point_Flag_1 = 0;
uint8 R_Up_Turn_Right_Point_Position_1 = 0;
int16 R_Up_Turn_Right_Point_Angle_1 = 0;
void Get_R_Up_Turn_Right_Point_1(uint8 Grade)
{
    uint8 i = 0;
    R_Up_Turn_Right_Point_Flag_1 = 0;
    R_Up_Turn_Right_Point_1[0] = 0;
    R_Up_Turn_Right_Point_1[1] = 0;
    R_Up_Turn_Right_Point_Position_1 = 0;
    R_Up_Turn_Right_Point_Angle_1 = 0;

    if(Adaptive_R_Statics < 8)
    {
        return;
    }

    switch(Grade)
    {
        case 1:
        {
            for (i = 4; i < (Adaptive_R_Statics - 4); i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i - 2] == -2 || Adaptive_R_Grow_Dir[i - 2] == 1) && (Adaptive_R_Grow_Dir[i - 4] == -2 || Adaptive_R_Grow_Dir[i - 4] == 1) &&
                        (Adaptive_R_Grow_Dir[i + 2] == 3) && (Adaptive_R_Grow_Dir[i + 4] == 3) && (Adaptive_R_Grow_Dir[i] == 2 || Adaptive_R_Grow_Dir[i] == 3 || Adaptive_R_Grow_Dir[i] == 4))
                    {
                        R_Up_Turn_Right_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Up_Turn_Right_Point_1[1] = Adaptive_R_Line[i][1];
                        R_Up_Turn_Right_Point_Flag_1 = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 2:
        {
            for (i = 4; i < (Adaptive_R_Statics - 4); i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i - 2] == -2 || Adaptive_R_Grow_Dir[i - 2] == 1) && (Adaptive_R_Grow_Dir[i - 4] == -2 || Adaptive_R_Grow_Dir[i - 4] == 1) &&
                        (Adaptive_R_Grow_Dir[i + 2] == 2 || Adaptive_R_Grow_Dir[i + 2] == 3) && (Adaptive_R_Grow_Dir[i + 4] == 2 || Adaptive_R_Grow_Dir[i + 4] == 3) &&
                        (Adaptive_R_Grow_Dir[i] == 2 || Adaptive_R_Grow_Dir[i] == 3 || Adaptive_R_Grow_Dir[i] == 4))
                    {
                        R_Up_Turn_Right_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Up_Turn_Right_Point_1[1] = Adaptive_R_Line[i][1];
                        R_Up_Turn_Right_Point_Flag_1 = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 3:
        {
            for (i = 4; i < (Adaptive_R_Statics - 4); i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i - 2] == -2 || Adaptive_R_Grow_Dir[i - 2] == 1 || Adaptive_R_Grow_Dir[i - 2] == 4) && (Adaptive_R_Grow_Dir[i - 4] == -2 || Adaptive_R_Grow_Dir[i - 4] == 1 || Adaptive_R_Grow_Dir[i - 4] == 4) &&
                        (Adaptive_R_Grow_Dir[i + 2] == 2 || Adaptive_R_Grow_Dir[i + 2] == 3 || Adaptive_R_Grow_Dir[i + 2] == 4) && (Adaptive_R_Grow_Dir[i + 4] == 2 || Adaptive_R_Grow_Dir[i + 4] == 3 || Adaptive_R_Grow_Dir[i + 4] == 4) &&
                        (Adaptive_R_Grow_Dir[i] == 2 || Adaptive_R_Grow_Dir[i] == 3 || Adaptive_R_Grow_Dir[i] == 4))
                    {
                        R_Up_Turn_Right_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Up_Turn_Right_Point_1[1] = Adaptive_R_Line[i][1];
                        R_Up_Turn_Right_Point_Flag_1 = 1;
                        break;
                    }
                }
            }
            break;
        }
    }
//    if(R_Up_Turn_Right_Point_Flag_1 == 1)
//    {
//        R_Up_Turn_Right_Point_Position_1 = i;
//        R_Up_Turn_Right_Point_Angle_1 = Get_Turn_Point_Angle(Adaptive_R_Line[i - 5][0], Adaptive_R_Line[i - 5][1], Adaptive_R_Line[i][0], Adaptive_L_Line[i][1], Adaptive_R_Line[i + 5][0], Adaptive_R_Line[i + 5][1]);
//    }
}

/**
* 函数功能：      寻找上转右拐点
* 特殊说明：      通过检测X值的跳变判定拐点的存在
*                 寻找方式为遍历一维数组边线的每个点
* 形  参：        uint8 Region_Start      起始行
*                 uint8 Region_End        截止行
*
* 示例：          Get_R_Up_Turn_Right_Point_2(2, 57);
* 返回值：        无
*/
uint8 R_Up_Turn_Right_Point_2[2] = {0};
uint8 R_Up_Turn_Right_Point_Flag_2 = 0;
uint8 R_Up_Turn_Right_Point_Position_2 = 0;
void Get_R_Up_Turn_Right_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    R_Up_Turn_Right_Point_2[0] = 0;
    R_Up_Turn_Right_Point_2[1] = 0;
    R_Up_Turn_Right_Point_Position_2 = 0;
    if(Region_Start <= Region_End && Region_Start < 2)
    {
        Region_Start = 2;
    }
    if(Region_Start <= Region_End)
    {
        for(i = Region_Start; i <= Region_End; i++)
        {
            if((int8)R_Border[i] - (int8)R_Border[i - 1] <= -6)
            {
                if((int8)R_Border[i + 1] - (int8)R_Border[i - 2] <= -6)
                {
                    if(R_Border[i - 1] == 77 && R_Border[i - 2] == 77 && R_Border[i] != 77)
                    {
                        R_Up_Turn_Right_Point_2[0] = R_Border[i];
                        R_Up_Turn_Right_Point_2[1] = i;
                        R_Up_Turn_Right_Point_Flag_2 = 1;
                        R_Up_Turn_Right_Point_Position_2  = i;
                    }
                }
            }
        }
    }
}
uint8 R_Up_Turn_Right_Point[2] = {0};
uint8 R_Up_Turn_Right_Point_Flag = 0;
void Get_R_Up_Turn_Right_Point(void)
{
    R_Up_Turn_Right_Point_Flag = 0;
    Get_R_Up_Turn_Right_Point_1(2);
    if(R_Up_Turn_Right_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_1[1] >= 7)
    {
        R_Up_Turn_Right_Point_Flag_1 = 0;
        Get_R_Up_Turn_Right_Point_2(R_Up_Turn_Right_Point_1[1] - 7, R_Up_Turn_Right_Point_1[1] + 7);
        {
            if(R_Up_Turn_Right_Point_Flag_2 == 1)
            {
                R_Up_Turn_Right_Point_Flag_2 = 0;
                R_Up_Turn_Right_Point_Flag = 1;
                R_Up_Turn_Right_Point[0] = R_Up_Turn_Right_Point_1[0];
                R_Up_Turn_Right_Point[1] = R_Up_Turn_Right_Point_1[1];
            }
        }
    }
}

/**
* 函数功能：      寻找左转上拐点
* 特殊说明：      分为三个严格等级，根据每个点的生长方向判断
*                 寻找方式为遍历二维数组边线的每个点
* 形  参：        uint8 Grade      选择判定严格等级，常用2，即中等等级
*
* 示例：          Get_R_Left_Turn_Up_Point_1(2);
* 返回值：        无
*/
uint8 R_Left_Turn_Up_Point_1[2] = {0};
uint8 R_Left_Turn_Up_Point_Flag_1 = 0;
uint8 R_Left_Turn_Up_Point_Position_1 = 0;
int16 R_Left_Turn_Up_Point_Angle_1 = 0;
void Get_R_Left_Turn_Up_Point_1(uint8 Grade)
{
    uint8 i =0;
    R_Left_Turn_Up_Point_Flag_1 = 0;
    R_Left_Turn_Up_Point_1[0] = 0;
    R_Left_Turn_Up_Point_1[1] = 0;
    R_Left_Turn_Up_Point_Position_1 = 0;
    R_Left_Turn_Up_Point_Angle_1 = 0;

    if(Adaptive_R_Statics < 12)
    {
        return;
    }

    switch(Grade)
    {
        case 1:
        {
            for (i = 5; (i + 7) < Adaptive_R_Statics; i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i + 2] == -2 || Adaptive_R_Grow_Dir[i + 2] == 1) && (Adaptive_R_Grow_Dir[i + 5] == -2 || Adaptive_R_Grow_Dir[i + 5] == 1) && (Adaptive_R_Grow_Dir[i + 7] == -2 || Adaptive_R_Grow_Dir[i + 7] == 1) &&
                        (Adaptive_R_Grow_Dir[i - 2] == -3) && (Adaptive_R_Grow_Dir[i - 5] == -3) && (Adaptive_R_Grow_Dir[i] == -2 || Adaptive_R_Grow_Dir[i] == 1))
                    {
                        R_Left_Turn_Up_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Left_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                        R_Left_Turn_Up_Point_Flag_1 = 1;
                        break;
                    }
                }
            }
            break;
        }
        case 2:
        {
            for (i = 5; (i + 7) < Adaptive_R_Statics; i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i + 2] == -2 || Adaptive_R_Grow_Dir[i + 2] == 1) && (Adaptive_R_Grow_Dir[i + 5] == -2 || Adaptive_R_Grow_Dir[i + 5] == 1) && (Adaptive_R_Grow_Dir[i + 7] == -2 || Adaptive_R_Grow_Dir[i + 7] == 1) &&
                        (Adaptive_R_Grow_Dir[i - 2] == -3 || Adaptive_R_Grow_Dir[i - 2] == -4) && (Adaptive_R_Grow_Dir[i - 5] == -3 || Adaptive_R_Grow_Dir[i - 5] == -4) && (Adaptive_R_Grow_Dir[i] == -2 || Adaptive_R_Grow_Dir[i] == 1))
                    {
                        R_Left_Turn_Up_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Left_Turn_Up_Point_1[1] = Adaptive_R_Line[i][1];
                        R_Left_Turn_Up_Point_Flag_1 = 1;;
                        break;
                    }
                }
            }
            break;
        }
        case 3:
        {
            for (i = 5; (i + 7) < Adaptive_R_Statics; i++)
            {
                if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
                {
                    if ((Adaptive_R_Grow_Dir[i + 2] == -2 || Adaptive_R_Grow_Dir[i + 2] == 1 || Adaptive_R_Grow_Dir[i + 2] == 4) && (Adaptive_R_Grow_Dir[i + 5] == -2 || Adaptive_R_Grow_Dir[i + 5] == 1 || Adaptive_R_Grow_Dir[i + 5] == 4) &&
                        (Adaptive_R_Grow_Dir[i + 7] == -2 || Adaptive_R_Grow_Dir[i + 7] == 1 || Adaptive_R_Grow_Dir[i + 7] == 4) &&
                        (Adaptive_R_Grow_Dir[i - 2] == -3 || Adaptive_R_Grow_Dir[i - 2] == -4) && (Adaptive_R_Grow_Dir[i - 5] == -3 || Adaptive_R_Grow_Dir[i - 5] == -4) && (Adaptive_R_Grow_Dir[i] == -2 || Adaptive_R_Grow_Dir[i] == 1))
                    {
                        R_Left_Turn_Up_Point_1[0] = Adaptive_R_Line[i][0];
                        R_Left_Turn_Up_Point_1[1] = Adaptive_R_Line[i][1];
                        R_Left_Turn_Up_Point_Flag_1 = 1;;
                        break;
                    }
                }
            }
            break;
        }
    }
//    if(R_Left_Turn_Up_Point_Flag_1 == 1)
//    {
//        R_Left_Turn_Up_Point_Position_1 = i;
//        R_Left_Turn_Up_Point_Angle_1 = Get_Turn_Point_Angle(Adaptive_R_Line[i - 5][0], Adaptive_R_Line[i - 5][1], Adaptive_R_Line[i][0], Adaptive_R_Line[i][1], Adaptive_R_Line[i + 5][0], Adaptive_R_Line[i + 5][1]);
//    }
}

/**
* 函数功能：      寻找左转上拐点
* 特殊说明：      通过检测X值的跳变判定拐点的存在
*                 寻找方式为遍历一维数组边线的每个点
* 形  参：        uint8 Region_Start      起始行
*                 uint8 Region_End        截止行
*
* 示例：          Get_R_Left_Turn_Up_Point_2(2, 57);
* 返回值：        无
*/
uint8 R_Left_Turn_Up_Point_2[2] = {0};
uint8 R_Left_Turn_Up_Point_Flag_2 = 0;
uint8 R_Left_Turn_Up_Point_Position_2 = 0;
void Get_R_Left_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    if(Region_Start <= Region_End && Region_Start < 2)
    {
        Region_Start = 2;
    }
    if(Region_Start <= Region_End)
    {
        for(i = Region_Start; i <= Region_End; i++)
        {
            if((int8)R_Border[i] - (int8)R_Border[i + 1] <= -6)
            {
                if((int8)R_Border[i - 1] - (int8)R_Border[i + 2] <= -6)
                {
                    if(R_Border[i + 1] == 77 && R_Border[i + 2] == 77 && R_Border[i] != 77)
                    {
                        R_Left_Turn_Up_Point_2[0] = R_Border[i];
                        R_Left_Turn_Up_Point_2[1] = i;
                        R_Left_Turn_Up_Point_Flag_2 = 1;
                        R_Left_Turn_Up_Point_Position_2 = i;
                    }
                }
            }
        }
    }
}
uint8 R_Left_Turn_Up_Point[2] = {0};
uint8 R_Left_Turn_Up_Point_Flag = 0;
void Get_R_Left_Turn_Up_Point(void)
{
    R_Left_Turn_Up_Point_Flag = 0;
    Get_R_Left_Turn_Up_Point_1(2);
    if(R_Left_Turn_Up_Point_Flag_1 == 1 && R_Left_Turn_Up_Point_1[1] >= 7)
    {
        R_Left_Turn_Up_Point_Flag_1 = 0;
        Get_R_Left_Turn_Up_Point_2(R_Left_Turn_Up_Point_1[1] - 7, R_Left_Turn_Up_Point_1[1] + 7);
        {
            if(R_Left_Turn_Up_Point_Flag_2 == 1)
            {
                R_Left_Turn_Up_Point_Flag_2 = 0;
                R_Left_Turn_Up_Point_Flag = 1;
                R_Left_Turn_Up_Point[0] = R_Left_Turn_Up_Point_1[0];
                R_Left_Turn_Up_Point[1] = R_Left_Turn_Up_Point_1[1];
            }
        }
    }
}

/**
* 函数功能：      寻找左侧圆弧拐点
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_L_Arc_Turn_Point();
* 返回值：        无
*/
uint8 L_Arc_Turn_Point[3][2] = {{0}};   //最多找三个就足够使用
uint8 L_Arc_Turn_Point_Flag = 0;    //找到一个就挂出标志位
uint8 L_Arc_Turn_Point_Num = 0;     //记录找到的个数
void Get_L_Arc_Turn_Point(void)
{
    uint8 i = 0;
    L_Arc_Turn_Point_Flag = 0;

    for(i = 0; i < 3; i++)      //每次调用先清零
    {
        L_Arc_Turn_Point_Num = 0;
        L_Arc_Turn_Point[i][0] = 0;
        L_Arc_Turn_Point[i][1] = 0;
    }

    if(Adaptive_L_Statics >= 12)
    {
        for(i = 7; i < (Adaptive_L_Statics - 5); i++)
        {
            if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 7)))
            {
                if((Adaptive_L_Line[i][0] > Adaptive_L_Line[i - 7][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i - 4][0]) && (Adaptive_L_Line[i][0] >= Adaptive_L_Line[i - 1][0]) &&
                   (Adaptive_L_Line[i][0] >= Adaptive_L_Line[i + 1][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i + 3][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i + 5][0]) &&
                   (Adaptive_L_Grow_Dir[i + 1] == -2 || Adaptive_L_Grow_Dir[i + 1] == 1) && (Adaptive_L_Grow_Dir[i + 3] == -2 || Adaptive_L_Grow_Dir[i + 3] == 1) &&
                   (Adaptive_L_Grow_Dir[i - 1] == 1 || Adaptive_L_Grow_Dir[i + 1] == 4) && (Adaptive_L_Grow_Dir[i - 3] == 1 || Adaptive_L_Grow_Dir[i - 3] == 4))                    //左侧内凹
                {
                    L_Arc_Turn_Point[L_Arc_Turn_Point_Num][0] = Adaptive_L_Line[i][0];
                    L_Arc_Turn_Point[L_Arc_Turn_Point_Num][1] = Adaptive_L_Line[i][1];
                    L_Arc_Turn_Point_Flag = 1;
                    L_Arc_Turn_Point_Num ++;
                    if(i <= (Adaptive_L_Statics - 5 - 15))   // 防止溢出
                    {
                        i += 15;    //每个圆弧拐点前后几个点可能都满足圆弧拐点的条件，所以找到后加15个点
                    }
                    if(L_Arc_Turn_Point_Num == 3)
                    {
                        break;
                    }
                }
            }
        }
    }

    if(Adaptive_L_Statics >= 12)
    {
        for(i = 7; i < (Adaptive_L_Statics - 5); i++)
        {
            if((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 7)))
            {

                if((Adaptive_L_Line[i][0] < Adaptive_L_Line[i - 7][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i - 4][0]) && (Adaptive_L_Line[i][0] <= Adaptive_L_Line[i - 1][0]) &&
                   (Adaptive_L_Line[i][0] <= Adaptive_L_Line[i + 1][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i + 3][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i + 5][0]) &&
                   (Adaptive_L_Grow_Dir[i + 1] == 4 || Adaptive_L_Grow_Dir[i + 1] == 1) && (Adaptive_L_Grow_Dir[i + 3] == 4 || Adaptive_L_Grow_Dir[i + 3] == 1) &&
                   (Adaptive_L_Grow_Dir[i - 1] == 1 || Adaptive_L_Grow_Dir[i + 1] == -2) && (Adaptive_L_Grow_Dir[i - 3] == 1 || Adaptive_L_Grow_Dir[i - 3] == -2))                    //左侧外凸
                {
                    L_Arc_Turn_Point[L_Arc_Turn_Point_Num][0] = Adaptive_L_Line[i][0];
                    L_Arc_Turn_Point[L_Arc_Turn_Point_Num][1] = Adaptive_L_Line[i][1];
                    L_Arc_Turn_Point_Flag = 1;
                    L_Arc_Turn_Point_Num ++;
                    if(i <= (Adaptive_L_Statics - 5 - 15))   // 防止溢出
                    {
                        i += 15;
                    }
                    if(L_Arc_Turn_Point_Num == 3)
                    {
                        break;
                    }
                }
            }
        }
    }
}

/**
* 函数功能：      寻找右侧圆弧拐点
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_R_Arc_Turn_Point();
* 返回值：        无
*/
uint8 R_Arc_Turn_Point[3][2] = {{0}};
uint8 R_Arc_Turn_Point_Flag = 0;
uint8 R_Arc_Turn_Point_Num = 0;
void Get_R_Arc_Turn_Point(void)
{
    uint8 i = 0;
    R_Arc_Turn_Point_Flag = 0;
    for(i = 0; i < 3; i++)
    {
        R_Arc_Turn_Point_Num = 0;
        R_Arc_Turn_Point[i][0] = 0;
        R_Arc_Turn_Point[i][1] = 0;
    }

    if(Adaptive_R_Statics >= 12)
    {
        for(i = 7; i < (Adaptive_R_Statics - 5); i++)
        {
            if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if((Adaptive_R_Line[i][0] < Adaptive_R_Line[i - 7][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i - 4][0]) && (Adaptive_R_Line[i][0] <= Adaptive_R_Line[i - 1][0]) &&
                   (Adaptive_R_Line[i][0] <= Adaptive_R_Line[i + 1][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i + 3][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i + 5][0]) &&
                   (Adaptive_R_Grow_Dir[i + 1] == 4 || Adaptive_R_Grow_Dir[i + 1] == 1) && (Adaptive_R_Grow_Dir[i + 3] == 4 || Adaptive_R_Grow_Dir[i + 3] == 1) &&
                   (Adaptive_R_Grow_Dir[i - 1] == 1 || Adaptive_R_Grow_Dir[i + 1] == -2) && (Adaptive_R_Grow_Dir[i - 3] == 1 || Adaptive_R_Grow_Dir[i - 3] == -2))                    //右侧内凹
                {
                    R_Arc_Turn_Point[R_Arc_Turn_Point_Num][0] = Adaptive_R_Line[i][0];
                    R_Arc_Turn_Point[R_Arc_Turn_Point_Num][1] = Adaptive_R_Line[i][1];
                    R_Arc_Turn_Point_Flag = 1;
                    R_Arc_Turn_Point_Num ++;
                    if(i <= (Adaptive_R_Statics - 5 - 15))   // 防止溢出
                    {
                        i += 15;
                    }
                    if(R_Arc_Turn_Point_Num == 3)
                    {
                        break;
                    }
                }
            }
        }
    }

    if(Adaptive_R_Statics >= 12)
    {
        for(i = 7; i < (Adaptive_R_Statics - 5); i++)
        {
            if((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if((Adaptive_R_Line[i][0] > Adaptive_R_Line[i - 7][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i - 4][0]) && (Adaptive_R_Line[i][0] >= Adaptive_R_Line[i - 1][0]) &&
                   (Adaptive_R_Line[i][0] >= Adaptive_R_Line[i + 1][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i + 3][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i + 5][0]) &&
                   (Adaptive_R_Grow_Dir[i + 1] == -2 || Adaptive_R_Grow_Dir[i + 1] == 1) && (Adaptive_R_Grow_Dir[i + 3] == -2 || Adaptive_R_Grow_Dir[i + 3] == 1) &&
                   (Adaptive_R_Grow_Dir[i - 1] == 1 || Adaptive_R_Grow_Dir[i + 1] == 4) && (Adaptive_R_Grow_Dir[i - 3] == 1 || Adaptive_R_Grow_Dir[i - 3] == 4))                    //右侧外凸
                {
                    R_Arc_Turn_Point[R_Arc_Turn_Point_Num][0] = Adaptive_R_Line[i][0];
                    R_Arc_Turn_Point[R_Arc_Turn_Point_Num][1] = Adaptive_R_Line[i][1];
                    R_Arc_Turn_Point_Flag = 1;
                    R_Arc_Turn_Point_Num ++;
                    if(i <= (Adaptive_R_Statics - 5 - 15))   // 防止溢出
                    {
                        i += 15;
                    }
                    if(R_Arc_Turn_Point_Num == 3)
                    {
                        break;
                    }
                }
            }
        }
    }
}

int16 Max_Slope_Dif_Thre = 60;     //原0.6*100
int16 Min_Slope_Dif_Thre = 25;    //原0.15*100，放宽至0.25
//斜率和截距
int16 R_Straightaway_Lope_Rate_A = 0;   //第一段的斜率 *100  //前方线段斜率
int16 R_Straightaway_Lope_Rate_B = 0;   //第二段的斜率 *100  //后方线段斜率
int16 R_Straightaway_Lope_Rate_C = 0;   //整段边线的斜率 *100
int16 R_Intercept = 0;      //整段边线的截距
uint8 R_Straight_Flag = 0;      //右侧边线是否为直线标志位

/**
* 函数功能：      求右侧边线斜率截距，并判断是否为直线
* 特殊说明：      无
* 形  参：        uint8 start          //右侧边线起始点Y坐标
*                 uint8 end            //右侧边线终止点Y坐标
*
* 示例：          Get_R_Intercept_And_Slope(Adaptive_R_Line[Adaptive_R_Start_Point][1] + 5, Adaptive_R_Line[0][1]);
* 返回值：        无
*/
void Get_R_Intercept_And_Slope(uint8 start, uint8 end)
{
    R_Straightaway_Lope_Rate_A = 0;
    R_Straightaway_Lope_Rate_B = 0;
    R_Straightaway_Lope_Rate_C = 0;
    R_Intercept = 0;
    R_Straight_Flag = 0;            //所有值先清零

    R_Straightaway_Lope_Rate_A = Slope_Calculate(start, ((end - start) / 2) + start, R_Border);
    R_Straightaway_Lope_Rate_B = Slope_Calculate(((end - start) / 2) + start, end, R_Border);       //计算两段斜率
    Calculate_Slope_Intercept(start, end, R_Border, &R_Straightaway_Lope_Rate_C, &R_Intercept);     //计算整段边线斜率和截距

    if( ( (R_Straightaway_Lope_Rate_A > R_Straightaway_Lope_Rate_B) ? (R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_B) : (R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_A) ) <= Min_Slope_Dif_Thre &&
        ( (R_Straightaway_Lope_Rate_B > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_B) ) <= Min_Slope_Dif_Thre &&
        ( (R_Straightaway_Lope_Rate_A > R_Straightaway_Lope_Rate_C) ? (R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_C) : (R_Straightaway_Lope_Rate_C - R_Straightaway_Lope_Rate_A) ) <= Min_Slope_Dif_Thre)      //判断是否为直线
    {
        R_Straight_Flag = 1;
    }
}


int16 L_Straightaway_Lope_Rate_A = 0;
int16 L_Straightaway_Lope_Rate_B = 0;
int16 L_Straightaway_Lope_Rate_C = 0;
int16 L_Intercept = 0;
uint8 L_Straight_Flag = 0;

/**
* 函数功能：      求左侧边线斜率截距，并判断是否为直线
* 特殊说明：      无
* 形  参：        uint8 start          //左侧边线起始点Y坐标
*                 uint8 end            //左侧边线终止点Y坐标
*
* 示例：          Get_L_Intercept_And_Slope(Adaptive_L_Line[Adaptive_L_Start_Point][1] + 5, Adaptive_L_Line[0][1]);
* 返回值：        无
*/
void Get_L_Intercept_And_Slope(uint8 start, uint8 end)
{
    L_Straightaway_Lope_Rate_A = 0;
    L_Straightaway_Lope_Rate_B = 0;
    L_Straightaway_Lope_Rate_C = 0;
    L_Intercept = 0;
    L_Straight_Flag = 0;

    L_Straightaway_Lope_Rate_A = Slope_Calculate(start, ((end - start) / 2) + start, L_Border);
    L_Straightaway_Lope_Rate_B = Slope_Calculate(((end - start) / 2) + start, end, L_Border);
    Calculate_Slope_Intercept(start, end, L_Border, &L_Straightaway_Lope_Rate_C, &L_Intercept);

    if( ( (L_Straightaway_Lope_Rate_A > L_Straightaway_Lope_Rate_B) ? (L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_B) : (L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_A) ) <= Min_Slope_Dif_Thre &&
        ( (L_Straightaway_Lope_Rate_B > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_B) ) <= Min_Slope_Dif_Thre &&
        ( (L_Straightaway_Lope_Rate_A > L_Straightaway_Lope_Rate_C) ? (L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_C) : (L_Straightaway_Lope_Rate_C - L_Straightaway_Lope_Rate_A) ) <= Min_Slope_Dif_Thre)
    {
        L_Straight_Flag = 1;
    }
}

////拟合曲线
//int16 L_Fitting_Line[20] = {0};
//int16 R_Fitting_Line[20] = {0};
////方差
//uint16 L_Variance = 0;
//uint16 R_Variance = 0;

///**
//* 函数功能：      计算左右两侧边线的方差
//* 特殊说明：      每三行计算一次，减少计算量，但获取的信息不那么精确，可以选择计算每一行
//* 形  参：        无
//*
//* 示例：          Get_Fitting_Line_And_Variance();
//* 返回值：        无
//*/
//void Get_Fitting_Line_And_Variance(void)
//{
//    uint8 i = 0;

//    for(i = 1; i <= 19; i ++)
//    {
//        L_Fitting_Line[i] = (int16)( ( (int32)L_Straightaway_Lope_Rate_C * (i * 3) ) / 100 + L_Intercept );
//        L_Fitting_Line[i] = Limit_16(L_Fitting_Line[i], X_Border_Min + 2, X_Border_Max - 2);

//        R_Fitting_Line[i] = (int16)( ( (int32)R_Straightaway_Lope_Rate_C * (i * 3) ) / 100 + R_Intercept );
//        R_Fitting_Line[i] = Limit_16(R_Fitting_Line[i], X_Border_Min + 2, X_Border_Max - 2);
//    }

//    if(Adaptive_L_Statics >= 2)
//    {
//        Get_Variance(Adaptive_L_Line[Adaptive_L_Statics - 1][1] + 1, Adaptive_L_Line[0][1], L_Fitting_Line, L_Border, &L_Variance, 3);
//    }
//    if(Adaptive_R_Statics >= 2)
//    {
//        Get_Variance(Adaptive_R_Line[Adaptive_R_Statics - 1][1] + 1, Adaptive_R_Line[0][1], R_Fitting_Line, R_Border, &R_Variance, 3);
//    }
//}


/**
* 函数功能：      寻找左侧边线的边界点个数
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_L_Border_Point_Num();
* 返回值：        无
*/
uint8 L_Border_Point_Num = 0;
uint8 L_UP_Border_Point_Num = 0;    //记录位于顶部黑框上的边界点个数
void Get_L_Border_Point_Num(void)
{
    uint8 i = 0;
    L_Border_Point_Num = 0;
    L_UP_Border_Point_Num = 0;

    // 添加边界检查，避免无限循环
    if(Adaptive_L_Statics < 2 || Adaptive_L_Line[0][1] <= Adaptive_Y_Meet)
    {
        return;
    }

    for(i = Adaptive_Y_Meet; i < Adaptive_L_Line[0][1]; i++)
    {
        if(L_Border[i] == 3)    //左右两侧用一维边线即可
        {
            L_Border_Point_Num ++;
        }
    }
    if(Adaptive_L_Statics > 42)  // 确保 40 < Adaptive_L_Statics - 2
    {
        for(i = 40; i < Adaptive_L_Statics - 2; i++)
        {
            if(Adaptive_L_Line[i][1] == 2)   //顶部的边界点使用二维边线
            {
                L_UP_Border_Point_Num ++;
            }
        }
    }
}

/**
* 函数功能：      寻找右侧边线的边界点个数
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_R_Border_Point_Num();
* 返回值：        无
*/
uint8 R_Border_Point_Num = 0;
uint8 R_UP_Border_Point_Num = 0;
void Get_R_Border_Point_Num(void)
{
    uint8 i = 0;
    R_Border_Point_Num = 0;
    R_UP_Border_Point_Num = 0;

    // 添加边界检查，避免无限循环
    if(Adaptive_R_Statics < 2 || Adaptive_R_Line[0][1] <= Adaptive_Y_Meet)
    {
        return;
    }

    for(i = Adaptive_Y_Meet; i < Adaptive_R_Line[0][1]; i++)
    {
        if(R_Border[i] == 76)
        {
            R_Border_Point_Num ++;
        }
    }
    if(Adaptive_R_Statics > 42)  // 确保 40 < Adaptive_R_Statics - 2
    {
        for(i = 40; i < Adaptive_R_Line[0][1]; i++)
        {
            if(Adaptive_R_Line[i][1] == 2)
            {
                R_UP_Border_Point_Num ++;
            }
        }
    }
}

/**
* 函数功能：      寻找对位边界行的个数
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_Relative_Border_Point_Num();
* 返回值：        无
*/
uint8 Relative_Border_Point_Num = 0;        //存储对位边界行的个数
void Get_Relative_Border_Point_Num(void)
{
    uint8 i = 0;
    Relative_Border_Point_Num = 0;  //函数每张图像边线都调用一次，每调用一次都要清零
    for(i = Adaptive_Y_Meet; i < Adaptive_L_Start_Point[1]; i++)
    {
        if(L_Border[i] == 3 && R_Border[i] == 76)   //2为左侧边界的X坐标，77为右侧边界
        {
            Relative_Border_Point_Num ++;
        }
    }
}

int8 Row_Difference[20] = {0};
uint8 Max_Row_Dif_Line_Num = 0;     //当某一行左侧边线点位于左侧边界上，右侧边线点位于右侧边线上，与对位边界行同理，可以记录下来
/**
* 函数功能：      寻找对位边界行的个数
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_Relative_Border_Point_Num();
* 返回值：        无
*/
void Get_Row_Difference(uint8 *l_border, uint8 *r_border)
{
    uint8 i = 0;
    Max_Row_Dif_Line_Num = 0;
    for(i = 0; i < 20; i ++)
    {
        Row_Difference[i] = (int8)(r_border[i * 3] - l_border[i * 3]);
        if(Row_Difference[i] == 73)
        {
            Max_Row_Dif_Line_Num ++;
        }
    }
}

/**
* 函数功能：      获取所有信息
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_Information();
* 返回值：        无
*/
void Get_Information(void)
{
     Get_L_Border_Point_Num();
     Get_R_Border_Point_Num();
     Get_Relative_Border_Point_Num();
     Get_L_Up_Turn_Left_Point_1(2);
     Get_L_Right_Turn_Up_Point_1(2);
     Get_R_Up_Turn_Right_Point_1(2);
     Get_R_Left_Turn_Up_Point_1(2);
     Get_L_Arc_Turn_Point();
     Get_R_Arc_Turn_Point();
			if(Adaptive_L_Statics >= 2)
			{
					Get_L_Intercept_And_Slope(Adaptive_L_Line[Adaptive_L_Statics - 1][1] + 3, Adaptive_L_Line[0][1]);
			}
			if(Adaptive_R_Statics >= 2)
			{
					Get_R_Intercept_And_Slope(Adaptive_R_Line[Adaptive_R_Statics - 1][1] + 3, Adaptive_R_Line[0][1]);
			}
				
			
//    Get_Fitting_Line_And_Variance();
    Get_Row_Difference(L_Border, R_Border);
//    if(Image_Num % 10 == 0)
//    {
//        Barrier_Distance = TOF_Get_Distance_mm();
//    }
}


////**************************************图像偏差计算********************************************
int8 Deviation_Value = 0;
int8 Mid_Line_FFF = 0;
int8 Mid_Line_FF = 0;
int8 Mid_Line_F = 0;

uint16 C_Variance = 0;
uint16 Last_C_Variance = 0;

int8 Last_Deviation_Value = 0;

//偏差值滑动均值滤波数组
int16 Deviation_Value_Array[20] = {0};
//用于相遇点滑动均值滤波数组
uint8 Y_Meet_Array[20] = {0};

void Get_Straight_Deviation_Value(uint8 element , uint8 start , uint8 end , uint8 *c_line , uint8 element_state)
{
    uint8 i = 0;
    int32 Angle_Err = 0;
    int32 Weight_Sum = 0;
    int32 Sum = 0;
    int16 deviation_value = 0;
    int16 Slope = 0;
	int16 Deviation_Value_Sum = 0;
	    // ====================== 直线专用权重表 ======================
//   static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    2, 3, 4, 4, 5, 6, 6, 7, 8, 9,
//          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
//	  if(Element == L_Circle || Element == R_Circle) 
//	  {
//		  if(start <= 34){
//			start = 34;
//		  }
//	  }else{
//		  start = 28;
//		  end = 33;
//	  }
		
	//  end = 35;
	   
	switch(element)
    {
		case 2 :
		{
			 static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 2, 5, 6, 8, 14, 15, 15, 17, 17, 17, 20, 18, 18, 16, 14,
          10, 10, 8, 6, 6, 5, 5,6,6,5,  4,2,3,3,3,2, 2, 2, 1, 1,   1, 0, 1, 0, 1, 0, 1, 0, 0, 0 };
//			static const uint8 Weight_2[60] = { 0, 1, 0, 1, 0, 0, 0,      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 2, 5, 6, 8, 14, 15, 15, 17, 17, 17, 20, 18, 18, 16, 14,
//          10, 10, 8, 6, 6, 5, 5,6,6,5,  4,2,3,3,3,2, 2, 2, 1, 1,   1, 0, 1}; 
				 
				for(i = start + 10; i < end; i++)
						{
						//if(c_line[i] == 39)continue;
							//if(L_Border[i] == 2 || R_Border[i] == 77) continue;
							Sum += (int32)Weight_2[i] * c_line[i];
							Weight_Sum += Weight_2[i];
						}
				
			break;
//				uint8 eff_start;
//				static const uint8 Weight_2[60] = {
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // row 0-29 全部清零
//           4, 5, 6, 7, 9,11,14,16,18,19,   // row 30-39: 缓升
//          20,20,18,15,12,10, 8, 6, 4, 3,   // row 40-49: 峰值+衰减
//           3, 2, 2, 1, 1, 1, 1, 0, 0, 0    // row 50-59
//				};
//				eff_start = (Adaptive_Y_Meet < 30) ? 30 : Adaptive_Y_Meet;
//				for(i = eff_start; i < end; i++)
//				{
//						Sum += (int32)Weight_2[i] * c_line[i];
//						Weight_Sum += Weight_2[i];
//				}
//				break;
		}
	
		case 18 :
		{
			static const uint8 Weight_5_2[60] = { 0, 0, 0, 0, 0, 0, 7, 7, 7, 7,    10,10,10,10,10,10,10,10,10,10,    10,10,10,10,10,10,10,10,10,10,
                                       10,10,10,10,10, 8, 8, 8, 8, 9,    9, 9, 9, 8, 8, 8, 8, 8, 7, 7,     7, 6, 6, 5, 4, 3, 2, 0, 0, 0 };

                        for(i = 6; i < end; i++)
                        {
                            Sum += (int32)Weight_5_2[i] * C_Line[i];
                            Weight_Sum += Weight_5_2[i];
                        }
                        break;
		}
		case L_Circle:
            {
				if(start <= 24){
					start = 24;
				}
				
                switch(element_state)
                {

                    case 2:
                    {
                        static const uint8 Weight_5_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 6, 6, 7, 8, 9, 9,
          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };

                        for(i = start + 5; i < end; i++)
                        {
                            Sum += (int32)Weight_5_2[i] * C_Line[i];
                            Weight_Sum += Weight_5_2[i];
                        }
                        break;
                    }
					default :
					{
						static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    8, 8, 9, 9, 10, 10, 10, 10, 11, 11,
          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
	 
						for(i = start; i < end; i++)
						{
						//if(c_line[i] == 39)continue;
							Sum += (int32)Weight_2[i] * c_line[i];
							Weight_Sum += Weight_2[i];
						}	
						break;
					}
         }
                break;
            }
            case R_Circle:
            {
				if(start <= 24){
					start = 24;
				}
                switch(element_state)
                {
					case 2:{
										 static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    2, 3, 4, 4, 5, 6, 6, 7, 8, 9,
          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
	 
						for(i = start + 5; i < end; i++)
						{
						//if(c_line[i] == 39)continue;
							Sum += (int32)Weight_2[i] * c_line[i];
							Weight_Sum += Weight_2[i];
						}	
						break;
					}
					default :
										
									{
										 static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    8, 8, 9, 9, 10, 10, 10, 10, 11, 11,
          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
	 
						for(i = start; i < end; i++)
						{
						//if(c_line[i] == 39)continue;
							Sum += (int32)Weight_2[i] * c_line[i];
							Weight_Sum += Weight_2[i];
						}	
						break;
									}

            }
						break;
		}
						//原来的表
//			default :
//			{
//					    static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    2, 3, 4, 4, 5, 6, 6, 7, 8, 9,
//          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
//	 
//						for(i = start; i < end; i++)
//						{
//						//if(c_line[i] == 39)continue;
//							Sum += (int32)Weight_2[i] * c_line[i];
//							Weight_Sum += Weight_2[i];
//						}	
//				break;
//			}	
		
		//新表
//		default :
//			{
//					    static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    4, 4, 5, 5, 6, 12, 13, 14, 14, 13,
//          12, 12, 11, 10, 6, 5, 5,4,4,3,    3,2,3,3,3,2, 2, 2, 1, 1,   1, 0, 1, 0, 1, 0, 1, 0, 0, 0 };
//	 
//						for(i = start; i < end; i++)
//						{
//						//if(c_line[i] == 39)continue;
//							Sum += (int32)Weight_2[i] * c_line[i];
//							Weight_Sum += Weight_2[i];
//						}	
//				break;
//			}
					//新新表
		default :
			{
					    static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     2, 3, 4, 4, 5, 6, 6, 7, 8, 9,
          9,10,11,12,12,13,13,13,13,13,   13,14,14,14,13,12,10, 9, 7, 6,   5, 4, 3, 2, 1, 1, 1, 0, 0, 0 };
//							static const uint8 Weight_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    0, 0, 0, 0, 0, 2, 5, 6, 8, 14,    15, 15, 17, 17, 17, 20, 18, 18, 16, 14,
//          10, 10, 8, 6, 6, 5, 5,6,6,5,  4,2,3,3,3,2, 2, 2, 1, 1,   1, 0, 1, 0, 1, 0, 1, 0, 0, 0 };
							
				if(start <= 30){
					start = 30;
				}
							
							
						for(i = start + 10; i < end; i++)
						{
						//if(c_line[i] == 39)continue;
							//if(L_Border[i] == 2 || R_Border[i] == 77) continue;
							Sum += (int32)Weight_2[i] * c_line[i];
							Weight_Sum += Weight_2[i];
						}	
				break;
//					uint8 eff_start;
//					static const uint8 Weight_2[60] = {
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
//           4, 5, 6, 7, 9,11,14,16,18,19,
//          20,20,18,15,12,10, 8, 6, 4, 3,
//					3, 2, 2, 1, 1, 1, 1, 0, 0, 0
//					};
//					eff_start = (Adaptive_Y_Meet < 30) ? 30 : Adaptive_Y_Meet;
//					for(i = eff_start; i < end; i++)
//					{
//							Sum += (int32)Weight_2[i] * c_line[i];
//							Weight_Sum += Weight_2[i];
//					}
//					break;
			}
	
	}

//	for(i = start; i < end; i++)
//	{
//		//if(c_line[i] == 39)continue;
//		Sum += (int32)Weight_2[i] * c_line[i];
//		Weight_Sum += Weight_2[i];
//	}
	
	if(Weight_Sum == 0)
    {
        Deviation_Value = Last_Deviation_Value;
		return;
    }
	
    // ====================== 计算中心偏差 ======================
    // 加权平均 - 赛道中心基准值(39.5) = 偏差
	
    Angle_Err = (int32)(Sum * 100 / Weight_Sum )   - 3950;
	
    // 归一化到 [-100, 100] 范围
    Angle_Err = (int32)(Angle_Err  / 17 ) ;

//    // ====================== 滤波（让输出更平滑） ======================
//    // 三阶滤波（你原代码的滤波逻辑）
//
//    Mid_Line_FFF = Mid_Line_FF;
//    Mid_Line_FF = Mid_Line_F;
//    Mid_Line_F = Angle_Err;

//    Angle_Err = (Mid_Line_FFF * 5 + Mid_Line_FF * 3 + Mid_Line_F * 2) / 10;
//    Angle_Err = (Angle_Err * 8 + Slope * 2) / 10;

    // 限幅，保证输出在 [-100, 100]
    deviation_value = Limit_16(Angle_Err, -100, 100);

        Deviation_Value = deviation_value;
        Last_Deviation_Value = deviation_value;
        if(photo_flag)photo_cnt+=1;

				//tft180_show_int16(60,  112, Deviation_Value);
//			len = sprintf(buf, "%d,%d,%d,%d\r\n",Sum, Weight_Sum, Angle_Err ,Deviation_Value  );
//			if( !wifi_spi_send_buffer((uint8*)buf, len) ){s++;};
			
}


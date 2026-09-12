#include <opencv2/opencv.hpp>
#include <iostream>
#include <cstdint>

using namespace std;
using namespace cv;

typedef unsigned char   uint8;	//  8 bits
typedef unsigned int  	uint16;	// 16 bits
typedef unsigned long  	uint32;	// 32 bits


typedef signed char     int8;	//  8 bits
typedef signed int      int16;	// 16 bits
typedef signed long     int32;	// 32 bits

#define IMG_W 188
#define IMG_H 120

#define Image_X 80
#define Image_Y 60

#define ZIP_W 80
#define ZIP_H 60

#define MAX_LINE_POINTS         250  //边线储存个数
#define X_Border_Min			0
#define X_Border_Max			79
#define Y_Border_Min			0
#define Y_Border_Max			59

#define Thres_Interfere         0  //人工干预灰度值计算
#define Thres_Num_Interfere     25  //共25点处的灰度值，用于做除法
#define Thres_Filiter_Flag_1    1  //开启反向平滑滤波，改善亮处到暗处或暗处亮处的灰度阈值
#define Thres_Filiter_Flag_2    1  //开启反向平滑滤波，改善亮处到暗处或暗处亮处的灰度阈值

uint8_t mt9v03x_image[IMG_H][IMG_W];

uint8_t Find_Line_Image[ZIP_H][ZIP_W];

uint8 Adaptive_L_Start_Point[2] = { 0 };		//���Ѳ����ʼ��
uint8 Adaptive_R_Start_Point[2] = { 0 };		//�Ҳ�Ѳ����ʼ��

uint8 Adaptive_L_Line[(uint8)160][2] = { {0} };   // ���������
uint8 Adaptive_R_Line[(uint8)160][2] = { {0} };   // ���������
int8 Adaptive_L_Grow_Dir[(uint8)160] = { 0 };     //���ÿ����������
int8 Adaptive_R_Grow_Dir[(uint8)160] = { 0 };     //�Ҳ�ÿ����������
uint16 Adaptive_L_Statics = 0;         //��������õ����
uint16 Adaptive_R_Statics = 0;         //�ұ�Ѱ�õ����

uint8 Adaptive_X_Meet = 0;
uint8 Adaptive_Y_Meet = 0;

//存放边线的一维数组
uint8 R_Border[Image_Y] = { 0 };
uint8 L_Border[Image_Y] = { 0 };

/**
 * @brief 压缩图像：将188x120压缩为80x60
 * @param src 原始图像数组 [IMG_H][IMG_W]
 * @param dst 压缩后数组 [ZIP_H][ZIP_W]
 */
void copy_zip_image(const uint8_t src[IMG_H][IMG_W], uint8_t dst[ZIP_H][ZIP_W])
{
    for (int i = 0; i < ZIP_H; ++i)
    {
        for (int j = 0; j < ZIP_W; ++j)
        {
            dst[i][j] = src[i * 2][(uint8)(j * 235 / 100)];
        }
    }
}

int16 Limit_16(int16 x, int16 a, int16 b)               //*
{
    if (x > b)
    {
        return b;
    }
    else if (x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

/**
 * @brief 画黑框
 * @param black_box_value 黑框灰度值
 * @param image 图像数组 [ZIP_H][ZIP_W]
 */
void draw_black_box(uint8_t black_box_value, uint8_t(*image)[ZIP_W])
{
    for (int i = 2; i < ZIP_H; ++i)
    {
        image[i][ZIP_W - 3] = black_box_value;
        image[i][2] = black_box_value;
    }
    for (int j = 2; j < ZIP_W - 3; ++j)
    {
        image[1][j] = black_box_value;
    }
}



/**
* 函数功能：      32位整形变量取绝对值
* 特殊说明：      注意调用时参数类型，摄像头代码参数较多，不同类型错误传参可能导致计算结果出现较大问题，严重时卡死单片机运行
* 形  参：        value：       32位整型变量
* 示例：          My_ABS();
* 返回值：        传入参数的绝对值
*/
int My_ABS(int value)               //*
{
    if (value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}

int32 My_ABS_I(int32 value)               //*
{
    if (value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}

float My_ABS_F(float value)               //*
{
    if (value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}


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
int16 Compare_Num(int16 a, int16 b, uint8 compare_value)
{
    if ((((a - b) << 7) / (a + b)) > compare_value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
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
uint8 Compare_Value = 40;
uint8 Black_Box_Value_1 = 0;
uint8 Get_Start_Point(uint8 start_row, uint8(*image)[Image_X], uint8* l_start_point, uint8* r_start_point, uint8 l_border_x, uint8 r_border_x)//*****
{
    uint8 i = 0, j = 0;
    uint8 L_Is_Found = 0, R_Is_Found = 0;   //找到起点时挂出对应标志位
    uint8 Start_X = 0;                     //起始X坐标，第一张图像取图像的行中点，后续图像用上一次图像左右两侧起始点的中间值
    uint8 Start_Row_0 = 0;                  //起始Y坐标

    Start_Row_0 = start_row;
    Start_X = Image_X / 2;
    //从中间往左边，先找起点
    for (j = 0; j < 10; j++)        //指定的行没找到起点时，向上走一行继续找，最多找十行
    {
        l_start_point[1] = start_row;//y
        r_start_point[1] = start_row;//y

        if (Start_Flag == 0)       //第一张图像和遇到斑马线时，起始X坐标选用图像的行中点
        {
            Start_X = Image_X / 2;
        }
        else
        {
            Start_X = (l_start_point[0] + r_start_point[0]) / 2;    //否则起始X坐标用上一次图像左右两侧起始点的中间值
        }

        {
            for (i = Start_X; i > l_border_x - 1; i--)      //向左找起始点
            {

                //				if (Compare_Num(image[start_row][i+8], image[start_row][i], Compare_Value))
                //				{
                //					//if( Compare_Num(image[start_row][i+7],image[start_row][i-1],Compare_Value)
                //					//|| Compare_Num(image[start_row][i+6],image[start_row][i-2],Compare_Value) )
                //					//{
                //						l_start_point[0] = i; //X指
                //						L_Is_Found = 1;
                //						break;
                //					//}
                //				}

                if (Compare_Num(image[start_row][i + 5], image[start_row][i], Compare_Value))//差比和为真
                {
                    // int judge_flag_l = 0;
                    // uint8 continuous_l = 0;
                    // uint8 k;
                    // for( k = 1; k < 3; k++) 
                    // {  // 检查后续2个点
                    // 	if(Compare_Num(image[start_row][i - k + 5],image[start_row][i - k], Compare_Value))
                    // 	{
                    // 			continuous_l++;
                    // 	}
                    // }
                    // if(continuous_l >= 1)judge_flag_l = 1;
                    // if(judge_flag_l == 1)
                    // {
                    l_start_point[0] = i;   //找到后记录X坐标
                    L_Is_Found = 1;         //挂出找见标志位
                    break;
                    // }
                }
            }

            for (i = Start_X; i < r_border_x + 1; i++)      //向右找起始点
            {

                //				if (Compare_Num(image[start_row][i - 8], image[start_row][i], Compare_Value))
                //				{
                //					//if( Compare_Num(image[start_row][i - 7],image[start_row][i + 1],Compare_Value)
                //					//|| Compare_Num(image[start_row][i - 6],image[start_row][i + 2],Compare_Value) )
                //					//{
                //						l_start_point[0] = i;
                //						L_Is_Found = 1;
                //						break;
                //					//
                //				}

                if (Compare_Num(image[start_row][i - 5], image[start_row][i], Compare_Value))//差比和为真
                {
                    {
                        // int judge_flag_r = 0;
                        // uint8 continuous_r = 0;
                        // uint8 k;
                        // for( k = 1; k < 3; k++) 
                        // {  // 检查后续2个点
                        // 	if(Compare_Num(image[start_row][i + k - 8],image[start_row][i + k], Compare_Value))
                        // 	{
                        // 		continuous_r++;
                        // 	}
                        // }
                        // if(continuous_r >= 1)judge_flag_r = 1;
                        // if(judge_flag_r == 1)
                        // {
                        r_start_point[0] = i;
                        R_Is_Found = 1;
                        break;
                        // }
                    }
                }
            }
            if (L_Is_Found && R_Is_Found)
            {
                Start_Flag = 1;    //是否为第一张图像标志位

                // if(l_start_point[0] != l_border_x && r_start_point[0] != r_border_x)
                // {
                //     Black_Box_Value_1 = 100 - 40;
                //     Black_Box_Value_1 = Limit_u8(Black_Box_Value_1, 0, 255);
                // }
                // else if(l_start_point[0] != l_border_x && r_start_point[0] == r_border_x)
                // {
                //     Black_Box_Value_1 = image[start_row][l_start_point[0]] - 20;
                //    Black_Box_Value_1 = Limit_u8(Black_Box_Value_1, 0, 255);
                // }
                // else if(l_start_point[0] == l_border_x && r_start_point[0] != r_border_x)
                // {
                //     Black_Box_Value_1 = image[start_row][r_start_point[0]] - 20;
                //     Black_Box_Value_1 = Limit_u8(Black_Box_Value_1, 0, 255);
                // }
                // else
                // {
                //     Black_Box_Value_1 = 20;
                // }

                return 1;
            }
            else
            {
                start_row = start_row - 1;      //当此行有一侧没找到，就向上移动一行重新找
            }
        }
    }
}

uint16 L_Thres_Record[MAX_LINE_POINTS];
uint16 R_Thres_Record[MAX_LINE_POINTS];
// 
const int8 L_Face_Dir[4][2] = { {0,-1},{1,0},{0,1},{-1,0} };  //左侧迷宫面向
//  0
//3   1
//  2

const int8 L_Face_Dir_L[4][2] = { {-1,-1},{1,-1},{1,1},{-1,1} };  //左侧面向的左前方
//0   1
//
//3   2

const int8 R_Face_Dir[4][2] = { {0,-1},{1,0},{0,1},{-1,0} };  //右侧迷宫面向
//  0
//3   1
//  2

const int8 R_Face_Dir_R[4][2] = { {1,-1},{1,1},{-1,1},{-1,-1} };  //右侧面向的右前方
//3   0
//
//2   1

const int8 Square_0[25][2] = {              //一个5 * 5的矩阵，用来求中心点周围的局部阈值（局部阈值）
{-2,-2},{-1,-2},{0,-2},{+1,-2},{+2,-2},
{-2,-1},{-1,-1},{0,-1},{+1,-1},{+2,-1},
{-2,-0},{-1, 0},{0, 0},{+1, 0},{+2,-0},
{-2,+1},{-1,+1},{0,+1},{+1,+1},{+2,+1},
{-2,+2},{-1,+2},{0,+2},{+1,+2},{+2,+2}
};
uint8 Clip_Value = 2;

//迷宫单侧停止爬线标志位
uint8 L_Stop_Flag = 0;
uint8 R_Stop_Flag = 0;
//**********************************************************
/******
* 函数功能：      求取赛道二维数组边线
* 特殊说明：      基于上交代码的自适应迷宫优化后的自适应八向迷宫
* 形  参：        uint16 Break_Flag         最大循环次数，防止卡死程序，一般为3~4倍图像宽度
*                uint8(*image)[Image_X]     提取边线的图像
*                uint8(*l_line)[2]          存放左侧边线的二维数组
*                uint8(*r_line)[2]          存放右侧边线的二维数组
*                int8 *l_dir                存放左侧边线每个点的生长方向
*                int8 *r_dir                存放右侧边线每个点的生长方向
*                uint16 *l_stastic          记录左侧边线点的个数
*                uint16 *r_stastic          记录右侧边线点的个数
*                uint8 *x_meet              记录左右两侧爬线相遇点的X坐标
*                uint8 *y_meet              记录左右两侧爬线相遇点的Y坐标
*                uint8 l_start_x            左侧爬线起始点的X坐标
*                uint8 l_start_y            左侧爬线起始点的Y坐标
*                uint8 r_start_x            右侧爬线起始点的X坐标
*                uint8 r_start_y            右侧爬线起始点的Y坐标
*                uint8 clip_value           计算每个阈值时相加的经验值，一般为-5 ~ 5，避免强行分割，可直接设为0
*
* 示例：         Dir_Labyrinth_5((uint16)Use_Num, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
                   Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1], Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], 0);
* 返回值：        无
*/
void Dir_Labyrinth_5(uint16 Break_Flag, uint8(*image)[Image_X], uint8(*l_line)[2], uint8(*r_line)[2], int8* l_dir, int8* r_dir, uint16* l_stastic, uint16* r_stastic, uint8* x_meet, uint8* y_meet,
    uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8 clip_value)
{
    uint8 j = 0;


    //左边变量
    uint8 L_Center_Point[2] = { 0 };     //存放每次找到的XY坐标
    uint16 L_Data_Statics = 0;          //统计左边找到的边线点的个数

    uint8  L_Front_Value = 0;           //左侧 面向的前方点的灰度值
    uint8  L_Front_L_Value = 0;         //左侧 面向的左前方点的灰度值

    uint8  L_Dir = 0;                   //此参数用于转向
    uint8  L_Turn_Num = 0;              //记录转向次数，若中心点前后左右都是黑色像素，就会在一个点转向四次，记录到四次时退出循环防止卡死程序
    uint16 L_Pixel_Value_Sum = 0;       //中心点与周围24个点的像素值和
    uint16 L_Thres = 0;                 //局部阈值,即L_Pixel_Value_Sum / 25

    //右边变量
    uint8  R_Center_Point[2] = { 0 };     //存放每次找到的XY坐标
    uint16 R_Data_Statics = 0;          //统计右边找到的边线点的个数

    uint8  R_Front_Value = 0;           //右侧 面向的前方点的灰度值
    uint8  R_Front_R_Value = 0;         //右侧 面向的左前方点的灰度值

    uint8  R_Dir = 0;                   //此参数用于转向
    uint8  R_Turn_Num = 0;              //记录转向次数，若中心点前后左右都是黑色像素，就会在一个点转向四次，记录到四次时退出循环防止卡死程序
    uint16 R_Pixel_Value_Sum = 0;       //中心点与周围24个点的像素值和
    uint16 R_Thres = 0;                 //局部阈值

    //第一次更新坐标点  将找到的起点值传进来
    L_Center_Point[0] = l_start_x + 1; //x
    L_Center_Point[1] = l_start_y; //y
    R_Center_Point[0] = r_start_x - 1;//x
    R_Center_Point[1] = r_start_y;//y

    L_Stop_Flag = 0;  // 0:继续爬线    1：进入死区，停止爬线
    R_Stop_Flag = 0;

    //开启方向迷宫循环
    while (Break_Flag--)
    {
        //左边
       //判定出死区后，挂出停止标志位，单侧爬线停止。
        if (L_Stop_Flag == 0)
        {
            l_line[L_Data_Statics][0] = L_Center_Point[0];  //找到的中心点X坐标计入左边线数组
            l_line[L_Data_Statics][1] = L_Center_Point[1];  //找到的中心点Y坐标计入左边线数组

            if (L_Data_Statics != 0)
            {
                switch (l_dir[L_Data_Statics - 1])  //下面这一坨可以根据上一个点的生长方向大幅优化爬线时间
                {
                    //从第二个点开始，第一个点的阈值要25个点全部加一遍
                    //当横向或纵向生长时，将原先的25次加法运算简化为十次计算
                    //当斜向生长时，将原先的25次加法计算简化为十八次运算
                    //可以画出图来，跟着代码走几遍，就可以很快速的理解
                case 1:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 3][L_Center_Point[0] + 2] - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 0] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2];
                    break;
                }
                case -2:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 0][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 2][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 3][L_Center_Point[0] + 2]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 0]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2];
                    break;
                }
                case -3:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 2][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 0][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 2][L_Center_Point[0] + 3]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2];
                    break;
                }
                case -4:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 2]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 2][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 0][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2];
                    break;
                }
                case -1:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 3][L_Center_Point[0] - 2] - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2];
                    break;
                }
                case 2:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 0][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 2][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 3][L_Center_Point[0] - 2]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2];
                    break;
                }
                case 3:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 2][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 0][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 2][L_Center_Point[0] - 3]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2];
                    break;
                }
                case 4:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 0]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 2]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 2][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 0][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2];
                    break;
                }
                }
            }
            else
            {
                for (j = 0; j < 25; j++)    //第一个阈值将25个点全部加一遍，后续点的阈值根据生长方向计算
                {
                    L_Pixel_Value_Sum += image[L_Center_Point[1] + Square_0[j][1]][L_Center_Point[0] + Square_0[j][0]];
                }
            }

            L_Thres = (L_Pixel_Value_Sum + Thres_Interfere) / Thres_Num_Interfere;   //阈值为25个点灰度值的平均值
            L_Thres -= clip_value;              //将得到的灰度阈值减去一个经验值，用来优化判定

            //这里为反向平滑滤波（不知网上是否有这种算法，此处为本人原创），用于适应由较暗到（局部）高亮区域，或反之的光线情况。后续详细讲解原理
            if (Thres_Filiter_Flag_1 == 1 || Thres_Filiter_Flag_2 == 1)
            {
                if (L_Data_Statics > 3)
                {
                    L_Thres = L_Thres * 1.3f - L_Thres_Record[L_Data_Statics - 1] * 0.2f - L_Thres_Record[L_Data_Statics - 2] * 0.1f;
                }
            }

            L_Thres_Record[L_Data_Statics] = L_Thres;
            L_Data_Statics++;                   //每找到一个点统计个数+1

        L_Judge_Again:    //L_Judge_Again 与 goto 配合使用
            if (L_Stop_Flag == 0)
            {
                L_Front_Value = image[L_Center_Point[1] + L_Face_Dir[L_Dir][1]][L_Center_Point[0] + L_Face_Dir[L_Dir][0]];          //记录面向的前方点的灰度值
                L_Front_L_Value = image[L_Center_Point[1] + L_Face_Dir_L[L_Dir][1]][L_Center_Point[0] + L_Face_Dir_L[L_Dir][0]];    //记录面向的左前方点的灰度值
                if (L_Front_Value < L_Thres)     //面向的前方点是黑色
                {
                    L_Dir = (L_Dir + 1) % 4;    //需右转一次
                    L_Turn_Num++;
                    if (L_Turn_Num == 4)        //死区处理
                    {
                        L_Stop_Flag = 1;       //当前后左右都是黑色时，进入死区，停止左侧爬线
                    }
                    goto L_Judge_Again;
                }
                else if (L_Front_L_Value < L_Thres)   //左前方点是黑色，前方点是白色
                {

                    // 【新增】先计算下一步的坐标
                    // int next_x = L_Center_Point[0] + L_Face_Dir[L_Dir][0];
                    // int next_y = L_Center_Point[1] + L_Face_Dir[L_Dir][1];

                    // 【新增】边界判断：如果下一步超出图像范围，停止爬线
                    // 注意：Image_X 是图像宽度，Image_Y 是图像高度，请替换为你代码中的实际宏定义
                    if (L_Center_Point[0] + L_Face_Dir[L_Dir][0] < 3 || L_Center_Point[0] + L_Face_Dir[L_Dir][0] >= 77 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] < 2 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] >= 58)
                    {
                        L_Stop_Flag = 1;
                    }
                    else if (L_Center_Point[1] + L_Face_Dir[R_Dir][1] <= 5 && L_Center_Point[0] + L_Face_Dir[R_Dir][0] > 38)
                    {
                        L_Stop_Flag = 1;
                    }
                    else {
                        L_Center_Point[0] += L_Face_Dir[L_Dir][0];
                        L_Center_Point[1] += L_Face_Dir[L_Dir][1];      //向前走一步
                        l_dir[L_Data_Statics - 1] = (L_Face_Dir[L_Dir][0] * 3) - L_Face_Dir[L_Dir][1];
                        L_Turn_Num = 0;
                    }

                }
                else        //左前方和前方都是白色点
                {

                    // 【新增】先计算下一步的坐标
                    // int next_x = L_Center_Point[0] + L_Face_Dir[L_Dir][0];
                    // int next_y = L_Center_Point[1] + L_Face_Dir[L_Dir][1];

                    // 【新增】边界判断：如果下一步超出图像范围，停止爬线
                    // 注意：Image_X 是图像宽度，Image_Y 是图像高度，请替换为你代码中的实际宏定义
                    if (L_Center_Point[0] + L_Face_Dir[L_Dir][0] < 3 || L_Center_Point[0] + L_Face_Dir[L_Dir][0] >= 77 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] < 2 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] >= 58)
                    {
                        L_Stop_Flag = 1;
                    }
                    else if (L_Center_Point[1] + L_Face_Dir[R_Dir][1] <= 5 && L_Center_Point[0] + L_Face_Dir[R_Dir][0] > 38)
                    {
                        L_Stop_Flag = 1;
                    }
                    else {
                        L_Center_Point[0] += L_Face_Dir_L[L_Dir][0];
                        L_Center_Point[1] += L_Face_Dir_L[L_Dir][1];        //向左前方走一步
                        l_dir[L_Data_Statics - 1] = (L_Face_Dir_L[L_Dir][0] * 3) - L_Face_Dir_L[L_Dir][1];
                        L_Dir = (L_Dir + 3) % 4;        //左转一次
                        L_Turn_Num = 0;
                    }

                }
                if (L_Data_Statics >= 5)     //O环处理，即转了一圈后回到原处，也是一种死区，当立即停止爬线
                {
                    if (l_line[L_Data_Statics - 1][0] == l_line[L_Data_Statics - 5][0] &&
                        l_line[L_Data_Statics - 1][1] == l_line[L_Data_Statics - 5][1])
                    {
                        L_Stop_Flag = 1;
                    }
                }
            }
        }

        //右侧与左侧同理，代码也类似，理解左侧后右侧就很简单
        if (R_Stop_Flag == 0)
        {
            r_line[R_Data_Statics][0] = R_Center_Point[0];
            r_line[R_Data_Statics][1] = R_Center_Point[1];

            if (R_Data_Statics != 0)
            {
                switch (r_dir[R_Data_Statics - 1])
                {
                case 1:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 3][R_Center_Point[0] + 2] - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 0] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2];
                    break;
                }
                case -2:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 0][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 2][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 3][R_Center_Point[0] + 2]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 0]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2];
                    break;
                }
                case -3:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 2][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 0][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 2][R_Center_Point[0] + 3]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2];
                    break;
                }
                case -4:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 2]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 2][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 0][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2];
                    break;
                }
                case -1:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 3][R_Center_Point[0] - 2] - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2];
                    break;
                }
                case 2:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 0][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 2][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 3][R_Center_Point[0] - 2]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2];
                    break;
                }
                case 3:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 2][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 0][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 2][R_Center_Point[0] - 3]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2];
                    break;
                }
                case 4:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 0]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 2]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 2][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 0][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2];
                    break;
                }
                }
            }
            else
            {
                for (j = 0; j < 25; j++)
                {
                    R_Pixel_Value_Sum += image[R_Center_Point[1] + Square_0[j][1]][R_Center_Point[0] + Square_0[j][0]];
                }
            }

            R_Thres = (R_Pixel_Value_Sum + Thres_Interfere) / Thres_Num_Interfere;
            R_Thres -= clip_value;

            if (Thres_Filiter_Flag_1 == 1 || Thres_Filiter_Flag_2 == 1)
            {
                if (R_Data_Statics > 3)
                {
                    R_Thres = R_Thres * 1.3f - R_Thres_Record[R_Data_Statics - 1] * 0.2f - R_Thres_Record[R_Data_Statics - 2] * 0.1f;
                }
            }

            R_Thres_Record[R_Data_Statics] = R_Thres;

            R_Data_Statics++;

        R_Judgme_Again:
            if (R_Stop_Flag == 0)
            {
                R_Front_Value = image[R_Center_Point[1] + R_Face_Dir[R_Dir][1]][R_Center_Point[0] + R_Face_Dir[R_Dir][0]];
                R_Front_R_Value = image[R_Center_Point[1] + R_Face_Dir_R[R_Dir][1]][R_Center_Point[0] + R_Face_Dir_R[R_Dir][0]];
                if (R_Front_Value < R_Thres)
                {
                    R_Dir = (R_Dir + 3) % 4;
                    R_Turn_Num++;
                    if (R_Turn_Num == 4)
                    {
                        R_Stop_Flag = 1;
                    }
                    goto R_Judgme_Again;
                }
                else if (R_Front_R_Value < R_Thres)
                {
                    // 【新增】先计算下一步的坐标（向前）
                    // int next_x = R_Center_Point[0] + R_Face_Dir[R_Dir][0];
                    // int next_y = R_Center_Point[1] + R_Face_Dir[R_Dir][1];

                    // 【新增】边界判断
                    if (R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 3 || R_Center_Point[0] + R_Face_Dir[R_Dir][0] >= 77 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] < 2 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] >= 58)
                    {
                        R_Stop_Flag = 1;
                    }
                    else if (R_Center_Point[1] + R_Face_Dir[R_Dir][1] <= 5 && R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 42)
                    {
                        R_Stop_Flag = 1;
                    }
                    else {
                        R_Center_Point[0] += R_Face_Dir[R_Dir][0];
                        R_Center_Point[1] += R_Face_Dir[R_Dir][1];
                        r_dir[R_Data_Statics - 1] = R_Face_Dir[R_Dir][0] * 3 - R_Face_Dir[R_Dir][1];
                        R_Turn_Num = 0;
                    }

                }
                else
                {

                    // 【新增】先计算下一步的坐标（向前）
                    // int next_x = R_Center_Point[0] + R_Face_Dir[R_Dir][0];
                    // int next_y = R_Center_Point[1] + R_Face_Dir[R_Dir][1];

                    // 【新增】边界判断
                    if (R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 3 || R_Center_Point[0] + R_Face_Dir[R_Dir][0] >= 77 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] < 2 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] >= 58)
                    {
                        R_Stop_Flag = 1;
                    }
                    else if (R_Center_Point[1] + R_Face_Dir[R_Dir][1] <= 5 && R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 42)
                    {
                        R_Stop_Flag = 1;
                    }
                    else {
                        R_Center_Point[0] += R_Face_Dir_R[R_Dir][0];
                        R_Center_Point[1] += R_Face_Dir_R[R_Dir][1];
                        r_dir[R_Data_Statics - 1] = R_Face_Dir_R[R_Dir][0] * 3 - R_Face_Dir_R[R_Dir][1];
                        R_Dir = (R_Dir + 1) % 4;
                        R_Turn_Num = 0;
                    }

                }
                if (R_Data_Statics >= 5)
                {
                    if (r_line[R_Data_Statics - 1][0] == r_line[R_Data_Statics - 5][0] &&
                        r_line[R_Data_Statics - 1][1] == r_line[R_Data_Statics - 5][1])
                    {
                        R_Stop_Flag = 1;
                    }
                }
            }
        }

        if (L_Stop_Flag == 0 && R_Stop_Flag == 0)
        {
            if ((My_ABS(r_line[R_Data_Statics - 1][0] - l_line[L_Data_Statics - 1][0]) <= 3)
                && (My_ABS(r_line[R_Data_Statics - 1][1] - l_line[L_Data_Statics - 1][1]) <= 3))        //两侧爬线相遇，退出循环，一张图像爬线结束
            {
                *y_meet = (r_line[R_Data_Statics - 1][1] + l_line[L_Data_Statics - 1][1]) >> 1;  //记录相遇点Y
                *x_meet = (r_line[R_Data_Statics - 1][0] + l_line[L_Data_Statics - 1][0]) >> 1;  //记录相遇点X
                break;
            }
        }
        //有一侧存在死区时，对相遇点的判定放宽松一些，防止实际相遇但没有判定出，导致爬线紊乱的情况
        else
        {
            if ((My_ABS(r_line[R_Data_Statics - 1][0] - l_line[L_Data_Statics - 1][0]) <= 5)
                && (My_ABS(r_line[R_Data_Statics - 1][1] - l_line[L_Data_Statics - 1][1]) <= 5))        //两侧爬线相遇，退出循环，一张图像爬线结束
            {
                *y_meet = (r_line[R_Data_Statics - 1][1] + l_line[L_Data_Statics - 1][1]) >> 1;  //记录相遇点Y
                *x_meet = (r_line[R_Data_Statics - 1][0] + l_line[L_Data_Statics - 1][0]) >> 1;  //记录相遇点X
                break;
            }
        }
    }
    L_Stop_Flag = 0;
    R_Stop_Flag = 0;
    *l_stastic = L_Data_Statics;    //记录左侧边线点个数
    *r_stastic = R_Data_Statics;    //记录右侧边线点个数
}

void Get_Border(uint16 l_total, uint16 r_total, uint8 start, uint8 end, uint8* l_border, uint8* r_border, uint8(*l_line)[2], uint8(*r_line)[2])
{
    uint8 i;
    uint16 j;

    // 初始化
    for (i = 0; i < Image_Y; i++)
    {
        l_border[i] = X_Border_Min;
        r_border[i] = X_Border_Max;
    }

    // ========== 提取左边界：同一行取最右侧的点 ==========
    for (j = 0; j < l_total; j++)
    {
        uint8 x = l_line[j][0];
        uint8 y = l_line[j][1];

        // 只处理有效行
        if (y >= end && y <= start)
        {
            // 保留最大X = 最靠右的左边界
            if (x > l_border[y])
            {
                l_border[y] = x;
            }
        }
    }

    // ========== 提取右边界：同一行取最左侧的点 ==========
    for (j = 0; j < r_total; j++)
    {
        uint8 x = r_line[j][0];
        uint8 y = r_line[j][1];

        if (y >= end && y <= start)
        {
            // 保留最小X = 最靠左的右边界
            if (r_border[y] == X_Border_Max || x < r_border[y])
            {
                r_border[y] = x;
            }
        }
    }
}

//*************************************************************************提取信息，判断元素**********************************************************************************
#define Q_SHIFT 12
#define Q_SCALE (1 << Q_SHIFT)

/**
* 函数功能：      最小二乘法计算斜率(定点数版本)
* 特殊说明：      无
* 形  参：        uint8 begin                输入起点
*                 uint8 end                  输入终点
*                 uint8 *border              输入需要计算斜率的一维边线数组
* 示例：          Slope_Calculate(start, end, border);
* 返回值：        Result    计算出的斜率（定点数Q12格式）
*/
int32 Slope_Calculate(uint8 begin, uint8 end, uint8* border)    //注：begin 必须小于 end,一般 begin 位于图像上方， end 位于图像下方
{
    int32 X_Sum = 0, Y_Sum = 0, XY_Sum = 0, X2_Sum = 0;
    int16 i = 0;
    int32 Result = 0;
    static int32 Result_Last;
    int16 n = end - begin;

    for (i = begin; i < end; i++)
    {
        X_Sum += i;
        Y_Sum += border[i];
        XY_Sum += (int32)i * border[i];
        X2_Sum += (int32)i * i;
    }

    int32 denominator = (int32)n * X2_Sum - (int32)X_Sum * X_Sum;
    if (denominator != 0)      //防止为0的情况出现
    {
        int32 numerator = (int32)n * XY_Sum - (int32)X_Sum * Y_Sum;
        Result = (int32)(((int64)numerator * Q_SCALE) / denominator);
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
* @param float *slope_rate          输入斜率地址
* @param float *intercept           输入截距地址
*  @see CTest       calculate_s_i(start, end, r_border, &slope_l_rate, &intercept_l);
* @return 返回说明
*     -<em>false</em> fail
*     -<em>true</em> succeed
*/

/**
* 函数功能：      计算斜率截距(定点数版本)
* 特殊说明：      调用最小二乘法计算斜率
* 形  参：        uint8 start                输入起点
*                 uint8 end                  输入终点
*                 uint8 *border              输入需要计算斜率的一维边线数组
*                 int32 *slope_rate          存储斜率的变量地址（定点数Q12格式）
*                 int32 *intercept           存储截距的变量地址（定点数Q12格式）
* 示例：          Calculate_Slope_Intercept(start, end, L_Border, &L_Straightaway_Lope_Rate_C, &L_Intercept);
* 返回值：        无
*/
void Calculate_Slope_Intercept(uint8 start, uint8 end, uint8* border, int32* slope_rate, int32* intercept)
{
    uint16 i, Num = 0;
    int32 X_Sum = 0, Y_Sum = 0;
    int32 Y_Average = 0, X_Average = 0;

    for (i = start; i < end; i++)
    {
        X_Sum += i;
        Y_Sum += border[i];
        Num++;
    }

    if (Num)
    {
        X_Average = (int32)(((int64)X_Sum * Q_SCALE) / Num);
        Y_Average = (int32)(((int64)Y_Sum * Q_SCALE) / Num);
    }

    *slope_rate = Slope_Calculate(start, end, border);
    int32 slope_x_xavg = (int32)(((int64)(*slope_rate) * X_Average) >> Q_SHIFT);
    *intercept = Y_Average - slope_x_xavg;
}

uint8 Straight_Thres = 15;
uint8 Sml_Arc_Thres = 10;
uint8 Big_Arc_thres = 130;

//求方差(定点数版本)
//uint8 Num = 26;
void Get_Variance(uint8 start, uint8 end, int16* fitting_line, uint8* actual_line, int32* variance, uint8 multiple)
{
    uint8 i = 0;
    int32 S = 0;
    uint8 total_num = 0;
    int16 Temp = 0;

    if (start % 3 != 0)
    {
        for (i = 1; i < 3; i++)
        {
            start = start + i;
            if (start % 3 == 1)
            {
                break;
            }
        }
    }

    total_num = (uint8)((end - start) / multiple);

    for (i = 0; i < total_num; i++)
    {
        uint8 Temp_Value = 0;
        Temp_Value = i * multiple + start;
        Temp = (int16)(actual_line[Temp_Value] - (uint8)fitting_line[Temp_Value / 3]);
        S += (int32)Temp * Temp;
    }
    *variance = (int32)(((int64)S * Q_SCALE) / total_num);
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
uint8 L_Up_Turn_Left_Point_1[2] = { 0 };      //存储上转左拐点的坐标
uint8 L_Up_Turn_Left_Point_Flag_1 = 0;      //上转左拐点存在标志位，找到时置1
uint8 L_Up_Turn_Left_Point_Position_1 = 0;  //记录位置，即是二维数组中的第几个点
float L_Up_Turn_Left_Point_Angle_1 = 0;     //记录找到的拐点的角度（逆透视求取）
void Get_L_Up_Turn_Left_Point_1(uint8 Grade)
{
    uint8 i = 0;
    L_Up_Turn_Left_Point_Flag_1 = 0;
    L_Up_Turn_Left_Point_1[0] = 0;
    L_Up_Turn_Left_Point_1[1] = 0;
    L_Up_Turn_Left_Point_Position_1 = 0;
    L_Up_Turn_Left_Point_Angle_1 = 0;       //将各个参数清零
    switch (Grade)
    {
    case 1:     //严格判断
    {
        for (i = 4; i < (Adaptive_L_Statics - 4); i++)
        {
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1) &&
                    (Adaptive_L_Grow_Dir[i + 2] == -3) && (Adaptive_L_Grow_Dir[i + 4] == -3) && (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3))
                {
                    L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Up_Turn_Left_Point_Flag_1 = 1;
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
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1) &&
                    (Adaptive_L_Grow_Dir[i + 2] == -3 || Adaptive_L_Grow_Dir[i + 2] == -4) && (Adaptive_L_Grow_Dir[i + 4] == -3 || Adaptive_L_Grow_Dir[i + 4] == -4) &&
                    (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == -4))
                {
                    L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Up_Turn_Left_Point_Flag_1 = 1;
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
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i - 2] == 4 || Adaptive_L_Grow_Dir[i - 2] == 1 || Adaptive_L_Grow_Dir[i - 2] == -2) && (Adaptive_L_Grow_Dir[i - 4] == 4 || Adaptive_L_Grow_Dir[i - 4] == 1 || Adaptive_L_Grow_Dir[i - 2] == -2) &&
                    (Adaptive_L_Grow_Dir[i + 2] == -3 || Adaptive_L_Grow_Dir[i + 2] == -4 || Adaptive_L_Grow_Dir[i + 2] == -2) && (Adaptive_L_Grow_Dir[i + 4] == -3 || Adaptive_L_Grow_Dir[i + 4] == -4 || Adaptive_L_Grow_Dir[i + 4] == -2) &&
                    (Adaptive_L_Grow_Dir[i] == -2 || Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == -4))
                {
                    L_Up_Turn_Left_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Up_Turn_Left_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Up_Turn_Left_Point_Flag_1 = 1;
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
uint8 L_Up_Turn_Left_Point_2[2] = { 0 };
uint8 L_Up_Turn_Left_Point_Flag_2 = 0;
uint8 L_Up_Turn_Left_Point_Position_2 = 0;
void Get_L_Up_Turn_Left_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    L_Up_Turn_Left_Point_2[0] = 0;
    L_Up_Turn_Left_Point_2[1] = 0;
    L_Up_Turn_Left_Point_Position_2 = 0;
    if (Region_Start <= Region_End)
    {
        for (i = Region_Start; i <= Region_End; i++)
        {
            if (L_Border[i] - L_Border[i - 1] >= 6)
            {
                if (L_Border[i + 1] - L_Border[i - 2] >= 6)
                {
                    if (L_Border[i - 1] == 2 && L_Border[i - 2] == 2 && L_Border[i] != 2)
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
uint8 L_Up_Turn_Left_Point[2] = { 0 };
uint8 L_Up_Turn_Left_Point_Flag = 0;
void Get_L_Up_Turn_Left_Point(void)
{
    L_Up_Turn_Left_Point_Flag = 0;
    Get_L_Up_Turn_Left_Point_1(2);
    if (L_Up_Turn_Left_Point_Flag_1 == 1)
    {
        L_Up_Turn_Left_Point_Flag_1 = 0;
        Get_L_Up_Turn_Left_Point_2(L_Up_Turn_Left_Point_1[1] - 7, L_Up_Turn_Left_Point_1[1] + 7);
        {
            if (L_Up_Turn_Left_Point_Flag_2 == 1)
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
uint8 L_Right_Turn_Up_Point_1[2] = { 0 };
uint8 L_Right_Turn_Up_Point_Flag_1 = 0;
uint8 L_Right_Turn_Up_Point_Position_1 = 0;
float L_Right_Turn_Up_Point_Angle_1 = 0;
void Get_L_Right_Turn_Up_Point_1(uint8 Grade)
{
    uint8 i = 0;
    L_Right_Turn_Up_Point_Flag_1 = 0;
    L_Right_Turn_Up_Point_1[0] = 0;
    L_Right_Turn_Up_Point_1[1] = 0;
    L_Right_Turn_Up_Point_Position_1 = 0;
    L_Right_Turn_Up_Point_Angle_1 = 0;
    switch (Grade)
    {
    case 1:
    {
        for (i = 5; (i + 7) < Adaptive_L_Statics; i++)
        {
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1) && (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1) &&
                    (Adaptive_L_Grow_Dir[i - 2] == 3) && (Adaptive_L_Grow_Dir[i - 5] == 3) && (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                {
                    L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Right_Turn_Up_Point_Flag_1 = 1;
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
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1) && (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1) &&
                    (Adaptive_L_Grow_Dir[i - 2] == 2 || Adaptive_L_Grow_Dir[i - 2] == 3) && (Adaptive_L_Grow_Dir[i - 5] == 2 || Adaptive_L_Grow_Dir[i - 5] == 3) &&
                    (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                {
                    L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Right_Turn_Up_Point_Flag_1 = 1;
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
            if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_L_Grow_Dir[i + 2] == 4 || Adaptive_L_Grow_Dir[i + 2] == 1 || Adaptive_L_Grow_Dir[i + 2] == -2) && (Adaptive_L_Grow_Dir[i + 5] == 4 || Adaptive_L_Grow_Dir[i + 5] == 1 || Adaptive_L_Grow_Dir[i + 5] == -2) &&
                    (Adaptive_L_Grow_Dir[i + 7] == 4 || Adaptive_L_Grow_Dir[i + 7] == 1 || Adaptive_L_Grow_Dir[i + 7] == -2) &&
                    (Adaptive_L_Grow_Dir[i - 2] == 2 || Adaptive_L_Grow_Dir[i - 2] == 3 || Adaptive_L_Grow_Dir[i - 2] == 4) && (Adaptive_L_Grow_Dir[i - 5] == 2 || Adaptive_L_Grow_Dir[i - 5] == 3 || Adaptive_L_Grow_Dir[i - 5] == 4) &&
                    (Adaptive_L_Grow_Dir[i] == 4 || Adaptive_L_Grow_Dir[i] == 1))
                {
                    L_Right_Turn_Up_Point_1[0] = Adaptive_L_Line[i][0];
                    L_Right_Turn_Up_Point_1[1] = Adaptive_L_Line[i][1];
                    L_Right_Turn_Up_Point_Flag_1 = 1;
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
uint8 L_Right_Turn_Up_Point_2[2] = { 0 };
uint8 L_Right_Turn_Up_Point_Flag_2 = 0;
uint8 L_Right_Turn_Up_Point_Position_2 = 0;
void Get_L_Right_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    L_Right_Turn_Up_Point_2[0] = 0;
    L_Right_Turn_Up_Point_2[1] = 0;
    L_Right_Turn_Up_Point_Position_2 = 0;
    if (Region_Start <= Region_End)
    {
        for (i = Region_Start; i <= Region_End; i++)
        {
            if (L_Border[i] - L_Border[i + 1] >= 6)
            {
                if (L_Border[i - 1] - L_Border[i + 2] >= 6)
                {
                    if (L_Border[i + 1] == 2 && L_Border[i + 2] == 2 && L_Border[i] != 2)
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
uint8 L_Right_Turn_Up_Point[2] = { 0 };
uint8 L_Right_Turn_Up_Point_Flag = 0;
void Get_L_Right_Turn_Up_Point(void)
{
    L_Right_Turn_Up_Point_Flag = 0;
    Get_L_Right_Turn_Up_Point_1(2);
    if (L_Right_Turn_Up_Point_Flag_1 == 1)
    {
        L_Right_Turn_Up_Point_Flag_1 = 0;
        Get_L_Right_Turn_Up_Point_2(L_Right_Turn_Up_Point_1[1] - 7, L_Right_Turn_Up_Point_1[1] + 7);
        {
            if (L_Right_Turn_Up_Point_Flag_2 == 1)
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
* 特殊说明：      分为三个严格等级，根据每个点的生长方向判断
*                 寻找方式为遍历二维数组边线的每个点
* 形  参：        uint8 Grade      选择判定严格等级，常用2，即中等等级
*
* 示例：          Get_R_Up_Turn_Right_Point_1(2);
* 返回值：        无
*/
uint8 R_Up_Turn_Right_Point_1[2] = { 0 };
uint8 R_Up_Turn_Right_Point_Flag_1 = 0;
uint8 R_Up_Turn_Right_Point_Position_1 = 0;
float R_Up_Turn_Right_Point_Angle_1 = 0;
void Get_R_Up_Turn_Right_Point_1(uint8 Grade)
{
    uint8 i = 0;
    R_Up_Turn_Right_Point_Flag_1 = 0;
    R_Up_Turn_Right_Point_1[0] = 0;
    R_Up_Turn_Right_Point_1[1] = 0;
    R_Up_Turn_Right_Point_Position_1 = 0;
    R_Up_Turn_Right_Point_Angle_1 = 0;

    switch (Grade)
    {
    case 1:
    {
        for (i = 4; i < (Adaptive_R_Statics - 4); i++)
        {
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
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
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
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
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
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
    //        R_Up_Turn_Right_Point_Angle_1 = Get_Turn_Point_Angle(Adaptive_R_Line[i - 5][0], Adaptive_R_Line[i - 5][1], Adaptive_R_Line[i][0], Adaptive_R_Line[i][1], Adaptive_R_Line[i + 5][0], Adaptive_R_Line[i + 5][1]);
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
uint8 R_Up_Turn_Right_Point_2[2] = { 0 };
uint8 R_Up_Turn_Right_Point_Flag_2 = 0;
uint8 R_Up_Turn_Right_Point_Position_2 = 0;
void Get_R_Up_Turn_Right_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    R_Up_Turn_Right_Point_2[0] = 0;
    R_Up_Turn_Right_Point_2[1] = 0;
    R_Up_Turn_Right_Point_Position_2 = 0;
    if (Region_Start <= Region_End)
    {
        for (i = Region_Start; i <= Region_End; i++)
        {
            if ((int8)R_Border[i] - (int8)R_Border[i - 1] <= -6)
            {
                if ((int8)R_Border[i + 1] - (int8)R_Border[i - 2] <= -6)
                {
                    if (R_Border[i - 1] == 77 && R_Border[i - 2] == 77 && R_Border[i] != 77)
                    {
                        R_Up_Turn_Right_Point_2[0] = R_Border[i];
                        R_Up_Turn_Right_Point_2[1] = i;
                        R_Up_Turn_Right_Point_Flag_2 = 1;
                        R_Up_Turn_Right_Point_Position_2 = i;
                    }
                }
            }
        }
    }
}
uint8 R_Up_Turn_Right_Point[2] = { 0 };
uint8 R_Up_Turn_Right_Point_Flag = 0;
void Get_R_Up_Turn_Right_Point(void)
{
    R_Up_Turn_Right_Point_Flag = 0;
    Get_R_Up_Turn_Right_Point_1(2);
    if (R_Up_Turn_Right_Point_Flag_1 == 1)
    {
        R_Up_Turn_Right_Point_Flag_1 = 0;
        Get_R_Up_Turn_Right_Point_2(R_Up_Turn_Right_Point_1[1] - 7, R_Up_Turn_Right_Point_1[1] + 7);
        {
            if (R_Up_Turn_Right_Point_Flag_2 == 1)
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
uint8 R_Left_Turn_Up_Point_1[2] = { 0 };
uint8 R_Left_Turn_Up_Point_Flag_1 = 0;
uint8 R_Left_Turn_Up_Point_Position_1 = 0;
float R_Left_Turn_Up_Point_Angle_1 = 0;
void Get_R_Left_Turn_Up_Point_1(uint8 Grade)
{
    uint8 i = 0;
    R_Left_Turn_Up_Point_Flag_1 = 0;
    R_Left_Turn_Up_Point_1[0] = 0;
    R_Left_Turn_Up_Point_1[1] = 0;
    R_Left_Turn_Up_Point_Position_1 = 0;
    R_Left_Turn_Up_Point_Angle_1 = 0;
    switch (Grade)
    {
    case 1:
    {
        for (i = 5; (i + 7) < Adaptive_R_Statics; i++)
        {
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
            {
                if ((Adaptive_R_Grow_Dir[i + 2] == -2 || Adaptive_R_Grow_Dir[i + 2] == 1) && (Adaptive_R_Grow_Dir[i + 5] == -2 || Adaptive_R_Grow_Dir[i + 5] == 1) && (Adaptive_R_Grow_Dir[i + 7] == -2 || Adaptive_R_Grow_Dir[i + 7] == 1) &&
                    (Adaptive_R_Grow_Dir[i - 2] == -3) && (Adaptive_R_Grow_Dir[i - 5] == -3) && (Adaptive_R_Grow_Dir[i] == -2 || Adaptive_R_Grow_Dir[i] == 1))
                {
                    R_Left_Turn_Up_Point_1[0] = Adaptive_R_Line[i][0];
                    R_Left_Turn_Up_Point_1[1] = Adaptive_R_Line[i][1];
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
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
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
            if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
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
uint8 R_Left_Turn_Up_Point_2[2] = { 0 };
uint8 R_Left_Turn_Up_Point_Flag_2 = 0;
uint8 R_Left_Turn_Up_Point_Position_2 = 0;
void Get_R_Left_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End)
{
    uint8 i = 0;
    if (Region_Start <= Region_End)
    {
        for (i = Region_Start; i <= Region_End; i++)
        {
            if ((int8)R_Border[i] - (int8)R_Border[i + 1] <= -6)
            {
                if ((int8)R_Border[i - 1] - (int8)R_Border[i + 2] <= -6)
                {
                    if (R_Border[i + 1] == 77 && R_Border[i + 2] == 77 && R_Border[i] != 77)
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
uint8 R_Left_Turn_Up_Point[2] = { 0 };
uint8 R_Left_Turn_Up_Point_Flag = 0;
void Get_R_Left_Turn_Up_Point(void)
{
    R_Left_Turn_Up_Point_Flag = 0;
    Get_R_Left_Turn_Up_Point_1(2);
    if (R_Left_Turn_Up_Point_Flag_1 == 1)
    {
        R_Left_Turn_Up_Point_Flag_1 = 0;
        Get_R_Left_Turn_Up_Point_2(R_Left_Turn_Up_Point_1[1] - 7, R_Left_Turn_Up_Point_1[1] + 7);
        {
            if (R_Left_Turn_Up_Point_Flag_2 == 1)
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
uint8 L_Arc_Turn_Point[3][2] = { {0} };   //最多找三个就足够使用
uint8 L_Arc_Turn_Point_Flag = 0;    //找到一个就挂出标志位
uint8 L_Arc_Turn_Point_Num = 0;     //记录找到的个数
void Get_L_Arc_Turn_Point(void)
{
    uint8 i = 0;
    L_Arc_Turn_Point_Flag = 0;

    for (i = 0; i < 3; i++)      //每次调用先清零
    {
        L_Arc_Turn_Point_Num = 0;
        L_Arc_Turn_Point[i][0] = 0;
        L_Arc_Turn_Point[i][1] = 0;
    }

    for (i = 7; i < (Adaptive_L_Statics - 5); i++)
    {
        if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 7)))
        {
            if ((Adaptive_L_Line[i][0] > Adaptive_L_Line[i - 7][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i - 4][0]) && (Adaptive_L_Line[i][0] >= Adaptive_L_Line[i - 1][0]) &&
                (Adaptive_L_Line[i][0] >= Adaptive_L_Line[i + 1][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i + 3][0]) && (Adaptive_L_Line[i][0] > Adaptive_L_Line[i + 5][0]) &&
                (Adaptive_L_Grow_Dir[i + 1] == -2 || Adaptive_L_Grow_Dir[i + 1] == 1) && (Adaptive_L_Grow_Dir[i + 3] == -2 || Adaptive_L_Grow_Dir[i + 3] == 1) &&
                (Adaptive_L_Grow_Dir[i - 1] == 1 || Adaptive_L_Grow_Dir[i + 1] == 4) && (Adaptive_L_Grow_Dir[i - 3] == 1 || Adaptive_L_Grow_Dir[i - 3] == 4))                    //左侧内凹
            {
                L_Arc_Turn_Point[L_Arc_Turn_Point_Num][0] = Adaptive_L_Line[i][0];
                L_Arc_Turn_Point[L_Arc_Turn_Point_Num][1] = Adaptive_L_Line[i][1];
                L_Arc_Turn_Point_Flag = 1;
                L_Arc_Turn_Point_Num++;
                i += 15;    //每个圆弧拐点前后几个点可能都满足圆弧拐点的条件，所以找到后加15个点
                if (L_Arc_Turn_Point_Num == 3)
                {
                    break;
                }
            }
        }
    }

    for (i = 7; i < (Adaptive_L_Statics - 5); i++)
    {
        if ((Adaptive_L_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_L_Line[i][1] <= (Y_Border_Max - 7)))
        {

            if ((Adaptive_L_Line[i][0] < Adaptive_L_Line[i - 7][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i - 4][0]) && (Adaptive_L_Line[i][0] <= Adaptive_L_Line[i - 1][0]) &&
                (Adaptive_L_Line[i][0] <= Adaptive_L_Line[i + 1][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i + 3][0]) && (Adaptive_L_Line[i][0] < Adaptive_L_Line[i + 5][0]) &&
                (Adaptive_L_Grow_Dir[i + 1] == 4 || Adaptive_L_Grow_Dir[i + 1] == 1) && (Adaptive_L_Grow_Dir[i + 3] == 4 || Adaptive_L_Grow_Dir[i + 3] == 1) &&
                (Adaptive_L_Grow_Dir[i - 1] == 1 || Adaptive_L_Grow_Dir[i + 1] == -2) && (Adaptive_L_Grow_Dir[i - 3] == 1 || Adaptive_L_Grow_Dir[i - 3] == -2))                    //左侧外凸
            {
                L_Arc_Turn_Point[L_Arc_Turn_Point_Num][0] = Adaptive_L_Line[i][0];
                L_Arc_Turn_Point[L_Arc_Turn_Point_Num][1] = Adaptive_L_Line[i][1];
                L_Arc_Turn_Point_Flag = 1;
                L_Arc_Turn_Point_Num++;
                i += 15;
                if (L_Arc_Turn_Point_Num == 3)
                {
                    break;
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
uint8 R_Arc_Turn_Point[3][2] = { {0} };
uint8 R_Arc_Turn_Point_Flag = 0;
uint8 R_Arc_Turn_Point_Num = 0;
void Get_R_Arc_Turn_Point(void)
{
    uint8 i = 0;
    R_Arc_Turn_Point_Flag = 0;
    for (i = 0; i < 3; i++)
    {
        R_Arc_Turn_Point_Num = 0;
        R_Arc_Turn_Point[i][0] = 0;
        R_Arc_Turn_Point[i][1] = 0;
    }

    for (i = 7; i < (Adaptive_R_Statics - 5); i++)
    {
        if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
        {
            if ((Adaptive_R_Line[i][0] < Adaptive_R_Line[i - 7][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i - 4][0]) && (Adaptive_R_Line[i][0] <= Adaptive_R_Line[i - 1][0]) &&
                (Adaptive_R_Line[i][0] <= Adaptive_R_Line[i + 1][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i + 3][0]) && (Adaptive_R_Line[i][0] < Adaptive_R_Line[i + 5][0]) &&
                (Adaptive_R_Grow_Dir[i + 1] == 4 || Adaptive_R_Grow_Dir[i + 1] == 1) && (Adaptive_R_Grow_Dir[i + 3] == 4 || Adaptive_R_Grow_Dir[i + 3] == 1) &&
                (Adaptive_R_Grow_Dir[i - 1] == 1 || Adaptive_R_Grow_Dir[i + 1] == -2) && (Adaptive_R_Grow_Dir[i - 3] == 1 || Adaptive_R_Grow_Dir[i - 3] == -2))                    //右侧内凹
            {
                R_Arc_Turn_Point[R_Arc_Turn_Point_Num][0] = Adaptive_R_Line[i][0];
                R_Arc_Turn_Point[R_Arc_Turn_Point_Num][1] = Adaptive_R_Line[i][1];
                R_Arc_Turn_Point_Flag = 1;
                R_Arc_Turn_Point_Num++;
                i += 15;
                if (R_Arc_Turn_Point_Num == 3)
                {
                    break;
                }
            }
        }
    }
    for (i = 7; i < (Adaptive_R_Statics - 5); i++)
    {
        if ((Adaptive_R_Line[i][1] >= (Y_Border_Min + 2)) && (Adaptive_R_Line[i][1] <= (Y_Border_Max - 2)))
        {
            if ((Adaptive_R_Line[i][0] > Adaptive_R_Line[i - 7][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i - 4][0]) && (Adaptive_R_Line[i][0] >= Adaptive_R_Line[i - 1][0]) &&
                (Adaptive_R_Line[i][0] >= Adaptive_R_Line[i + 1][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i + 3][0]) && (Adaptive_R_Line[i][0] > Adaptive_R_Line[i + 5][0]) &&
                (Adaptive_R_Grow_Dir[i + 1] == -2 || Adaptive_R_Grow_Dir[i + 1] == 1) && (Adaptive_R_Grow_Dir[i + 3] == -2 || Adaptive_R_Grow_Dir[i + 3] == 1) &&
                (Adaptive_R_Grow_Dir[i - 1] == 1 || Adaptive_R_Grow_Dir[i + 1] == 4) && (Adaptive_R_Grow_Dir[i - 3] == 1 || Adaptive_R_Grow_Dir[i - 3] == 4))                    //右侧外凸
            {
                R_Arc_Turn_Point[R_Arc_Turn_Point_Num][0] = Adaptive_R_Line[i][0];
                R_Arc_Turn_Point[R_Arc_Turn_Point_Num][1] = Adaptive_R_Line[i][1];
                R_Arc_Turn_Point_Flag = 1;
                R_Arc_Turn_Point_Num++;
                i += 15;
                if (R_Arc_Turn_Point_Num == 3)
                {
                    break;
                }
            }
        }
    }
}

int32 Max_Slope_Dif_Thre = 2458;     //0.6 * 4096 = 2458（定点数Q12格式），实测得出，并未使用
int32 Min_Slope_Dif_Thre = 1024;     //0.25 * 4096 = 1024（定点数Q12格式），实测得出，当两段线段的斜率低于这个阈值时，可认为斜率相等
//斜率和截距（定点数Q12格式）
int32 R_Straightaway_Lope_Rate_A = 0;   //第一段的斜率
int32 R_Straightaway_Lope_Rate_B = 0;   //第二段的斜率
int32 R_Straightaway_Lope_Rate_C = 0;   //整段边线的斜率
int32 R_Intercept = 0;      //整段边线的截距
uint8 R_Straight_Flag = 0;      //右侧边线是否为直线标志位

/**
* 函数功能：      求右侧边线斜率截距，并判断是否为直线(定点数版本)
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

    if (My_ABS_I(R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_B) <= Min_Slope_Dif_Thre &&
        My_ABS_I(R_Straightaway_Lope_Rate_B - R_Straightaway_Lope_Rate_C) <= Min_Slope_Dif_Thre &&
        My_ABS_I(R_Straightaway_Lope_Rate_A - R_Straightaway_Lope_Rate_C) <= Min_Slope_Dif_Thre)      //判断是否为直线
    {
        R_Straight_Flag = 1;
    }
}


int32 L_Straightaway_Lope_Rate_A = 0;
int32 L_Straightaway_Lope_Rate_B = 0;
int32 L_Straightaway_Lope_Rate_C = 0;
int32 L_Intercept = 0;
uint8 L_Straight_Flag = 0;

/**
* 函数功能：      求左侧边线斜率截距，并判断是否为直线(定点数版本)
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

    if (My_ABS_I(L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_B) <= Min_Slope_Dif_Thre &&
        My_ABS_I(L_Straightaway_Lope_Rate_B - L_Straightaway_Lope_Rate_C) <= Min_Slope_Dif_Thre &&
        My_ABS_I(L_Straightaway_Lope_Rate_A - L_Straightaway_Lope_Rate_C) <= Min_Slope_Dif_Thre)
    {
        L_Straight_Flag = 1;
    }
}

//拟合曲线
int16 L_Fitting_Line[] = { 0 };
int16 R_Fitting_Line[] = { 0 };
//方差（定点数Q12格式）
int32 L_Variance = 0;
int32 R_Variance = 0;

/**
* 函数功能：      计算左右两侧边线的方差(定点数版本)
* 特殊说明：      每三行计算一次，减少计算量，但获取的信息不那么精确，可以选择计算每一行
* 形  参：        无
*
* 示例：          Get_Fitting_Line_And_Variance();
* 返回值：        无
*/
void Get_Fitting_Line_And_Variance(void)
{
    uint8 i = 0;

    for (i = 1; i <= 19; i++)
    {
        int32 x = i * 3;
        int32 l_kx = (int32)(((int64)L_Straightaway_Lope_Rate_C * x) >> Q_SHIFT);
        L_Fitting_Line[i] = (int16)((l_kx + L_Intercept) >> Q_SHIFT); //y = kx+b(把XY轴交换下就能想通了)
        L_Fitting_Line[i] = Limit_16(L_Fitting_Line[i], X_Border_Min + 2, X_Border_Max - 2);

        int32 r_kx = (int32)(((int64)R_Straightaway_Lope_Rate_C * x) >> Q_SHIFT);
        R_Fitting_Line[i] = (int16)((r_kx + R_Intercept) >> Q_SHIFT);
        R_Fitting_Line[i] = Limit_16(R_Fitting_Line[i], X_Border_Min + 2, X_Border_Max - 2);
    }

    Get_Variance(Adaptive_L_Line[Adaptive_L_Statics][1] + 1, Adaptive_L_Line[0][1], L_Fitting_Line, L_Border, &L_Variance, 3);
    Get_Variance(Adaptive_R_Line[Adaptive_R_Statics][1] + 1, Adaptive_R_Line[0][1], R_Fitting_Line, R_Border, &R_Variance, 3);
}


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
    if (Adaptive_L_Statics < 2 || Adaptive_L_Line[0][1] <= Adaptive_Y_Meet)
    {
        return;
    }

    for (i = Adaptive_Y_Meet; i < Adaptive_L_Line[0][1]; i++)
    {
        if (L_Border[i] == 3)    //左右两侧用一维边线即可
        {
            L_Border_Point_Num++;
        }
    }
    if (Adaptive_L_Statics > 42)  // 确保 40 < Adaptive_L_Statics - 2
    {
        for (i = 40; i < Adaptive_L_Statics - 2; i++)
        {
            if (Adaptive_L_Line[i][1] == 2)   //顶部的边界点使用二维边线
            {
                L_UP_Border_Point_Num++;
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
    if (Adaptive_R_Statics < 2 || Adaptive_R_Line[0][1] <= Adaptive_Y_Meet)
    {
        return;
    }

    for (i = Adaptive_Y_Meet; i < Adaptive_R_Line[0][1]; i++)
    {
        if (R_Border[i] == 76)
        {
            R_Border_Point_Num++;
        }
    }
    if (Adaptive_R_Statics > 42)  // 确保 40 < Adaptive_R_Statics - 2
    {
        for (i = 40; i < Adaptive_R_Statics - 2; i++)
        {
            if (Adaptive_R_Line[i][1] == 2)
            {
                R_UP_Border_Point_Num++;
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
    for (i = Adaptive_Y_Meet; i < Adaptive_L_Start_Point[1]; i++)
    {
        if (L_Border[i] == 3 && R_Border[i] == 76)   //2为左侧边界的X坐标，77为右侧边界
        {
            Relative_Border_Point_Num++;
        }
    }
}

int8 Row_Difference[20] = { 0 };
uint8 Max_Row_Dif_Line_Num = 0;     //当某一行左侧边线点位于左侧边界上，右侧边线点位于右侧边线上，与对位边界行同理，可以记录下来
/**
* 函数功能：      寻找对位边界行的个数
* 特殊说明：      无
* 形  参：        无
*
* 示例：          Get_Relative_Border_Point_Num();
* 返回值：        无
*/
void Get_Row_Difference(uint8* l_border, uint8* r_border)
{
    uint8 i = 0;
    Max_Row_Dif_Line_Num = 0;
    for (i = 0; i < 20; i++)
    {
        Row_Difference[i] = (int8)(r_border[i * 3] - l_border[i * 3]);
        if (Row_Difference[i] == 73)
        {
            Max_Row_Dif_Line_Num++;
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
    Get_L_Intercept_And_Slope(Adaptive_L_Line[Adaptive_L_Statics][1] + 3, Adaptive_L_Line[0][1]);
    Get_R_Intercept_And_Slope(Adaptive_R_Line[Adaptive_R_Statics][1] + 3, Adaptive_R_Line[0][1]);
    //    Get_Fitting_Line_And_Variance();
    Get_Row_Difference(L_Border, R_Border);
    //    if(Image_Num % 10 == 0)
    //    {
    //        Barrier_Distance = TOF_Get_Distance_mm();
    //    }
}

void Print_Get_Information(void)
{
    cout << "========== 图像信息 ==========" << endl;
    cout << "--- 边线统计 ---" << endl;
    cout << "左边线点数: " << (int)L_Border_Point_Num << endl;
    cout << "右边线点数: " << (int)R_Border_Point_Num << endl;
    cout << "相对边线点数: " << (int)Relative_Border_Point_Num << endl;

    cout << "--- 起止点 ---" << endl;
    cout << "左边起点: (" << (int)Adaptive_L_Start_Point[0] << ", " << (int)Adaptive_L_Start_Point[1] << ")" << endl;
    cout << "右边起点: (" << (int)Adaptive_R_Start_Point[0] << ", " << (int)Adaptive_R_Start_Point[1] << ")" << endl;
    cout << "相遇点: (" << (int)Adaptive_X_Meet << ", " << (int)Adaptive_Y_Meet << ")" << endl;

    cout << "--- 边线拟合 ---" << endl;
    cout << "左边线截距: " << (int)L_Intercept << " 斜率: " << L_Straightaway_Lope_Rate_A << " " << L_Straightaway_Lope_Rate_B << " " << L_Straightaway_Lope_Rate_C << endl;
    cout << "右边线截距: " << (int)R_Intercept << " 斜率: " << R_Straightaway_Lope_Rate_A << " " << R_Straightaway_Lope_Rate_B << " " << R_Straightaway_Lope_Rate_C << endl;

    cout << "--- 拐点标志 ---" << endl;
    cout << "左上转左拐点: " << (int)L_Up_Turn_Left_Point_Flag_1 << " (" << (int)L_Up_Turn_Left_Point_1[0] << ", " << (int)L_Up_Turn_Left_Point_1[1] << ") [棕色]" << endl;
    cout << "右上转右拐点: " << (int)R_Up_Turn_Right_Point_Flag_1 << " (" << (int)R_Up_Turn_Right_Point_1[0] << ", " << (int)R_Up_Turn_Right_Point_1[1] << ") [白色]" << endl;
    cout << "左右转上拐点: " << (int)L_Right_Turn_Up_Point_Flag_1 << " (" << (int)L_Right_Turn_Up_Point_1[0] << ", " << (int)L_Right_Turn_Up_Point_1[1] << ") [青色]" << endl;
    cout << "右左转上拐点: " << (int)R_Left_Turn_Up_Point_Flag_1 << " (" << (int)R_Left_Turn_Up_Point_1[0] << ", " << (int)R_Left_Turn_Up_Point_1[1] << ") [品红色]" << endl;
    cout << "左弧拐点: " << (int)L_Arc_Turn_Point_Flag << " (" << (int)L_Arc_Turn_Point[0][0] << ", " << (int)L_Arc_Turn_Point[0][1] << ") [黄色]" << endl;
    cout << "右弧拐点: " << (int)R_Arc_Turn_Point_Flag << " (" << (int)R_Arc_Turn_Point[0][0] << ", " << (int)R_Arc_Turn_Point[0][1] << ") [黄色]" << endl;

    cout << "--- 赛道元素 ---" << endl;
    cout << "左直线标志: " << (int)L_Straight_Flag << endl;
    cout << "右直线标志: " << (int)R_Straight_Flag << endl;
    cout << "================================" << endl;
}

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

uint8 Element = 0;
uint8 buzzer_flag = 0;

uint8 Straightaway_Flag = 1;       //直道             0：关闭        1：开启
uint8 L_Turn_Flag = 1;       //左转弯道         0：关闭        1：开启
uint8 R_Turn_Flag = 1;       //右转弯道         0：关闭        1：开启
uint8 L_Circle_Flag = 1;       //左环岛           0：关闭        1：单边巡线处理     2.三点法补圆       3.二次贝塞尔补线     4.三次贝塞尔补线     5.拉格朗日补线   //最小二乘法拟合圆  牛顿插值补线   样条插值
uint8 R_Circle_Flag = 1;       //右环岛           0：关闭        1：单边巡线处理     2.三点法补圆       3.二次贝塞尔补线     4.三次贝塞尔补线     5.拉格朗日补线   //最小二乘法拟合圆  牛顿插值补线   样条插值
uint8 Cross_Flag = 1;       //十字路口         0：关闭        1：不做特殊处理     2.最小二乘法补线    3.二次贝塞尔补线处理
uint8 L_Oblique_Cross_Flag = 0;       //斜十字路口       0：关闭        1：开启（补线处理）
uint8 R_Oblique_Cross_Flag = 0;       //斜十字路口       0：关闭        1：开启（补线处理）
uint8 Zebra_Flag = 1;       //斑马线           0：关闭        1：开启
uint8 shot_CD = 0;          //射击冷却时间
uint8 stop_flag = 0;        //停止标志

// 斑马线检测点（用于绘图标记）
uint8 zebra_points[18][4] = { 0 };  // [行索引][first_point, end_point, is_valid, center_x]
uint8 zebra_shot_center[2] = { 0 }; // 射击中心坐标
uint8 zebra_found_flag = 0;       // 是否找到斑马线或圆环
uint8 ring_up_point = 0;          // 圆环上边界点
uint8 ring_down_point = 0;        // 圆环下边界点

uint8 Ramp_Flag = 0;       //坡道             0：关闭        1：开启
uint8 Three_Bif_Flag = 0;       //三岔             0：关闭        1：开启
uint8 Barrier_Flag = 0;       //障碍             0：关闭        1：开启
uint8 Disconnection_Flag = 0;       //断路             0：关闭        1：开启
uint8 T_Way_Flag = 0;       //T路口            0：关闭        1：开启
uint8 L_Garage_Flag = 0;       //左车库           0：关闭        1：开启
uint8 R_Garage_Flag = 0;       //右车库           0：关闭        1：开启
uint8 Small_S_Flag = 0;       //小S弯            0:关闭         1:识别

// 出界检测相关变量
uint8 Line_Lost_Flag = 0;       // 丢线标志（0：正常，1：丢线）
uint8 Out_Bound_Cnt = 0;        // 出界计数器（连续异常帧计数）
uint8 Out_Bound_Flag = 0;       // 出界标志（0：正常，1：出界）

uint8 Zebra_Count_Flag = 0;

// 打靶检测函数原型声明
void ring_shot_judge(uint8* l_border, uint8* r_border, uint8 image[ZIP_H][ZIP_W]);

uint8 Element_Judgement(uint8(*image)[Image_X], uint16 l_total_num, uint16 r_total_num, int8* l_dir, int8* r_dir, uint8(*l_line)[2], uint8(*r_line)[2], uint8 Adaptive_X_Meet, uint8 Adaptive_Y_Meet,
    uint8* l_border, uint8* r_border)
{
    uint8 Element = 0;
    uint8 i = 0;

    // 重置斑马线检测点
    for (i = 0; i < 4; i++) {
        zebra_points[i][0] = 0;
        zebra_points[i][1] = 0;
        zebra_points[i][2] = 0;
        zebra_points[i][3] = 0;
    }
    zebra_shot_center[0] = 0;
    zebra_shot_center[1] = 0;
    zebra_found_flag = 0;

    //扫行判断斑马线
    if (Zebra_Flag != 0)
    {
        uint8 zebra_check = 0;
        for (i = l_border[41]; i < r_border[41]; i++)
        {
            if ((image[41][i] - image[41][i + 1] >= 15 || image[41][i] - image[41][i + 1] <= -15))
            {
                zebra_check++;
            }
        }
        for (i = l_border[46]; i < r_border[46]; i++)
        {
            if ((image[46][i] - image[46][i + 1] >= 15 || image[46][i] - image[46][i + 1] <= -15))
            {
                zebra_check++;
            }
        }
        if (zebra_check >= 12)
        {
            if (stop_flag == 1) stop_flag = 2;
            zebra_found_flag = 1;
        }
    }

    // 调用打靶检测函数
    ring_shot_judge(l_border, r_border, image);

    //直线判断
    if (Element == 0 && Straightaway_Flag != 0)
    {
        if (L_Straight_Flag == 1 && R_Straight_Flag == 1)
        {
            //            if(L_Variance <= Straight_Thres && R_Variance <= Straight_Thres
            //               && My_ABS_F(R_Straightaway_Lope_Rate_A) <= 0.6 && My_ABS_F(R_Straightaway_Lope_Rate_B) <= 0.6 && My_ABS_F(R_Straightaway_Lope_Rate_C) <= 0.6
            //               && My_ABS_F(L_Straightaway_Lope_Rate_A) <= 0.6 && My_ABS_F(L_Straightaway_Lope_Rate_B) <= 0.6 && My_ABS_F(L_Straightaway_Lope_Rate_C) <= 0.6)
            //            {
            //                if((L_Border_Point_Num + R_Border_Point_Num <= 5) || (L_Border_Point_Num <= 10 && R_Border_Point_Num == 0) || (L_Border_Point_Num == 0 && R_Border_Point_Num <= 10))
            //                {
            //                    {
            if (Adaptive_Y_Meet <= 6 && Adaptive_X_Meet <= 55 && Adaptive_X_Meet >= 25)
            {
                Element = Straightaway;
            }
            //                    }
            //                }
            //            }
        }

        //        //坡道判断嵌入直线判断内
        //        if(Element == Straightaway && Ramp_Flag != 0 && Image_Num >= 10)
        //        {
        //            if(Barrier_Distance <= 700 && L_Border_Point_Num <= 8 && R_Border_Point_Num <= 8 && Adaptive_Y_Meet <= 4)
        //            {
        //                Element = Ramp;
        //            }
        //        }
    }

    //左弯道判断
    if (Element == 0 && L_Turn_Flag != 0)
    {
        if (Adaptive_X_Meet <= 30 && Adaptive_Y_Meet >= 10 && L_Border_Point_Num >= 7 && L_UP_Border_Point_Num == 0 && R_UP_Border_Point_Num == 0 && R_Border_Point_Num <= 1)
        {
            Element = L_Turn;
        }
    }
    //右弯道判断
    if (Element == 0 && R_Turn_Flag != 0)
    {
        if (Adaptive_X_Meet >= 50 && Adaptive_Y_Meet >= 10 && R_Border_Point_Num >= 7 && L_UP_Border_Point_Num == 0 && R_UP_Border_Point_Num == 0 && L_Border_Point_Num <= 1)
        {
            Element = R_Turn;
        }
    }

    //十字路口判断
        //正入判断
    if (Element == 0 && Cross_Flag != 0)
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
                    Element = Cross;
                }
            }
        }
    }
    //    左斜入判断
        // if(Element == 0 && Cross_Flag != 0)
        // {
        //     if(Adaptive_R_Start_Point[0] >= 65 && Adaptive_L_Start_Point[0] >= 20 && L_Up_Turn_Left_Point_Flag_1 == 1 && R_Left_Turn_Up_Point_Flag_1 == 1)
        //     {
        //         if(R_Left_Turn_Up_Point_1[1] <= 35)
        //         {
        //             uint8 Temp_Num = 0;
        //             for(i = 57; i > R_Left_Turn_Up_Point_1[1]; i--)
        //             {
        //                 if(R_Border[i] == 77)
        //                 {
        //                     Temp_Num ++;
        //                 }
        //             }
        //             if((float)Temp_Num / (float)(57 - R_Left_Turn_Up_Point_1[1]) >= 0.9f)
        //             {
        //                 if(Adaptive_Y_Meet >= 4 && L_Border_Point_Num >= 12 && R_Border_Point_Num >= 12)
        //                 {
        //                     if(Relative_Border_Point_Num >= 12)
        //                     {
        //                         Element = L_Oblique_Cross;
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
        // //右斜入判断
        // if(Element == 0 && Cross_Flag != 0)
        // {
        //     if(Adaptive_L_Start_Point[0] <= 15 && Adaptive_R_Start_Point[0] <= 60 && L_Right_Turn_Up_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 1)
        //     {
        //         if(L_Right_Turn_Up_Point_1[1] <= 35)
        //         {
        //             uint8 Temp_Num = 0;
        //             for(i = 57; i > L_Right_Turn_Up_Point_1[1]; i--)
        //             {
        //                 if(L_Border[i] == 2)
        //                 {
        //                     Temp_Num ++;
        //                 }
        //             }
        //             if((float)Temp_Num / (float)(57 - L_Right_Turn_Up_Point_1[1]) >= 0.9f)
        //             {
        //                 if(Adaptive_Y_Meet >= 4 && L_Border_Point_Num >= 12 && R_Border_Point_Num >= 12)
        //                 {
        //                     if(Relative_Border_Point_Num >= 12)
        //                     {
        //                         Element = R_Oblique_Cross;
        //                     }
        //                 }
        //             }
        //         }
        //     }
        // }
    //    //转完45度右转弯后斜入判断
    //    if(Element == 0 && Cross_Flag != 0)
    //    {
    //        if(L_Arc_Turn_Point_Flag == 0 && R_Arc_Turn_Point_Flag == 1)
    //        {
    //            if(R_Border_Point_NUm >= 5)
    //        }
    //    }
    //判断左环岛
        //一次远距判断
    //if ( /*Element == 0 &&*/ L_Circle_Flag != 0)
    //{
    //    if (L_Up_Turn_Left_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 0 && R_Straight_Flag == 1)
    //    {
    //        if (/* R_UP_Border_Point_Num >= 3   && */ R_Border_Point_Num <= 18 && L_Border_Point_Num >= 10 && L_Up_Turn_Left_Point_1[1] >= 10)
    //        {
    //            uint8 Temp_Num = 0;
    //            for (i = 0; i < Adaptive_R_Statics / 2; i++)
    //            {
    //                if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
    //                {
    //                    Temp_Num++;
    //                }
    //            }
    //            if (Temp_Num <= 4)
    //            {
    //                if (Relative_Border_Point_Num <= 8)
    //                {
    //                    buzzer_flag = 1;
    //                    Element = L_Circle;
    //                }
    //            }
    //        }
    //    }
    //}
    //二次近距判断
    //if (/*Element == 0 &&*/ L_Circle_Flag != 0)
    //{
    //    //判断左存在拐点, 判断右侧为直线
    //    if (L_Up_Turn_Left_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 0 && R_Straight_Flag == 1 /* && L_Arc_Turn_Point_Flag == 1 */)
    //    {
    //        if (L_Up_Turn_Left_Point_1[1] > 30 /* && (L_Up_Turn_Left_Point_1[1] - L_Arc_Turn_Point[0][1]) >= 15 */ )//(L_Up_Turn_Left_Point_1[1] - L_Arc_Turn_Point[0][1]) > 15)
    //        {
    //            //判断右侧直线不在边界上
    //            if (R_Border_Point_Num <= 10 && L_Border_Point_Num >= 10)
    //            {
    //                uint8 Temp_Num = 0;
    //                for (i = 0; i < Adaptive_R_Statics / 2; i++)
    //                {
    //                    if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
    //                    {
    //                        Temp_Num++;
    //                    }
    //                }
    //                if (Temp_Num <= 4)
    //                {
    //                    if (Relative_Border_Point_Num <= 8)
    //                    {
    //                        //buzzer_flag = 1;
    //                        Element = L_Circle;
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}
    //三次斜入判断(偏向圆环一侧)
    if (/*Element == 0 &&*/  L_Circle_Flag != 0)
    {
        if (Adaptive_Y_Meet >= 3 && Adaptive_X_Meet <= 30 && Adaptive_R_Start_Point[0] == 76)
        {
            uint8 Temp_Num = 0;
            for (i = 57; i > 2; i--)
            {
                if (L_Border[i] == 3 && R_Border[i] != 76)
                {
                    Temp_Num++;
                }
                if (L_Border[i] != 3 && R_Border[i] == 76)
                {
                    Temp_Num++;
                }
            }
            if (Temp_Num >= 30 /* && R_UP_Border_Point_Num >= 3 */ && L_Border_Point_Num >= 12)
            {
                uint8 Temp_Num_2 = 0;
                for (i = 0; i < Adaptive_R_Statics / 2; i++)
                {
                    if (Adaptive_R_Grow_Dir[i] == -3 || Adaptive_R_Grow_Dir[i] == 3)
                    {
                        Temp_Num_2++;
                    }
                }
                if (Temp_Num_2 <= 4)
                {
                    if (Relative_Border_Point_Num <= 8)
                    {
                        //buzzer_flag = 1;
                        Element = L_Circle;
                    }
                }
            }
        }
    }
    //    //四次斜入判断（偏向直线一侧）
    if (Element == 0 && L_Circle_Flag != 0)
    {
        if (Adaptive_L_Start_Point[0] <= 10 && Adaptive_R_Start_Point[0] <= 50)
        {
            if (L_Border_Point_Num >= 25 && L_Arc_Turn_Point_Flag == 1 && L_Up_Turn_Left_Point_Flag_1 == 0)
            {
                if (R_Straight_Flag == 1 && R_Border_Point_Num == 0)
                {
                    if (Adaptive_Y_Meet >= 3 && Adaptive_X_Meet <= 30)
                    {
                        //buzzer_flag = 1;
                        Element = L_Circle;
                    }
                }
            }
        }
    }
    // //判断右环岛
    //     //一次远距判断
    if (Element == 0 && R_Circle_Flag != 0)
    {
        if (L_Up_Turn_Left_Point_Flag_1 == 0 && R_Up_Turn_Right_Point_Flag_1 == 1 && L_Straight_Flag == 1)
        {
            if (L_UP_Border_Point_Num >= 3 && L_Border_Point_Num <= 18 && R_Border_Point_Num >= 18 && Adaptive_Y_Meet >= 3 && R_Up_Turn_Right_Point_1[1] >= 20)
            {
                uint8 Temp_Num = 0;
                for (i = 0; i < Adaptive_R_Statics / 2; i++)
                {
                    if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                    {
                        Temp_Num++;
                    }
                }
                if (Temp_Num <= 4)
                {
                    if (Relative_Border_Point_Num <= 8)
                    {
                        buzzer_flag = 1;
                        Element = R_Circle;
                    }
                }
            }
        }
    }
    //二次近距判断
    if (Element == 0 && R_Circle_Flag != 0)
    {
        //判断右存在拐点, 判断左侧为直线
        if (R_Up_Turn_Right_Point_Flag_1 == 1 && L_Up_Turn_Left_Point_Flag_1 == 0 && L_Straight_Flag == 1 && R_Arc_Turn_Point_Flag == 1)
        {
            if (R_Up_Turn_Right_Point_1[1] > 30 && (R_Up_Turn_Right_Point_1[1] - R_Arc_Turn_Point[0][1]) >= 15)//(R_Up_Turn_Right_Point_1[1] - R_Arc_Turn_Point[0][1]) > 15)
            {
                //判断左侧直线不在边界上
                if (L_Border_Point_Num <= 10 && R_Border_Point_Num >= 18)
                {
                    uint8 Temp_Num = 0;
                    for (i = 0; i < Adaptive_R_Statics / 2; i++)
                    {
                        if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                        {
                            Temp_Num++;
                        }
                    }
                    if (Temp_Num <= 4)
                    {
                        if (Relative_Border_Point_Num <= 8)
                        {
                            buzzer_flag = 1;
                            Element = R_Circle;
                        }
                    }
                }
            }
        }
    }
    //三次斜入判断
    if (Element == 0 && R_Circle_Flag != 0)
    {
        if (Adaptive_Y_Meet >= 3 && Adaptive_X_Meet >= 50 && Adaptive_L_Start_Point[0] == 1)
        {
            uint8 Temp_Num = 0;
            for (i = 57; i > 2; i--)
            {
                if (L_Border[i] == 2 && R_Border[i] != 77)
                {
                    Temp_Num++;
                }
                if (L_Border[i] != 2 && R_Border[i] == 77)
                {
                    Temp_Num++;
                }
            }
            if (Temp_Num >= 30 && L_UP_Border_Point_Num >= 3 && L_Border_Point_Num >= 13 && R_Border_Point_Num >= 16)
            {
                uint8 Temp_Num_2 = 0;
                for (i = 0; i < Adaptive_R_Statics / 2; i++)
                {
                    if (Adaptive_L_Grow_Dir[i] == -3 || Adaptive_L_Grow_Dir[i] == 3)
                    {
                        Temp_Num_2++;
                    }
                }
                if (Temp_Num_2 <= 4)
                {
                    if (Relative_Border_Point_Num <= 8)
                    {
                        buzzer_flag = 1;
                        Element = R_Circle;
                    }
                }
            }
        }
    }
    //    //四次斜入判断（偏向直线一侧）
    //    if(Element == 0 && R_Circle_Flag != 0)
    //    {
    //        if(Adaptive_L_Start_Point[0] >= 30 && Adaptive_R_Start_Point[0] >= 70)
    //        {
    //            if(R_Border_Point_Num >= 25 && R_Arc_Turn_Point_Flag == 1 && R_Up_Turn_Right_Point_Flag_1 == 0)
    //            {
    //                if(L_Straight_Flag == 1 && L_Border_Point_Num == 0)
    //                {
    //                    if(Adaptive_Y_Meet >= 3 && Adaptive_X_Meet >= 50)
    //                    {
    //                        buzzer_flag = 1;
    //                        Element = R_Circle;
    //                    }
    //                }
    //            }
    //        }
    //    }
   //     //小S弯判断
   //    if(Element == 0 && Small_S_Flag != 0)
   //    {
   //        if(L_Arc_Turn_Point_Flag == 1 && R_Arc_Turn_Point_Flag == 1 && ((L_Arc_Turn_Point_Num + R_Arc_Turn_Point_Num) >= 3))
   //        {
   //            if(Adaptive_Y_Meet <= 5)
   //            {
   //                Element = Small_S;
   //            }
   //        }
   //    }
   //    //左侧障碍物识别
   //    if((Element == 0 || Element == Straightaway) && Barrier_Flag != 0)
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
   //                        Element = L_Barrier;
   //                    }
   //                }
   //            }
   //        }
   //    }
   //    //障碍物判断
   //    if(Element == 0 && Barrier_Flag != 0)
   //    {
   //    }
   //    //出赛道判断
   //    if(Adaptive_Y_Meet >= 40 && L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_B && L_Straightaway_Lope_Rate_A == L_Straightaway_Lope_Rate_C
   //                    && R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_B && R_Straightaway_Lope_Rate_A == R_Straightaway_Lope_Rate_C)
   //    {
   //        Element = Derailment;
   //    }
    return Element;
}

#define L_Patching      1
#define R_Patching      2
#define L_R_Patching    3

#define L_Arc_Patching_State_2     4
#define R_Arc_Patching_State_2     5
#define L_Arc_Patching_State_4     6
#define R_Arc_Patching_State_4     7

#define L_Unilateral_Patrol         1
#define R_Unilateral_Patrol         2

#define L_Arc_Unilateral_Patrol_2     3
#define R_Arc_Unilateral_Patrol_2     4
#define L_Arc_Unilateral_Patrol_4     5
#define R_Arc_Unilateral_Patrol_4     6

uint8 C_Line[(uint16)Image_Y] = { 0 };
void Get_Middle_Line(uint8* l_border, uint8* r_border, uint8* c_line, uint8 point_num /* , uint8 patching_flag */, uint8 unilateral_patrol_flag)
{
    uint8 i;

    switch (unilateral_patrol_flag)  //单边巡线
    {
    case 0:
    {
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = (l_border[i] + r_border[i]) >> 1;
        }
        break;
    }
    case L_Unilateral_Patrol:
    {
        uint8 L_Staright[60] = { 0, 0, 4, 5, 5, 5, 6, 6, 6, 7,
                                 7, 7, 8, 8, 9, 9,10,11,11,12,
                                12,13,13,13,13,14,14,15,15,16,
                                16,16,17,17,18,18,19,19,19,20,
                                20,21,21,21,21,22,22,23,23,24,
                                24,24,25,25,25,26,26,27,27,28 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = l_border[i] + L_Staright[i];
        }
        break;
    }
    case R_Unilateral_Patrol:
    {
        uint8 R_Staright[60] = { 0, 0, 4, 5, 5, 5, 6, 6, 6, 7,
                                 7, 7, 8, 8, 9, 9,10,11,11,12,
                                12,13,13,13,13,14,14,15,15,16,
                                16,16,17,17,18,18,19,19,19,20,
                                20,21,21,21,21,22,22,23,23,24,
                                24,24,25,25,25,26,26,27,27,28 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = r_border[i] - R_Staright[i] - 4;
        }
        break;
    }
    case L_Arc_Unilateral_Patrol_2:
    {
        uint8 L_Arc_1[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 1, 1, 2, 2, 2, 3, 5,
                              6, 8, 9,11,12,13,14,15,16,16,
                             17,18,18,19,19,20,21,22,22,22,
                             23,23,23,24,25,25,26,26,26,26,
                             27,27,28,29,29,29,30,30, 0, 0 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = l_border[i] + L_Arc_1[i];
        }
        break;
    }
    case R_Arc_Unilateral_Patrol_2:
    {
        uint8 R_Arc_1[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 1, 1, 2, 2, 2, 3, 5,
                              6, 8, 9,11,12,13,14,15,16,16,
                             17,18,18,19,19,20,21,22,22,22,
                             23,23,23,24,25,25,26,26,26,26,
                             27,27,28,29,29,29,30,30, 0, 0 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = r_border[i] - R_Arc_1[i];
        }
        break;
    }
    case L_Arc_Unilateral_Patrol_4:
    {
        uint8 L_Arc_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 1, 2, 3, 3, 5, 6, 8,
                              9,11,12,14,15,16,17,18,19,19,
                             20,21,21,21,22,23,23,24,24,24,
                             25,25,25,25,26,26,27,27,27,27,
                             28,28,29,30,30,30,31,31, 0, 0 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = l_border[i] + L_Arc_2[i];
        }
        break;
    }
    case R_Arc_Unilateral_Patrol_4:
    {
        uint8 R_Arc_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              0, 0, 0, 1, 2, 3, 3, 5, 6, 8,
                              9,11,12,14,15,16,17,18,19,19,
                             20,21,21,21,22,23,23,24,24,24,
                             25,25,25,25,26,26,27,27,27,27,
                             28,28,29,30,30,30,31,31, 0, 0 };
        for (i = 2; i < point_num + 2; i++)
        {
            c_line[i] = r_border[i] - R_Arc_2[i];
        }
        break;
    }
    }
}

//***************************************************************************************

/**
 * @brief 将左右边线绘制到彩色图像上（使用十字形状，与Draw_Border_Points逻辑一致）
 * @param gray_array 灰度图像数组 [ZIP_H][ZIP_W]
 * @param L_Line 左边线坐标数组 [][2]，格式为 [x, y]
 * @param R_Line 右边线坐标数组 [][2]，格式为 [x, y]
 * @param L_count 左边线点数
 * @param R_count 右边线点数
 * @param C_Line 中间线坐标数组 [Image_Y]（X坐标）
 * @return 彩色图像Mat
 */
cv::Mat draw_border_lines_color(const uint8_t gray_array[ZIP_H][ZIP_W], const uint8 L_Line[][2], const uint8 R_Line[][2],
    uint16 L_count, uint16 R_count, const uint8 C_Line[Image_Y])
{
    // 将灰度数组转换为彩色图像
    cv::Mat color_img(ZIP_H, ZIP_W, CV_8UC3);
    for (int y = 0; y < ZIP_H; y++)
    {
        for (int x = 0; x < ZIP_W; x++)
        {
            uint8_t gray = gray_array[y][x];
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(gray, gray, gray);
        }
    }

    // 1. 绘制左侧边线（红色）
    for (uint16 i = 0; i < L_count; i++)
    {
        uint16 x = L_Line[i][0];
        uint16 y = L_Line[i][1];

        if (x < ZIP_W && y < ZIP_H)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 255);     // 红色
        }
    }

    // 2. 绘制右侧边线（蓝色）
    for (uint16 i = 0; i < R_count; i++)
    {
        uint16 x = R_Line[i][0];
        uint16 y = R_Line[i][1];

        if (x < ZIP_W && y < ZIP_H)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 0, 0);     // 蓝色
        }
    }

    // 3. 绘制中间边线（黄色）
    for (uint16 i = 1; i < Image_Y - 3; i++)
    {
        uint16 x = C_Line[i];
        uint16 y = i;

        if (x < ZIP_W && y < ZIP_H)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 255);  // 黄色
        }
    }

    // 4. 标记相遇点（绿色十字）- 用于调试
    if (L_count > 0 && R_count > 0)
    {
        uint16 meet_x = (R_Line[R_count - 1][0] + L_Line[L_count - 1][0]) >> 1;
        uint16 meet_y = (R_Line[R_count - 1][1] + L_Line[L_count - 1][1]) >> 1;

        // 画十字标记
        if (meet_x > 0 && meet_x < ZIP_W - 1 && meet_y > 0 && meet_y < ZIP_H - 1)
        {
            color_img.at<cv::Vec3b>(meet_y, meet_x - 1) = cv::Vec3b(0, 255, 0);
            color_img.at<cv::Vec3b>(meet_y, meet_x + 1) = cv::Vec3b(0, 255, 0);
            color_img.at<cv::Vec3b>(meet_y - 1, meet_x) = cv::Vec3b(0, 255, 0);
            color_img.at<cv::Vec3b>(meet_y + 1, meet_x) = cv::Vec3b(0, 255, 0);
            color_img.at<cv::Vec3b>(meet_y, meet_x) = cv::Vec3b(0, 255, 0);
        }
    }

    // 5. 绘制上转左拐点（棕色十字）- L_Up_Turn_Left_Point_1
    if (L_Up_Turn_Left_Point_Flag_1)
    {
        uint16 x = L_Up_Turn_Left_Point_1[0];
        uint16 y = L_Up_Turn_Left_Point_1[1];
        if (x > 0 && x < ZIP_W - 1 && y > 0 && y < ZIP_H - 1)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(165, 42, 42);      // 棕色
            color_img.at<cv::Vec3b>(y, x - 1) = cv::Vec3b(165, 42, 42);  // 棕色
            color_img.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(165, 42, 42);  // 棕色
            color_img.at<cv::Vec3b>(y - 1, x) = cv::Vec3b(165, 42, 42);  // 棕色
            color_img.at<cv::Vec3b>(y + 1, x) = cv::Vec3b(165, 42, 42);  // 棕色
        }
    }

    // 6. 绘制上转右拐点（白色十字）- R_Up_Turn_Right_Point_1
    if (R_Up_Turn_Right_Point_Flag_1)
    {
        uint16 x = R_Up_Turn_Right_Point_1[0];
        uint16 y = R_Up_Turn_Right_Point_1[1];
        if (x > 0 && x < ZIP_W - 1 && y > 0 && y < ZIP_H - 1)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);      // 白色
            color_img.at<cv::Vec3b>(y, x - 1) = cv::Vec3b(255, 255, 255);  // 白色
            color_img.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(255, 255, 255);  // 白色
            color_img.at<cv::Vec3b>(y - 1, x) = cv::Vec3b(255, 255, 255);  // 白色
            color_img.at<cv::Vec3b>(y + 1, x) = cv::Vec3b(255, 255, 255);  // 白色
        }
    }

    // 7. 绘制左上转右拐点（青色十字）- L_Right_Turn_Up_Point_1
    if (L_Right_Turn_Up_Point_Flag_1)
    {
        uint16 x = L_Right_Turn_Up_Point_1[0];
        uint16 y = L_Right_Turn_Up_Point_1[1];
        if (x > 0 && x < ZIP_W - 1 && y > 0 && y < ZIP_H - 1)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 255);      // 青色
            color_img.at<cv::Vec3b>(y, x - 1) = cv::Vec3b(0, 255, 255);  // 青色
            color_img.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(0, 255, 255);  // 青色
            color_img.at<cv::Vec3b>(y - 1, x) = cv::Vec3b(0, 255, 255);  // 青色
            color_img.at<cv::Vec3b>(y + 1, x) = cv::Vec3b(0, 255, 255);  // 青色
        }
    }

    // 8. 绘制右左上拐点（品红色十字）- R_Left_Turn_Up_Point_1
    if (R_Left_Turn_Up_Point_Flag_1)
    {
        uint16 x = R_Left_Turn_Up_Point_1[0];
        uint16 y = R_Left_Turn_Up_Point_1[1];
        if (x > 0 && x < ZIP_W - 1 && y > 0 && y < ZIP_H - 1)
        {
            color_img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 0, 255);      // 品红色
            color_img.at<cv::Vec3b>(y, x - 1) = cv::Vec3b(255, 0, 255);  // 品红色
            color_img.at<cv::Vec3b>(y, x + 1) = cv::Vec3b(255, 0, 255);  // 品红色
            color_img.at<cv::Vec3b>(y - 1, x) = cv::Vec3b(255, 0, 255);  // 品红色
            color_img.at<cv::Vec3b>(y + 1, x) = cv::Vec3b(255, 0, 255);  // 品红色
        }
    }

    // 9. 绘制斑马线判断线（黄色，隔一行一条）
    // 18条判断线分别在第33-50行，只绘制偶数索引行（33,35,37,39,41,43,45,47,49）
    uint8 zebra_rows[18] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 };
    for (int i = 0; i < 18; i += 2) {  // 隔一行一条
        uint8 row = zebra_rows[i];
        // 绘制整行作为判断线（黄色）
        for (int x = 0; x < ZIP_W; x++) {
            color_img.at<cv::Vec3b>(row, x) = cv::Vec3b(255, 255, 0);  // 黄色
        }
    }

    // 10. 绘制斑马线检测点（黄色圆点）
    for (int i = 0; i < 18; i += 2) {  // 只绘制隔行的检测点
        uint8 row = zebra_rows[i];
        if (zebra_points[i][2] == 1) {
            uint8 first_x = zebra_points[i][0];
            uint8 end_x = zebra_points[i][1];
            uint8 center_x = zebra_points[i][3];

            // 标记起始点（黄色）
            if (first_x > 0 && first_x < ZIP_W) {
                color_img.at<cv::Vec3b>(row, first_x) = cv::Vec3b(255, 100, 0);  // 黄色
            }
            // 标记结束点（橙色）
            if (end_x > 0 && end_x < ZIP_W) {
                color_img.at<cv::Vec3b>(row, end_x) = cv::Vec3b(0, 165, 255);  // 橙色
            }
            // 标记中心点（红色）
            //if (center_x > 0 && center_x < ZIP_W) {
            //    color_img.at<cv::Vec3b>(row, center_x) = cv::Vec3b(0, 0, 255);  // 红色
            //}
        }
    }

    // 11. 绘制射击中心点（紫色十字）
    if (zebra_found_flag && zebra_shot_center[0] > 0 && zebra_shot_center[1] > 0) {
        uint16 x = zebra_shot_center[0];
        uint16 y = zebra_shot_center[1];
        if (x > 0 && x < ZIP_W - 1 && y > 0 && y < ZIP_H - 1) {
            // 绘制紫色十字
            for (int dx = -2; dx <= 2; dx++) {
                if (x + dx >= 0 && x + dx < ZIP_W) {
                    color_img.at<cv::Vec3b>(y, x + dx) = cv::Vec3b(100, 0, 160);  // 紫色
                }
            }
            for (int dy = -2; dy <= 2; dy++) {
                if (y + dy >= 0 && y + dy < ZIP_H) {
                    color_img.at<cv::Vec3b>(y + dy, x) = cv::Vec3b(100, 0, 160);  // 紫色
                }
            }
        }
    }

    // 12. 绘制圆环上下边界线（红色垂直线，偏移中心1像素避免被黄色中线覆盖）
    if (zebra_found_flag && zebra_shot_center[0] > 0 && zebra_shot_center[1] > 0) {
        uint16 x = zebra_shot_center[0];
        uint16 y_center = zebra_shot_center[1];
        
        // 设置默认边界范围
        uint16 up_y = (y_center > 5) ? (y_center - 5) : 0;
        uint16 down_y = (y_center < ZIP_H - 6) ? (y_center + 5) : (ZIP_H - 1);
        
        // 如果找到实际边界点，使用实际值
        if (ring_down_point > ring_up_point) {
            up_y = ring_up_point;
            down_y = ring_down_point;
        }
        
        // 在中心左侧1像素位置绘制红线，避免被黄色中线覆盖
        uint16 draw_x = (x > 0) ? (x - 1) : x;
        for (int y = up_y; y <= down_y; y++) {
            if (draw_x >= 0 && draw_x < ZIP_W && y >= 0 && y < ZIP_H) {
                color_img.at<cv::Vec3b>(y, draw_x) = cv::Vec3b(0, 0, 255);  // 红色
            }
        }
    }

    return color_img;
}


bool read_image(const char* path, uint8_t array[IMG_H][IMG_W])
{
    cv::Mat img = cv::imread(path, cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        std::cerr << "读取失败: " << path << std::endl;
        return false;
    }
    if (img.rows != IMG_H || img.cols != IMG_W) {
        std::cerr << "尺寸错误!" << std::endl;
        return false;
    }
    for (int y = 0; y < IMG_H; ++y)
        for (int x = 0; x < IMG_W; ++x)
            array[y][x] = img.at<uchar>(y, x);
    return true;
}

/**
 * @brief 显示任意尺寸数组为图像（模板版本）
 * @param array 二维数组 [rows][cols]
 * @param win_name 窗口名称
 * @param scale 放大倍数（默认5倍）
 */
template<size_t H, size_t W>
void show_image(const uint8_t(&array)[H][W], const char* win_name = "Image", int scale = 5)
{
    cv::Mat img(H, W, CV_8UC1, const_cast<uint8_t*>(array[0]));
    // 创建放大后的图像
    cv::Mat img_scaled;
    cv::resize(img, img_scaled, cv::Size(W * scale, H * scale), 0, 0, cv::INTER_NEAREST);
    cv::imshow(win_name, img_scaled);
}

/**
 * @brief 打靶检测函数（圆环检测）
 * @param l_border 左边框数组
 * @param r_border 右边框数组
 * @param image 灰度图像数组 [ZIP_H][ZIP_W]
 */
void ring_shot_judge(uint8* l_border, uint8* r_border, uint8 image[ZIP_H][ZIP_W])
{
    uint8 check_rows[18] = { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50 };
    uint8 Black_Num = 0;
    uint8 first_points[18] = { 0 };
    uint8 end_points[18] = { 0 };
    uint8 row_idx;

    uint16 shot_center_x = 0;
    uint16 shot_center_y = 0;
    uint8 find_cnt = 0;
    uint8 start_row = 0;
    uint8 end_row = 0;

    // 第一步：扫描所有行，检测灰度跳变
    for (row_idx = 0; row_idx < 18; row_idx++)
    {
        uint8 row = check_rows[row_idx];
        // 放宽边框条件：只要边框有效就检测
        if (l_border[row] > 0 && r_border[row] < 80 && (r_border[row] - l_border[row]) > 10)
        {
            uint8 i;
            for (i = l_border[row]; i < r_border[row] - 1; i++)
            {
                if (Compare_Num(image[row][i], image[row][i + 1], 15))
                {
                    if (!first_points[row_idx]) first_points[row_idx] = i;
                    end_points[row_idx] = i+2;
                    Black_Num++;
                }
            }
            // 更新斑马点用于绘图
            zebra_points[row_idx][0] = first_points[row_idx];
            zebra_points[row_idx][1] = end_points[row_idx];
            zebra_points[row_idx][2] = 1;
            zebra_points[row_idx][3] = (first_points[row_idx] + end_points[row_idx]) / 2;
        }
    }

    // 第二步：找出符合条件的圆环候选
    for (row_idx = 0; row_idx < 18; row_idx++)
    {
        uint8 width = end_points[row_idx] - first_points[row_idx];
        uint8 row = check_rows[row_idx];

        if (width >= 4 && width <= 18 && first_points[row_idx] > 5)
        {
            if (!start_row) start_row = row;
            if (row > end_row) end_row = row;
            shot_center_x += (first_points[row_idx] + end_points[row_idx]) / 2;
            shot_center_y += row;
            find_cnt++;
        }
    }

    // 第三步：验证圆环并计算精确中心
    if (find_cnt >= 5)
    {
        uint8 centers_ok = 1;

        if (centers_ok)
        {
            uint8 start_point_up;
            uint8 start_point_down;
            uint8 up_point;
            uint8 down_point;
            uint8 up_stop_flag;
            uint8 down_stop_flag;
            uint8 ring_height;

            shot_center_x = shot_center_x / find_cnt;
            shot_center_y = shot_center_y / find_cnt;

            start_point_up = shot_center_y - 1;
            start_point_down = shot_center_y + 1;
            up_point = shot_center_y;
            down_point = shot_center_y;
            up_stop_flag = 0;
            down_stop_flag = 0;

            // 在垂直方向找圆环的上下边界
            while (!up_stop_flag && start_point_up > 0)
            {
                if ( Compare_Num(image[start_point_up][shot_center_x] , image[start_point_up - 1][shot_center_x] , 15) )
                {
                    up_stop_flag = 1;
                    up_point = start_point_up;
                }
                start_point_up--;
            }

            while (!down_stop_flag && start_point_down < ZIP_H - 1)
            {
                if ( Compare_Num(image[start_point_down][shot_center_x] ,image[start_point_down + 1][shot_center_x] ,15 ) )
                {
                    down_stop_flag = 1;
                    down_point = start_point_down;
                }
                start_point_down++;
            }

            // 验证圆环高度合理（6-15像素）
            ring_height = down_point - up_point;

            if (ring_height >= 2 && ring_height <= 25)
            {
                // 保存上下边界点
                ring_up_point = up_point;
                ring_down_point = down_point;
                
                shot_center_y = (down_point + up_point) / 2;

                zebra_shot_center[0] = shot_center_x;
                zebra_shot_center[1] = shot_center_y;

                // 标记找到
                zebra_found_flag = 1;

                if (shot_center_y >= 20 && shot_center_y <= 60)
                {
                    if (!shot_CD)
                    {
                        shot_CD = 7;
                    }
                }
            }
        }
    }
}

/**
 * @brief 显示彩色图像（可放大）
 * @param color_img 彩色图像Mat (CV_8UC3)
 * @param win_name 窗口名称
 * @param scale 放大倍数
 */
void show_color_image(const cv::Mat& color_img, const char* win_name = "Color Image", int scale = 5)
{
    cv::Mat img_scaled;
    cv::resize(color_img, img_scaled, cv::Size(color_img.cols * scale, color_img.rows * scale), 0, 0, cv::INTER_NEAREST);
    cv::imshow(win_name, img_scaled);
}

int main()
{
    // 读取图像
    if (read_image("D:\\桌面\\Pictures\\2026_05_25_11_18_01_Image.bmp", mt9v03x_image)) {
        // 显示图像
        copy_zip_image(mt9v03x_image, Find_Line_Image);

        // 显示压缩后的图像（放大4倍）
        cv::Mat zip_gray_img(ZIP_H, ZIP_W, CV_8UC1);
        for (int i = 0; i < ZIP_H; ++i) {
            for (int j = 0; j < ZIP_W; ++j) {
                zip_gray_img.at<uint8_t>(i, j) = Find_Line_Image[i][j];
            }
        }
        cv::Mat zip_gray_resized;
        cv::resize(zip_gray_img, zip_gray_resized, cv::Size(ZIP_W * 4, ZIP_H * 4), 0, 0, cv::INTER_LINEAR);
        cv::imshow("压缩后图像", zip_gray_resized);
        draw_black_box(0, Find_Line_Image);
        cv::waitKey(1);



        if (Get_Start_Point(Image_Y - 3, Find_Line_Image, Adaptive_L_Start_Point, Adaptive_R_Start_Point, 2, 77) == 1)
        {
            cout << endl << endl;
            // 强制转换为 int 输出，避免被当作字符
            cout << "左边起点：" << (int)Adaptive_L_Start_Point[0] << " " << (int)Adaptive_L_Start_Point[1] << endl;
            cout << "右边起点：" << (int)Adaptive_R_Start_Point[0] << " " << (int)Adaptive_R_Start_Point[1] << endl;

            Dir_Labyrinth_5((uint8)155, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
                Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1], Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], Clip_Value);

            if ((Adaptive_L_Statics <= 10 && (Adaptive_R_Statics - Adaptive_L_Statics >= 50)) || (Adaptive_R_Statics <= 10 && (Adaptive_L_Statics - Adaptive_R_Statics >= 50)))
            {
                Adaptive_L_Statics = 0;
                Adaptive_R_Statics = 0;
                if (Get_Start_Point(47, Find_Line_Image, Adaptive_L_Start_Point, Adaptive_R_Start_Point, 2, 77) == 1)
                {

                    Dir_Labyrinth_5((uint8)100, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
                        Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1], Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], Clip_Value);

                }
            }
        }
        Get_Border(Adaptive_L_Statics, Adaptive_R_Statics, Image_Y - 3, 2, L_Border, R_Border, Adaptive_L_Line, Adaptive_R_Line);//��ȡһά����
        cout << endl;
        for (int i = 0; i <= 59; i++)
        {
            cout << (int)L_Border[i] << "   " << (int)R_Border[i] << endl;
        }
        cout << endl;

        Get_Information();

        Element = Element_Judgement(Find_Line_Image, Adaptive_L_Statics, Adaptive_R_Statics, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, Adaptive_L_Line, Adaptive_R_Line, Adaptive_X_Meet, Adaptive_Y_Meet, L_Border, R_Border);

        Print_Get_Information();

        cout << endl << (int)Element << endl;

        Get_Middle_Line(L_Border, R_Border, C_Line, Image_Y - 5, 0);

        // 绘制彩色边线并显示（放大8倍）
        cv::Mat color_img = draw_border_lines_color(Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Statics, Adaptive_R_Statics, C_Line);
        cv::Mat color_resized;
        cv::resize(color_img, color_resized, cv::Size(color_img.cols * 8, color_img.rows * 8), 0, 0, cv::INTER_LINEAR);
        cv::imshow("处理后图像", color_resized);
        cv::waitKey(0);

    }

    return 0;
}
/*
修改内容：
1、爬点个数增加，十字容易爬不完线
2、爬线防止越界
3、十字横向生长点数量，边界相对点个数减少
*/
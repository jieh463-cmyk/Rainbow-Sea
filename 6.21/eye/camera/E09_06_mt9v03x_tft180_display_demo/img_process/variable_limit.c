#include "file_h_all.h"

/**
* 函数功能：      16位整数快速开平方
* 特殊说明：      使用二进制搜索算法，输入范围0-65535，输出范围0-255
* 形  参：        val：       16位无符号整数
* 示例：          Fast_Sqrt16(sum);
* 返回值：        开方结果（8位）
*/
uint8 Fast_Sqrt16(uint16 val)
{
    uint8 low = 0, high = 255, mid;
    while(low <= high)
    {
        mid = (low + high) >> 1;
        if((uint16)mid * mid <= val)
        {
            low = mid + 1;
        }
        else
        {
            high = mid - 1;
        }
    }
    return high;
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
    if(value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}

/**
* 函数功能：      浮点型变量取绝对值
* 特殊说明：      注意调用时参数类型，摄像头代码参数较多，不同类型错误传参可能导致计算结果出现较大问题，严重时卡死单片机运行
* 形  参：        value：       浮点型变量
* 示例：          My_ABS_F();
* 返回值：        传入参数的绝对值
*/
float My_ABS_F(float value)               //*
{
    if(value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}

uint8 My_ABS_uint8(uint8 value)               //*
{
    if(value >= 0)
    {
        return value;
    }
    else
    {
        return -value;
    }
}
/**
* 函数功能：      32位整形变量限幅
* 特殊说明：      注意调用时参数类型，摄像头代码参数较多，不同类型错误传参可能导致计算结果出现较大问题，严重时卡死单片机运行
* 形  参：        x：       要限幅的参数
*                a：       最小值
*                b：       最大值
* 示例：          Limit(x， a， b);
* 返回值：        传入参数限幅后的值
*/
int Limit(int x, int a, int b)               //*
{
    if(x > b)
    {
        return b;
    }
    else if(x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

//8位无符号整形变量限幅
//不多讲了，肯定能看懂
uint8 Limit_u8(uint8 x, uint8 a, uint8 b)               //*
{
    if(x > b)
    {
        return b;
    }
    else if(x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}
//16位有符号整形变量限幅
int16 Limit_16(int16 x, int16 a, int16 b)               //*
{
    if(x > b)
    {
        return b;
    }
    else if(x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

//16位无符号整型变量限幅
uint16 Limit_u16(uint16 x, uint16 a, uint16 b)               //*
{
    if(x > b)
    {
        return b;
    }
    else if(x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

//浮点型变量限幅
float Limit_Float(float x, float a, float b)               //*
{
    if(x > b)
    {
        return b;
    }
    else if(x < a)
    {
        return a;
    }
    else
    {
        return x;
    }
}

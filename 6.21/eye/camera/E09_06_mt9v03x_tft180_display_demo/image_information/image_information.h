#ifndef _IMAGE_INFORMATION_H
#define _IMAGE_INFORMATION_H


//=============================================================================
// 函数声明
//=============================================================================
int16 Slope_Calculate(uint8 begin, uint8 end, uint8 *border);
void Calculate_Slope_Intercept(uint8 start, uint8 end, uint8 *border, int16 *slope_rate, int16 *intercept);
void Get_Variance(uint8 start, uint8 end, int16 *fitting_line, uint8 *actual_line, float *variance, uint8 multiple);

void Get_L_Up_Turn_Left_Point_1(uint8 Grade);
void Get_L_Up_Turn_Left_Point_2(uint8 Region_Start, uint8 Region_End);
void Get_L_Up_Turn_Left_Point(void);

void Get_L_Right_Turn_Up_Point_1(uint8 Grade);
void Get_L_Right_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End);
void Get_L_Right_Turn_Up_Point(void);

void Get_R_Up_Turn_Right_Point_1(uint8 Grade);
void Get_R_Up_Turn_Right_Point_2(uint8 Region_Start, uint8 Region_End);
void Get_R_Up_Turn_Right_Point(void);

void Get_R_Left_Turn_Up_Point_1(uint8 Grade);
void Get_R_Left_Turn_Up_Point_2(uint8 Region_Start, uint8 Region_End);
void Get_R_Left_Turn_Up_Point(void);

void Get_L_Arc_Turn_Point(void);
void Get_R_Arc_Turn_Point(void);

void Get_R_Intercept_And_Slope(uint8 start, uint8 end);
void Get_L_Intercept_And_Slope(uint8 start, uint8 end);

//void Get_Fitting_Line_And_Variance(void);
void Get_L_Border_Point_Num(void);
void Get_R_Border_Point_Num(void);
void Get_Relative_Border_Point_Num(void);
void Get_Row_Difference(uint8 *l_border, uint8 *r_border);

void Get_Information(void);

////=============================================================================

// 阈值
extern uint8 Straight_Thres;
extern uint8 Sml_Arc_Thres;
extern uint8 Big_Arc_thres;

extern int16 Max_Slope_Dif_Thre;
extern int16 Min_Slope_Dif_Thre;

// 上转左拐点
extern uint8 L_Up_Turn_Left_Point_1[2];
extern uint8 L_Up_Turn_Left_Point_Flag_1;
extern uint8 L_Up_Turn_Left_Point_Position_1;
extern int16 L_Up_Turn_Left_Point_Angle_1;

extern uint8 L_Up_Turn_Left_Point_2[2];
extern uint8 L_Up_Turn_Left_Point_Flag_2;
extern uint8 L_Up_Turn_Left_Point_Position_2;

extern uint8 L_Up_Turn_Left_Point[2];
extern uint8 L_Up_Turn_Left_Point_Flag;

// 右转上拐点
extern uint8 L_Right_Turn_Up_Point_1[2];
extern uint8 L_Right_Turn_Up_Point_Flag_1;
extern uint8 L_Right_Turn_Up_Point_Position_1;
extern int16 L_Right_Turn_Up_Point_Angle_1;

extern uint8 L_Right_Turn_Up_Point_2[2];
extern uint8 L_Right_Turn_Up_Point_Flag_2;
extern uint8 L_Right_Turn_Up_Point_Position_2;

extern uint8 L_Right_Turn_Up_Point[2];
extern uint8 L_Right_Turn_Up_Point_Flag;

// 上转右拐点
extern uint8 R_Up_Turn_Right_Point_1[2];
extern uint8 R_Up_Turn_Right_Point_Flag_1;
extern uint8 R_Up_Turn_Right_Point_Position_1;
extern int16 R_Up_Turn_Right_Point_Angle_1;

extern uint8 R_Up_Turn_Right_Point_2[2];
extern uint8 R_Up_Turn_Right_Point_Flag_2;
extern uint8 R_Up_Turn_Right_Point_Position_2;

extern uint8 R_Up_Turn_Right_Point[2];
extern uint8 R_Up_Turn_Right_Point_Flag;

// 左转上拐点
extern uint8 R_Left_Turn_Up_Point_1[2];
extern uint8 R_Left_Turn_Up_Point_Flag_1;
extern uint8 R_Left_Turn_Up_Point_Position_1;
extern int16 R_Left_Turn_Up_Point_Angle_1;

extern uint8 R_Left_Turn_Up_Point_2[2];
extern uint8 R_Left_Turn_Up_Point_Flag_2;
extern uint8 R_Left_Turn_Up_Point_Position_2;

extern uint8 R_Left_Turn_Up_Point[2];
extern uint8 R_Left_Turn_Up_Point_Flag;

// 圆弧拐点
extern uint8 L_Arc_Turn_Point[3][2];
extern uint8 L_Arc_Turn_Point_Flag;
extern uint8 L_Arc_Turn_Point_Num;

extern uint8 R_Arc_Turn_Point[3][2];
extern uint8 R_Arc_Turn_Point_Flag;
extern uint8 R_Arc_Turn_Point_Num;

//// 斜率截距相关
extern int16 R_Straightaway_Lope_Rate_A;
extern int16 R_Straightaway_Lope_Rate_B;
extern int16 R_Straightaway_Lope_Rate_C;
extern int16 R_Intercept;
extern uint8 R_Straight_Flag;

extern int16 L_Straightaway_Lope_Rate_A;
extern int16 L_Straightaway_Lope_Rate_B;
extern int16 L_Straightaway_Lope_Rate_C;
extern int16 L_Intercept;
extern uint8 L_Straight_Flag;

//// 拟合与方差
//extern int16 L_Fitting_Line[];
//extern int16 R_Fitting_Line[];
//extern float L_Variance;
//extern float R_Variance;

//// 边界点统计
extern uint8 L_Border_Point_Num;
extern uint8 L_UP_Border_Point_Num;

extern uint8 R_Border_Point_Num;
extern uint8 R_UP_Border_Point_Num;

extern uint8 Relative_Border_Point_Num;

extern int8 Row_Difference[20];
extern uint8 Max_Row_Dif_Line_Num;


extern int8 Deviation_Value;
extern int8 Mid_Line_FFF;
extern int8 Mid_Line_FF;
extern int8 Mid_Line_F;

extern uint16 C_Variance;
extern uint16 Last_C_Variance;

extern int8 Last_Deviation_Value;

// floa Last_Position_conpensate = 0;
// floa Position_conpensate = 0;

// floa Cross_Dif_Correct = 0;
// uint16 Circle_Temp_Num_2 = 0;
// uint8 Small_S_Break_Flag = 0;

// in Cross_State_4_Direction = 0;

//偏差值滑动均值滤波数组
extern int16 Deviation_Value_Array[20];
//用于相遇点滑动均值滤波数组
extern uint8 Y_Meet_Array[20];
void Get_Straight_Deviation_Value(uint8 element, uint8 start, uint8 end, uint8 *c_line, uint8 element_state);

#endif
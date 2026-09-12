#ifndef _find_line_h
#define _find_line_h

#define Thres_Interfere         0  //人工干预灰度值计算
#define Thres_Num_Interfere     25  //共25点处的灰度值，用于做除法
#define Thres_Filiter_Flag_1    1  //开启反向平滑滤波，改善亮处到暗处或暗处亮处的灰度阈值
#define Thres_Filiter_Flag_2    1  //开启反向平滑滤波，改善亮处到暗处或暗处亮处的灰度阈值

#define MAX_LINE_POINTS         250  //边线储存个数
#define X_Border_Min			0
#define X_Border_Max			79
#define Y_Border_Min			0
#define Y_Border_Max			59



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


extern uint16 L_Thres_Record[MAX_LINE_POINTS];
extern uint16 R_Thres_Record[MAX_LINE_POINTS];

extern const int8 L_Face_Dir[4][2];  //左侧迷宫面向
//  0
//3   1
//  2
 
extern const int8 L_Face_Dir_L[4][2];  //左侧面向的左前方
//0   1
//
//3   2
 
extern const int8 R_Face_Dir[4][2];  //右侧迷宫面向
//  0
//3   1
//  2
 
extern const int8 R_Face_Dir_R[4][2];  //右侧面向的右前方
//3   0
//
//2   1
 
extern const int8 Square_0[25][2];

extern uint8 Clip_Value;

extern uint8 C_Line[(uint16)Image_Y];
 
//迷宫单侧停止爬线标志位
extern uint8 L_Stop_Flag;
extern uint8 R_Stop_Flag;
extern uint16 L_Boundary_Limit;
extern uint16 R_Boundary_Limit;

void Dir_Labyrinth_5(uint16 Break_Flag, uint8(*image)[Image_X], uint8(*l_line)[2], uint8(*r_line)[2], int8 *l_dir, int8 *r_dir, uint16 *l_stastic, uint16 *r_stastic, uint8 *x_meet, uint8 *y_meet,
                     uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8 clip_value);

void Get_Border(uint16 l_total, uint16 r_total, uint8 start, uint8 end, uint8 *l_border, uint8 *r_border, uint8(*l_line)[2], uint8(*r_line)[2]);

void Draw_Border_Points(uint8(*l_line)[2], uint8(*r_line)[2],uint16 l_stastic, uint16 r_stastic);

void Get_Middle_Line(uint8 *l_border, uint8 *r_border, uint8 *c_line, uint8 point_num /*, uint8 patching_flag */ , uint8 unilateral_patrol_flag );
	
void Draw_Border_Points_1(uint8(*l_line)[2], uint8(*r_line)[2], uint8 *c_line ,
                        uint16 l_stastic, uint16 r_stastic);
#endif

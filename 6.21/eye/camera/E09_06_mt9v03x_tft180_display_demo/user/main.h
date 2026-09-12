#ifndef _main_h
#define _main_h

#define BOUNDARY_NUM            (Image_Y * 3 / 2)
// 只有X边界
extern uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];

// 只有Y边界
extern uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];

// X Y边界都是单独指定的
extern uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
extern uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];

// 图像备份数组，在发送前将图像备份再进行发送，这样可以避免图像出现撕裂的问题
extern uint8 image_copy[Image_Y][Image_X];

extern uint8 Adaptive_L_Start_Point[2];		//سӠѲПǰʼ֣
extern uint8 Adaptive_R_Start_Point[2];		//ԒӠѲПǰʼ֣
extern uint8 Adaptive_L_Line[(uint8)160][2];   // سПظҪ֣
extern uint8 Adaptive_R_Line[(uint8)160][2];   // ԒПظҪ֣
extern int8 Adaptive_L_Grow_Dir[(uint8)160];     //سIZÿ֣ʺ={}׽в
extern int8 Adaptive_R_Grow_Dir[(uint8)160];     //ԒӠÿ֣ʺ={}׽в
extern uint16 Adaptive_L_Statics;         //سҟ����֣ٶ˽
extern uint16 Adaptive_L_Statics;         //سҟѰփ֣ٶ˽
extern uint16 Adaptive_R_Statics;         //ԒҟѰփ֣ٶ˽

extern uint8 Adaptive_X_Meet;
extern uint8 Adaptive_Y_Meet;

//存放边线的一维数组
extern uint8 R_Border[Image_Y];
extern uint8 L_Border[Image_Y];

extern uint8 R_Unilateral_Patrol_Border[Image_Y];
extern uint8 L_Unilateral_Patrol_Border[Image_Y];

extern uint8 cnt;

extern int16 Image_Count;
//开启图像计时标志位
extern uint8 Image_Count_Flag;

extern float Z_Yaw;
extern uint8 IMU_JF_Flag;
extern uint8 imu_flag;

extern uint8 Element_State;

//wifi传送数据
extern char buf[32];
extern int len;
extern int s;

extern int32 time_ms;
extern uint8 time_flag;
extern uint8 photo_flag;
extern int32 photo_cnt ;

#endif

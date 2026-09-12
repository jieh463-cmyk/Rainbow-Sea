#ifndef _element_judge_h
#define _element_judge_h

extern uint8 Element;
extern volatile uint8 buzzer_flag;

#define Derailment     1        //出赛道
#define Straightaway   2        //直道
#define L_Turn         3        //左转弯道
#define R_Turn         4        //右转弯道
#define L_Circle       5        //左环岛
#define R_Circle       6        //右环岛
#define Cross          7        //十字路口
#define Zebra          8        //斑马线
#define Ramp           9        //坡道
#define Three_Bif      10       //三岔
#define Barrier        11       //障碍
#define Disconnection  12       //断路
#define T_Way          13       //T路口
#define L_Garage       14       //左车库
#define R_Garage       15       //右车库
#define L_Oblique_Cross 16		//左斜入
#define R_Oblique_Cross 17		//右斜入
#define Small_S        18       //小S弯

/************************ 道路元素识别核心变量 ************************/
// 最终识别的道路元素结果（0：未识别，其他值对应具体元素）
extern uint8 Element; 
// 蜂鸣器触发标志（0：关闭，1：开启）
extern volatile uint8 buzzer_flag;

/************************ 各道路场景使能标志 ************************/
// 直道：0-关闭 1-开启
extern uint8 Straightaway_Flag;       
// 左转弯道：0-关闭 1-开启
extern uint8 L_Turn_Flag;             
// 右转弯道：0-关闭 1-开启
extern uint8 R_Turn_Flag;             
// 左环岛：0-关闭 1-单边巡线处理 2-三点法补圆 3-二次贝塞尔补线 4-三次贝塞尔补线 5-拉格朗日补线
extern uint8 L_Circle_Flag;           
// 右环岛：0-关闭 1-单边巡线处理 2-三点法补圆 3-二次贝塞尔补线 4-三次贝塞尔补线 5-拉格朗日补线
extern uint8 R_Circle_Flag;           
// 十字路口：0-关闭 1-不做特殊处理 2-最小二乘法补线 3-二次贝塞尔补线处理
extern uint8 Cross_Flag;              
// 左斜十字路口：0-关闭 1-开启（补线处理）
extern uint8 L_Oblique_Cross_Flag;    
// 右斜十字路口：0-关闭 1-开启（补线处理）
extern uint8 R_Oblique_Cross_Flag;    
// 斑马线：0-关闭 1-开启
extern uint8 Zebra_Flag;        

extern uint8 stop_flag;


/************************ 出界检测相关变量 ************************/
// 丢线标志（0：正常，1：丢线）
extern uint8 Line_Lost_Flag;
// 出界计数器（连续异常帧计数）
extern uint8 Out_Bound_Cnt;
// 出界标志（0：正常，1：出界）
extern uint8 Out_Bound_Flag;

/************************ 出界检测相关常量 ************************/
// 丢线判定最小点数
#define LINE_LOST_MIN_NUM 8
// 偏差出界阈值
#define DEV_OUT_LIMIT 95

/************************ 出界检测函数声明 ************************/
// 纯整数出界检测
void Bound_Lost_Detect(void); 

// 坡道：0-关闭 1-开启
extern uint8 Ramp_Flag;               
// 三岔路口：0-关闭 1-开启
extern uint8 Three_Bif_Flag;          
// 障碍物：0-关闭 1-开启
extern uint8 Barrier_Flag;            
// 断路：0-关闭 1-开启
extern uint8 Disconnection_Flag;      
// T路口：0-关闭 1-开启
extern uint8 T_Way_Flag;              
// 左车库：0-关闭 1-开启
extern uint8 L_Garage_Flag;           
// 右车库：0-关闭 1-开启
extern uint8 R_Garage_Flag;           
// 小S弯：0-关闭 1-识别
extern uint8 Small_S_Flag;            

// 斑马线计数辅助标志
extern uint8 Zebra_Count_Flag;

extern uint8 shot_CD;
extern uint8 shot_CD_flag ;
extern uint8 shot_CD_cnt ;

// 靶子识别
extern uint16 shot_center_x;
extern uint16 shot_center_y;
extern uint8  lamp;
 //extern uint8 laser_fired;   // 激光相关，已禁用
 
void draw_target_center(void);

uint8 Element_Judgement(uint8(*image)[Image_X], uint16 l_total_num, uint16 r_total_num, int8 *l_dir, int8 *r_dir, uint8(*l_line)[2], uint8(*r_line)[2], uint8 x_meet, uint8 y_meet,
                        uint8 *l_border, uint8 *r_border);
						
void Cross_Border_Reshape(void);

void ring_shot_judge(uint8(*image)[Image_X],uint8(*l_line)[2], uint8(*r_line)[2],uint8* l_border, uint8* r_border);

extern uint8 ring_shot_flag;
extern uint8 cnt;

#endif

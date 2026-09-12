#include "file_h_all.h"
//
#define WIFI_SSID_TEST          "null"
#define WIFI_PASSWORD_TEST      "00000000"

#define low_speed 130
#define high_speed low_speed+20

#define BOUNDARY_NUM            (Image_Y * 3 / 2)

uint8 xy_x1_boundary[BOUNDARY_NUM], xy_x2_boundary[BOUNDARY_NUM], xy_x3_boundary[BOUNDARY_NUM];
uint8 xy_y1_boundary[BOUNDARY_NUM], xy_y2_boundary[BOUNDARY_NUM], xy_y3_boundary[BOUNDARY_NUM];
uint8 x1_boundary[MT9V03X_H], x2_boundary[MT9V03X_H], x3_boundary[MT9V03X_H];
uint8 y1_boundary[MT9V03X_W], y2_boundary[MT9V03X_W], y3_boundary[MT9V03X_W];
uint8 image_copy[Image_Y][Image_X];

uint8 j = 0;
uint8 i_photo_send = 0;

uint8 Adaptive_L_Start_Point[2] = { 0 };
uint8 Adaptive_R_Start_Point[2] = { 0 };
uint8 Adaptive_L_Line[(uint8)160][2] = {{0}};
uint8 Adaptive_R_Line[(uint8)160][2] = {{0}};
int8 Adaptive_L_Grow_Dir[(uint8)160] = {0};
int8 Adaptive_R_Grow_Dir[(uint8)160] = {0};
uint16 Adaptive_L_Statics = 0;
uint16 Adaptive_R_Statics = 0;

uint8 Adaptive_X_Meet = 0;
uint8 Adaptive_Y_Meet = 0;

uint8 R_Border[Image_Y] = {0};
uint8 L_Border[Image_Y] = {0};
uint8 R_Unilateral_Patrol_Border[Image_Y] = {0};
uint8 L_Unilateral_Patrol_Border[Image_Y] = {0};

int16 Image_Count = 0;
uint8 Image_Count_Flag = 0;
uint8 Element_State = 0;

uint8 time_flag=0;
uint8 photo_flag=0;
int32 time_ms=0;
int32 photo_cnt=0 ;

volatile uint8 buzzer_flag = 0;
volatile uint16 buzzer_count = 0;

float Z_Yaw = 0;
uint8 IMU_JF_Flag = 0;
uint8 imu_flag=0;

 char buf[32];
        int len;
		 int s=0;

// ============================================================
// ★ 全程周期性激光发射方案（注释待启用）
// 思路：不依赖靶子识别，激光全程按固定间隔自动脉冲
//      车正常巡线跑过靶子，脉冲命中哪个是哪个
// PIT 基准: 10ms/拍 (TIM0_PIT @ 100Hz)
// 间隔 100ms(10拍), 脉宽 20ms(2拍), 占空比 ~17%
// 车速 ~2m/s 时，靶子区域 40cm 通过时间 ~200ms
// → 每个靶子经过时至少命中 1~2 次
// ============================================================
 //#define LASER_PERIODIC_INTERVAL  10   // 发射间隔: 10拍 = 100ms
 //#define LASER_PERIODIC_DURATION   2   // 脉冲宽度:  2拍 =  20ms

 //static uint8 laser_periodic_timer = 0;    // 脉冲定时器 (10ms/拍)
 //static uint8 laser_periodic_on    = 0;    // 1=激光正在发射中

 //void laser_periodic_handler(void)
 //{
     //if (laser_periodic_timer > 0)
     //{
         //laser_periodic_timer--;
     //}

     //if (laser_periodic_timer == 0)
     //{
         //if (laser_periodic_on)
         //{
             // --- 脉冲结束，关闭激光 ---
             //gpio_set_level(IO_P67, 0);
             //gpio_set_level(IO_P76, 0);
             //gpio_set_level(IO_P74, 0);
             //gpio_set_level(IO_P77, 0);
             //gpio_set_level(IO_P75, 0);
             //laser_periodic_on    = 0;
             //laser_periodic_timer = LASER_PERIODIC_INTERVAL;   // 下一轮间隔
         //}
         //else
         //{
             // --- 开始新脉冲，打开激光 ---
             //gpio_set_level(IO_P67, 1);
             //gpio_set_level(IO_P76, 1);
             //gpio_set_level(IO_P74, 1);
             //gpio_set_level(IO_P77, 1);
            // gpio_set_level(IO_P75, 1);
             //laser_periodic_on    = 1;
             //laser_periodic_timer = LASER_PERIODIC_DURATION;   // 脉冲保持
         //}
     //}
 //}
// ============================================================

void buzzer_pit_handler(void)
{
    if(buzzer_flag && buzzer_count)
    {
        buzzer_count--;
        if(buzzer_count == 0)
        {
            buzzer_flag = 0;
            gpio_set_level(IO_P65, 0);
        }
    }
	if(shot_CD != 0){
		shot_CD-- ;
	}
	if(time_flag){ time_ms += 1;}

//	laser_periodic_handler();   // ★ 周期性激光脉冲（已禁用）
}

void buzzer_beep(void)
{
    buzzer_flag = 1;
    buzzer_count = 2;
    gpio_set_level(IO_P65, 1);
}

void main(void)
{

    clock_init(SYSTEM_CLOCK_96M);
    debug_init();

	pit_ms_init(TIM0_PIT, 100, buzzer_pit_handler);
	gpio_init(IO_P65, GPO, 0, GPO_PUSH_PULL);
	buzzer_beep();

  tft180_init();
	tft180_set_dir(TFT180_PORTAIT);
	
	iap_init();
	iap_read_buff(0x00, (uint8*)flash_buff, sizeof(flash_buff));
	button_init();
	servo_init();
	motor_init();
	motor1_pid_init();
	motor2_pid_init();
	servo_ppd_init();
	encoder_init();
	up_pid1_init();
	up_pid2_init();
//	gpio_init(IO_P67, GPO, 0, GPO_PUSH_PULL);   // ★ 周期性激光: P67 推挽输出（已禁用）
//	gpio_init(IO_P76, GPO, 0, GPO_PUSH_PULL);
//	gpio_init(IO_P74, GPO, 0, GPO_PUSH_PULL);
//	gpio_init(IO_P75, GPO, 0, GPO_PUSH_PULL);
//	gpio_init(IO_P77, GPO, 0, GPO_PUSH_PULL);
	while(1)
	{
		if(imu660ra_init())
		{
			tft180_show_string(0, 0, "imu reinit.");
		}
		else break;
	}

		imu_flag = 1;
	servo_control(servo_motor_duty);
//	pwm_set_duty(725);
//	while(1);
   while(1)
   {
       if(mt9v03x_init()){}
       else break;
   }
	 //tft180_show_int32(0, 100, 0, 1);
		system_delay_ms(500);

	   	t=low_speed;
		time_flag=1;
		photo_flag=1;

		 while(1)
	    {

				int16 left_edge, right_edge;
				static uint8 stable_element = 0;
				static uint8 elem_candidate = 0;
				static uint8 elem_debounce = 0;

        if(mt9v03x_finish_flag)
        {
					L_Boundary_Limit = (L_Up_Turn_Left_Point_Flag_1 == 1 && R_Up_Turn_Right_Point_Flag_1 == 0 && R_Straight_Flag == 1) ? 65 : 48;
					R_Boundary_Limit = (R_Up_Turn_Right_Point_Flag_1 == 1 && L_Up_Turn_Left_Point_Flag_1 == 0 && L_Straight_Flag == 1) ? 65 : 48;

					Process_Image();

			Get_Information();
			Element = Element_Judgement(Find_Line_Image, Adaptive_L_Statics, Adaptive_R_Statics, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, Adaptive_L_Line, Adaptive_R_Line, Adaptive_X_Meet, Adaptive_Y_Meet, L_Border, R_Border);
			tft180_show_int32(50, 64, Element,8);
			if (Element != elem_candidate) {
					elem_candidate = Element;
					elem_debounce = 0;
			} else {
					elem_debounce++;
					if (elem_debounce >= 3) {     // ����3֡һ�²��л�
							stable_element = elem_candidate;
					}
			}
//		    tft180_show_int32(0, 100, L_Straight_Flag, 3);
//			tft180_show_int32(30, 100,  R_Straight_Flag, 3);
//			tft180_show_int32(60, 100,  Adaptive_Y_Meet, 1);
//			tft180_show_int32(90, 100,  Adaptive_X_Meet, 1);



	 	switch(stable_element){
			case 2: //  Straightaway
			{
					Get_Middle_Line(L_Border , R_Border , C_Line , Image_Y - 3 , 0);
			  Get_Straight_Deviation_Value(2, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);

				{
						uint8 far_row   = Adaptive_Y_Meet + 3;
						int16 far_width  = (int16)R_Border[far_row] - (int16)L_Border[far_row];
						int16 near_width = (int16)R_Border[50] - (int16)L_Border[50];
					
						// 丢线保护: 远处或近处任一侧边线丢失时(L_Border==3/R_Border==76),
						// far_width 会被撑到 ~73 虚高, 误判为开阔直道而加速 -> 入弯前冲一下
						if(L_Border[far_row] == 3 || R_Border[far_row] == 76 ||
						   L_Border[50]     == 3 || R_Border[50]     == 76)
						{
								t = low_speed - 15;
						}
						else if(far_width > near_width )
								t = low_speed - 15;
						else
								t = high_speed;
				}

				ring_shot_judge(Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, L_Border, R_Border);
				speed_control();
				break;

			}
			case 3: //   L_Turn
			{
				if(My_ABS(Deviation_Value) >= 35)
					t = low_speed - 18;
				else if(My_ABS(Deviation_Value) >= 20)
					t = low_speed - 13;
				else
					t = low_speed - 10;
				Get_Middle_Line(L_Border , R_Border , C_Line , Image_Y - 3 , 0);
			  Get_Straight_Deviation_Value(0, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);

			  ring_shot_judge(Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line , L_Border , R_Border);

			speed_control();

				break;
			}
			case 4: //   R_Turn
			{
				if(My_ABS(Deviation_Value) >= 35)
					t = low_speed - 18;
				else if(My_ABS(Deviation_Value) >= 20)
					t = low_speed - 13;
				else
					t = low_speed - 10;
				Get_Middle_Line(L_Border , R_Border , C_Line , Image_Y - 3 , 0);
				Get_Straight_Deviation_Value(0, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);

				ring_shot_judge(Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line , L_Border , R_Border);

				speed_control();

				break;
			}
			case 5: //   L_Circle
			{
				t=low_speed - 25;
				L_Circle_Process( );
				break;
			}
			case 6: //   R_Circle
			{
				t=low_speed - 25;
				L_Boundary_Limit = 35; R_Boundary_Limit = 35;
				R_Circle_Process( );
				break;
			}
			case 18: //   Small_s
			{;
			}
			default :
				t = low_speed - 18;
				Get_Middle_Line(L_Border , R_Border , C_Line , Image_Y - 3 , 0);
				Get_Straight_Deviation_Value(0, Adaptive_Y_Meet, Image_Y - 3, C_Line, 0);
				ring_shot_judge(Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line , L_Border , R_Border);
				speed_control();
		}

			tft180_show_gray_image(0, 0,
				(const uint8 *)Find_Line_Image,
              Image_X,
              Image_Y,
               80,
               60,
               0);
			Draw_Border_Points_1(Adaptive_L_Line, Adaptive_R_Line, C_Line , Adaptive_L_Statics, Adaptive_R_Statics);
//			draw_target_center();
            mt9v03x_finish_flag = 0;
        }

    }
}

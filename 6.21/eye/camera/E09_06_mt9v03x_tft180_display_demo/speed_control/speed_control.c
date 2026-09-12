#include "file_h_all.h"

int8 out_flag=1;
int16 lock_time;

//#define BRAKE_FRAMES   25    // 刹车持续帧数
//#define BRAKE_SPEED    25    // 刹车期间的目标速度
//#define REARM_FRAMES   30    // 靶子消失多少帧后允许打下一个

//static uint8  laser_done      = 0;   // 本次靶子已处理
//static uint8  brake_cnt       = 0;   // 刹车倒计时
//static uint16 laser_rearm_cnt = 0;   // 重新装弹倒计时

void speed_control()
{
	 Bound_Lost_Detect();
		
		if(Out_Bound_Flag == 1 || stop_flag == 2 )         //出界
		{
			if(up_flag == 1)   // ★只有跑起来之后才永久锁死
      {
          out_flag = 0;
      }
			
			photo_flag=0;
			time_flag=0;
			out_flag=0;
			motor1_pid.Target=0;
			motor2_pid.Target=0;
			
//			pwm_set_duty(BRUSHLESS_PWM_3, 500);
//            pwm_set_duty(BRUSHLESS_PWM_4, 500);
			
//			tft180_show_string(0,80, "");			
//			tft180_show_int32(64, 80, time_ms);
//			tft180_show_string(0 , 2*16, "t_ms:");   tft180_show_int32(6*6, 2*16, time_ms,5);
//			
//			tft180_show_string(0,64, "p_cnt:");			
//			tft180_show_int32(8*6, 64, photo_cnt,8);
//			
//			motor1_control(motor1_duty);
//			motor2_control(motor2_duty);
    
			// ????????,????
			Last_Deviation_Value = 0;
		}
		
		else                           //未出界
		{
			
//			if(Deviation_Value > -5 && Deviation_Value < 5)
//			{
//				servo_motor_duty = 3000;
//				servo_control(servo_motor_duty);
//			}
//			else
//			{
//				imu660ra_get_gyro();
//				// === 刹车倒计时：覆盖 t ===
//				if(brake_cnt > 0)
//				{
//						t = BRAKE_SPEED;
//						brake_cnt--;
//				}

//				// === 靶子检测 →触发刹车 ===
//				if(ring_shot_flag && !laser_done)
//				{
//						brake_cnt = BRAKE_FRAMES;
//						laser_done = 1;
//				}

//				// === 靶子离开后重新装弹 ===
//				if(laser_done)
//				{
//						if(!ring_shot_flag)
//								laser_rearm_cnt++;
//						else
//								laser_rearm_cnt = 0;

//						if(laser_rearm_cnt > REARM_FRAMES)
//						{
//								laser_done = 0;
//								laser_rearm_cnt = 0;
//								laser_fired = 0;
//						}
//				}
				servo_update(&servo_pid);
				servo_motor_duty = 2950 + servo_pid.servo_out;
				servo_control(servo_motor_duty);
	
////			}
//			imu_flag=1;
			
			if(out_flag == 1)     
			{
				
				up_pid_1.Target=50;
				up_pid_2.Target=50;
				if(servo_pid.servo_out>=0)//右转
				{
//					motor1_pid.Target=t;///左轮加速
//					motor2_pid.Target=t;
					motor1_pid.Target=t+speed_diff(t)*0.5;///左轮加速
					motor2_pid.Target=t-speed_diff(t)*1.1;
				}
				else ///左转
				{
//					motor1_pid.Target=t;///左轮加速
//					motor2_pid.Target=t;
					motor1_pid.Target=t-speed_diff(t)*1.1;//左轮减速
					motor2_pid.Target=t+speed_diff(t)*0.5;
				}
				
//				motor1_pid.Target=t;
//				motor2_pid.Target=t;
			}
					

		}
		
//    if((encoder_data_dir_1<30&&motor2_pid.Out>5000)||(encoder_data_dir_2<30&&motor2_pid.Out>5000))  //防堵转
//		{
//			 lock_time++;
//		}
//		else 
//		{
//			lock_time=0;
//		}
//		
//		if(lock_time>=50)
//		{
//			Out_Bound_Flag = 1 ;
//		}


}

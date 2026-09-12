/*********************************************************************************************************************
* STC32G Opensourec Library ¼´£¨STC32G ¿ªÔ´¿â£©ÊÇÒ»¸ö»ùÓÚ¹Ù·½ SDK ½Ó¿ÚµÄµÚÈý·½¿ªÔ´¿â
* Copyright (c) 2025 SEEKFREE Öð·É¿Æ¼¼
*
* ±¾ÎÄ¼þÊÇSTC32G144K¿ªÔ´¿âµÄÒ»²¿·Ö
*
* STC32G ¿ªÔ´¿â ÊÇÃâ·ÑÈí¼þ
* Äú¿ÉÒÔ¸ù¾Ý×ÔÓÉÈí¼þ»ù½ð»á·¢²¼µÄ GPL£¨GNU General Public License£¬¼´ GNUÍ¨ÓÃ¹«¹²Ðí¿ÉÖ¤£©µÄÌõ¿î
* ¼´ GPL µÄµÚ3°æ£¨¼´ GPL3.0£©»ò£¨ÄúÑ¡ÔñµÄ£©ÈÎºÎºóÀ´µÄ°æ±¾£¬ÖØÐÂ·¢²¼ºÍ/»òÐÞ¸ÄËü
*
* ±¾¿ªÔ´¿âµÄ·¢²¼ÊÇÏ£ÍûËüÄÜ·¢»Ó×÷ÓÃ£¬µ«²¢Î´¶ÔÆä×÷ÈÎºÎµÄ±£Ö¤
* ÉõÖÁÃ»ÓÐÒþº¬µÄÊÊÏúÐÔ»òÊÊºÏÌØ¶¨ÓÃÍ¾µÄ±£Ö¤
* ¸ü¶àÏ¸½ÚÇë²Î¼û GPL
*
* ÄúÓ¦¸ÃÔÚÊÕµ½±¾¿ªÔ´¿âµÄÍ¬Ê±ÊÕµ½Ò»·Ý GPL µÄ¸±±¾
* Èç¹ûÃ»ÓÐ£¬Çë²ÎÔÄ<https://www.gnu.org/licenses/>
*
* ¶îÍâ×¢Ã÷£º
* ±¾¿ªÔ´¿âÊ¹ÓÃ GPL3.0 ¿ªÔ´Ðí¿ÉÖ¤Ð­Òé ÒÔÉÏÐí¿ÉÉêÃ÷ÎªÒëÎÄ°æ±¾
* Ðí¿ÉÉêÃ÷Ó¢ÎÄ°æÔÚ libraries/doc ÎÄ¼þ¼ÐÏÂµÄ GPL3_permission_statement.txt ÎÄ¼þÖÐ
* Ðí¿ÉÖ¤¸±±¾ÔÚ libraries ÎÄ¼þ¼ÐÏÂ ¼´¸ÃÎÄ¼þ¼ÐÏÂµÄ LICENSE ÎÄ¼þ
* »¶Ó­¸÷Î»Ê¹ÓÃ²¢´«²¥±¾³ÌÐò µ«ÐÞ¸ÄÄÚÈÝÊ±±ØÐë±£ÁôÖð·É¿Æ¼¼µÄ°æÈ¨ÉùÃ÷£¨¼´±¾ÉùÃ÷£©
*
* ÎÄ¼þÃû³Æ          
* ¹«Ë¾Ãû³Æ          ³É¶¼Öð·É¿Æ¼¼ÓÐÏÞ¹«Ë¾
* °æ±¾ÐÅÏ¢          ²é¿´ libraries/doc ÎÄ¼þ¼ÐÄÚ version ÎÄ¼þ °æ±¾ËµÃ÷
* ¿ª·¢»·¾³          MDK FOR C251
* ÊÊÓÃÆ½Ì¨          STC32G144K
* µêÆÌÁ´½Ó          https://seekfree.taobao.com/
*
* ÐÞ¸Ä¼ÇÂ¼
* ÈÕÆÚ              ×÷Õß           ±¸×¢
* 2024-08-01        ´óW            first version
********************************************************************************************************************/

#include "file_h_all.h"
// *************************** Àý³ÌÓ²¼þÁ¬½ÓËµÃ÷ ***************************
// Ê¹ÓÃ type-c Á¬½Ó
//      Ö±½Ó½«type-c²åÈëºËÐÄ°å£¬¼´¿ÉÊ¹ÓÃUSB-CDC
//
// ½ÓÈë·½Ïò±àÂëÆ÷Á¬½Ó
//      Ä£¿é¹Ü½Å            µ¥Æ¬»ú¹Ü½Å
//      LSB                 ENCODER_DIR_PULSE_1 ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P60
//      DIR                 ENCODER_DIR_DIR_1   ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P62
//      GND                 ºËÐÄ°åµçÔ´µØ GND
//      3V3                 ºËÐÄ°å 3V3 µçÔ´
//
//      LSB                 ENCODER_DIR_PULSE_2 ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P40
//      DIR                 ENCODER_DIR_DIR_2   ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P42
//      GND                 ºËÐÄ°åµçÔ´µØ GND
//      3V3                 ºËÐÄ°å 3V3 µçÔ´
//
//      LSB                 ENCODER_DIR_PULSE_3 ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P80
//      DIR                 ENCODER_DIR_DIR_3   ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P45
//      GND                 ºËÐÄ°åµçÔ´µØ GND
//      3V3                 ºËÐÄ°å 3V3 µçÔ´
//
//      LSB                 ENCODER_DIR_PULSE_4 ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P90
//      DIR                 ENCODER_DIR_DIR_4   ºê¶¨ÒåµÄÒý½Å 	Ä¬ÈÏ P46
//      GND                 ºËÐÄ°åµçÔ´µØ GND
//      3V3                 ºËÐÄ°å 3V3 µçÔ´
//
// *************************** Àý³Ì²âÊÔËµÃ÷ ***************************
// 1.ºËÐÄ°åÉÕÂ¼Íê³É±¾Àý³Ì£¬Ê¹ÓÃ type-c½Ó¿Ú£¬²¢Á¬½ÓºÃ±àÂëÆ÷£¬ÔÚ¶ÏµçÇé¿öÏÂÍê³ÉÁ¬½Ó
//
// 2.½« type-cµÄUSB-CDC½Ó¿ÚÁ¬½Ó µçÄÔ£¬Íê³ÉÉÏµç
//
// 3.µçÄÔÉÏÊ¹ÓÃ´®¿ÚÖúÊÖ´ò¿ª¶ÔÓ¦µÄ´®¿Ú£¬´®¿Ú²¨ÌØÂÊÎª zf_common_debug.h ÎÄ¼þÖÐ DEBUG_UART_BAUDRATE ºê¶¨Òå Ä¬ÈÏ 115200£¬ºËÐÄ°å°´ÏÂ¸´Î»°´¼ü
//
// 4.¿ÉÒÔÔÚ´®¿ÚÖúÊÖÉÏ¿´µ½ÈçÏÂ´®¿ÚÐÅÏ¢£º
//      encoder_data_dir_1 counter     x .
//      encoder_data_dir_2 counter     x .
//      encoder_data_dir_3 counter     x .
//      encoder_data_dir_4 counter     x .
//
// 5.×ª¶¯±àÂëÆ÷¾Í»á¿´µ½ÊýÖµ±ä»¯
//
// STC32G144KÖ»ÓÐPWM½Ó¿ÚÖ§³ÖÕý½»½âÂë±àÂëÆ÷.¶¨Ê±Æ÷½Ó¿Ú²»Ö§³ÖÕý½»±àÂëÆ÷¡£
// STC32G144KÖ»ÓÐPWM½Ó¿ÚÖ§³ÖÕý½»½âÂë±àÂëÆ÷.¶¨Ê±Æ÷½Ó¿Ú²»Ö§³ÖÕý½»±àÂëÆ÷¡£
// STC32G144KÖ»ÓÐPWM½Ó¿ÚÖ§³ÖÕý½»½âÂë±àÂëÆ÷.¶¨Ê±Æ÷½Ó¿Ú²»Ö§³ÖÕý½»±àÂëÆ÷¡£


// **************************** ´úÂëÇøÓò ****************************
#define PIT_CH                          (TIM1_PIT )                 // Ê¹ÓÃµÄÖÜÆÚÖÐ¶Ï±àºÅ Èç¹ûÐÞ¸Ä ÐèÒªÍ¬²½¶ÔÓ¦ÐÞ¸ÄÖÜÆÚÖÐ¶Ï±àºÅÓë isr.c ÖÐµÄµ÷ÓÃ
//#define PIT_PRIORITY                    (TIM1_IRQn)               TIM1µÄÖÐ¶ÏÓÅÏÈ¼¶Ä¬ÈÏ×îµÍ£¬²»¿ÉÐÞ¸Ä£¬¾ßÌå¿´ÊÖ²á¡£

#define ENCODER_DIR_1                 	(PWMA_ENCODER)              // ´ø·½Ïò±àÂëÆ÷¶ÔÓ¦Ê¹ÓÃµÄ±àÂëÆ÷½Ó¿Ú 
#define ENCODER_DIR_PULSE_1            	(PWMA_ENCODER_CH1P_P60)     // PULSE ¶ÔÓ¦µÄÒý½Å
#define ENCODER_DIR_DIR_1              	(PWMA_ENCODER_CH2P_P62)     // DIR ¶ÔÓ¦µÄÒý½Å

#define ENCODER_DIR_2                 	(PWMC_ENCODER)              // ´ø·½Ïò±àÂëÆ÷¶ÔÓ¦Ê¹ÓÃµÄ±àÂëÆ÷½Ó¿Ú
#define ENCODER_DIR_PULSE_2       		(PWMC_ENCODER_CH1P_P40)     // PULSE ¶ÔÓ¦µÄÒý½Å
#define ENCODER_DIR_DIR_2           	(PWMC_ENCODER_CH2P_P42)     // DIR ¶ÔÓ¦µÄÒý½Å

int16 encoder_data_dir_1 = 0;
int16 encoder_data_dir_2 = 0;

int16 encoder_filtered_1 = 0;
int16 encoder_filtered_2 = 0;  //一阶滤波
//int16 flash_buff[]={1,1,0,0,0,0,0,0,0,0};
int16 flash_buff[]={1,1,0,0,19,0,0,0,0,0};///PID
int16 down_buff[10]={0};

int8 up_flag=0;///0或者1

PID_t up_pid_1;
PID_t up_pid_2;
PID_t motor1_pid;
PID_t motor2_pid;
PPD_t servo_pid;

void up_pid1_init(void)
{
	up_pid_1.Target=0;
	up_pid_1.Kp=15;      
	up_pid_1.Ki=8;       
	up_pid_1.Kd=0;
	up_pid_1.OutMax=4000;
	up_pid_1.OutMin=-4000;
}

void up_pid2_init(void)
{
	up_pid_2.Target=0;
	up_pid_2.Kp=15;      
	up_pid_2.Ki=8;       
	up_pid_2.Kd=0;
	up_pid_2.OutMax=4000;
	up_pid_2.OutMin=-4000;
}

void motor1_pid_init(void)
{
	motor1_pid.Target=0;
	motor1_pid.Kp=50;      //flash_buff[0]
	motor1_pid.Ki=0;       //flash_buff[1]
	motor1_pid.Kd=10;
	motor1_pid.OutMax=6000;
	motor1_pid.OutMin=-6000;
}

void motor2_pid_init(void)
{
	motor2_pid.Target=0;
	motor2_pid.Kp=50;   //27   //flash_buff[2]
	motor2_pid.Ki=0;   //51    //flash_buff[3]
	motor2_pid.Kd=10;
	motor2_pid.OutMax=6000;
	motor2_pid.OutMin=-6000;
}

void servo_ppd_init()
{
//	servo_pid.Kp1 = flash_buff[4];           //24     //flash_buff[4]
//	servo_pid.Kp2 = flash_buff[8];           //7     
//	servo_pid.Kd1 = flash_buff[5];			//50					//flash_buff[5] 5
//	servo_pid.Kd2 = flash_buff[6];            //40   //flash_buff[6]  20
	servo_pid.Kp1 = 30;  // 原27，降低以减少直道晃动  41        //  35  速度120  //24
	servo_pid.Kp2 = 3;           //   0
	servo_pid.Kd1 = 12;			//50					//flash_buff[5] 5
	servo_pid.Kd2 = -165;            //40   //flash_buff[6]  20        52 3 6 -190       40 3 6 -185
	

//	servo_pid.Kp1 = 0;           //  35  速度120   
//	servo_pid.Kp2 = 0;    	       //   0    
//	servo_pid.Kd1 = 0;			//50					//flash_buff[5] 5
//	servo_pid.Kd2 = -200;            //40   //flash_buff[6]  20因为转向陀螺仪安装反向，因此要修改
	
	servo_pid.servoMax= 390;
	servo_pid.servoMin= -390;
}
void pit_handler (void);

void encoder_init(void)
{

    encoder_dir_init(ENCODER_DIR_1, ENCODER_DIR_PULSE_1, ENCODER_DIR_DIR_1);   	// ³õÊ¼»¯±àÂëÆ÷Ä£¿éÓëÒý½Å ´ø·½ÏòÔöÁ¿±àÂëÆ÷Ä£Ê½
    encoder_dir_init(ENCODER_DIR_2, ENCODER_DIR_PULSE_2, ENCODER_DIR_DIR_2);   // ³õÊ¼»¯±àÂëÆ÷Ä£¿éÓëÒý½Å ´ø·½ÏòÔöÁ¿±àÂëÆ÷Ä£Ê½

    
	// ÉèÖÃ10msµÄÖÜÆÚ¶¨Ê±Æ÷
    pit_ms_init(PIT_CH, 5, pit_handler);                                      // ³õÊ¼»¯ PIT ÎªÖÜÆÚÖÐ¶Ï 5ms ÖÜÆÚ

//    interrupt_set_priority(PIT_PRIORITY, 0);                                  // TIM1µÄÖÐ¶ÏÓÅÏÈ¼¶Ä¬ÈÏ×îµÍ£¬²»¿ÉÐÞ¸Ä£¬¾ßÌå¿´ÊÖ²á¡£

}


//        printf("encoder_data_dir_1 counter %.1f .\r\n", encoder_data_dir_1);      // Êä³ö±àÂëÆ÷¼ÆÊýÐÅÏ¢
//        printf("encoder_data_dir_2 counter %.1f .\r\n", encoder_data_dir_2);      // Êä³ö±àÂëÆ÷¼ÆÊýÐÅÏ¢
//        system_delay_ms(500);

//-------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------


void pit_handler (void)
{
		int16 gyro_z;
    encoder_data_dir_1= -encoder_get_count(ENCODER_DIR_1);                  // »ñÈ¡±àÂëÆ÷¼ÆÊý
    encoder_data_dir_2 = encoder_get_count(ENCODER_DIR_2);                  // »ñÈ¡±àÂëÆ÷¼ÆÊý

    encoder_clear_count(ENCODER_DIR_1);                                		// Çå¿Õ±àÂëÆ÷¼ÆÊý
    encoder_clear_count(ENCODER_DIR_2);                                		// Çå¿Õ±àÂëÆ÷¼ÆÊý
		
		encoder_filtered_1 += (encoder_data_dir_1 - encoder_filtered_1) >> 3;
		encoder_filtered_2 += (encoder_data_dir_2 - encoder_filtered_2) >> 3;   
		
			if(imu_flag == 1)
			{
					imu660ra_get_gyro();
					gyro_z = imu660ra_gyro_transition(imu660ra_gyro_z);
			}
	////先不进行打靶	
//	if(shot_CD_flag == 1)
//	{
//		shot_CD_cnt += 5;
//		if(shot_CD_cnt >= 25 )
//		{
//			shot_CD = 15;
//			shot_CD_flag = 0;
//			shot_CD_cnt = 0; 
//			gpio_set_level(IO_P76, 0);
//			gpio_set_level(IO_P74, 0);
//			gpio_set_level(IO_P77, 0);
//			gpio_set_level(IO_P75, 0);
//			gpio_set_level(IO_P67, 0);

//		}
//	}
		
	
    if(IMU_JF_Flag == 1)
    {
        Z_Yaw += gyro_z * 0.005;
    }
	//if(encoder_data_dir_1<50&&up_flag==0)
	if((ads(encoder_data_dir_1) < 50 || ads(encoder_data_dir_2) < 50) && up_flag == 0)
	{
		up_pid_1.Actual=encoder_filtered_2;		//motor1
  	    PID_Update(&up_pid_1);
	    motor1_duty=up_pid_1.Out;
		motor1_control(motor1_duty);
	
	    up_pid_2.Actual=encoder_filtered_1;		//motor2
  	    PID_Update(&up_pid_2);
	    motor2_duty=up_pid_2.Out;
		motor2_control(motor2_duty);
	}
	else 
	{
			up_flag=1;

			motor1_pid.Actual=encoder_filtered_2;		//motor1
			PID_Update(&motor1_pid);
			motor1_duty=motor1_pid.Out;
			motor1_control(motor1_duty);
		
			motor2_pid.Actual=encoder_filtered_1;		//motor2
			PID_Update(&motor2_pid);
			motor2_duty=motor2_pid.Out;
			motor2_control(motor2_duty);
	}

}

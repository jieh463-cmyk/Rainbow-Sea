#include "file_h_all.h"
// *************************** Àý³ÌÓ²¼þÁ¬½ÓËµÃ÷ ***************************
// Ê¹ÓÃ  ºËÐÄ°åÖ±½Ó½ÓÏß½øÐÐ²âÊÔ
//      Ä£¿é¹Ü½Å            µ¥Æ¬»ú¹Ü½Å
//      1DIR                P51
//      1PWM                P50
//      GND                 GND
//      2DIR                P53
//      2PWM                P52
//      GND                 GND
//      3DIR                P75
//      3PWM                P74
//      GND                 GND
//      4DIR                P77
//      4PWM                P76
//      GND                 GND
//      ½ÓÏß¶Ë×Ó +          µç³ØÕý¼«
//      ½ÓÏß¶Ë×Ó -          µç³Ø¸º¼«
//
// Ê¹ÓÃ  Ñ§Ï°Ö÷°å½øÐÐ²âÊÔ
//      ½«Ä£¿éµÄµçÔ´½ÓÏß¶Ë×ÓÓëÖ÷°åµÄÇý¶¯¹©µç¶Ë×ÓÁ¬½Ó
//      ½«Ä£¿éµÄÐÅºÅ½Ó¿ÚÊ¹ÓÃÅäÌ×»ÒÅÅÏßÓëÖ÷°åµç»úÐÅºÅ½Ó¿ÚÁ¬½Ó Çë×¢Òâ½ÓÏß·½Ïò ²»È·¶¨·½Ïò¾ÍÊÇÓÃÍòÓÃ±íÈ·ÈÏÒ»ÏÂ Òý½Å²Î¿¼ÉÏ·½ºËÐÄ°åÁ¬½Ó
//      ½«Ö÷°åÓë¹©µçµç³ØÕýÈ·Á¬½Ó


// *************************** Àý³Ì²âÊÔËµÃ÷ ***************************
// 1.ºËÐÄ°åÉÕÂ¼Íê³É±¾Àý³Ì Ö÷°åµç³Ø¹©µç
//
// 2.Èç¹û½ÓÁËµç»ú ¿ÉÒÔ¿´µ½µç»úÖÜÆÚÕý·´×ª
//
// 3.Èç¹ûÃ»ÓÐ½Óµç»ú Ê¹ÓÃÍòÓÃ±í¿ÉÒÔÔÚÇý¶¯µç»úÊä³ö¶Ë×ÓÉÏ²âÁ¿µ½Êä³öµçÑ¹±ä»¯
//
// Èç¹û·¢ÏÖÏÖÏóÓëËµÃ÷ÑÏÖØ²»·û Çë²ÎÕÕ±¾ÎÄ¼þ×îÏÂ·½ Àý³Ì³£¼ûÎÊÌâËµÃ÷ ½øÐÐÅÅ²é

// **************************** ´úÂëÇøÓò ****************************
                                      
#define MAX_DUTY            ( 50 )                                               // 最大 MAX_DUTY% 占空比

#define DIR_1               ( IO_P51 ) 
#define PWM_1               ( PWMD_CH1_P50 )
                              
#define DIR_2               ( IO_P53 ) 
#define PWM_2               ( PWMD_CH3_P52 )
                               

int16 motor1_duty=0;
int16 motor2_duty=0;
void motor_init(void)
{
		gpio_init(DIR_1, GPO, GPIO_HIGH, GPO_PUSH_PULL); 
    pwm_init(PWM_1, 17000, 0); 
		
		gpio_init(DIR_2, GPO, GPIO_HIGH, GPO_PUSH_PULL);
	  pwm_init(PWM_2, 17000, 0); 
}

void motor1_control(int16 motor_duty)
{
   if(motor_duty >= 0)                                                           // Õý×ª
   {
			  gpio_set_level(DIR_1, GPIO_HIGH);
        pwm_set_duty(PWM_1, motor_duty);                   // ¼ÆËãÕ¼¿Õ±È
	 }
	 else 
	 {
				gpio_set_level(DIR_1, GPIO_LOW);
	      pwm_set_duty(PWM_1, -motor_duty);  
	 }
                           
}

void motor2_control(int16 motor_duty)
{
   if(motor_duty >= 0)                                                           // Õý×ª
   {
				gpio_set_level(DIR_2, GPIO_HIGH);
        pwm_set_duty(PWM_2,motor_duty );                   // ¼ÆËãÕ¼¿Õ±È
	 }
	 else 
	 {
				gpio_set_level(DIR_2, GPIO_LOW);
	      pwm_set_duty(PWM_2, -motor_duty);  
//      pwm_set_duty(PWM_2, -motor_duty );  
	 }
                           
}
  
int16 diffspeed;
int16 speed_diff(int16 target)
{
	if(Deviation_Value > -5 && Deviation_Value < 5)
	{
			diffspeed=0;
		return diffspeed;
	}
	else 
	{
		float tn=tan( (servo_pid.servo_out/4) /62.8*0.42);
	  diffspeed=0.4*target*tn;
		if(diffspeed<0) diffspeed=-diffspeed;
	
		return diffspeed;
	}

}
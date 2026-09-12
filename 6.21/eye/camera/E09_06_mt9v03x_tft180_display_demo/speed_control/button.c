#include "file_h_all.h"

// ���尴������
#define KEY1_PIN        IO_PB2
#define KEY2_PIN        IO_PB3
#define KEY3_PIN        IO_PB4
#define KEY4_PIN        IO_P32

#define SWITCH1_PIN     IO_PB0
#define SWITCH2_PIN     IO_PB1

// ����״̬����
uint8 key1_status;
uint8 key2_status;
uint8 key3_status;
uint8 key4_status;

// ��һ�ο���״̬����
uint8 key1_last_status=1;
uint8 key2_last_status=1;
uint8 key3_last_status=1;
uint8 key4_last_status=1;

// ���ر�־λ
uint8 key1_flag;
uint8 key2_flag;
uint8 key3_flag;
uint8 key4_flag;

uint8 test1=0,test2=0,test3=0,test4=0;
int16 t;


void button_init(void)
{
	gpio_init(KEY1_PIN, GPI, 1, GPI_PULL_UP);
	gpio_init(KEY2_PIN, GPI, 1, GPI_PULL_UP);
	gpio_init(KEY3_PIN, GPI, 1, GPI_PULL_UP);
	gpio_init(KEY4_PIN, GPI, 1, GPI_PULL_UP);

    gpio_init(SWITCH1_PIN, GPI, 1, GPI_PULL_UP);
    gpio_init(SWITCH2_PIN, GPI, 1, GPI_PULL_UP);

}

void button_control(void)
{
													
		// ��ȡ��ǰ����״̬
        key1_status = gpio_get_level(KEY1_PIN);
        key2_status = gpio_get_level(KEY2_PIN);
        key3_status = gpio_get_level(KEY3_PIN);
        key4_status = gpio_get_level(KEY4_PIN);
        
        if(key1_status && !key1_last_status)    key1_flag = 1;
        if(key2_status && !key2_last_status)    key2_flag = 1;
        if(key3_status && !key3_last_status)    key3_flag = 1;
        if(key4_status && !key4_last_status)    key4_flag = 1;
        
        if(key1_flag)   
        {
					
            key1_flag = 0;
            test1++;
						test1=test1%5;
						test2=0;
						tft180_clear(tft180_bgcolor);
					down_buff[0]=motor1_pid.Kp;
					down_buff[1]=motor1_pid.Ki;
					down_buff[2]=motor2_pid.Kp;
					down_buff[3]=motor2_pid.Ki;
					down_buff[4]=servo_pid.Kp1;
					down_buff[5]=servo_pid.Kd1;
					down_buff[6]=servo_pid.Kd2;
					down_buff[7]=t;
					down_buff[8]=servo_pid.Kp2;
					down_buff[9]=0;
					 iap_erase_page(0);	
					iap_write_buff(0x00, (uint8*)down_buff, sizeof(down_buff));
        }
        
        if(key2_flag)   
        {
            key2_flag = 0;  //�
           test2++;
					if(test1==0)
					 {
							test2=test2%4;
					 }
					 else if(test1==1)
					 {
							test2=test2%4;
					 }
					 else if(test1==2)
					 {
							test2=test2%4;
					 }
        }
        
        if(key3_flag)   
        {
            key3_flag = 0;
            if(test1 == 0 )
            {
                switch(test2)
                {
                    case 0: motor1_pid.Kp += 1; break;
                    case 1: motor1_pid.Ki += 1; break;
                    case 2: motor1_pid.Kd += 1; break;
                    case 3: t += 10; break;

                }
            }
			else if( test1 == 1)
            {
                switch(test2)
                {
                    case 0: motor2_pid.Kp += 1; break;
                    case 1: motor2_pid.Ki += 1; break;
                    case 2: motor2_pid.Kd += 1; break;
                    case 3: t += 10; break;

                }
            }
            else if(test1 == 2)
            {
                switch(test2)
                {
                    case 0: servo_pid.Kp1 += 5; break;
                    case 1: servo_pid.Kd1 += 5; break;
                    case 2: servo_pid.Kd2 += 5; break;
                    case 3: servo_pid.Kp2 += 5; break;
                }
            }
        }
        
        if(key4_flag)   
        {
            key4_flag = 0;
            if(test1 == 0 )
            {
                switch(test2)
                {
                    case 0: motor1_pid.Kp -= 1; break;
                    case 1: motor1_pid.Ki -= 1; break;
                    case 2: motor1_pid.Kd -= 1; break;
                    case 3: t -= 10; break;
                   
                }
            }
						else if( test1 == 1)
            {
                switch(test2)
                {
                    case 0: motor2_pid.Kp -= 1; break;
                    case 1: motor2_pid.Ki -= 1; break;
                    case 2: motor2_pid.Kd -= 1; break;
                    case 3: t -= 10; break;
                }
            }
            else if(test1 == 2)
            {
                switch(test2)
                {
                    case 0: servo_pid.Kp1 -= 1; break;
                    case 1: servo_pid.Kd1 -= 1; break;
                    case 2: servo_pid.Kd2 -= 1; break;
                    case 3: servo_pid.Kp2 -= 1; break;
                }
            }
        }
		  key1_last_status = key1_status;
        key2_last_status = key2_status;
        key3_last_status = key3_status;
        key4_last_status = key4_status;
        
}

void tft180_show(void)
{
				if(test1==0)
				{
					if(test2==0)
					{
						tft180_show_string(0 , 0*16, "->p:");   tft180_show_int16(11*6, 0*16, motor1_pid.Kp);
					}
					else 
					{
						tft180_show_string(0 , 0*16, "  p:");   tft180_show_int16(11*6, 0*16, motor1_pid.Kp);
					}
					if(test2==1)
					{
						tft180_show_string(0 , 1*16, "->i:");   tft180_show_int16(11*6, 1*16, motor1_pid.Ki);
					}
					else 
					{
						tft180_show_string(0 , 1*16, "  i:");   tft180_show_int16(11*6, 1*16, motor1_pid.Ki);
					}
					if(test2==2)
					{
						tft180_show_string(0 , 2*16, "->d:");   tft180_show_int16(11*6, 2*16, motor1_pid.Kd);
					}
					else 
					{
						tft180_show_string(0 , 2*16, "  d:");   tft180_show_int16(11*6, 2*16, motor1_pid.Kd);
					}
					if(test2==3)
					{
						tft180_show_string(0 , 3*16, "->t:");   tft180_show_int16(11*6, 3*16, motor1_pid.Target);
					}
					else 
					{
						tft180_show_string(0 , 3*16, "  t:");   tft180_show_int16(11*6, 3*16, motor1_pid.Target);
					}
					if(test2==4)
					{
						tft180_show_string(0 , 4*16, "->en1:");   tft180_show_int16(11*6, 4*16, encoder_data_dir_1);
					}
					else 
					{
						tft180_show_string(0 , 4*16, "  en1:");   tft180_show_int16(11*6, 4*16, encoder_data_dir_1);
					}
					if(test2==5)
					{
						tft180_show_string(0 , 5*16, "->motor1");   tft180_show_int16(11*6, 5*16, motor1_duty);
					}
					else 
					{
						tft180_show_string(0 , 5*16, "  motor1");   tft180_show_int16(11*6, 5*16, motor1_duty);
					}
					
					tft180_show_string(0 , 7*16, "  straight:r ");
				}
			  else if(test1==1)
				{
					if(test2==0)
					{
						tft180_show_string(0 , 0*16, "->p:");   tft180_show_int16(11*6, 0*16, motor2_pid.Kp);
					}
					else 
					{
						tft180_show_string(0 , 0*16, "  p:");   tft180_show_int16(11*6, 0*16, motor2_pid.Kp);
					}
					if(test2==1)
					{
						tft180_show_string(0 , 1*16, "->i:");   tft180_show_int16(11*6, 1*16, motor2_pid.Ki);
					}
					else 
					{
						tft180_show_string(0 , 1*16, "  i:");   tft180_show_int16(11*6, 1*16, motor2_pid.Ki);
					}
					if(test2==2)
					{
						tft180_show_string(0 , 2*16, "->d:");   tft180_show_int16(11*6, 2*16, motor2_pid.Kd);
					}
					else 
					{
						tft180_show_string(0 , 2*16, "  d:");   tft180_show_int16(11*6, 2*16, motor2_pid.Kd);
					}
					if(test2==3)
					{
						tft180_show_string(0 , 3*16, "->t:");   tft180_show_int16(11*6, 3*16, motor2_pid.Target);
					}
					else 
					{
						tft180_show_string(0 , 3*16, "  t:");   tft180_show_int16(11*6, 3*16, motor2_pid.Target);
					}
					if(test2==4)
					{
						tft180_show_string(0 , 4*16, "->en1:");   tft180_show_int16(11*6, 4*16, encoder_data_dir_2);
					}
					else 
					{
						tft180_show_string(0 , 4*16, "  en1:");   tft180_show_int16(11*6, 4*16, encoder_data_dir_2);
					}
					if(test2==5)
					{
						tft180_show_string(0 , 5*16, "->motor1");   tft180_show_int16(11*6, 5*16, motor2_duty);
					}
					else 
					{
						tft180_show_string(0 , 5*16, "  motor1");   tft180_show_int16(11*6, 5*16, motor2_duty);
					}
					
					tft180_show_string(0 , 7*16, "  straight:l ");
				}
				else if(test1==2)
				{
					if(test2==0)
					{
						tft180_show_string(0 , 0*16, "->p1:");   tft180_show_float(11*6, 0*16, servo_pid.Kp1,2,0);
					}
					else 
					{
						tft180_show_string(0 , 0*16, "  p1:");   tft180_show_float(11*6, 0*16, servo_pid.Kp1,2,0);
					}
					if(test2==1)
					{
						tft180_show_string(0 , 1*16, "->d1:");   tft180_show_int16(11*6, 1*16, servo_pid.Kd1);
					}
					else 
					{
						tft180_show_string(0 , 1*16, "  d1:");   tft180_show_int16(11*6, 1*16, servo_pid.Kd1);
					}
					if(test2==2)
					{
						tft180_show_string(0 , 2*16, "->d:");   tft180_show_int16(11*6, 2*16, servo_pid.Kd2);
					}
					else 
					{
						tft180_show_string(0 , 2*16, "  d:");   tft180_show_int16(11*6, 2*16, servo_pid.Kd2);
					}
					if(test2==3)
					{
						tft180_show_string(0 , 3*16, "->p2:");   tft180_show_int16(11*6, 3*16, servo_pid.Kp2);
					}
					else 
					{
						tft180_show_string(0 , 3*16, "  p2:");   tft180_show_int16(11*6, 3*16, servo_pid.Kp2);
					}
					
					tft180_show_string(0 , 7*16, "   angle ");
					
				}
}

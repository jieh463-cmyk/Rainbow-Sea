#ifndef __MOTOR_H_
#define __MOTOR_H_

#define BRUSHLESS_PWM_3              (PWMF_CH3_PA5)
#define BRUSHLESS_PWM_4              (PWMF_CH4_PA7)
#define BRUSHLESS_FREQ               (50)                                                // ?????50HZ,????300HZ(??)
 
extern int16 motor1_duty;
extern int16 motor2_duty;
void motor_init(void);
void motor1_control(int16 motor_duty);
void brushless_init(void);
void motor2_control(int16 motor_duty);

int16 speed_diff(int16 target);
#endif

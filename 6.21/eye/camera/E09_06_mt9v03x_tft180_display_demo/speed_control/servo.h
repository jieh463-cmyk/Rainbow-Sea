#ifndef __SERVO_H_
#define __SERVO_H_
extern int16 servo_motor_duty;
void servo_init(void);

void servo_control(servo_motor_duty);



#endif
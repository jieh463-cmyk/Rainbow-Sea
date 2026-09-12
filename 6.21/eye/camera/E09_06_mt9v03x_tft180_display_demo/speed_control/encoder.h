#ifndef __ENCODER_H_
#define __ENCODER_H_

#include "pid.h"
#include "servo.h"

void encoder_init(void);
void pit_handler (void);
extern int16 encoder_data_dir_1;
extern int16 encoder_data_dir_2;
extern int8 up_flag;
void motor1_pid_init(void);
void motor2_pid_init(void);
void servo_ppd_init(void);
void up_pid1_init(void);
void up_pid2_init(void);
extern PID_t motor1_pid;
extern PID_t motor2_pid;
extern PID_t up_pid_1;
extern PID_t up_pid_2;
extern PPD_t servo_pid;
extern int16 flash_buff[10];
extern int16 down_buff[10];
#endif

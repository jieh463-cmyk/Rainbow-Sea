#ifndef __PID_H
#define __PID_H

typedef struct {
	int16 Target;
	int16 Actual;
	int16 Out;
	
	int16 Kp;
	int16 Ki;
	int16 Kd;
	
	int16 Error0;
	int16 Error1;
	int16 Error2;
	
	int16 OutMax;
	int16 OutMin;
} PID_t;

typedef struct {

	int16 servo_out;
	
	float Kp1;
	int16 Kp2;
	int16 Kd1;
	int16 Kd2;
	
	int16 Error0;
	int16 Error1;
	
	int16 servoMax;
	int16 servoMin;
} PPD_t;

void PID_Update(PID_t *p);

int16 ads(int16 a);
void servo_update(PPD_t *p);

#endif

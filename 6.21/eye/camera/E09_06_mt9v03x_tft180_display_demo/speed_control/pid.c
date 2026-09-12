#include "file_h_all.h"

int16 ads(int16 a)
{
	if(a<0)a=-a;
	return a;
}


void PID_Update(PID_t *p)
{
	/*获取本次误差和上次误差*/
	p->Error2 = p->Error1;
	p->Error1 = p->Error0;					//获取上次误差
	p->Error0 = p->Target - p->Actual;		//获取本次误差，目标值减实际值，即为误差值



	/*PID计算*/
	p->Out += p->Kp * (p->Error0-p->Error1)
		   + (p->Ki/10.0) * p->Error0
		   + p->Kd * (p->Error0 - 2*p->Error1+p->Error2);




	/*输出限幅*/
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}


void servo_update(PPD_t *p)
{
	static int16 error_i = 0;

	p->Error1 = p->Error0;
	p->Error0 = -Deviation_Value;

	/* 小积分项：仅直道(偏差<10)慢慢累积消除稳态偏置，弯道清零 */
	if (ads(Deviation_Value) < 10) {
		error_i += p->Error0;
		if (error_i >  200) error_i =  200;
		if (error_i < -200) error_i = -200;
	} else {
		error_i = 0;
	}
	
	if(L_Circle_State == 3 || L_Circle_State == 4) {
		p->Kd2 = -65;    //-70
	}
	
//	if(Element == Straightaway) {
//		p->Kd2 += 20;
//	}
	
	p->servo_out = p->Kp1 / 10.0 * p->Error0
	             + p->Kp2 / 100.0 * p->Error0 * ads(p->Error0)
	             + (p->Kd1 * (p->Error0 - p->Error1)) / 10.0
	             + p->Kd2 * (imu660ra_gyro_z / 1000.0)
	             + error_i / 100;

	if (p->servo_out > p->servoMax) { p->servo_out = p->servoMax; }
	if (p->servo_out < p->servoMin) { p->servo_out = p->servoMin; }
}

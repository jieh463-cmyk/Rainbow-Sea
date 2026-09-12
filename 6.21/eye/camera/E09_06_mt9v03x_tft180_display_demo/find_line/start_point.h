#ifndef _start_point_h
#define _start_point_h
#define Compare_Num(a, b, th)  \
    ((uint16)((((a) > (b)) ? ((a) - (b)) : ((b) - (a))) << 7) > (uint16)(th) * ((uint16)(a) + (uint16)(b)))
	
//int16 Compare_Num(int16 a, int16 b, uint8 compare_value);
uint8 Get_Start_Point(uint8 start_row, uint8(*image)[Image_X], uint8 *l_start_point, uint8 *r_start_point, uint8 l_border_x, uint8 r_border_x);

extern uint8 Start_Flag;
extern uint8 Compare_Value;
extern uint8 Black_Box_Value_1;

#endif

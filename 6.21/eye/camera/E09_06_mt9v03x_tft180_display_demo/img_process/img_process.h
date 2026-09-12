#ifndef _img_process_h
#define _img_process_h

#define Image_X  80   //Ñ¹ËõÍ¼Ïñ¿í
#define Image_Y  60  //Ñ¹ËõÍ¼Ïñ³¤

void Copy_Zip_Image(void);  //Ñ¹ËõÍ¼Ïñ
void Draw_Black_Box(uint8 black_box_value, uint8(*image)[Image_X]);	  //²¹ºÚ¿ò
void Thres_Record_Process(void);
uint8 Fast_Sqrt8(uint16 val);
void Process_Image(void);
void photo_send(void);
	
extern uint8 Inverse_Flag;  //ÄæÍ¸ÊÓ±êÖ¾Î»   0¹Ø±Õ£¬1¿ªÆô 
extern uint8 far Find_Line_Image[Image_Y][Image_X];  //Ñ¹ËõºóÍ¼Ïñ
extern uint8 Image_Num;

extern uint8 Black_Box_Value;  //²¹ºÚ¿ò»Ò¶ÈÖµ
extern uint8 Black_Box_Value_FFF;
extern uint8 Black_Box_Value_FF;
extern uint8 Black_Box_Value_F;

extern uint8 Adaptive_L_Thres_Max;     //å·¦ä¾§é˜ˆå€¼æœ€å¤§å€¼
extern uint8 Adaptive_R_Thres_Max;     //å³ä¾§é˜ˆå€¼æœ€å¤§å€¼
extern uint8 Adaptive_L_Thres_Min;     //å·¦ä¾§é˜ˆå€¼æœ€å°å€¼
extern uint8 Adaptive_R_Thres_Min;     //å³ä¾§é˜ˆå€¼æœ€å°å€¼
extern uint8 Adaptive_Thres_Average;   //é˜ˆå€¼å‡å€¼
extern uint8 Last_Adaptive_Thres_Average;  //ç”¨äºé˜ˆå€¼å‡å€¼æ»¤æ³¢

#endif

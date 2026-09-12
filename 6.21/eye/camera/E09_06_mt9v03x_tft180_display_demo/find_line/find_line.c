#include "file_h_all.h"
//**************�������****************
//��¼����ʱ��Ҫ��ÿ�������Ϊ��һ���֣����ڼ򻯺����㷨�����������ķ�����ʱ��    ˳ʱ�룩
//3  2  1               1  2  3
//4     0               0     4
//5  6  7               7  6  5
/////////////////////////////////
//{-1,-1},{0,-1},{+1,-1},
//{-1, 0},       {+1, 0},
//{-1,+1},{0,+1},{+1,+1},
//�Թ�ȱ�����޷��������һ���𲽼�¼���򣬵��������޷Ǿ������ϰ˸��㣬�������ĵ��ֵ�̶����������Թ��㷨�ĳ�����ƶ�������ı䣬����Ƚ�ÿ��������� 3
//{-3,-1},{0,-1},{+3,-1},
//{-3, 0},       {+3, 0},
//{-3,+1},{0,+1},{+3,+1},
//�ٽ��������ȥ������
// -2, 1, 4
// -3,    3
// -4,-1, 2
//�ɴ˿��Եõ�һ�����ڷ������ꡣֻ����ÿ���ƶ����ڷ����������¼��Ӧ���֣��Ϳ�ȷ����������
//���㷨���κ�ԭ���ֻ��Ϊ�˵õ��˸���һ����ֵ���������ж����򣬺�������Գ˴��� 2 ������ֵ��2���ڻ�����ظ�
//************************************
//******************����Ӧ�����Թ�����************************
uint16 L_Thres_Record[MAX_LINE_POINTS];
uint16 R_Thres_Record[MAX_LINE_POINTS];
// 
const int8 L_Face_Dir[4][2] = { {0,-1},{1,0},{0,1},{-1,0} };  //����Թ�����
//  0
//3   1
//  2

const int8 L_Face_Dir_L[4][2] = { {-1,-1},{1,-1},{1,1},{-1,1} };  //����������ǰ��
//0   1
//
//3   2

const int8 R_Face_Dir[4][2] = { {0,-1},{1,0},{0,1},{-1,0} };  //�Ҳ��Թ�����
//  0
//3   1
//  2

const int8 R_Face_Dir_R[4][2] = { {1,-1},{1,1},{-1,1},{-1,-1} };  //�Ҳ��������ǰ��
//3   0
//
//2   1

const int8 Square_0[25][2] = {              //һ��5 * 5�ľ������������ĵ���Χ�ľֲ���ֵ���ֲ���ֵ��
{-2,-2},{-1,-2},{0,-2},{+1,-2},{+2,-2},
{-2,-1},{-1,-1},{0,-1},{+1,-1},{+2,-1},
{-2,-0},{-1, 0},{0, 0},{+1, 0},{+2,-0},
{-2,+1},{-1,+1},{0,+1},{+1,+1},{+2,+1},
{-2,+2},{-1,+2},{0,+2},{+1,+2},{+2,+2}
};
uint8 Clip_Value = 1; //ֵԽ��Խ��������

//�Թ�����ֹͣ���߱�־λ
uint8 L_Stop_Flag = 0;
uint8 R_Stop_Flag = 0;
uint16 L_Boundary_Limit = 45;  //������ߵ������ޣ���⵽����ʱ����
uint16 R_Boundary_Limit = 45;  //�Ҳ����ߵ������ޣ���⵽����ʱ����
//**********************************************************
/******
* �������ܣ�      ��ȡ������ά�������
* ����˵����      �����Ͻ����������Ӧ�Թ��Ż��������Ӧ�����Թ�
* ��  �Σ�        uint16 Break_Flag         ���ѭ����������ֹ��������һ��Ϊ3~4��ͼ����
*                uint8(*image)[Image_X]     ��ȡ���ߵ�ͼ��
*                uint8(*l_line)[2]          ��������ߵĶ�ά����
*                uint8(*r_line)[2]          ����Ҳ���ߵĶ�ά����
*                int8 *l_dir                ���������ÿ�������������
*                int8 *r_dir                ����Ҳ����ÿ�������������
*                uint16 *l_stastic          ��¼�����ߵ�ĸ���
*                uint16 *r_stastic          ��¼�Ҳ���ߵ�ĸ���
*                uint8 *x_meet              ��¼�������������������X����
*                uint8 *y_meet              ��¼�������������������Y����
*                uint8 l_start_x            ���������ʼ���X����
*                uint8 l_start_y            ���������ʼ���Y����
*                uint8 r_start_x            �Ҳ�������ʼ���X����
*                uint8 r_start_y            �Ҳ�������ʼ���Y����
*                uint8 clip_value           ����ÿ����ֵʱ��ӵľ���ֵ��һ��Ϊ-5 ~ 5������ǿ�зָ��ֱ����Ϊ0
*
* ʾ����         Dir_Labyrinth_5((uint16)Use_Num, Find_Line_Image, Adaptive_L_Line, Adaptive_R_Line, Adaptive_L_Grow_Dir, Adaptive_R_Grow_Dir, &Adaptive_L_Statics, &Adaptive_R_Statics, &Adaptive_X_Meet, &Adaptive_Y_Meet,
                   Adaptive_L_Start_Point[0], Adaptive_L_Start_Point[1], Adaptive_R_Start_Point[0], Adaptive_R_Start_Point[1], 0);
* ����ֵ��        ��
*/
void Dir_Labyrinth_5(uint16 Break_Flag, uint8(*image)[Image_X], uint8(*l_line)[2], uint8(*r_line)[2], int8* l_dir, int8* r_dir, uint16* l_stastic, uint16* r_stastic, uint8* x_meet, uint8* y_meet,
    uint8 l_start_x, uint8 l_start_y, uint8 r_start_x, uint8 r_start_y, uint8 clip_value)
{
    uint8 j = 0;


    //��߱���
    uint8 L_Center_Point[2] = { 0 };     //���ÿ���ҵ���XY����
    uint16 L_Data_Statics = 0;          //ͳ������ҵ��ı��ߵ�ĸ���

    uint8  L_Front_Value = 0;           //��� �����ǰ����ĻҶ�ֵ
    uint8  L_Front_L_Value = 0;         //��� �������ǰ����ĻҶ�ֵ

    uint8  L_Dir = 0;                   //�˲�������ת��
    uint8  L_Turn_Num = 0;              //��¼ת������������ĵ�ǰ�����Ҷ��Ǻ�ɫ���أ��ͻ���һ����ת���ĴΣ���¼���Ĵ�ʱ�˳�ѭ����ֹ��������
    uint16 L_Pixel_Value_Sum = 0;       //���ĵ�����Χ24���������ֵ��
    uint16 L_Thres = 0;                 //�ֲ���ֵ,��L_Pixel_Value_Sum / 25

    //�ұ߱���
    uint8  R_Center_Point[2] = { 0 };     //���ÿ���ҵ���XY����
    uint16 R_Data_Statics = 0;          //ͳ���ұ��ҵ��ı��ߵ�ĸ���

    uint8  R_Front_Value = 0;           //�Ҳ� �����ǰ����ĻҶ�ֵ
    uint8  R_Front_R_Value = 0;         //�Ҳ� �������ǰ����ĻҶ�ֵ

    uint8  R_Dir = 0;                   //�˲�������ת��
    uint8  R_Turn_Num = 0;              //��¼ת������������ĵ�ǰ�����Ҷ��Ǻ�ɫ���أ��ͻ���һ����ת���ĴΣ���¼���Ĵ�ʱ�˳�ѭ����ֹ��������
    uint16 R_Pixel_Value_Sum = 0;       //���ĵ�����Χ24���������ֵ��
    uint16 R_Thres = 0;                 //�ֲ���ֵ

    //��һ�θ��������  ���ҵ������ֵ������
    L_Center_Point[0] = l_start_x + 1; //x
    L_Center_Point[1] = l_start_y; //y
    R_Center_Point[0] = r_start_x - 1;//x
    R_Center_Point[1] = r_start_y;//y

    L_Stop_Flag = 0;  // 0:��������    1������������ֹͣ����
    R_Stop_Flag = 0;

    //��������Թ�ѭ��
    while (Break_Flag--)
    {
        //���
       //�ж��������󣬹ҳ�ֹͣ��־λ����������ֹͣ��
        if (L_Stop_Flag == 0)
        {
            l_line[L_Data_Statics][0] = L_Center_Point[0];  //�ҵ������ĵ�X����������������
            l_line[L_Data_Statics][1] = L_Center_Point[1];  //�ҵ������ĵ�Y����������������

            if (L_Data_Statics != 0)
            {
                switch (l_dir[L_Data_Statics - 1])  //������һ����Ը�����һ����������������Ż�����ʱ��
                {
                    //�ӵڶ����㿪ʼ����һ�������ֵҪ25����ȫ����һ��
                    //���������������ʱ����ԭ�ȵ�25�μӷ������Ϊʮ�μ���
                    //��б������ʱ����ԭ�ȵ�25�μӷ������Ϊʮ�˴�����
                    //���Ի���ͼ�������Ŵ����߼��飬�Ϳ��Ժܿ��ٵ����
                case 1:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 3][L_Center_Point[0] + 2] - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 0] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2];
                    break;
                }
                case -2:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 0][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 2][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 3][L_Center_Point[0] + 2]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 0]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2];
                    break;
                }
                case -3:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 2][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 0][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 2][L_Center_Point[0] + 3]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2];
                    break;
                }
                case -4:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 2]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 3] - image[L_Center_Point[1] - 2][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] + 3] - image[L_Center_Point[1] + 0][L_Center_Point[0] + 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] + 3]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] - 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2];
                    break;
                }
                case -1:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] - 3][L_Center_Point[0] - 2] - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2];
                    break;
                }
                case 2:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 0][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 2][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 3][L_Center_Point[0] - 2]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] - 3][L_Center_Point[0] + 0]
                        - image[L_Center_Point[1] - 3][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] + 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] - 2];
                    break;
                }
                case 3:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 2][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 0][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 2][L_Center_Point[0] - 3]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] + 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2];
                    break;
                }
                case 4:
                {
                    L_Pixel_Value_Sum = L_Pixel_Value_Sum - image[L_Center_Point[1] + 3][L_Center_Point[0] + 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 0]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 1] - image[L_Center_Point[1] + 3][L_Center_Point[0] - 2]
                        - image[L_Center_Point[1] + 3][L_Center_Point[0] - 3] - image[L_Center_Point[1] + 2][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] + 1][L_Center_Point[0] - 3] - image[L_Center_Point[1] - 0][L_Center_Point[0] - 3]
                        - image[L_Center_Point[1] - 1][L_Center_Point[0] - 3]
                        + image[L_Center_Point[1] + 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] + 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 0][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 1][L_Center_Point[0] + 2]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 2] + image[L_Center_Point[1] - 2][L_Center_Point[0] + 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] + 0] + image[L_Center_Point[1] - 2][L_Center_Point[0] - 1]
                        + image[L_Center_Point[1] - 2][L_Center_Point[0] - 2];
                    break;
                }
                }
            }
            else
            {
                for (j = 0; j < 25; j++)    //��һ����ֵ��25����ȫ����һ�飬���������ֵ���������������
                {
                    L_Pixel_Value_Sum += image[L_Center_Point[1] + Square_0[j][1]][L_Center_Point[0] + Square_0[j][0]];
                }
            }

            L_Thres = (L_Pixel_Value_Sum + Thres_Interfere) / Thres_Num_Interfere;   //��ֵΪ25����Ҷ�ֵ��ƽ��ֵ
            L_Thres -= clip_value;              //���õ��ĻҶ���ֵ��ȥһ������ֵ�������Ż��ж�

            //����Ϊ����ƽ���˲�����֪�����Ƿ��������㷨���˴�Ϊ����ԭ������������Ӧ�ɽϰ������ֲ����������򣬻�֮�Ĺ��������������ϸ����ԭ��
//            if (Thres_Filiter_Flag_1 == 1 || Thres_Filiter_Flag_2 == 1)
//            {
//                if (L_Data_Statics > 3)
//                {
//                    L_Thres = L_Thres * 1.3f - L_Thres_Record[L_Data_Statics - 1] * 0.2f - L_Thres_Record[L_Data_Statics - 2] * 0.1f;
//                }
//            }

            L_Thres_Record[L_Data_Statics] = L_Thres;
            L_Data_Statics++;                   //ÿ�ҵ�һ����ͳ�Ƹ���+1
            if (L_Data_Statics > L_Boundary_Limit) { L_Stop_Flag = 1; }    //��ֹ180��������������

        L_Judge_Again:    //L_Judge_Again �� goto ���ʹ��
            if (L_Stop_Flag == 0)
            {
                L_Front_Value = image[L_Center_Point[1] + L_Face_Dir[L_Dir][1]][L_Center_Point[0] + L_Face_Dir[L_Dir][0]];          //��¼�����ǰ����ĻҶ�ֵ
                L_Front_L_Value = image[L_Center_Point[1] + L_Face_Dir_L[L_Dir][1]][L_Center_Point[0] + L_Face_Dir_L[L_Dir][0]];    //��¼�������ǰ����ĻҶ�ֵ
                if (L_Front_Value < L_Thres)     //�����ǰ�����Ǻ�ɫ
                {
                    L_Dir = (L_Dir + 1) % 4;    //����תһ��
                    L_Turn_Num++;
                    if (L_Turn_Num == 4)        //��������
                    {
                        L_Stop_Flag = 1;       //��ǰ�����Ҷ��Ǻ�ɫʱ������������ֹͣ�������
                    }
                    goto L_Judge_Again;
                }
                else if (L_Front_L_Value < L_Thres)   //��ǰ�����Ǻ�ɫ��ǰ�����ǰ�ɫ
                {

                    // ���������ȼ�����һ��������
                    // int next_x = L_Center_Point[0] + L_Face_Dir[L_Dir][0];
                    // int next_y = L_Center_Point[1] + L_Face_Dir[L_Dir][1];

                    // ���������߽��жϣ������һ������ͼ��Χ��ֹͣ����
                    // ע�⣺Image_X ��ͼ���ȣ�Image_Y ��ͼ��߶ȣ����滻Ϊ������е�ʵ�ʺ궨��
                    if (L_Center_Point[0] + L_Face_Dir[L_Dir][0] < 3 || L_Center_Point[0] + L_Face_Dir[L_Dir][0] >= 77 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] < 2 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] >= 58)
                    {
                        L_Stop_Flag = 1;
                    }
                    else if (L_Center_Point[1] + L_Face_Dir[L_Dir][1] <= 4 && L_Center_Point[0] + L_Face_Dir[L_Dir][0] > 38)
                    {
                        L_Stop_Flag = 1;
                    }
                    else {
                        L_Center_Point[0] += L_Face_Dir[L_Dir][0];
                        L_Center_Point[1] += L_Face_Dir[L_Dir][1];      //��ǰ��һ��
                        l_dir[L_Data_Statics - 1] = (L_Face_Dir[L_Dir][0] * 3) - L_Face_Dir[L_Dir][1];
                        L_Turn_Num = 0;
                    }

                }
                else        //��ǰ����ǰ�����ǰ�ɫ��
                {

                    // ���������ȼ�����һ��������
                    // int next_x = L_Center_Point[0] + L_Face_Dir[L_Dir][0];
                    // int next_y = L_Center_Point[1] + L_Face_Dir[L_Dir][1];

                    // ���������߽��жϣ������һ������ͼ��Χ��ֹͣ����
                    // ע�⣺Image_X ��ͼ���ȣ�Image_Y ��ͼ��߶ȣ����滻Ϊ������е�ʵ�ʺ궨��
                    if (L_Center_Point[0] + L_Face_Dir[L_Dir][0] < 3 || L_Center_Point[0] + L_Face_Dir[L_Dir][0] >= 77 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] < 2 || L_Center_Point[1] + L_Face_Dir[L_Dir][1] >= 58)
                    {
                        L_Stop_Flag = 1;
                    }
                    else if (L_Center_Point[1] + L_Face_Dir[L_Dir][1] <= 4 && L_Center_Point[0] + L_Face_Dir[L_Dir][0] >38)
                    {
                        L_Stop_Flag = 1;
                    }
                    else {
                        L_Center_Point[0] += L_Face_Dir_L[L_Dir][0];
                        L_Center_Point[1] += L_Face_Dir_L[L_Dir][1];        //����ǰ����һ��
                        l_dir[L_Data_Statics - 1] = (L_Face_Dir_L[L_Dir][0] * 3) - L_Face_Dir_L[L_Dir][1];
                        L_Dir = (L_Dir + 3) % 4;        //��תһ��
                        L_Turn_Num = 0;
                    }

                }
                // �յ㶯̬���磺����յ�=�������磬˫��յ�=ʮ������
//                if (L_Data_Statics >= 8 && L_Boundary_Limit >= 50)
//                {
//                    int8 d_cur  = l_dir[L_Data_Statics - 1];
//                    int8 d_prev = l_dir[L_Data_Statics - 3];
//                    // ��ת��յ㣺���������(1)������(4)תΪ����(-3,-4)�����Ϲ���(-2)
//                    if ((d_cur == -3 || d_cur == -4 || d_cur == -2) &&
//                        (d_prev == 1 || d_prev == 4))
//                    {
//                        // ����Ҳ�Ҳ�������˫��յ�=ʮ�֡�����Ĭ��
//                        if (R_Boundary_Limit >= 55)
//                        {
//                            L_Boundary_Limit = 48;
//                            R_Boundary_Limit = 48;
//                        }
//                        else
//                        {
//                            L_Boundary_Limit = 55;   // ����յ�=��������չ
//                        }
//                    }
//                }
                if (L_Data_Statics >= 5)     //O���������ת��һȦ��ص�ԭ����Ҳ��һ��������������ֹͣ����
                {
                    if (l_line[L_Data_Statics - 1][0] == l_line[L_Data_Statics - 5][0] &&
                        l_line[L_Data_Statics - 1][1] == l_line[L_Data_Statics - 5][1])
                    {
                        L_Stop_Flag = 1;
                    }
                }
            }
        }

        //�Ҳ������ͬ�������Ҳ���ƣ���������Ҳ�ͺܼ�
        if (R_Stop_Flag == 0)
        {
            r_line[R_Data_Statics][0] = R_Center_Point[0];
            r_line[R_Data_Statics][1] = R_Center_Point[1];

            if (R_Data_Statics != 0)
            {
                switch (r_dir[R_Data_Statics - 1])
                {
                case 1:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 3][R_Center_Point[0] + 2] - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 0] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2];
                    break;
                }
                case -2:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 0][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 2][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 3][R_Center_Point[0] + 2]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 0]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2];
                    break;
                }
                case -3:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 2][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 0][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 2][R_Center_Point[0] + 3]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2];
                    break;
                }
                case -4:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 2]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 3] - image[R_Center_Point[1] - 2][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] + 3] - image[R_Center_Point[1] + 0][R_Center_Point[0] + 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] + 3]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] - 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2];
                    break;
                }
                case -1:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] - 3][R_Center_Point[0] - 2] - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2];
                    break;
                }
                case 2:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 0][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 2][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 3][R_Center_Point[0] - 2]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] - 3][R_Center_Point[0] + 0]
                        - image[R_Center_Point[1] - 3][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] + 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] - 2];
                    break;
                }
                case 3:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 2][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 0][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 2][R_Center_Point[0] - 3]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] + 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2];
                    break;
                }
                case 4:
                {
                    R_Pixel_Value_Sum = R_Pixel_Value_Sum - image[R_Center_Point[1] + 3][R_Center_Point[0] + 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 0]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 1] - image[R_Center_Point[1] + 3][R_Center_Point[0] - 2]
                        - image[R_Center_Point[1] + 3][R_Center_Point[0] - 3] - image[R_Center_Point[1] + 2][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] + 1][R_Center_Point[0] - 3] - image[R_Center_Point[1] - 0][R_Center_Point[0] - 3]
                        - image[R_Center_Point[1] - 1][R_Center_Point[0] - 3]
                        + image[R_Center_Point[1] + 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] + 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 0][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 1][R_Center_Point[0] + 2]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 2] + image[R_Center_Point[1] - 2][R_Center_Point[0] + 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] + 0] + image[R_Center_Point[1] - 2][R_Center_Point[0] - 1]
                        + image[R_Center_Point[1] - 2][R_Center_Point[0] - 2];
                    break;
                }
                }
            }
            else
            {
                for (j = 0; j < 25; j++)
                {
                    R_Pixel_Value_Sum += image[R_Center_Point[1] + Square_0[j][1]][R_Center_Point[0] + Square_0[j][0]];
                }
            }

            R_Thres = (R_Pixel_Value_Sum + Thres_Interfere) / Thres_Num_Interfere;
            R_Thres -= clip_value;

//            if (Thres_Filiter_Flag_1 == 1 || Thres_Filiter_Flag_2 == 1)
//            {
//                if (R_Data_Statics > 3)
//                {
//                    R_Thres = R_Thres * 1.3f - R_Thres_Record[R_Data_Statics - 1] * 0.2f - R_Thres_Record[R_Data_Statics - 2] * 0.1f;
//                }
//            }

            R_Thres_Record[R_Data_Statics] = R_Thres;

            R_Data_Statics++;
            if (R_Data_Statics > R_Boundary_Limit) { R_Stop_Flag = 1; }    //��ֹ180��������������

        R_Judgme_Again:
            if (R_Stop_Flag == 0)
            {
                R_Front_Value = image[R_Center_Point[1] + R_Face_Dir[R_Dir][1]][R_Center_Point[0] + R_Face_Dir[R_Dir][0]];
                R_Front_R_Value = image[R_Center_Point[1] + R_Face_Dir_R[R_Dir][1]][R_Center_Point[0] + R_Face_Dir_R[R_Dir][0]];
                if (R_Front_Value < R_Thres)
                {
                    R_Dir = (R_Dir + 3) % 4;
                    R_Turn_Num++;
                    if (R_Turn_Num == 4)
                    {
                        R_Stop_Flag = 1;
                    }
                    goto R_Judgme_Again;
                }
                else if (R_Front_R_Value < R_Thres)
                {
                    // ���������ȼ�����һ�������꣨��ǰ��
                    // int next_x = R_Center_Point[0] + R_Face_Dir[R_Dir][0];
                    // int next_y = R_Center_Point[1] + R_Face_Dir[R_Dir][1];

                    // ���������߽��ж�
                    if (R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 3 || R_Center_Point[0] + R_Face_Dir[R_Dir][0] >= 77 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] < 2 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] >= 58)
                    {
                        R_Stop_Flag = 1;
                    }
                    else if (R_Center_Point[1] + R_Face_Dir[R_Dir][1] <=4  &&  R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 42)
                    {
                        R_Stop_Flag = 1;
                    }
                    else {
                        R_Center_Point[0] += R_Face_Dir[R_Dir][0];
                        R_Center_Point[1] += R_Face_Dir[R_Dir][1];
                        r_dir[R_Data_Statics - 1] = R_Face_Dir[R_Dir][0] * 3 - R_Face_Dir[R_Dir][1];
                        R_Turn_Num = 0;
                    }

                }
                else
                {

                    // ���������ȼ�����һ�������꣨��ǰ��
                    // int next_x = R_Center_Point[0] + R_Face_Dir[R_Dir][0];
                    // int next_y = R_Center_Point[1] + R_Face_Dir[R_Dir][1];

                    // ���������߽��ж�
                    if (R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 3 || R_Center_Point[0] + R_Face_Dir[R_Dir][0] >= 77 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] < 2 || R_Center_Point[1] + R_Face_Dir[R_Dir][1] >= 58)
                    {
                        R_Stop_Flag = 1;
                    }
                    else if (R_Center_Point[1] + R_Face_Dir[R_Dir][1] <= 4 && R_Center_Point[0] + R_Face_Dir[R_Dir][0] < 42)
                    {
                        R_Stop_Flag = 1;
                    }
                    else {
                        R_Center_Point[0] += R_Face_Dir_R[R_Dir][0];
                        R_Center_Point[1] += R_Face_Dir_R[R_Dir][1];
                        r_dir[R_Data_Statics - 1] = R_Face_Dir_R[R_Dir][0] * 3 - R_Face_Dir_R[R_Dir][1];
                        R_Dir = (R_Dir + 1) % 4;
                        R_Turn_Num = 0;
                    }

                }
                // �յ㶯̬���磺����յ�=�������磬˫��յ�=ʮ������
                if (R_Data_Statics >= 8 && R_Boundary_Limit >= 50)
                {
                    int8 d_cur  = r_dir[R_Data_Statics - 1];
                    int8 d_prev = r_dir[R_Data_Statics - 3];
                    // ��ת�ҹյ㣺�������/б��(-2,1)תΪ��(3,4,2)
                    if ((d_cur == 3 || d_cur == 4 || d_cur == 2) &&
                        (d_prev == -2 || d_prev == 1))
                    {
                        // ������Ҳ�������˫��յ�=ʮ�֡�����Ĭ��
                        if (L_Boundary_Limit >= 55)
                        {
                            L_Boundary_Limit = 48;
                            R_Boundary_Limit = 48;
                        }
                        else
                        {
                            R_Boundary_Limit = 55;   // ����յ�=��������չ
                        }
                    }
                }
                if (R_Data_Statics >= 5)
                {
                    if (r_line[R_Data_Statics - 1][0] == r_line[R_Data_Statics - 5][0] &&
                        r_line[R_Data_Statics - 1][1] == r_line[R_Data_Statics - 5][1])
                    {
                        R_Stop_Flag = 1;
                    }
                }
            }
        }

        if (L_Stop_Flag == 0 && R_Stop_Flag == 0)
        {
            if ((My_ABS(r_line[R_Data_Statics - 1][0] - l_line[L_Data_Statics - 1][0]) <= 3)
                && (My_ABS(r_line[R_Data_Statics - 1][1] - l_line[L_Data_Statics - 1][1]) <= 3))        //���������������˳�ѭ����һ��ͼ�����߽���
            {
                *y_meet = (r_line[R_Data_Statics - 1][1] + l_line[L_Data_Statics - 1][1]) >> 1;  //��¼������Y
                *x_meet = (r_line[R_Data_Statics - 1][0] + l_line[L_Data_Statics - 1][0]) >> 1;  //��¼������X
                break;
            }
        }
        //��һ���������ʱ������������ж��ſ���һЩ����ֹʵ��������û���ж����������������ҵ����
        else
        {
            if ((My_ABS(r_line[R_Data_Statics - 1][0] - l_line[L_Data_Statics - 1][0]) <= 5)
                && (My_ABS(r_line[R_Data_Statics - 1][1] - l_line[L_Data_Statics - 1][1]) <= 5))        //���������������˳�ѭ����һ��ͼ�����߽���
            {
                *y_meet = (r_line[R_Data_Statics - 1][1] + l_line[L_Data_Statics - 1][1]) >> 1;  //��¼������Y
                *x_meet = (r_line[R_Data_Statics - 1][0] + l_line[L_Data_Statics - 1][0]) >> 1;  //��¼������X
                break;
            }
        }
    }
    L_Stop_Flag = 0;
    R_Stop_Flag = 0;
    *l_stastic = L_Data_Statics;    //��¼�����ߵ����
    *r_stastic = R_Data_Statics;    //��¼�Ҳ���ߵ����
}


//*****************************������ȡ*********************************

/**
* �������ܣ�      �ɶ�ά����������ȡһά����
* ����˵����      ��
* ��  �Σ�        uint16 l_total       //����ά���ߵ�ĸ���
*                 uint16 r_total      //�Ҳ��ά���ߵ�ĸ���
*                 uint8 start         //��ʼ�У�ͼ��ײ���
*                 uint8 end           //��ֹ�У�ͼ�񶥲���
*                 uint8 *l_border     //�洢���һά���ߵ�����
*                 uint8 *r_border     //�洢�Ҳ�һά���ߵ�����
*                 uint8(*l_line)[2]   //�洢����ά���ߵ�����
*                 uint8(*r_line)[2]   //�洢�Ҳ��ά���ߵ�����
*
* ʾ����          Get_Border(L_Statics, R_Statics, Image_Y - 3, 2, L_Border, R_Border, L_Line, R_Line);
* ����ֵ��        ��
*/
//void Get_Border(uint16 l_total, uint16 r_total, uint8 start, uint8 end, uint8 *l_border, uint8 *r_border, uint8(*l_line)[2], uint8(*r_line)[2])
//{
//    uint8 i = 0;
//    uint16 j = 0;
//    uint8 h = 0;
//    for (i = 0; i < Image_Y; i++)
//    {
//        l_border[i] = X_Border_Min;
//        r_border[i] = X_Border_Max;     //�ұ��߳�ʼ���ŵ����ұߣ�����߷ŵ�����ߣ������պ�����������߾ͻ����м䣬������ŵõ�������
//    }
//    h = start;
//    //�ұ�
//    for (j = 0; j < r_total ; j++)
//    {
//        if (r_line[j][1] == h)
//        {
//            r_border[h] = r_line[j][0];
//        }
//        else
//        {
//            continue;//ÿ��ֻȡһ���㣬û����һ�оͲ���¼
//        }
//        h--;
//        if (h == end)
//        {
//            break;//�����һ���˳�
//        }
//    }
//    h = start;
//    for (j = 0; j < l_total; j++)
//    {
//        if (l_line[j][1] == h)
//        {
//            l_border[h] = l_line[j][0];
//        }
//        else
//        {
//            continue;//ÿ��ֻȡһ���㣬û����һ�оͲ���¼
//        }
//        h--;
//        if (h == end)
//        {
//            break;//�����һ���˳�
//        }
//    }
//}

void Get_Border(uint16 l_total, uint16 r_total, uint8 start, uint8 end, uint8 *l_border, uint8 *r_border, uint8(*l_line)[2], uint8(*r_line)[2])
{
    uint8 i;
    uint16 j;
    for (i = 0; i < Image_Y; i++)
    {
        l_border[i] = X_Border_Min;
        r_border[i] = X_Border_Max;
    }
    
    if(Element == R_Circle){
			
			for (i = 0; i < Image_Y; i++)
    {
        R_Unilateral_Patrol_Border[i] = 40 ;
    }
    // ========== ��ȡ�ұ߽磺ͬһ��ȡ�����ĵ� ==========
    for (j = 0; j < r_total; j++)
    {
        uint8 x = r_line[j][0];
        uint8 y = r_line[j][1];

        if (y >= end && y <= start)
        {
            // ������СX = �����ұ߽�
            if ( x > R_Unilateral_Patrol_Border[y])
            {
                R_Unilateral_Patrol_Border[y] = x;
            }
        }
    }
  }
	
  if(Element == L_Circle){
			
	for (i = 0; i < Image_Y; i++)
    {
        L_Unilateral_Patrol_Border[i] = 40 ;
    }
    // ========== ��ȡ�ұ߽磺ͬһ��ȡ�����ĵ� ==========
    for (j = 0; j < r_total; j++)
    {
        uint8 x = l_line[j][0];
        uint8 y = l_line[j][1];

        if (y >= end && y <= start)
        {
            // ������СX = �����ұ߽�
            if ( x < L_Unilateral_Patrol_Border[y])
            {
                L_Unilateral_Patrol_Border[y] = x;
            }
        }
    }
  }
    // ��ʼ��
    

    // ========== ��ȡ��߽磺ͬһ��ȡ���Ҳ�ĵ� ==========
    for (j = 0; j < l_total; j++)
    {
        uint8 x = l_line[j][0];
        uint8 y = l_line[j][1];

        // ֻ������Ч��
        if (y >= end && y <= start)
        {
            // �������X = ��ҵ���߽�
            if (x > l_border[y])
            {
                l_border[y] = x;
            }
        }
    }

    // ========== ��ȡ�ұ߽磺ͬһ��ȡ�����ĵ� ==========
    for (j = 0; j < r_total; j++)
    {
        uint8 x = r_line[j][0];
        uint8 y = r_line[j][1];

        if (y >= end && y <= start)
        {
            // ������СX = �����ұ߽�
            if (r_border[y] == X_Border_Max || x < r_border[y])
            {
                r_border[y] = x;
            }
        }
    }
      
}
//***************************************************************************************

//****************************************������ȡ**********************************************

// l_border	uint8 *	��߽�����ָ�룬�洢��߽��������� / ��ֵ��Ϣ
// r_border	uint8 *	�ұ߽�����ָ�룬�洢�ұ߽��������� / ��ֵ��Ϣ
// c_line	uint8 *	�����������������ָ�룬������������߽�����������
// point_num	uint8	�߽�����������Ч������������ڿ���ѭ�������ķ�Χ
// patching_flag	uint8	����ģʽ��־�����Ʋ�ͬ�����µĲ����߼��������ߡ��Ҳ��ߡ�Բ�����ߵȣ�
// unilateral_patrol_flag	uint8	����Ѳ��ģʽ��־�����Ʋ�ͬ�����µĵ���Ѳ���߼������󵥱ߡ��ҵ���Ѳ�ߵȣ�
//uint8 Cross_Center[60] = {38,38,38,38,38, 39,39,39,39,39, 39,39,39,39,39,  40,40,40,40,40, 40,40,40,40,40, 40,40,40,40,40,   40,40,40,40,40,  40,40,40,40,40,  40,40,40,40,40,  40,40,40,40,40,  40,40,40,40,40,  40,40,40,40,40};
uint8 C_Line[(uint16)Image_Y] = {0};
void Get_Middle_Line(uint8 *l_border, uint8 *r_border, uint8 *c_line, uint8 point_num /* , uint8 patching_flag */, uint8 unilateral_patrol_flag )
{
	uint8 i;

	switch(unilateral_patrol_flag)  //����Ѳ��
    {
        case 0:
        {
            for(i = 2; i < point_num + 2; i++)
            {
                if(Element == L_Turn)
                    c_line[i] = (l_border[i] + r_border[i] + 1) >> 1;  //�����������ƣ�����߽�?
                else if(Element == R_Turn)
                    c_line[i] = (l_border[i] + r_border[i] - 1) >> 1;  //�����������ƣ�����߽�?
//                else if(l_border[i] <= 2 && r_border[i] >= 77) 							
//										c_line[i] = Cross_Center[i]; 
                else  
									  c_line[i] = (l_border[i] + r_border[i]) >> 1;
            }
            break;
        }
        case L_Unilateral_Patrol:
        {
            uint8 L_Staright[60] = { 0, 0, 4, 5, 5, 5, 6, 6, 6, 7,
                                     7, 7, 8, 8, 9, 9,10,11,11,12,
                                    12,13,13,13,13,14,14,15,15,16,
                                    16,16,17,17,18,18,19,19,19,20,
                                    20,21,21,21,21,22,22,23,23,24,
                                    24,24,25,25,25,26,26,27,27,28};
            for(i = 2; i < point_num + 2; i++)
            {
                c_line[i] = l_border[i] + L_Staright[i] + 6;
            }
            break;
        }
        case R_Unilateral_Patrol:
        {
            uint8 R_Staright[60] = { 0, 0, 4, 5, 5, 5, 6, 6, 6, 7,
                                     7, 7, 8, 8, 9, 9,10,11,11,12,
                                    12,13,13,13,13,14,14,15,15,16,
                                    16,16,17,17,18,18,19,19,19,20,
                                    20,21,21,21,21,22,22,23,23,24,
                                    24,24,25,25,25,26,26,27,27,28};
            for(i = 2; i < point_num + 2; i++)
            {
                c_line[i] = r_border[i] - R_Staright[i] - 4;
            }
            break;
        }
        case L_Arc_Unilateral_Patrol_2:
        {
            uint8 L_Arc_1[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 1, 1, 2, 2, 2, 3, 5,
                                  6, 8, 9,11,12,13,14,15,16,16,
                                 17,18,18,19,19,20,21,22,22,22,
                                 23,23,23,24,25,25,26,26,26,26,
                                 27,27,28,29,29,29,30,0, 0, 0};
            for(i = 2; i < point_num + 2; i++)
            {
                c_line[i] = L_Unilateral_Patrol_Border[i] + L_Arc_1[i] + 1;
            }
            break;
        }
        case R_Arc_Unilateral_Patrol_2:
        {
            uint8 R_Arc_1[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 1, 1, 2, 2, 2, 3, 5,
                                  6, 8, 9,11,12,13,14,15,16,16,
                                 17,18,18,19,19,20,21,22,22,22,
                                 23,23,23,24,25,25,26,26,26,26,
                                 27,27,28,29,29,29,30,30, 0, 0};
            for(i = 2; i < point_num + 2; i++)
            {
							
                c_line[i] = R_Unilateral_Patrol_Border[i] - R_Arc_1[i] - 1 ;
            }
            break;
        }
        case L_Arc_Unilateral_Patrol_4:
        {
            uint8 L_Arc_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 1, 2, 3, 3, 5, 6, 8,
                                  9,11,12,14,15,16,17,18,19,19,
                                 20,21,21,21,22,23,23,24,24,24,
                                 25,25,25,25,26,26,27,27,27,27,
                                 28,28,29,30,30,30,31,31, 0, 0};
            for(i = 2; i < point_num + 2; i++)
            {
                c_line[i] = L_Unilateral_Patrol_Border[i] + L_Arc_2[i] - 3;
            }
            break;
        }
        case R_Arc_Unilateral_Patrol_4:
        {
            uint8 R_Arc_2[60] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 1, 2, 3, 3, 5, 6, 8,
                                  9,11,12,14,15,16,17,18,19,19,
                                 20,21,21,21,22,23,23,24,24,24,
                                 25,25,25,25,26,26,27,27,27,27,
                                 28,28,29,30,30,30,31,31, 0, 0};
            for(i = 2; i < point_num + 2; i++)
            {
                c_line[i] = R_Unilateral_Patrol_Border[i] - R_Arc_2[i] + 3;
            }
            break;
        }
    }
}


//*****************************TFT180��Ļ����***********************************
// �ڱ�����ȡ��ɺ���ô˺������п��ӻ�
// ����˵����
// l_line, r_line: Dir_Labyrinth_5����ı�������
// l_stastic, r_stastic: ���ұ��ߵ��������Dir_Labyrinth_5���ص�*l_stastic/*r_stastic��
void Draw_Border_Points(uint8(*l_line)[2], uint8(*r_line)[2], 
                        uint16 l_stastic, uint16 r_stastic)
{
    uint16 i;
    
    // 1. ���������ߣ���ɫ��
    for (i = 0; i < l_stastic; i++) 
    {
        uint16 x = l_line[i][0];
        uint16 y = l_line[i][1];
        
        // ���������ȷ������Ļ��Χ�ڣ�
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point(x*2, y*2,   RGB565_RED);					// ����ú�ɫ���
			tft180_draw_point(x*2, y*2-1 ,RGB565_RED);
        }
    }
    
    // 2. �����Ҳ���ߣ���ɫ��
    for (i = 0; i < r_stastic; i++) 
    {
        uint16 x = r_line[i][0];
        uint16 y = r_line[i][1];
        
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point(x*2, y*2, RGB565_BLUE);					
			tft180_draw_point(x*2,y*2-1,RGB565_BLUE); 			// �Ҳ�����ɫ���
        }
    }
	
	// 3. �����м���ߣ���ɫ��
    for (i = 1; i < Image_Y - 3; i++) 
    {
        uint16 x = C_Line[i] ;
        uint16 y = i;
        
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point( x*2, y*2   , RGB565_YELLOW);					
			tft180_draw_point( x*2, y*2-1 , RGB565_YELLOW); 			// �м��û�ɫ���
        }
    }
    
    // 4. ��ѡ����������㣨��ɫʮ�֣�- ���ڵ���
    if (l_stastic > 0 && r_stastic > 0)
    {
        uint16 meet_x = (r_line[r_stastic-1][0] + l_line[l_stastic-1][0]) >> 1;
        uint16 meet_y = (r_line[r_stastic-1][1] + l_line[l_stastic-1][1]) >> 1;
        
        // ��ʮ�ֱ��
        if (meet_x > 0 && meet_x < tft180_x_max - 1 && meet_y > 0 && meet_y < tft180_y_max - 1)
        {
            tft180_draw_point(meet_x*2 - 1, meet_y*2, RGB565_GREEN);
            tft180_draw_point(meet_x*2 + 1, meet_y*2, RGB565_GREEN);
            tft180_draw_point(meet_x*2, meet_y*2 - 1, RGB565_GREEN);
            tft180_draw_point(meet_x*2, meet_y*2 + 1, RGB565_GREEN);
            tft180_draw_point(meet_x*2, meet_y*2, RGB565_GREEN);
        }
    }
}

void Draw_Border_Points_1(uint8(*l_line)[2], uint8(*r_line)[2], uint8 *c_line ,
                        uint16 l_stastic, uint16 r_stastic)
{
    uint16 i;
    
    // 1. ���������ߣ���ɫ��
    for (i = 0; i < l_stastic; i++) 
    {
        uint16 x = l_line[i][0];
        uint16 y = l_line[i][1];
        
        // ���������ȷ������Ļ��Χ�ڣ�
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point(x, y,   RGB565_RED);					// ����ú�ɫ���
        }
    }
    
    // 2. �����Ҳ���ߣ���ɫ��
    for (i = 0; i < r_stastic; i++) 
    {
        uint16 x = r_line[i][0];
        uint16 y = r_line[i][1];
        
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point(x, y, RGB565_BLUE);					
			// �Ҳ�����ɫ���
        }
    }
	
	// 3. �����м���ߣ���ɫ��
    for (i = 1; i < Image_Y - 3; i++) 
    {
        uint16 x = c_line[i] ;
        uint16 y = i;
        
        if (x < tft180_x_max && y < tft180_y_max) 
        {
            tft180_draw_point( x , y , RGB565_YELLOW);					
			// �м��û�ɫ���
        }
    }
    
    // 4. ��ѡ����������㣨��ɫʮ�֣�- ���ڵ���
    if (l_stastic > 0 && r_stastic > 0)
    {
        uint16 meet_x = (r_line[r_stastic-1][0] + l_line[l_stastic-1][0]) >> 1;
        uint16 meet_y = (r_line[r_stastic-1][1] + l_line[l_stastic-1][1]) >> 1;

        // ��ʮ�ֱ��
        if (meet_x > 0 && meet_x < tft180_x_max - 1 && meet_y > 0 && meet_y < tft180_y_max - 1)
        {
            tft180_draw_point(meet_x - 1, meet_y, RGB565_GREEN);
            tft180_draw_point(meet_x + 1, meet_y, RGB565_GREEN);
            tft180_draw_point(meet_x, meet_y - 1, RGB565_GREEN);
            tft180_draw_point(meet_x, meet_y + 1, RGB565_GREEN);
            tft180_draw_point(meet_x, meet_y, RGB565_GREEN);
        }
    }

    // 5. ������ת��յ㣨��ɫʮ�֣�- L_Up_Turn_Left_Point_1
//    if(L_Up_Turn_Left_Point_Flag_1)
//    {
//        uint16 x = L_Up_Turn_Left_Point_1[0];
//        uint16 y = L_Up_Turn_Left_Point_1[1];
//        // ���������ȷ��ʮ�ֵ�5���㶼����Ļ��Χ�� (x*2, y*2, x*2-1, x*2+1, y*2-1, y*2+1)
//        if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//        {
//            tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//        }
//    }

//    // 6. ������ת�ҹյ㣨��ɫʮ�֣�- R_Up_Turn_Right_Point_1
//    if(R_Up_Turn_Right_Point_Flag_1)
//    {
//        uint16 x = R_Up_Turn_Right_Point_1[0];
//        uint16 y = R_Up_Turn_Right_Point_1[1];
//        if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//        {
//            tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//        }
//    }

    // 7. ������ת�Ϲյ㣨��ɫʮ�֣�- L_Right_Turn_Up_Point_1
//    if(L_Right_Turn_Up_Point_Flag_1)
//    {
//        uint16 x = L_Right_Turn_Up_Point_1[0];
//        uint16 y = L_Right_Turn_Up_Point_1[1];
//        if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//        {
//            tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//        }
//    }

    // 8. ������ת�Ϲյ㣨��ɫʮ�֣�- R_Left_Turn_Up_Point_1
//    if(R_Left_Turn_Up_Point_Flag_1)
//    {
//        uint16 x = R_Left_Turn_Up_Point_1[0];
//        uint16 y = R_Left_Turn_Up_Point_1[1];
//        if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//        {
//            tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//        }
//    }

    // 9. ������Բ���յ㣨��ɫʮ�֣�- L_Arc_Turn_Point
//    if(L_Arc_Turn_Point_Flag)
//    {
//        uint8 j;
//        for(j = 0; j < L_Arc_Turn_Point_Num && j < 3; j++)
//        {
//            uint16 x = L_Arc_Turn_Point[j][0];
//            uint16 y = L_Arc_Turn_Point[j][1];
//            if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//            {
//                tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//            }
//        }
//    }

    // 10. ������Բ���յ㣨��ɫʮ�֣�- R_Arc_Turn_Point
//    if(R_Arc_Turn_Point_Flag)
//    {
//        uint8 j;
//        for(j = 0; j < R_Arc_Turn_Point_Num && j < 3; j++)
//        {
//            uint16 x = R_Arc_Turn_Point[j][0];
//            uint16 y = R_Arc_Turn_Point[j][1];
//            if(x >= 1 && x <= X_Border_Max - 1 && y >= 1 && y <= Y_Border_Max - 1)
//            {
//               tft180_draw_point(x, y, RGB565_BROWN);
//            tft180_draw_point(x - 1, y, RGB565_BROWN);
//            tft180_draw_point(x + 1, y, RGB565_BROWN);
//            tft180_draw_point(x, y - 1, RGB565_BROWN);
//            tft180_draw_point(x, y + 1, RGB565_BROWN);
//            }
//        }
//    }
}

// ʹ��ʾ������Dir_Labyrinth_5���ú�ִ�У���
// Dir_Labyrinth_5(... , &l_stastic, &r_stastic, ...);  // ִ�б�����ȡ
// Draw_Border_Points(Adaptive_L_Line, Adaptive_R_Line, l_stastic, r_stastic);  // ������ʾ




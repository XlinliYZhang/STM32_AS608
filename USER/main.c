/**
 * ָ��ģ�� 8������, ֻ��Ҫ���� 
 * Vi  ģ���Դ�������  �� 3.3V
 * Tx  �����������      �� PD6
 * Rx  ������������      �� PD5
 * GND �źŵ�           �� GND
 * WAK ��Ӧ�ź����      �� PD4
 * Vt ������Ӧ��Դ����� �� 3.3V
 * 
 * ����1, ������115200
 * ¼��ָ������ݴ�����ʾ����
 * 
 * KEY 3 ����¼��ָ��(�Զ����ӱ��ʽ¼��)
 * KEY 4 ����ɾ��ȫ��ָ��
 * 
 * ʶ��ָ�Ƴɹ� LED 1  ��
 * ʶ��ָ��ʧ�� ������  ��
 * (��ΪAS608�����������Ƚ�����, ���Է�������LED��ֻ��ʶ��ָ��ʱ������Ӧ, ��ʱ�䴥��������Ӧ)
 */
#include "sys.h"
#include "led.h"
#include "lcd_init.h"
#include "lcd.h"
#include "delay.h"
#include "usart.h"
#include "key.h"
#include "beep.h"
#include "usart2.h"
#include "AS608.h"
SysPara AS608Para;			//ָ��ģ��AS608����
uint16_t ValidN;			//ģ������Чָ�Ƹ���
void Add_FR(int Finger_ID); //¼ָ��
void Del_FR(int Finger_ID); //ɾ��ָ��
int press_FR(void);			//ˢָ��
int main(void)
{
	uint8_t Press_Key = 0;
	int Delate_ID = 0;
	int Get_Data = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//����ϵͳ�ж����ȼ�����2
	delay_init();											//��ʼ����ʱ����
	LCD_Init();												//LCD��ʼ��
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					//����Ϊ����
	uart_init(115200);										//��ʼ������1������Ϊ115200
	usart2_init(57600);										//��ʼ������2������Ϊ57600
	PS_StaGPIO_Init();										//��ʼ��FR��״̬����
	BEEP_Init();											//��ʼ��������
	KEY_Init();												//������ʼ��
	LED_Init();												//��ʼ��LED
	delay_ms(150);											//��ʱ150ms�ȴ��ϵ����
	LCD_ShowChinese(0, 0, "���ڳ�ʼ��", RED, BLACK, 16, 0); //��ʾ���ڳ�ʼ��
	while (PS_HandShake(&AS608Addr))						//��AS608ģ������
	{
		delay_ms(400);
		LED0 = 1;
		LED1 = 0;
		printf("δ��⵽ģ��!!!\r\n");
		delay_ms(400);
		LED0 = 0;
		LED1 = 1;
		printf("��������ģ��...\r\n");
		Press_Key++;
		if (Press_Key >= 15)
		{
			LCD_ShowChinese(0, 16, "��ʼ��ʧ��", RED, BLACK, 16, 0); //��ʾ��ʼ��ʧ��
			LED0 = 0;												 //�����
			LED1 = 1;												 //�̵���
			while (1)												 //��ѭ��
				;
		}
	}
	LED0 = LED1 = 1;										   //Ϩ��ȫ��LED
	PS_ValidTempleteNum(&ValidN);							   //����ָ�Ƹ���
	LCD_ShowChinese(0, 16, "��ʼ���ɹ�", GREEN, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
	LCD_ShowChinese(0, 32, "ָ������", GREEN, BLACK, 16, 0);   //��ʾ��ʼ���ɹ�
	LCD_ShowString(64, 32, ":", GREEN, BLACK, 16, 0);		   //��ʾð��
	LCD_ShowIntNum(80, 32, ValidN, 3, GREEN, BLACK, 16);	   //��ʾָ������
	delay_ms(1000);											   //��ʱ1000ms
	delay_ms(1000);											   //��ʱ1000ms
	delay_ms(1000);											   //��ʱ1000ms
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //����Ϊ����,�ֿ������LCDҪ�ڴ˴�Ϩ�𱳹�
	PS_Pwr = 1;												   //�ر�ָ��ģ���Դ
	while (1)
	{
		Press_Key = KEY_Scan(0);	//ɨ�谴ť, ���ɳ���
		if (Press_Key == KEY2_PRES) //�����ť2����
		{
			ValidN = 0;
			Press_Key = 0;												 //��հ���ֵ
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);						 //����Ϊ����
			LCD_ShowChinese(0, 0, "��ʼ¼��ָ��", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			LCD_ShowChinese(0, 16, "������ָ��", YELLOW, BLACK, 16, 0);	 //��ʾ��ʼ���ɹ�
			LCD_ShowString(80, 16, "ID", YELLOW, BLACK, 16, 0);			 //��ʾð��
			LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32);		 //��ʾָ��ID
			while (Press_Key != KEY2_PRES)
			{
				Press_Key = KEY_Scan(0); //ɨ�谴ť, ���ɳ���
				if (Press_Key == KEY1_PRES)
				{
					if (ValidN > 0)
						ValidN -= 1;
					LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32); //��ʾָ��ID
				}
				else if (Press_Key == WKUP_PRES)
				{
					if (ValidN < 300)
						ValidN += 1;
					LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32); //��ʾָ��ID
				}
			}
			PS_Pwr = 0;							 //����ָ�Ƶ�Դ
			delay_ms(200);						 //�ȴ�ָ���ϵ�
			Add_FR(ValidN);						 //¼��ָ��
			PS_Pwr = 1;							 //�ر�ָ�Ƶ�Դ
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //����Ϊ����,�ֿ������LCDҪ�ڴ˴�Ϩ�𱳹�
		}
		else if (Press_Key == KEY0_PRES)
		{
			Delate_ID = 0;
			Press_Key = 0;												  //��հ���ֵ
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);						  //����Ϊ����
			LCD_ShowChinese(0, 0, "ѡ��Ҫɾ����ָ��", RED, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32);		  //��ʾָ��ID
			while (Press_Key != KEY0_PRES)
			{
				Press_Key = KEY_Scan(0); //ɨ�谴ť, ���ɳ���
				if (Press_Key == KEY1_PRES)
				{
					if (Delate_ID > 0)
					{
						Delate_ID -= 1;
						LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32); //��ʾָ��ID
					}
					else if (Delate_ID == 0)
					{
						Delate_ID = -1;
						LCD_ShowString(16, 16, "ALL", RED, BLACK, 32, 0); //��ʾָ��ID
					}
				}
				else if (Press_Key == WKUP_PRES)
				{
					if (Delate_ID < 300)
						Delate_ID += 1;
					LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32); //��ʾָ��ID
				}
			}
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);				//����Ϊ����
			LCD_ShowChinese(0, 0, "ɾ����", RED, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			PS_Pwr = 0;											//����ָ�Ƶ�Դ
			delay_ms(200);										//�ȴ�ָ���ϵ�
			Del_FR(Delate_ID);									//ɾ��ָ��ָ��
			delay_ms(1000);										//��ʱ1000ms
			PS_Pwr = 1;											//�ر�ָ�Ƶ�Դ
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);				//����Ϊ����,�ֿ������LCDҪ�ڴ˴�Ϩ�𱳹�
		}
		if (PS_Sta == 1) //�����⵽�ж�������
		{
			PS_Pwr = 0;												   //����ָ�Ƶ�Դ
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //����Ϊ����
			LCD_ShowChinese(0, 0, "ʶ�������Ժ�", BLUE, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			delay_ms(300);											   //��ʱ300ms
			Get_Data = press_FR();									   //��ȡָ��ID
			if (Get_Data >= 0)										   //���ʶ��ɹ�
			{
				LCD_ShowChinese(0, 16, "ʶ��ɹ�", GREEN, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
				LCD_ShowIntNum(16, 32, Get_Data, 3, GREEN, BLACK, 32);	 //��ʾָ��ID
				printf("ʶ��ɹ�, ID:%d\r\n", Get_Data);				 //��ӡ����
				LED0 = 1;												 //����LED0
				LED1 = 0;												 //Ϩ��LED0
			}
			else if (Get_Data == -2) //���ƥ��ʧ��(��ȡ��ָ��, ������δ¼���)
			{
				LCD_ShowChinese(0, 16, "ʶ��ʧ��", RED, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
				LED0 = 0;
				LED1 = 1;
				//BEEP = 1; //��������
			}
			else //���ƥ��ʧ��(��ȡ����ָ��)
			{
				LCD_ShowChinese(0, 16, "���ذ���ָ", BLUE, BLACK, 16, 0); //��ʾ��ʼ����
				LED0 = 0;
				LED1 = 1;
			}
			PS_Pwr = 1; //�ر�ָ�Ƶ�Դ
			delay_ms(1000);
			delay_ms(1000);
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //����Ϊ����
			LED0 = 1;
			LED1 = 1;
			BEEP = 0; //����������
		}
	}
}

//¼ָ��
void Add_FR(int Finger_ID)
{
	u8 i, ensure, processnum = 0;
	while (1)
	{
		switch (processnum)
		{
		case 0:
			i++;
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //����Ϊ����
			LCD_ShowChinese(0, 0, "�밴����ָ", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			while (!PS_Sta)
				;
			ensure = PS_GetImage();
			if (ensure == 0x00)
			{
				ensure = PS_GenChar(CharBuffer1); //��������
				if (ensure == 0x00)
				{
					LCD_ShowChinese(0, 16, "���ɿ���ָ", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
					while (PS_Sta)
						;
					i = 0;
					processnum = 1; //�����ڶ���
				}
				else
					printf("����ָ������ʧ��, ���ذ���ָ\r\n");
			}
			else
				printf("��ȡָ��ʧ��, �밴����ָ\r\n");
			break;

		case 1:
			i++;
			LCD_ShowChinese(0, 32, "���ٰ�����ָ", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
			ensure = PS_GetImage();
			if (ensure == 0x00)
			{
				ensure = PS_GenChar(CharBuffer2); //��������
				if (ensure == 0x00)
				{
					LCD_ShowChinese(0, 48, "���ɿ���ָ", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
					while (PS_Sta)
						;
					LCD_ShowChinese(0, 64, "���Ժ�", YELLOW, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
					i = 0;
					processnum = 2; //����������
				}
			}
			break;

		case 2:
			ensure = PS_Match();
			if (ensure == 0x00)
			{
				printf("��ȡָ�Ƴɹ�, ���Ժ�\r\n");
				processnum = 3; //�������Ĳ�
			}
			else
			{
				printf("��ȡָ��ʧ��, ������¼��ָ��\r\n");
				i = 0;
				processnum = 0; //���ص�һ��
			}
			delay_ms(1200);
			break;

		case 3:
			ensure = PS_RegModel();
			if (ensure == 0x00)
			{
				printf("����ָ�������ɹ�\r\n");
				processnum = 4; //�������岽
			}
			else
			{
				processnum = 0;
				printf("����ָ������ʧ��, ������¼��ָ��\r\n");
			}
			delay_ms(1200);
			break;

		case 4:
			printf("����ָ��ID: %d\r\n", Finger_ID);	   //���ڴ�ӡָ��ID
			ensure = PS_StoreChar(CharBuffer2, Finger_ID); //����ģ��
			if (ensure == 0x00)
			{
				LCD_ShowChinese(0, 80, "¼��ָ�Ƴɹ�", GREEN, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
				LCD_ShowChinese(0, 96, "ָ��", GREEN, BLACK, 16, 0);		 //��ʾ��ʼ���ɹ�
				LCD_ShowString(32, 96, "ID", GREEN, BLACK, 16, 0);			 //��ʾð��
				LCD_ShowIntNum(64, 96, Finger_ID, 3, GREEN, BLACK, 16);		 //��ʾָ��ID
				delay_ms(1000);
				delay_ms(1000);
				delay_ms(1000);
				return;
			}
			else
			{
				processnum = 0;
				printf("����ָ��ʧ��, ������¼��ָ��\r\n");
			}
			break;
		}
		delay_ms(400);
	}
}

//ˢָ��,�ɹ�����ID ���ɹ�����-1
int press_FR(void)
{
	SearchResult seach;
	uint8_t ensure;
	ensure = PS_GetImage();
	if (ensure == 0x00) //��ȡͼ��ɹ�
	{
		ensure = PS_GenChar(CharBuffer1);
		if (ensure == 0x00) //���������ɹ�
		{
			ensure = PS_Search(CharBuffer1, 0, 300, &seach);
			if (ensure == 0x00)		 //�����ɹ�
				return seach.pageID; //����ID
			else if (ensure == 9)
				return -2;
		}
		else
			printf("��������ʧ��: %d\r\n", ensure);
	}
	return -1;
}

//ɾ��ȫ��ָ��
void Del_FR(int Finger_ID)
{
	uint8_t ensure;
	if (Finger_ID == -1)
		ensure = PS_Empty(); //���ָ�ƿ�
	else
		ensure = PS_DeletChar(Finger_ID, 1); //ɾ������ָ��
	if (ensure == 0)
	{
		LCD_ShowChinese(0, 16, "ɾ���ɹ�", GREEN, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
	}
	else
	{
		LCD_ShowChinese(0, 16, "ɾ��ʧ��", GREEN, BLACK, 16, 0); //��ʾ��ʼ���ɹ�
	}
}

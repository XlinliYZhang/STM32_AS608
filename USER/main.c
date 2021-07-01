/**
 * 指纹模块 8个引脚, 只需要连接 
 * Vi  模块电源正输入端  到 3.3V
 * Tx  串行数据输出      到 PD6
 * Rx  串行数据输入      到 PD5
 * GND 信号地           到 GND
 * WAK 感应信号输出      到 PD4
 * Vt 触摸感应电源输入端 到 3.3V
 * 
 * 串口1, 波特率115200
 * 录入指纹请根据串口提示操作
 * 
 * KEY 3 按下录入指纹(自动增加编号式录入)
 * KEY 4 按下删除全部指纹
 * 
 * 识别指纹成功 LED 1  亮
 * 识别指纹失败 蜂鸣器  响
 * (因为AS608触摸传感器比较灵敏, 所以蜂鸣器和LED都只在识别到指纹时做出响应, 短时间触碰不会响应)
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
SysPara AS608Para;			//指纹模块AS608参数
uint16_t ValidN;			//模块内有效指纹个数
void Add_FR(int Finger_ID); //录指纹
void Del_FR(int Finger_ID); //删除指纹
int press_FR(void);			//刷指纹
int main(void)
{
	uint8_t Press_Key = 0;
	int Delate_ID = 0;
	int Get_Data = 0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//设置系统中断优先级分组2
	delay_init();											//初始化延时函数
	LCD_Init();												//LCD初始化
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					//设置为黑屏
	uart_init(115200);										//初始化串口1波特率为115200
	usart2_init(57600);										//初始化串口2波特率为57600
	PS_StaGPIO_Init();										//初始化FR读状态引脚
	BEEP_Init();											//初始化蜂鸣器
	KEY_Init();												//按键初始化
	LED_Init();												//初始化LED
	delay_ms(150);											//延时150ms等待上电完成
	LCD_ShowChinese(0, 0, "正在初始化", RED, BLACK, 16, 0); //显示正在初始化
	while (PS_HandShake(&AS608Addr))						//与AS608模块握手
	{
		delay_ms(400);
		LED0 = 1;
		LED1 = 0;
		printf("未检测到模块!!!\r\n");
		delay_ms(400);
		LED0 = 0;
		LED1 = 1;
		printf("尝试连接模块...\r\n");
		Press_Key++;
		if (Press_Key >= 15)
		{
			LCD_ShowChinese(0, 16, "初始化失败", RED, BLACK, 16, 0); //显示初始化失败
			LED0 = 0;												 //红灯亮
			LED1 = 1;												 //绿灯灭
			while (1)												 //死循环
				;
		}
	}
	LED0 = LED1 = 1;										   //熄灭全部LED
	PS_ValidTempleteNum(&ValidN);							   //读库指纹个数
	LCD_ShowChinese(0, 16, "初始化成功", GREEN, BLACK, 16, 0); //显示初始化成功
	LCD_ShowChinese(0, 32, "指纹数量", GREEN, BLACK, 16, 0);   //显示初始化成功
	LCD_ShowString(64, 32, ":", GREEN, BLACK, 16, 0);		   //显示冒号
	LCD_ShowIntNum(80, 32, ValidN, 3, GREEN, BLACK, 16);	   //显示指纹数量
	delay_ms(1000);											   //延时1000ms
	delay_ms(1000);											   //延时1000ms
	delay_ms(1000);											   //延时1000ms
	LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //设置为黑屏,分开背光的LCD要在此处熄灭背光
	PS_Pwr = 1;												   //关闭指纹模块电源
	while (1)
	{
		Press_Key = KEY_Scan(0);	//扫描按钮, 不可长按
		if (Press_Key == KEY2_PRES) //如果按钮2按下
		{
			ValidN = 0;
			Press_Key = 0;												 //清空按键值
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);						 //设置为白屏
			LCD_ShowChinese(0, 0, "开始录入指纹", YELLOW, BLACK, 16, 0); //显示初始化成功
			LCD_ShowChinese(0, 16, "请设置指纹", YELLOW, BLACK, 16, 0);	 //显示初始化成功
			LCD_ShowString(80, 16, "ID", YELLOW, BLACK, 16, 0);			 //显示冒号
			LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32);		 //显示指纹ID
			while (Press_Key != KEY2_PRES)
			{
				Press_Key = KEY_Scan(0); //扫描按钮, 不可长按
				if (Press_Key == KEY1_PRES)
				{
					if (ValidN > 0)
						ValidN -= 1;
					LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32); //显示指纹ID
				}
				else if (Press_Key == WKUP_PRES)
				{
					if (ValidN < 300)
						ValidN += 1;
					LCD_ShowIntNum(16, 32, ValidN, 3, YELLOW, BLACK, 32); //显示指纹ID
				}
			}
			PS_Pwr = 0;							 //开启指纹电源
			delay_ms(200);						 //等待指纹上电
			Add_FR(ValidN);						 //录入指纹
			PS_Pwr = 1;							 //关闭指纹电源
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //设置为黑屏,分开背光的LCD要在此处熄灭背光
		}
		else if (Press_Key == KEY0_PRES)
		{
			Delate_ID = 0;
			Press_Key = 0;												  //清空按键值
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);						  //设置为白屏
			LCD_ShowChinese(0, 0, "选择要删除的指纹", RED, BLACK, 16, 0); //显示初始化成功
			LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32);		  //显示指纹ID
			while (Press_Key != KEY0_PRES)
			{
				Press_Key = KEY_Scan(0); //扫描按钮, 不可长按
				if (Press_Key == KEY1_PRES)
				{
					if (Delate_ID > 0)
					{
						Delate_ID -= 1;
						LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32); //显示指纹ID
					}
					else if (Delate_ID == 0)
					{
						Delate_ID = -1;
						LCD_ShowString(16, 16, "ALL", RED, BLACK, 32, 0); //显示指纹ID
					}
				}
				else if (Press_Key == WKUP_PRES)
				{
					if (Delate_ID < 300)
						Delate_ID += 1;
					LCD_ShowIntNum(16, 16, Delate_ID, 3, RED, BLACK, 32); //显示指纹ID
				}
			}
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);				//设置为白屏
			LCD_ShowChinese(0, 0, "删除中", RED, BLACK, 16, 0); //显示初始化成功
			PS_Pwr = 0;											//开启指纹电源
			delay_ms(200);										//等待指纹上电
			Del_FR(Delate_ID);									//删除指定指纹
			delay_ms(1000);										//延时1000ms
			PS_Pwr = 1;											//关闭指纹电源
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);				//设置为黑屏,分开背光的LCD要在此处熄灭背光
		}
		if (PS_Sta == 1) //如果检测到有东西触摸
		{
			PS_Pwr = 0;												   //开启指纹电源
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //设置为黑屏
			LCD_ShowChinese(0, 0, "识别中请稍后", BLUE, BLACK, 16, 0); //显示初始化成功
			delay_ms(300);											   //延时300ms
			Get_Data = press_FR();									   //获取指纹ID
			if (Get_Data >= 0)										   //如果识别成功
			{
				LCD_ShowChinese(0, 16, "识别成功", GREEN, BLACK, 16, 0); //显示初始化成功
				LCD_ShowIntNum(16, 32, Get_Data, 3, GREEN, BLACK, 32);	 //显示指纹ID
				printf("识别成功, ID:%d\r\n", Get_Data);				 //打印参数
				LED0 = 1;												 //点亮LED0
				LED1 = 0;												 //熄灭LED0
			}
			else if (Get_Data == -2) //如果匹配失败(获取到指纹, 但是是未录入的)
			{
				LCD_ShowChinese(0, 16, "识别失败", RED, BLACK, 16, 0); //显示初始化成功
				LED0 = 0;
				LED1 = 1;
				//BEEP = 1; //蜂鸣器响
			}
			else //如果匹配失败(获取不到指纹)
			{
				LCD_ShowChinese(0, 16, "请重按手指", BLUE, BLACK, 16, 0); //显示初始化成
				LED0 = 0;
				LED1 = 1;
			}
			PS_Pwr = 1; //关闭指纹电源
			delay_ms(1000);
			delay_ms(1000);
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK); //设置为黑屏
			LED0 = 1;
			LED1 = 1;
			BEEP = 0; //蜂鸣器不响
		}
	}
}

//录指纹
void Add_FR(int Finger_ID)
{
	u8 i, ensure, processnum = 0;
	while (1)
	{
		switch (processnum)
		{
		case 0:
			i++;
			LCD_Fill(0, 0, LCD_W, LCD_H, BLACK);					   //设置为白屏
			LCD_ShowChinese(0, 0, "请按下手指", YELLOW, BLACK, 16, 0); //显示初始化成功
			while (!PS_Sta)
				;
			ensure = PS_GetImage();
			if (ensure == 0x00)
			{
				ensure = PS_GenChar(CharBuffer1); //生成特征
				if (ensure == 0x00)
				{
					LCD_ShowChinese(0, 16, "请松开手指", YELLOW, BLACK, 16, 0); //显示初始化成功
					while (PS_Sta)
						;
					i = 0;
					processnum = 1; //跳到第二步
				}
				else
					printf("生成指纹特征失败, 请重按手指\r\n");
			}
			else
				printf("获取指纹失败, 请按下手指\r\n");
			break;

		case 1:
			i++;
			LCD_ShowChinese(0, 32, "请再按下手指", YELLOW, BLACK, 16, 0); //显示初始化成功
			ensure = PS_GetImage();
			if (ensure == 0x00)
			{
				ensure = PS_GenChar(CharBuffer2); //生成特征
				if (ensure == 0x00)
				{
					LCD_ShowChinese(0, 48, "请松开手指", YELLOW, BLACK, 16, 0); //显示初始化成功
					while (PS_Sta)
						;
					LCD_ShowChinese(0, 64, "请稍候", YELLOW, BLACK, 16, 0); //显示初始化成功
					i = 0;
					processnum = 2; //跳到第三步
				}
			}
			break;

		case 2:
			ensure = PS_Match();
			if (ensure == 0x00)
			{
				printf("获取指纹成功, 请稍后\r\n");
				processnum = 3; //跳到第四步
			}
			else
			{
				printf("获取指纹失败, 请重新录入指纹\r\n");
				i = 0;
				processnum = 0; //跳回第一步
			}
			delay_ms(1200);
			break;

		case 3:
			ensure = PS_RegModel();
			if (ensure == 0x00)
			{
				printf("生成指纹特征成功\r\n");
				processnum = 4; //跳到第五步
			}
			else
			{
				processnum = 0;
				printf("生成指纹特征失败, 请重新录入指纹\r\n");
			}
			delay_ms(1200);
			break;

		case 4:
			printf("储存指纹ID: %d\r\n", Finger_ID);	   //串口打印指纹ID
			ensure = PS_StoreChar(CharBuffer2, Finger_ID); //储存模板
			if (ensure == 0x00)
			{
				LCD_ShowChinese(0, 80, "录入指纹成功", GREEN, BLACK, 16, 0); //显示初始化成功
				LCD_ShowChinese(0, 96, "指纹", GREEN, BLACK, 16, 0);		 //显示初始化成功
				LCD_ShowString(32, 96, "ID", GREEN, BLACK, 16, 0);			 //显示冒号
				LCD_ShowIntNum(64, 96, Finger_ID, 3, GREEN, BLACK, 16);		 //显示指纹ID
				delay_ms(1000);
				delay_ms(1000);
				delay_ms(1000);
				return;
			}
			else
			{
				processnum = 0;
				printf("储存指纹失败, 请重新录入指纹\r\n");
			}
			break;
		}
		delay_ms(400);
	}
}

//刷指纹,成功返回ID 不成功返回-1
int press_FR(void)
{
	SearchResult seach;
	uint8_t ensure;
	ensure = PS_GetImage();
	if (ensure == 0x00) //获取图像成功
	{
		ensure = PS_GenChar(CharBuffer1);
		if (ensure == 0x00) //生成特征成功
		{
			ensure = PS_Search(CharBuffer1, 0, 300, &seach);
			if (ensure == 0x00)		 //搜索成功
				return seach.pageID; //返回ID
			else if (ensure == 9)
				return -2;
		}
		else
			printf("生成特征失败: %d\r\n", ensure);
	}
	return -1;
}

//删除全部指纹
void Del_FR(int Finger_ID)
{
	uint8_t ensure;
	if (Finger_ID == -1)
		ensure = PS_Empty(); //清空指纹库
	else
		ensure = PS_DeletChar(Finger_ID, 1); //删除单个指纹
	if (ensure == 0)
	{
		LCD_ShowChinese(0, 16, "删除成功", GREEN, BLACK, 16, 0); //显示初始化成功
	}
	else
	{
		LCD_ShowChinese(0, 16, "删除失败", GREEN, BLACK, 16, 0); //显示初始化成功
	}
}

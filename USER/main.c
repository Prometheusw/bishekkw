#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "usmart.h"
#include "malloc.h"
#include "sdio_sdcard.h"
#include "w25qxx.h"
#include "ff.h"
#include "exfuns.h"
#include "text.h"
#include "piclib.h"
#include "string.h"
#include "math.h"
#include "ov7670.h"
#include "beep.h"
#include "timer.h"
#include "exti.h"
extern u8 ov_sta;	//在exit.c里面定义
extern u8 ov_frame;	//在timer.c里面定义
/*****************************************************************
//函数名称：LCD更新显示
//函数作用：刷新LCD屏幕，显示下一帧数据
//作者:wk
//返回值：无
******************************************************************/
void camera_refresh(void)
{
    u32 j;
    u16 color;
    if(ov_sta)//有帧中断更新？
    {
        LCD_Scan_Dir(U2D_L2R);		//从上到下,从左到右
/***当显示屏不同时，选择不同的设置**/
        if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//将显示区域设置到屏幕中央
        else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//将显示区域设置到屏幕中央
        LCD_WriteRAM_Prepare();     //开始写入GRAM
        OV7670_RRST=0;				//开始复位读指针
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST=1;				//复位读指针结束
        OV7670_RCK_H;
        for(j=0; j<76800; j++)
        {
            OV7670_RCK_L;
            color=GPIOC->IDR&0XFF;	//读数据，7670返回的颜色值是RGB565格式的
            OV7670_RCK_H;
            color<<=8;
            OV7670_RCK_L;
            color|=GPIOC->IDR&0XFF;	//读数据
            OV7670_RCK_H;
            LCD->LCD_RAM=color;

        }
        ov_sta=0;					//清零帧中断标记
        ov_frame++;
        LCD_Scan_Dir(DFT_SCAN_DIR);	//恢复默认扫描方向
    }
}
//文件名自增（避免覆盖）
//组合成:形如"0:PHOTO/PIC13141.bmp"的文件名
void camera_new_pathname(u8 *pname)
{
    u8 res;
    u16 index=0;
    while(index<0XFFFF)
    {
        sprintf((char*)pname,"0:PHOTO/PIC%05d.bmp",index);
        res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
        if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
        index++;
    }
}

/*****************************************************************
//函数名称：串口发送ESP8266命令，后接/r/n
//函数作用：串口发送一组数据
//作者:wk
//返回值：
******************************************************************/
int My_USARTSend(char *p)

   {
   	while (*p != '\0') {
   		USART_SendData(USART1, *p);
   		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
   			;
   		p++;
   	}
   	USART_SendData(USART1, '\r');
   	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
   		;
   	USART_SendData(USART1, '\n');
   	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
   		;
   	return 0;

   }
/*****************************************************************
//函数名称：串口发送数据
//函数作用：透传模式开启后通过串口发送数据到ESP8266
//作者:wk
//返回值：0
******************************************************************/
int My_USARTSendstop(char *p)

      {
      	while (*p != '\0') {
      		USART_SendData(USART1, *p);
      		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
      			;
      		p++;
      	}

      	return 0;

      }


/*****************************************************************
//函数名称：初始化ESP8266及透传配置
//函数作用：模式配置、服务器链接
//作者:wk
//返回值：0
******************************************************************/
int ESP8266_Init()
{
	My_USARTSendstop("+++");
	delay_ms(1200);
	My_USARTSendstop("+++");

	delay_ms(1000);
//My_USARTSend("AT+UART=921600,8,1,0,3");

		My_USARTSend("AT+CWMODE=1");
    Show_Str(30,600,400,24,"Set WIFIMODE……",24,1);
		delay_ms(1000);
		delay_ms(1000);

		//My_USARTSend("AT+RST");
		//delay_s(4);

		My_USARTSend("AT+CWJAP=\"ffb\",\"12345678\"");
    Show_Str(30,630,400,24,"Connet to WIFI",24,1);

		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);
		delay_ms(1000);

		//My_USARTSend("AT+RST");
		//delay_s(3);
		My_USARTSend("AT+CIPMUX=0");
		delay_ms(1000);

		My_USARTSend("AT+CIPMODE=1");
		delay_ms(1000);

		My_USARTSend("AT+CIPSTART=\"TCP\",\"192.168.137.1\",60000");
    Show_Str(30,660,400,24,"Connet to 192.168.137.1:60000",24,1);
		delay_ms(1000);
		delay_ms(1000);

		delay_ms(1000);
        My_USARTSend("AT+CIPSEND");
    Show_Str(30,690,400,24,"OSPF ON",24,1);
		delay_ms(1000);

            LCD_Fill(30,600,440,200,BLACK);


}

int main(void)
{
    u8 res;
    u8 *pname;				//带路径的文件名
    u8 key;					//键值
    u8 i;
    u8 sd_ok=1;				//0,sd卡不正常;1,SD卡正常.
////////////
	//u8 hig_send=0,low_send=0;
	u16 send_x;
	u16 send_y;
	u16 p=0,j=0;
	u16 point=0;
/////////////
    delay_init();	    	 //延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    uart_init(921600);	
		My_USARTSend("AT+RST");


 	//串口初始化为115200
    LED_Init();		  			//初始化与LED连接的硬件接口
    KEY_Init();					//初始化按键
    LCD_Init();	
	LCD_Clear(BLACK);		   		//初始化LCD
    BEEP_Init();        		//蜂鸣器初始化
    my_mem_init(SRAMIN);		//初始化内部内存池
    exfuns_init();				//为fatfs相关变量申请内存
    f_mount(fs[0],"0:",1); 		//挂载SD卡
    f_mount(fs[1],"1:",1); 		//挂载FLASH.
    POINT_COLOR=GREEN;
    while(font_init()) 				//检查字库
    {
        LCD_ShowString(30,50,200,16,16,"Font Error!");
        delay_ms(200);
        LCD_Fill(30,50,240,66,WHITE);//清除显示
    }


/*****************************************************************
//函数名称：无线网配置
//函数作用：给ESP发送命令配置
//作者:wk
//返回值：
******************************************************************/
    ESP8266_Init();


    Show_Str(30,20,400,24,"Terminal based on STM32",24,1);
    Show_Str(30,45,400,24,"Designer：WANGKAI",24,1);
    Show_Str(30,90,400,24,"KEY0 SEND",24,1);
    Show_Str(30,110,200,24,"hello world",24,1);
    Show_Str(30,130,200,24,"***********",24,1);
    res=f_mkdir("0:/PHOTO");		//创建PHOTO文件夹
    if(res!=FR_EXIST&&res!=FR_OK) 	//发生了错误
    {
        Show_Str(30,150,240,16,"SD卡错误!",16,1);
        delay_ms(200);
        Show_Str(30,170,240,16,"拍照功能将不可用!",16,1);
        sd_ok=0;
    } else
    {
        Show_Str(30,150,240,16,"SD卡正常!",16,1);
        delay_ms(200);
        Show_Str(30,170,240,16,"KEY_UP:拍照",16,1);
        sd_ok=1;
    }
    pname=mymalloc(SRAMIN,30);	//为带路径的文件名分配30个字节的内存
    while(pname==NULL)			//内存分配出错
    {
        Show_Str(30,190,240,16,"内存分配失败!",16,0);
        delay_ms(200);
        LCD_Fill(30,190,240,146,WHITE);//清除显示
        delay_ms(200);
    }
    while(OV7670_Init())//初始化OV7670
    {
        Show_Str(30,190,240,16,"OV7670 ERRO!",16,0);
        delay_ms(200);
        LCD_Fill(30,190,239,206,WHITE);
        delay_ms(200);
    }
    Show_Str(30,190,200,16,"OV7670 OK",16,1);
    delay_ms(1500);
    EXTI8_Init();						//使能定时器捕获
    OV7670_Window_Set(12,176,240,320);	//设置窗口
    OV7670_CS=0;
    //LCD_Clear(BLACK);
    while(1)
    {
        key=KEY_Scan(0);//不支持连按
		if(key==KEY1_PRES)
		{
			 send_y=(lcddev.height-320)/2+100;
			 send_x=(lcddev.width-240)/2;
			for(j=0;j<120;j++)
			{
				for(p=0;p<240;p++)
				{
					point=LCD_ReadPoint(send_x,send_y);
                     delay_us(10);
					USART_SendData(USART1,point>>8);  

      		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
                   delay_us(10);

					USART_SendData(USART1,point&0x00ff);

      		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
                     delay_us(10);

					send_x++;

				}
				send_y++;
				send_x=(lcddev.width-240)/2;
                                     delay_us(10);

			}
					
            //USART_SendData(USART1,'\0');
                     delay_us(10);
            //USART_SendData(USART1,'\0');
                     delay_us(10);
//      		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
// USART_SendData(USART1,0x0d);
//                     delay_us(10);

//      		while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET);
		}
        if(key==KEY0_PRES)
        {
            if(sd_ok)
            {
                LED1=0;	//点亮DS1,提示正在拍照
                camera_new_pathname(pname);//得到文件名
                if(bmp_encode(pname,(lcddev.width-240)/2,(lcddev.height-320)/2,240,320,0))//拍照有误
                {
                    Show_Str(40,130,240,12,"写入文件错误!",12,0);
                } else
                {
                    Show_Str(200,130,240,12,"拍照成功!",12,1);
                    Show_Str(200,150,240,12,"保存为:",12,1);
                    Show_Str(200+42,150,240,12,pname,12,1);
                    BEEP=1;	//蜂鸣器短叫，提示拍照完成
                    delay_ms(100);
                }
            } else //提示SD卡错误
            {
                Show_Str(40,130,240,12,"SD卡错误!",12,0);
                Show_Str(40,150,240,12,"拍照功能不可用!",12,0);
            }
            BEEP=0;//关闭蜂鸣器
            LED1=1;//关闭DS1
            delay_ms(1800);//等待1.8秒钟
            LCD_Fill(200,130,440,200,BLACK);
        } else delay_ms(5);
        camera_refresh();//更新显示
        i++;
        if(i==40)//DS0闪烁.
        {
            i=0;
            LED0=!LED0;
        }
    }
}














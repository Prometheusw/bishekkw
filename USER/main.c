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
extern u8 ov_sta;	//��exit.c���涨��
extern u8 ov_frame;	//��timer.c���涨��
/*****************************************************************
//�������ƣ�LCD������ʾ
//�������ã�ˢ��LCD��Ļ����ʾ��һ֡����
//����:wk
//����ֵ����
******************************************************************/
void camera_refresh(void)
{
    u32 j;
    u16 color;
    if(ov_sta)//��֡�жϸ��£�
    {
        LCD_Scan_Dir(U2D_L2R);		//���ϵ���,������
/***����ʾ����ͬʱ��ѡ��ͬ������**/
        if(lcddev.id==0X1963)LCD_Set_Window((lcddev.width-240)/2,(lcddev.height-320)/2,240,320);//����ʾ�������õ���Ļ����
        else if(lcddev.id==0X5510||lcddev.id==0X5310)LCD_Set_Window((lcddev.width-320)/2,(lcddev.height-240)/2,320,240);//����ʾ�������õ���Ļ����
        LCD_WriteRAM_Prepare();     //��ʼд��GRAM
        OV7670_RRST=0;				//��ʼ��λ��ָ��
        OV7670_RCK_L;
        OV7670_RCK_H;
        OV7670_RCK_L;
        OV7670_RRST=1;				//��λ��ָ�����
        OV7670_RCK_H;
        for(j=0; j<76800; j++)
        {
            OV7670_RCK_L;
            color=GPIOC->IDR&0XFF;	//�����ݣ�7670���ص���ɫֵ��RGB565��ʽ��
            OV7670_RCK_H;
            color<<=8;
            OV7670_RCK_L;
            color|=GPIOC->IDR&0XFF;	//������
            OV7670_RCK_H;
            LCD->LCD_RAM=color;

        }
        ov_sta=0;					//����֡�жϱ��
        ov_frame++;
        LCD_Scan_Dir(DFT_SCAN_DIR);	//�ָ�Ĭ��ɨ�跽��
    }
}
//�ļ������������⸲�ǣ�
//��ϳ�:����"0:PHOTO/PIC13141.bmp"���ļ���
void camera_new_pathname(u8 *pname)
{
    u8 res;
    u16 index=0;
    while(index<0XFFFF)
    {
        sprintf((char*)pname,"0:PHOTO/PIC%05d.bmp",index);
        res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
        if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
        index++;
    }
}

/*****************************************************************
//�������ƣ����ڷ���ESP8266������/r/n
//�������ã����ڷ���һ������
//����:wk
//����ֵ��
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
//�������ƣ����ڷ�������
//�������ã�͸��ģʽ������ͨ�����ڷ������ݵ�ESP8266
//����:wk
//����ֵ��0
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
//�������ƣ���ʼ��ESP8266��͸������
//�������ã�ģʽ���á�����������
//����:wk
//����ֵ��0
******************************************************************/
int ESP8266_Init()
{
	My_USARTSendstop("+++");
	delay_ms(1200);
	My_USARTSendstop("+++");

	delay_ms(1000);
//My_USARTSend("AT+UART=921600,8,1,0,3");

		My_USARTSend("AT+CWMODE=1");
    Show_Str(30,600,400,24,"Set WIFIMODE����",24,1);
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
    u8 *pname;				//��·�����ļ���
    u8 key;					//��ֵ
    u8 i;
    u8 sd_ok=1;				//0,sd��������;1,SD������.
////////////
	//u8 hig_send=0,low_send=0;
	u16 send_x;
	u16 send_y;
	u16 p=0,j=0;
	u16 point=0;
/////////////
    delay_init();	    	 //��ʱ������ʼ��
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(921600);	
		My_USARTSend("AT+RST");


 	//���ڳ�ʼ��Ϊ115200
    LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
    KEY_Init();					//��ʼ������
    LCD_Init();	
	LCD_Clear(BLACK);		   		//��ʼ��LCD
    BEEP_Init();        		//��������ʼ��
    my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
    exfuns_init();				//Ϊfatfs��ر��������ڴ�
    f_mount(fs[0],"0:",1); 		//����SD��
    f_mount(fs[1],"1:",1); 		//����FLASH.
    POINT_COLOR=GREEN;
    while(font_init()) 				//����ֿ�
    {
        LCD_ShowString(30,50,200,16,16,"Font Error!");
        delay_ms(200);
        LCD_Fill(30,50,240,66,WHITE);//�����ʾ
    }


/*****************************************************************
//�������ƣ�����������
//�������ã���ESP������������
//����:wk
//����ֵ��
******************************************************************/
    ESP8266_Init();


    Show_Str(30,20,400,24,"Terminal based on STM32",24,1);
    Show_Str(30,45,400,24,"Designer��WANGKAI",24,1);
    Show_Str(30,90,400,24,"KEY0 SEND",24,1);
    Show_Str(30,110,200,24,"hello world",24,1);
    Show_Str(30,130,200,24,"***********",24,1);
    res=f_mkdir("0:/PHOTO");		//����PHOTO�ļ���
    if(res!=FR_EXIST&&res!=FR_OK) 	//�����˴���
    {
        Show_Str(30,150,240,16,"SD������!",16,1);
        delay_ms(200);
        Show_Str(30,170,240,16,"���չ��ܽ�������!",16,1);
        sd_ok=0;
    } else
    {
        Show_Str(30,150,240,16,"SD������!",16,1);
        delay_ms(200);
        Show_Str(30,170,240,16,"KEY_UP:����",16,1);
        sd_ok=1;
    }
    pname=mymalloc(SRAMIN,30);	//Ϊ��·�����ļ�������30���ֽڵ��ڴ�
    while(pname==NULL)			//�ڴ�������
    {
        Show_Str(30,190,240,16,"�ڴ����ʧ��!",16,0);
        delay_ms(200);
        LCD_Fill(30,190,240,146,WHITE);//�����ʾ
        delay_ms(200);
    }
    while(OV7670_Init())//��ʼ��OV7670
    {
        Show_Str(30,190,240,16,"OV7670 ERRO!",16,0);
        delay_ms(200);
        LCD_Fill(30,190,239,206,WHITE);
        delay_ms(200);
    }
    Show_Str(30,190,200,16,"OV7670 OK",16,1);
    delay_ms(1500);
    EXTI8_Init();						//ʹ�ܶ�ʱ������
    OV7670_Window_Set(12,176,240,320);	//���ô���
    OV7670_CS=0;
    //LCD_Clear(BLACK);
    while(1)
    {
        key=KEY_Scan(0);//��֧������
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
                LED1=0;	//����DS1,��ʾ��������
                camera_new_pathname(pname);//�õ��ļ���
                if(bmp_encode(pname,(lcddev.width-240)/2,(lcddev.height-320)/2,240,320,0))//��������
                {
                    Show_Str(40,130,240,12,"д���ļ�����!",12,0);
                } else
                {
                    Show_Str(200,130,240,12,"���ճɹ�!",12,1);
                    Show_Str(200,150,240,12,"����Ϊ:",12,1);
                    Show_Str(200+42,150,240,12,pname,12,1);
                    BEEP=1;	//�������̽У���ʾ�������
                    delay_ms(100);
                }
            } else //��ʾSD������
            {
                Show_Str(40,130,240,12,"SD������!",12,0);
                Show_Str(40,150,240,12,"���չ��ܲ�����!",12,0);
            }
            BEEP=0;//�رշ�����
            LED1=1;//�ر�DS1
            delay_ms(1800);//�ȴ�1.8����
            LCD_Fill(200,130,440,200,BLACK);
        } else delay_ms(5);
        camera_refresh();//������ʾ
        i++;
        if(i==40)//DS0��˸.
        {
            i=0;
            LED0=!LED0;
        }
    }
}














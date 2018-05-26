#ifndef __18b20_H__
#define __18b20_H__

#include "stm32f10x_gpio.h"

#define GPIO_PORT_DS18B20 GPIOC
#define GPIO_PIN_DS18B20	GPIO_Pin_6
#define GPIO_CLK_DS18B20	RCC_APB2Periph_GPIOC




unsigned char init_1820(void);
//unsigned char init_1820(unsigned char us);
void Get_id(void);
unsigned char read_1820(void) ;
void write_1820(unsigned char data) ;
void gettemp(void) ;
void get1820temper(void);
unsigned char GetDQ(void);

void Gettemp_Start(void);  
void Gettemp_OK(void);  

extern int a[9];
extern u8 Tem_int;
extern u16 Tem_dec;


#endif 


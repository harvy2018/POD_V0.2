#include "SerialPort.h"
#include "stdio.h"
#include "PLCMessage.h"

int main()
{
    bool res = false;
    res = InitPort(6);
	if (!res)
	{
		printf("串口打开失败");
		return 0;
	}

	res = SendGetSystemInfoRequest();
	if (!res)
	{
		printf("%s", g_errInfo);
	}

	char buf[500] = {0};

    while(true)
	{
		if (g_init)
		{
			for (int i=1; i<=100; i++)
			{
				Sleep(1000);
				memcpy(buf, (char*)"wangwei", 7);
				char c[10] = {0}; 
				itoa(i,c,10);
				memcpy(buf+7, c, strlen(c));
				//memset(buf, 0, 500);
				//printf("input the data\n");
				//scanf("%s", buf);
				SendData((unsigned char*)buf, strlen(buf));
				//Sleep(1000);
			}
			Sleep(2000);
			break;
		}
	} 
 	res = ClosePort();
 	if (!res)
 	{
 		printf("串口关闭失败");
 		return 0;
 	}
    getchar();
	return 0;
}
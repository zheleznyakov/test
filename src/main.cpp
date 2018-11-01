/*
* Автор - Железняков Андрей
* Сайт - itworkclub.ru
* 
*/
#include "mbed.h"
#include "rtos.h"

#include "PowerControl.h"
#include "max6675.h"
#include "pid.h"
#include <string>

//using namespace std;
char readData[20];
char buf[20];
DigitalOut led(LED1);
Serial s(PA_2,PA_3);//tx,rx связь с компьютером по uart
Serial s2(PB_6,PA_10);// tx, rx связь с экраном nextion по uart
Thread th1, downHeater;

PowerControl P(D4,D11,D12,D13,D14,D15);
SPI spi(PB_15, PB_14, PB_13); // MOSI, MISO, SCLK
max6675 max_sensor(spi,PB_1); // SPI, CS - chip select
pid reg(max_sensor, 10,0,0);

void DownHeat()
{
    int p;
    while(1)
    {
        p = reg.Power();
        P.SetDimming(p, p,1,1,1);
        Thread::wait(500);
    }

}

void ReadCommands()
{
    char a[30];
    string str,command;
    int data = 0;
    str.clear();
    int c = 0;

    while(1) {
        if (s2.readable()) {
            s2.scanf("%s",a);
            c++;

            str.append(a);
        }
        if (str.length()>0)
        {
            int pos =str.find("=");
            if (pos!=-1)
            {
                command = str.substr(0,pos);
                data = a[pos+2];
                data = data<<8;
                data = data|a[pos+1];
                if (command=="setdown")
                {
                    reg.SetTemperature(data);
                }
                s.printf("%s, data=%d", command.c_str(),data);
                str.clear();
            }
        }

    }
    
}


int main()
{

    s.baud(115200);
    s2.baud(115200);
    //SetDimming(нагр1, нагр2, нагр3, нагр4, верх)
    //0-мин мощность 249-максимальная при 250 симистор не удерживается открытым
    P.SetDimming(10,1,1,1,1); 

    reg.SetTemperature(90);
    
    
    th1.start(ReadCommands);
    downHeater.start(DownHeat);

    int temp;
    while(1) {
        Thread::wait(500);
        //wait(1);
        temp = reg.ReadTemp();
        
        s2.printf("tempn.val=%d%c%c%c",temp,255,255,255);
    }
}
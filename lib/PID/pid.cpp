/*
* Автор - Железняков Андрей
* Сайт - itworkclub.ru
* Класс pid представляет реализацию ПИД регулятора для нагревателя
* Класс содержит таймер, который постоянно вычисляет требуемую мощность нагревателя
* исходя из текущей и требуемой температуры.
*/

#include "pid.h"

/* pid::pid(max6675 &m,int kp_) 
*  в конструктор передаем ссылку m на темопару max6675, ссылку pc на фазовый регулятор и коэффиценты регулятора kp_, kd_, ki_
*  flag - флаг SPI 1- занят; 0 - свободен
*/
pid::pid(max6675 &m, PowerControl &pc,double kp_, int kd_, double ki_, int *flag):max(m), pcontrol(pc)
{

    kp = kp_;
    kd = kd_;
    ki = ki_;
    previousError = 0;
    integral =0;
    requered_temp=30; //заданная температура по умолчанию
    power = 0; 
    spiFlag = flag;
    
    // tim2- таймер, который считывает температуру и вычисляет мощность по алгоритму ПИД регулятора
    tim2= new RtosTimer(Compute, this);
    tim2->start(5000);
}
float pid::ReadTemp()
{
    return current_temp;
}
void pid::SetTemperature(float t_)
{
    int dop=0; 
    if (t_>80) dop = 15;
    if (t_>110) dop=25;
    if (t_>140) dop+=20;
    if (t_>160) dop+=20;
    if (t_>180) dop+=20;
    if (t_>200) dop+=20;
    requered_temp = t_;
    integral = 0;
    if (t_>current_temp)
    {
        if (t_-current_temp<=15)
        {setMaxPower(10+dop);Compute(this);return;}
        if (t_-current_temp<=25)
        {setMaxPower(25+dop);Compute(this);return;}
        if (t_-current_temp<=50)
        {setMaxPower(60+dop);Compute(this);return;}
        if (t_-current_temp<=250)
        {setMaxPower(100+dop);Compute(this);return;}
        

    }
    Compute(this);
    
}
void pid::Compute(void const *arguments)
{
    pid *self = (pid*)arguments;
    double error,x;

    self->current_temp = self->temp();
      
    error = self->requered_temp-self->current_temp;
    //if (error<0) self->integral=0;
    x = (error - self->previousError)*self->kd;  
    self->previousError = error;
    x+=error*self->kp;
    self->integral+=self->ki*(double)error;
    x+=self->integral;
    if (x>0){
        if (x<=self->maxPower) self->power = x;;
        if (x>self->maxPower) self->power = self->maxPower;
    }
    else{
        self->power = 0;
    }
    for (int i=0;i<5;i++)
    {
        //self->pcontrol.SetDimming(self->power,self->power,self->power,self->power,1);
        if(self->heaters[i])
            self->pcontrol.SetDimming(i,self->power);
    }
}

int pid::Power()
{
    return power;
}
float pid::temp()
{
    while (*spiFlag){wait_ms(10);}
    *spiFlag = 1;
    current_temp = max.read_temp();
    *spiFlag = 0;
    return current_temp;
    
}
void pid::setMaxPower(int x)
{
    if (x>=0&&x<=249)
    {
        maxPower=x;
        return;
    }
    if (x<0) maxPower = 0;

}
//selctHeaters выбор нагревателей заполняет массив heaters[5]
void pid::selectHeaters(bool h1,bool h2,bool h3,bool h4,bool h5)
{
    heaters[0]=h1;
    heaters[1]=h2;
    heaters[2]=h3;
    heaters[3]=h4;
    heaters[4]=h5;
}

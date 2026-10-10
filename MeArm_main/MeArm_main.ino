#include <Servo.h>

//Servo base(0,z), rArm(1,y), fArm(2,x), claw(3);

Servo servoArr[4];//————————————————舵机数列
byte DSD = 5;//——————————————————————舵机每转动 1° 的间隔时间，默认为5ms（快）
byte mode = 1;//—————————————————————模式1：手柄模式；模式2：录制；模式3：播放  

void Servo_Control (byte index, int pos, byte time)//对舵机角度直接控制，运行时最根本的函数
{
  switch(index)//检测角度是否越界
  {
    case 0://base
      if (pos > 180||pos < 0)
      {
        Serial.println(F("Warning:Base Servo Value Out Of Range!"));
        return;
      }
      break;
    case 1://rArm
      if (pos > 180||pos < 45)
      {
        Serial.println(F("Warning:rArm Servo Value Out Of Range!"));
        return;
      }
      break;
    case 2://fArm
      if (pos > 120||pos < 35)
      {
        Serial.println(F("Warning:fArm Servo Value Out Of Range!"));
        return;
      }
      break;
    case 3://claw
      if (pos > 100||pos < 25)
      {
        Serial.println(F("Warning:Claw Servo Value Out Of Range!"));
        return;
      }
      break;
  }

  switch (index)
  {
    case 0:
      Serial.print(F("Servo:base  posision = "));
      Serial.println(pos);
      break;
    case 1:
      Serial.print(F("Servo:rArm  posision = "));
      Serial.println(pos);
      break;
    case 2:
      Serial.print(F("Servo:fArm  posision = "));
      Serial.println(pos);
      break;
    case 3:
      Serial.print(F("Servo:claw  posision = "));
      Serial.println(pos);
      break;
  }

  if (servoArr[index].read() < pos)//对比初始角度与目标角度大小，角度转动
  {
    for (int i = servoArr[index].read(); i <= pos; i++)
    { 
      servoArr[index].write(i);
      delay(time);
    }
  }
  else if (servoArr[index].read() > pos)
  {
    for (int i = servoArr[index].read(); i >= pos; i--)
    { 
      servoArr[index].write(i);
      delay(time);
    }
  }
  
}

void restore()//舵机复位函数————四个舵机全部转回初始角度（90、60）
{
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
  for (int i = 0; i < 3; i++)
  {
    Servo_Control(i, 90, DSD);
    delay(50);
  }
  Servo_Control(3, 60, DSD);
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
}

void report()
{
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
  Serial.print(F("Servo:base  posision = "));
  Serial.println(servoArr[0].read());
  Serial.print(F("Servo:rArm  posision = "));
  Serial.println(servoArr[1].read());
  Serial.print(F("Servo:fArm  posision = "));
  Serial.println(servoArr[2].read());
  Serial.print(F("Servo:claw  posision = "));
  Serial.println(servoArr[3].read());
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
}

void armJoyCmd()//手柄控制
{
  byte step = 3;
  if (mode == 1) 
  {
    step = 3;
    DSD = 5;
  }
  else 
  {
    step = 5;
    DSD = 3;
  }

  if (analogRead(A0) > 512+50)//base向左
  {
    Servo_Control(0, servoArr[0].read() - step, DSD);
  }
  if (analogRead(A0) < 512-50)//base向右
  {            
    Servo_Control(0, servoArr[0].read() + step, DSD);
  }
  if (analogRead(A1) < 512-50)//rArm向下
  {
    Servo_Control(1, servoArr[1].read() + step, DSD);
  }
  if (analogRead(A1) > 512+50)//rArm向上
  {  
    Servo_Control(1, servoArr[1].read() - step, DSD);
  }
  if (analogRead(A3) > 512+50)//fArm向上
  {
    Servo_Control(2, servoArr[2].read() + step, DSD);
  }
  if (analogRead(A3) < 512-50)//fArm向下
  {
    Servo_Control(2, servoArr[2].read() - step, DSD);
  } 
  if (analogRead(A2) > 512+50)//Claw关闭
  {
    Servo_Control(3, servoArr[3].read() + step, DSD);
  }
  if (analogRead(A2) < 512-50)//Claw打开
  { 
    Servo_Control(3, servoArr[3].read() - step, DSD);
  }
  if (digitalRead(5) == LOW)//复位
  {
    restore();
    delay(50);
  } 
}

typedef struct Frame
{
  byte base;
  byte rArm;
  byte fArm;
  byte claw;
} Frame;

Frame motionBuffer[300];
int total_time;

void Record()
{
  delay(50);
  Serial.println();
  Serial.println(F("切换录制模式，录制中…"));
  int time = 0;
  while(mode == 2)//循环记录舵机角度
  {
    if (digitalRead(4) == LOW)
    {
      mode = 1;//再次按下按键，结束录制，回到手柄模式
      total_time = time;
      while(Serial.available()>0) Serial.read();//清空串口残留数据
      Serial.println();
      Serial.println(F("结束录制"));
      delay(500);
      Serial.println(F("切换手柄模式"));
      return;
    }
    armJoyCmd();
  
    if (time >= 298)
    {
      mode = 1;
      total_time = time;
      while(Serial.available()>0) Serial.read();//清空串口残留数据
      Serial.println();
      Serial.println(F("结束录制(录制已满)"));
      delay(500);
      Serial.println(F("切换手柄模式"));
      return;
    }

    motionBuffer[time].base = servoArr[0].read();
    motionBuffer[time].rArm = servoArr[1].read();
    motionBuffer[time].fArm = servoArr[2].read();
    motionBuffer[time].claw = servoArr[3].read();

    time++;
    delay(50);
  }
}

void play_motion()
{
  while(Serial.available()>0) Serial.read(); //播放前清空串口缓存
  Servo_Control(3, motionBuffer[0].claw, 5);
  Servo_Control(2, motionBuffer[0].fArm, 5);
  Servo_Control(1, motionBuffer[0].rArm, 5);
  Servo_Control(0, motionBuffer[0].base, 5);
  delay(50);
  Serial.println(F("播放开始"));
  for (int i = 0; i < total_time; i++)
  {
    Servo_Control(3, motionBuffer[i].claw, 5);
    Servo_Control(2, motionBuffer[i].fArm, 5);
    Servo_Control(1, motionBuffer[i].rArm, 5);
    Servo_Control(0, motionBuffer[i].base, 5);
    delay(50);
  }
  mode = 1;
  Serial.println(F("播放结束"));
  Serial.println(F("切换摇杆模式"));
}

void setup() {
  servoArr[0].attach(9);
  servoArr[1].attach(8);
  servoArr[2].attach(7);
  servoArr[3].attach(6);

  for (short i = 0; i < 3; i++)
  {
    servoArr[i].write(90);
    delay(10);
  }
  servoArr[3].write(60);

  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  Serial.begin(9600);
  Serial.println(F("control your servo."));
}

void loop() {
  if (mode == 1)
  {
    armJoyCmd();
    if (digitalRead(4) == LOW)
    {
      Serial.println(F("切换录制模式"));
      delay(500);
      mode = 2;
    }
    if (digitalRead(12) == LOW)
    {
      Serial.println(F("切换播放模式"));
      delay(500);
      mode = 3;
    }
  }

  if (mode == 2)
  {
    Record();
  }

  if (mode == 3)
  {
    play_motion();
  }
  delay(10);
}
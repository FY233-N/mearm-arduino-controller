#include <Servo.h>

//Servo base(0,z), rArm(1,y), fArm(2,x), claw(3);

//储存角度极限值
const int baseMin = 0;
const int baseMax = 180;
const int rArmMin = 45;
const int rArmMax = 180;
const int fArmMin = 35;
const int fArmMax = 120;
const int clawMin = 25;
const int clawMax = 100;

Servo servoArr[4];//————————————————舵机数列
int servoPin[] = {11, 10, 9, 6};//——舵机连接引脚数列
int interval_time = 5;//————————————舵机每转动 1° 的间隔时间，默认为5ms（快）  

void Servo_Control (int index, int pos, int time)//对舵机角度直接控制
{
  switch(index)//检测角度是否越界
  {
    case 0://base
      if (pos > baseMax||pos < baseMin)
      {
        Serial.println("Warning:Base Servo Value Out Of Range!");
        return;
      }
      break;
    case 1://rArm
      if (pos > rArmMax||pos < rArmMin)
      {
        Serial.println("Warning:rArm Servo Value Out Of Range!");
        return;
      }
      break;
    case 2://fArm
      if (pos > fArmMax||pos < fArmMin)
      {
        Serial.println("Warning:fArm Servo Value Out Of Range!");
        return;
      }
      break;
    case 3://claw
      if (pos > clawMax||pos < clawMin)
      {
        Serial.println("Warning:Claw Servo Value Out Of Range!");
        return;
      }
      break;
  }

  switch (index)
  {
    case 0:
      Serial.print("Servo:base  posision = ");
      Serial.println(pos);
      break;
    case 1:
      Serial.print("Servo:rArm  posision = ");
      Serial.println(pos);
      break;
    case 2:
      Serial.print("Servo:fArm  posision = ");
      Serial.println(pos);
      break;
  }

  if (servoArr[index].read() < pos)//角度变化
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

void armDataCmd (Servo servoArr[],char name, int time)//通过串口通信，控制各个舵机
{
  int pos = Serial.parseInt();

  switch(name)
  {
    case 'x':
      Servo_Control(2, pos, time);
      break;
    case 'y':
      Servo_Control(1, pos, time);
      break;
    case 'z':
      Servo_Control(0, pos, time);
      break;
  }
  delay(200);

}

int Speed_Control (char SerialCmd)//速度控制，返回间隔时间——5（H，快）；15（L，慢）
{
  switch (SerialCmd)
  {
    case 'H':
      Serial.println("speed : HIGH");
      return 5;
    case 'L':
      Serial.println("speed : LOW");
      return 15;
    default:
      Serial.println("!speed ERROR!");
      return interval_time;
  }

}

void Claw_Control  (char command, int time)//爪控制
{
  int pos;
  switch (command)
  {
    case 'O':
      pos = clawMax;
      Serial.println("Claw : Open");
      break;
    case 'S':
      pos = clawMin;
      Serial.println("Claw : Close");
      break;
  }

  Servo_Control(3, pos, interval_time);
  
}

void restore()
{
  for (int i = 0; i < 4; i++)
  {
    Servo_Control(i, 90, 5);
    delay(50);
  }
}

void setup() {
  for (int i = 0; i < 4; i++)
  {
    servoArr[i].attach(servoPin[i]);
    servoArr[i].write(90);
    delay(200);
  }
  
  Serial.begin(9600);
  Serial.println("control your servo.");
}

void loop() {
  char character = '0';
  if (Serial.available() > 0)
  {
    character = Serial.read();
    if (character=='x'||character=='y'||character=='z')
    {
      armDataCmd (servoArr, character, interval_time);
    }
    else if (character == 'H' || character == 'L')
    {
      interval_time = Speed_Control(character);
    }
    else if (character == 'O' || character == 'S')
    {
      Claw_Control(character, interval_time);
    }
    else if (character == 'I')
    {
      restore();
    }
  }
  delay(10);
}

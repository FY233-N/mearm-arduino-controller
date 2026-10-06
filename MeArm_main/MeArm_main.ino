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
int DSD = 5;//————————————舵机每转动 1° 的间隔时间，默认为5ms（快）  

void Servo_Control (int index, int pos, int time)//对舵机角度直接控制，运行时最根本的函数
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

void armDataCmd (char name, int time)//通过串口通信，控制各个舵机
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
      return DSD;
  }

}

void Claw_Control  (char command, int time)//爪部控制————开（O）；关（S）
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

  Servo_Control(3, pos, DSD);
  
}

void restore()//舵机复位函数————四个舵机全部转回初始角度（90°）
{
  Serial.println("+++++++++++++++++++++++++++++++++++++++");
  for (int i = 0; i < 4; i++)
  {
    Servo_Control(i, 90, DSD);
    delay(50);
  }
  Serial.println("+++++++++++++++++++++++++++++++++++++++");
}

void MeArm_Control (char character, int DSD)//行为指令调用函数————接收串口指令，调用相应的复杂动作、功能函数（舵机转动、爪部开合、复位、移物等）
{
  switch (character)
  {
    case 'x'://前臂舵机
      armDataCmd (character, DSD);
      break;

    case 'y'://后臂舵机
      armDataCmd (character, DSD);
      break;
      
    case 'z'://底盘舵机
      armDataCmd (character, DSD);
      break;
      
    case 'O'://爪开
      Claw_Control(character, DSD);
      break;
    
    case 'S'://爪关
      Claw_Control(character, DSD);
      break;

    case 'I'://复位
      restore();
      break;
    
  }
}

void setup() {
  for (int i = 0; i < 4; i++)
  {
    servoArr[i].attach(servoPin[i]);
    servoArr[i].write(90);
    delay(10);
  }
  
  Serial.begin(9600);
  Serial.println("control your servo.");
}

void loop() {
  char character = '0';
  if (Serial.available() > 0)
  {
    character = Serial.read();
    if (character=='x'||character=='y'||character=='z'||character == 'O'||character == 'S'||character == 'I')
    {
      MeArm_Control(character, DSD);
    }
    else if (character == 'H' || character == 'L')
    {
      DSD = Speed_Control(character);
    }
   
  }
  delay(10);
}

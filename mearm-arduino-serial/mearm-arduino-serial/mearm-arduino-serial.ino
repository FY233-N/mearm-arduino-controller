#include <Servo.h>

//Servo base(0,z), rArm(1,y), fArm(2,x), claw(3);

Servo servoArr[4];//————————————————舵机数列
byte DSD = 5;//——————————————————————舵机每转动 1° 的间隔时间，默认为5ms（快）

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

void Claw_Control  (char command, byte time)//爪部控制————开（O）；关（S）
{
  int pos;
  switch (command)
  {
    case 'O':
      pos = 25;
      Serial.println(F("Claw : Open"));
      break;
    case 'S':
      pos = 100;
      Serial.println(F("Claw : Close"));
      break;
  }

  Servo_Control(3, pos, DSD);
  
}

void MeArm_Control (char character, byte DSD)//行为指令调用函数————接收串口指令，调用相应的复杂动作、功能函数（舵机转动、爪部开合、复位、移物等）
{
  switch (character)
  {
    case 'x'://前臂舵机
      Servo_Control(2, Serial.parseInt(), DSD);
      break;

    case 'y'://后臂舵机
      Servo_Control(1, Serial.parseInt(), DSD);
      break;
      
    case 'z'://底盘舵机
      Servo_Control(0, Serial.parseInt(), DSD);
      break;
      
    case 'O'://爪开
      Claw_Control(character, DSD);
      break;
    
    case 'S'://爪关
      Claw_Control(character, DSD);
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
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

  
  Serial.begin(9600);
  Serial.println(F("control your servo."));
}

void loop() {
  // put your main code here, to run repeatedly:
  char character = 0;
  if (Serial.available() > 0)
  {
    character = Serial.read();
    if (character == 'H')
    {
      DSD = 5;
      Serial.println("速度：快");
    }
    else if (character == 'L')
    {
      DSD = 15;
      Serial.println("速度：慢");
    }
    else
    {
      MeArm_Control(character, DSD);
      delay(10);
    }
  }
}

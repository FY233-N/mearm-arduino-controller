#include <Servo.h>

//Servo base(0,z), rArm(1,y), fArm(2,x), claw(3);

Servo servoArr[4];//————————————————舵机数列
byte DSD = 5;//——————————————————————舵机每转动 1° 的间隔时间，默认为5ms（快）
byte mode = 0;//—————————————————————模式0：指令模式；1：手柄模式；  
char Movement[1000];

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

void restore()//舵机复位函数————四个舵机全部转回初始角度（90°）
{
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
  for (int i = 0; i < 4; i++)
  {
    Servo_Control(i, 90, DSD);
    delay(50);
  }
  Serial.println(F("+++++++++++++++++++++++++++++++++++++++"));
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

    case 'I'://复位
      restore();
      break;
    
    case 'm'://切换至手柄模式
      mode = 1;
      Serial.println(F("Command: Switch to Joy-Stick Mode."));
      break;

    case 'r'://查看所有舵机目前角度
      report();
      break;

  }
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

void armJoyCmd(char serialCmd)//键盘模拟手柄控制
{
  if (serialCmd == 'x' || serialCmd == 'y' || serialCmd == 'z' || serialCmd == 'O' || serialCmd == 'S' )
  {
    Serial.println(F("Warning: Robot in Joy-Stick Mode..."));
    delay(100);
    while(Serial.available()>0) Serial.read();  //清除串口缓存的错误指令
    return;
  }

  switch (serialCmd)
  {
    case 'I'://复位
      restore();
      break;

    case 'r'://查看所有舵机目前角度
      report();
      break;
      
    case 'a'://base向左
      Serial.println(F("Base Turn Left")); 
      Servo_Control(0, servoArr[0].read() - 3, DSD);
      break;

    case 'd'://base向右
      Serial.println(F("Base Turn Right"));                
      Servo_Control(0, servoArr[0].read() + 3, DSD);
      break;

    case 's'://rArm向下
      Serial.println(F("Rear Arm Down"));
      Servo_Control(1, servoArr[1].read() + 3, DSD);
      break;

    case 'w'://rArm向下
      Serial.println(F("Rear Arm Up"));
      Servo_Control(1, servoArr[1].read() - 3, DSD);
      break;

    case '8'://fArm向上
      Serial.println(F("Front Arm Up"));
      Servo_Control(2, servoArr[2].read() + 3, DSD);
      break;

    case '5'://fArm向下
      Serial.println(F("Front Arm Down"));
      Servo_Control(2, servoArr[2].read() - 3, DSD);
      break;

    case '4'://Claw关闭
      Serial.println(F("Claw Close"));
      Servo_Control(3, servoArr[3].read() + 3, DSD);
      break;

    case '6'://Claw打开
      Serial.println(F("Claw Open"));
      Servo_Control(3, servoArr[3].read() - 3, DSD);
      break;

    case 'H'://提速
      DSD = 5;
      Serial.println(F("speed : HIGH"));
      break;

    case 'L'://降速
      DSD = 15;
      Serial.println(F("speed : LOW"));
      break;
    
    case 'm'://切换至指令模式
      Serial.println(F("Command: Switch to Instruction Mode."));
      mode = 0;
      break;
    
  }
}

byte clawpos;
byte fArmpos;
byte rArmpos;
byte basepos;

void Record()
{
  clawpos = servoArr[3].read();
  fArmpos = servoArr[2].read();
  rArmpos = servoArr[1].read();
  basepos = servoArr[0].read();
  
  Serial.println();
  Serial.println(F("Command: Switch to Record Mode."));
  int time = 0;
  while(mode == 2)//循环记录手柄操作
  {
    char command = 0;
    if (Serial.available() > 0)
    {
      command = Serial.read();

      if (command == 'k')
      {
        mode = 1;//再次按下k，结束录制，回到手柄模式
        Movement[time] = '\0';
        while(Serial.available()>0) Serial.read();//清空串口残留数据
        Serial.println();
        Serial.println(F("Command: Leave Record Mode."));
        Serial.println(F("Command: Switch to Joy-Stick Mode."));
        return;
      }
      armJoyCmd(command);
    }
    if (time >= 998)
    {
      mode = 1;//再次按下k，结束录制，回到手柄模式
      Movement[time] = '/';
      while(Serial.available()>0) Serial.read();//清空串口残留数据
      Serial.println();
      Serial.println(F("Command: Leave Record Mode."));
      Serial.println(F("Command: Switch to Joy-Stick Mode."));
      return;
    }

    Movement[time] = command;
    time++;
    delay(20);
  }
}

void Play_Vedio()
{
  while(Serial.available()>0) Serial.read(); //播放前清空串口缓存
  Servo_Control(3, clawpos, 5);
  Servo_Control(2, fArmpos, 5);
  Servo_Control(1, rArmpos, 5);
  Servo_Control(0, basepos, 5);
  Serial.println(F("Play Starts."));
  for (int i = 0; i < 1000; i++)
  {
    if (Movement[i] == '/')
    {
      mode = 1;//切换摇杆模式
      Serial.println();
      Serial.println(F("Playback Finished."));
      return;
    }

    armJoyCmd(Movement[i]);
    delay(20);
  }
  mode = 1;
  Serial.println(F("Playback Finished."));
}

void setup() {
    servoArr[0].attach(11);
    servoArr[1].attach(10);
    servoArr[2].attach(9);
    servoArr[3].attach(6);

    for (short i = 0; i < 4; i++)
    {
      servoArr[i].write(90);
      delay(10);
    }
  
  Serial.begin(9600);
  Serial.println(F("control your servo."));
}

void loop() {
  char character = 0;
  if (Serial.available() > 0)
  {
    character = Serial.read();
    if (character == 'k')
    {
      mode = 2;//切换录制模式
    }
    else if (character == 'l')
    {
      mode = 3;//切换播放模式
    }

    switch (mode)
    {
      case 0://指令
        if (character == 'H')
        {
          DSD = 5;
          Serial.println(F("speed : HIGH"));
        }
        else if (character == 'L')
        {
          DSD = 15;
          Serial.println(F("speed : LOW"));
        }
        else
        {
          MeArm_Control(character, DSD);
        }
        break;

      case 1://手柄
        armJoyCmd(character);
        break;

      case 2://录制
        Record();
        break;
    
      case 3://播放
        Play_Vedio();
        break;

    }
    character = 0;
  }
  delay(10);
}

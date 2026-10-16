#include <Servo.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897
#endif

//Servo base(0,z), rArm(1,y), fArm(2,x), claw(3);

Servo servoArr[4];//————————————————舵机数列
byte DSD = 5;//——————————————————————舵机每转动 1° 的间隔时间，默认为5ms（快）
byte mode = 0;//—————————————————————模式1：手柄模式；模式2：录制；模式3：播放  

const double h     = 61.0;    // 底座高度
const double len1  = 80.0;    // 后臂杆长
const double len2  = 80.0;    // 前臂杆长
const double len3  = 40.0;    // 爪子长度

// 各舵机硬件物理限位
const double BASE_MIN  = 0.0;
const double BASE_MAX  = 180.0;
const double RARM_MIN  = 5.0;
const double RARM_MAX  = 165.0;
const double FARM_MIN  = 0.0;
const double FARM_MAX  = 137.0;
const int CLAW_MIN = 0;
const int CLAW_MAX = 100;

// 最终计算结果
double calcC0 = 0.0;
double calcC1 = 0.0;
double calcC2 = 0.0;

void Servo_Control (byte index, int pos, byte time)//对舵机角度直接控制，运行时最根本的函数
{
  switch(index)//检测角度是否越界
  {
    case 0://base
      if (pos > BASE_MAX||pos < BASE_MIN)
      {
        Serial.println(F("Warning:Base Servo Value Out Of Range!"));
        return;
      }
      break;
    case 1://rArm
      if (pos > RARM_MAX||pos < RARM_MIN)
      {
        Serial.println(F("Warning:rArm Servo Value Out Of Range!"));
        return;
      }
      break;
    case 2://fArm
      if (pos > FARM_MAX||pos < FARM_MIN)
      {
        Serial.println(F("Warning:fArm Servo Value Out Of Range!"));
        return;
      }
      break;
    case 3://claw
      if (pos > CLAW_MAX||pos < CLAW_MIN)
      {
        Serial.println(F("Warning:Claw Servo Value Out Of Range!"));
        return;
      }
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

void restore()//舵机复位函数————四个舵机全部转回初始角度（90、110、60）
{
  Servo_Control(0, 90, DSD);
  Servo_Control(1, 80, DSD);
  Servo_Control(2, 115, DSD);
  Servo_Control(3, 100, DSD);
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
  if (mode != 1 && mode != 0 && mode != 4) 
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
  int time = 0;
  while(mode == 2)
  {
    if (Serial.available() > 0 && Serial.read() == '4')
    {
      mode = 1;
      total_time = time;
      Serial.println(F("结束录制，切换手柄模式"));
      delay(500);
      return;
    }
    armJoyCmd();

    if (time >= 298)
    {
      mode = 1;
      total_time = time;
      Serial.println(F("结束录制(满)，切换手柄模式"));
      delay(500);
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

void play_motion()//播放
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

int object1 = 0;
void move_object()//取物函数
{
  switch (object1)
  {
    case 0: //A
      static const byte arrA[6][4] = 
      {
        {10, 156, 49, 0},
        {10, 156, 49, 100},
        {10, 80, 115, 100},
        {62, 80, 115, 100},
        {62, 156, 49, 100},
        {62, 156, 49, 50}
      };
      for (int i = 0; i < 6; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrA[i][j], DSD+10);
          delay(50);
        }
      }
      Servo_Control(1, 80, DSD+10);
      restore();
      break;
    case 1: //B
      static const byte arrB[8][4] = 
      {
        {2, 80, 115, 0},
        {2, 148, 23, 0},
        {2, 148, 23, 100},
        {2, 80, 115, 100},
        {178, 80, 115, 100},
        {178, 148, 23, 100},
        {178, 148, 23, 0},
        {178, 80, 115, 0}
      };
      for (int i = 0; i < 8; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrB[i][j], DSD+10);
          delay(50);
        }
      }
      restore();
      break;

    case 2: //C
      static const byte arrC[8][4] = 
      {
        {168, 91, 137, 30},
        {168, 103, 76, 30},
        {168, 103, 76, 100},
        {168, 91, 137, 100},
        {77, 91, 137, 100},
        {77, 103, 76, 100},
        {77, 103, 76, 30},
        {77, 91, 137, 30}
      };
      for (int i = 0; i < 8; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrC[i][j], DSD+10);
          delay(50);
        }
      }
      restore();
      break;

  }

  object1++;
  if (object1 >= 3) object1 = 0;
}

// 运动学逆解 armMoveTo(X,Y,Z)
int armMoveTo(double X, double Y, double Z)
{
  // -------- 底座 C0 --------
  double S = sqrt(X * X + Y * Y);
  if (S < 1e-6)
  {
    S = 1e-6;
  }

  double C0_rad = atan2(Y, -X);
  double C0_deg = 180.0 / M_PI * C0_rad;

  // 归一化 [-180, 180]
  while (C0_deg < -180.0) C0_deg += 360.0;
  while (C0_deg >  180.0) C0_deg -= 360.0;

  if (C0_deg < BASE_MIN || C0_deg > BASE_MAX)
  {
    Serial.print("IK fail ret=-2 C0_deg=");
    Serial.println(C0_deg);
    return -2;
  }

  // -------- U V K --------
  double U = len3 - S;
  double V = Z - h;

  double K_numerator = len1 * len1 + len2 * len2 - (U * U + V * V);
  double K = K_numerator / (2.0 * len1 * len2);

  // 浮点容错钳位
  if (K < -1.0001) K = -1.0;
  if (K >  1.0001) K =  1.0;
  if (K < -1.0 || K > 1.0)
  {
    Serial.print("IK fail ret=-1 K=");
    Serial.println(K);
    return -1;
  }

  double asinK = asin(K);

  // 实际机械臂无法肘部向下，只能肘部向上。
  double R = M_PI - asinK;

  double A = len1 - len2 * sin(R);
  double B = len2 * cos(R);
  double denom = A * A + B * B;

  if (fabs(denom) < 1e-6)
  {
    Serial.println("IK fail ret=-1 denom=0");
    return -1;
  }

  double sinC1 = (A * V + B * U) / denom;
  double cosC1 = (A * U - B * V) / denom;

  if (sinC1 < -1.0) sinC1 = -1.0;
  if (sinC1 >  1.0) sinC1 =  1.0;
  if (cosC1 < -1.0) cosC1 = -1.0;
  if (cosC1 >  1.0) cosC1 =  1.0;

  double C1_rad = atan2(sinC1, cosC1);
  double C2_rad = R - C1_rad;

  double c1d = 180.0 / M_PI * C1_rad;
  double c2d = 180.0 / M_PI * C2_rad;

  c1d = normalizeDeg360(c1d);
  c2d = normalizeDeg360(c2d);

  // Serial.print("Try R2[肘部向上] c1d=");
  // Serial.print(c1d);
  // Serial.print(" c2d=");
  // Serial.println(c2d);

  // 做限位判断
  if (c1d < RARM_MIN || c1d > RARM_MAX ||
      c2d < FARM_MIN || c2d > FARM_MAX)
  {
    Serial.print("IK fail ret=-3 c1d=");
    Serial.print(c1d);
    Serial.print(" c2d=");
    Serial.println(c2d);
    return -3;
  }

  calcC0 = C0_deg;
  calcC1 = c1d;
  calcC2 = c2d;

  int t0 = (int)round(calcC0);
  int t1 = (int)round(calcC1);
  int t2 = (int)round(calcC2);

  Servo_Control(0, t0, 10);
  Servo_Control(1, t1, 10);
  Servo_Control(2, t2, 10);

  return 0;
}

// 角度归一化到 [0, 360)
double normalizeDeg360(double a)
{
  while (a < 0.0) a += 360.0;
  while (a >= 360.0) a -= 360.0;
  return a;
}

//正解
void forward(double C0, double C1, double C2, double &outX, double &outY, double &outZ)
{
  //角度转弧度
  double c0_rad = M_PI / 180.0 * C0;
  double c1_rad = M_PI / 180.0 * C1;
  double c2_rad = M_PI / 180.0 * C2;

  double S_term =  - len1 * cos(c1_rad) + len2 * sin(c2_rad) + len3;

  outX = S_term * (-cos(c0_rad));
  outY = S_term * sin(c0_rad);
  outZ = h + len1 * sin(c1_rad) - len2 * cos(c2_rad);
}

//画直线
void write_line ()
{
  armMoveTo(0.0, 110.0, 60.0);
  for (double i = 0.0; i < 20.0; i += 1.0)
  {
    int ret = armMoveTo(0.0, 120.0 - i, 60.0);
    if (ret != 0)
    {
      Serial.print("IK error ret=");
      Serial.println(ret);
    }

    if (Serial.available() > 0 && Serial.read() == '2')
    {
      Serial.println("暂停");
      delay(100);
      while (1)
      {
        delay(10);
        if (Serial.available() > 0)
        {
          char c1 = Serial.read();
          if (c1 == '2')
          {
            Serial.println("取消");
            restore();
            return;
          }
          if (c1 == '4')
          {
            Serial.println("继续");
            break;
          }
        }
      }
    }
  }
  armMoveTo(0.0, 100.0, 120.0);
  restore();
  Serial.println("结束");
  delay(100);
}

//画V字
void write_V ()
{
  armMoveTo(-30.0, 145.0, 60.0);

  for (double i = -30.0; i < 0; i += 1.0)
  {
    int ret = armMoveTo(i, 100.0 - 1.5 * i, 60.0);
    if (ret != 0) { Serial.print("IK error ret="); Serial.println(ret); }

    if (Serial.available() > 0 && Serial.read() == '2')
    {
      Serial.println("暂停");
      delay(100);
      while (1)
      {
        delay(10);
        if (Serial.available() > 0)
        {
          char c1 = Serial.read();
          if (c1 == '2') { Serial.println("取消"); restore(); return; }
          if (c1 == '4') { Serial.println("继续"); break; }
        }
      }
    }
  }

  delay(100);

  for (double i = 0.0; i < 30.1; i += 1.0)
  {
    int ret = armMoveTo(i, 100.0 + 1.5 * i, 60.0);
    if (ret != 0) { Serial.print("IK error ret="); Serial.println(ret); }

    if (Serial.available() > 0 && Serial.read() == '2')
    {
      Serial.println("暂停");
      delay(100);
      while (1)
      {
        delay(10);
        if (Serial.available() > 0)
        {
          char c1 = Serial.read();
          if (c1 == '2') { Serial.println("取消"); restore(); return; }
          if (c1 == '4') { Serial.println("继续"); break; }
        }
      }
    }
  }
  armMoveTo(30.0, 145.0, 120.0);
  restore();
  Serial.println("结束");
  delay(100);
}

// 五点折线
void write_fiveLine()
{
  while (Serial.available() > 0) Serial.read();

  typedef struct posision
  {
    double base;
    double rArm;
    double fArm;
  } posision;

  posision POS[5];

  for (int i = 0; i < 5; i++)
  {
    delay(100);
    Serial.print("记录点 ");
    Serial.print(i + 1);
    while (1)
    {
      armJoyCmd();
      if (Serial.available() > 0 && Serial.read() == '3')
      {
        delay(150);
        Serial.print("已记录点：");
        Serial.println(i + 1);
        POS[i].base = (double)servoArr[0].read();
        POS[i].rArm = (double)servoArr[1].read();
        POS[i].fArm = (double)servoArr[2].read();
        break;
      }
    }
  }

  delay(50);
  Servo_Control(0, 90, DSD);
  Servo_Control(1, 80, DSD);
  Servo_Control(2, 115, DSD);

  for (int i = 0; i < 5; i++)
  {
    double X, Y, Z;
    double X1, Y1, Z1;

    forward(POS[i].base, POS[i].rArm, POS[i].fArm, X, Y, Z);
    forward((double)servoArr[0].read(),
            (double)servoArr[1].read(),
            (double)servoArr[2].read(),
            X1, Y1, Z1);

    double dx = X - X1;
    double dy = Y - Y1;
    double dz = Z - Z1;

    // 最大分量决定步数
    double maxAbs = fabs(dx);
    if (fabs(dy) > maxAbs) maxAbs = fabs(dy);
    if (fabs(dz) > maxAbs) maxAbs = fabs(dz);

    int step_qty = (int)maxAbs;

    if (step_qty < 1)
    {
      armMoveTo(X, Y, Z);
      continue;
    }
    if (step_qty > 80) step_qty = 80;

    double X_step = dx / (double)step_qty;
    double Y_step = dy / (double)step_qty;
    double Z_step = dz / (double)step_qty;

    // j 从 1 到 step_qty，到达目标
    for (int j = 1; j <= step_qty; j++)
    {
      delay(80);

      double tarX = X1 + ((double)j * X_step);
      double tarY = Y1 + ((double)j * Y_step);
      double tarZ = Z1 + ((double)j * Z_step);

      if (Serial.available() > 0 && Serial.read() == '2')
      {
        Serial.println("暂停");
        delay(100);
        while (1)
        {
          delay(10);
          if (Serial.available() > 0)
          {
            char c1 = Serial.read();
            if (c1 == '2') { Serial.println("取消"); restore(); return; }
            if (c1 == '4') { Serial.println("继续"); break; }
          }
        }
      }

      int ret = armMoveTo(tarX, tarY, tarZ);
      if (ret != 0)
      {
        Serial.print("IK失败 i="); Serial.print(i);
        Serial.print(" j="); Serial.print(j);
        Serial.print(" ret="); Serial.println(ret);
      }
    }

    // 兜底：精确到达
    armMoveTo(X, Y, Z);
  }

  Serial.println("结束");
}

void move_object_serial(char object)//取物函数
{
  switch (object)
  {
    case 'A': 
      static const byte arrA[6][4] = 
      {
        {10, 156, 49, 0},
        {10, 156, 49, 100},
        {10, 80, 115, 100},
        {62, 80, 115, 100},
        {62, 156, 49, 100},
        {62, 156, 49, 50}
      };
      for (int i = 0; i < 6; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrA[i][j], DSD+10);
          delay(50);
        }
      }
      Servo_Control(1, 80, DSD+10);
      restore();
      break;
    case 'B': 
      static const byte arrB[8][4] = 
      {
        {168, 91, 137, 30},
        {168, 103, 76, 30},
        {168, 103, 76, 100},
        {168, 91, 137, 100},
        {77, 91, 137, 100},
        {77, 103, 76, 100},
        {77, 103, 76, 30},
        {77, 91, 137, 30}
      };
      for (int i = 0; i < 8; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrB[i][j], DSD+10);
          delay(50);
        }
      }
      restore();
      break;

    case 'C': 
      static const byte arrC[8][4] = 
      {
        {168, 91, 137, 30},
        {168, 103, 76, 30},
        {168, 103, 76, 100},
        {168, 91, 137, 100},
        {77, 91, 137, 100},
        {77, 103, 76, 100},
        {77, 103, 76, 30},
        {77, 91, 137, 30}
      };
      for (int i = 0; i < 8; i++)
      {
        for (int j = 0; j < 4; j++)
        {
          Servo_Control(j, arrC[i][j], DSD+10);
          delay(50);
        }
      }
      restore();
      break;

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
  servoArr[0].attach(9);
  servoArr[1].attach(7);
  servoArr[2].attach(8);
  servoArr[3].attach(6);

  Servo_Control(0, 90, DSD);
  Servo_Control(1, 80, DSD);
  Servo_Control(2, 115, DSD);
  Servo_Control(3, 100, DSD);
  //10，播放
  //5，复位 / V字
  //4，直线 / 继续 / 录制
  //3，循环拿物 / 五点折线记录
  //2，切换画线模式 / 暂停
  Serial.begin(9600);
  Serial.println(F("control your servo."));
}

void loop() {
  if (mode == 0)
  {
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
      else if (character == 'A' || character == 'B' || character == 'C')
      {
        Serial.println("取物");
        move_object_serial(character);
      }
      else if (character == 'm')
      {
        mode = 1;
        Serial.println(F("切换摇杆模式"));
      }
      else
      {
        MeArm_Control(character, DSD);
        delay(10);
      }
    }
  }

  if (mode == 1)
  {
    armJoyCmd();

    if (Serial.available() > 0)
    {
      char c = Serial.read();
      if (c == '3')//循环拿物
      {
        move_object();
        delay(50);
      }
      if (c == '4')//录制
      {
        Serial.println(F("切换录制模式"));
        delay(500);
        mode = 2;
      }
      if (c == '1')//播放
      {
        Serial.println(F("切换播放模式"));
        delay(500);
        mode = 3;
      }
      if (c == '2')//画线
      {
        Serial.println(F("切换画线模式"));
        delay(500);
        mode = 4;
      }
      if (c == '5')//复位
      {
        restore();
        delay(50);
      }
      if (c == 'm')//切换串口
      {
        mode = 0;
        Serial.println(F("切换串口模式"));
        delay(50);
      }
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

  if (mode == 4)
  {
    if (Serial.available() > 0)
    {
      char c = Serial.read();
      if (c == '2') // 手柄
      {
        Serial.println(F("切换手柄模式"));
        delay(500);
        mode = 1;
      }
      else if (c == '4') // 直线
      {
        write_line();
        delay(100);
      }
      else if (c == '5') // V字
      {
        write_V();
        delay(100);
      }
      else if (c == '3') // 五点折线
      {
        Serial.println(F("准备记录5个记录点"));
        delay(150);
        Serial.println(F("开始记录"));
        write_fiveLine();
        delay(100);
      }
    }
  }
}
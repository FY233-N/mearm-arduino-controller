void setup() {
  // put your setup code here, to run once:
  pinMode(2, INPUT_PULLUP);//切换画线模式 / 暂停
  pinMode(3, INPUT_PULLUP);//循环拿物 / 五点折线记录
  pinMode(4, INPUT_PULLUP);//播放 / 直线 / 继续 / 录制
  pinMode(5, INPUT_PULLUP);//复位 / V字
  pinMode(10, INPUT_PULLUP);//播放

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(2) == LOW)
  {
    Serial.write('2');
    delay(200);
  }
  else if(digitalRead(3) == LOW)
  {
    Serial.write('3');
    delay(200);
  }
  else if(digitalRead(4) == LOW)
  {
    Serial.write('4');
    delay(200);
  }
  else if(digitalRead(5) == LOW)
  {
    Serial.write('5');
    delay(200);
  }
  else if(digitalRead(10) == LOW)
  {
    Serial.write('1');
    delay(200);
  }
  
}
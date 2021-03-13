#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN    9  
#define SS_PIN    10
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件
// 腳位設定 (資料型別設定)
int ENA = 3;
int ENB = 5;
int IN1 = 4; //左輪
int IN2 = 6;
int IN3 = 7; //右輪
int IN4 = 8;
int l1 = A0;
int l2 = A1;
int m = A2 ;
int r2 = A4;
int r1 = A5;
int RX = 2;  //處理遙控車的部分
int TX = 1;

enum State{initial, waiting, righting, lefting, straighting, U_turning, pausing};
State state = initial;
int counter = 0;
int times = 3000;
int times_t = 2500;
int R1, R2, M, L1, L2;
bool start = false;
String route;
SoftwareSerial BT(RX, TX);


void MotorWriting(double vR, double vL)
{
  if (vR<0)
  {
    vR =-vR;
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW); 
  }
  else
  
  {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH); 
  }
  if (vL<0)
  {
    vL =-vL;
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else
  {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW); 
  }
   analogWrite(ENA, vL);
   analogWrite(ENB, vR);    
}



void Tracking(int R1, int R2, int M, int L2, int L1, State &state) {
  
  //static bool block = false;
  //static bool last_block = false;
  if(!L1 && L2 && M && R2 && !R1)
  {
    MotorWriting(240, 255);
  }
  if(!L1 &&!L2 && M && R2 && R1)
  {
    MotorWriting(170, 255);
  }
  if(!L1 && !L2 && !M && R2 && R1)
  {
    MotorWriting(85, 255);
  }
  if(L1 && L2 && M && !R2 && !R1)
  {
    MotorWriting(240, 170);
  }
  if(L1 && L2 && !M && !R2 && !R1)
  {
    MotorWriting(240, 85);
  }
  if(!L1 && !L2 && M && R2 && !R1)
  {
    MotorWriting(170, 255);
  }
  if(!L1 && L2 && M && !R2 && !R1)
  {
    MotorWriting(240, 170);
  }
  if(L1 && L2 && M && R2 && R1)
  {
    MotorWriting(240, 255);
    state = waiting;
    //Serial.println('a');
  }
}

void wait(State &state)
{
  static unsigned int n = 0;
    char order = route.charAt(n);
    //Serial.println(order);
    n++;
    switch(order)
    {
      case 'a': state = lefting; break;
      case 'd': state = righting; break;
      case 'w': state = straighting; break;
      case 's': state = U_turning; break;
      case 'p': state = pausing; break;
    }
}


void setup() {
// 腳位定義
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    Serial.begin(9600);
    BT.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();   // 初始化MFRC522讀卡機模組
    Serial.println(F("Read UID on a MIFARE PICC:")); 
    Serial.setTimeout(3000);

}
bool read_card(){
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
    {
      byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
      byte idSize = mfrc522.uid.size;   // 取得UID的長度
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      for (byte i = 0; i < idSize; i++) {  // 逐一顯示UID碼
          if (id[i]>=16)
            Serial.print(id[i],HEX);
          else{
            Serial.print('0');
            Serial.print(id[i],HEX);
          }
      }
      Serial.println();
      mfrc522.PICC_HaltA();  // 讓卡片進入停止模式
      return true;
    } 
    else{return false;}
}
void loop() {
  // put your main code here, to run repeatedly:
    //馬達電壓
    //digitalWrite(ENA, HIGH);
    //digitalWrite(ENB, HIGH);
    // 轉五秒 
    if(!start)
    {
      if(BT.available())
      {
        route = BT.readString();
        //Serial.println(route);
        while(!start)
        {
          if(BT.available())
          {
            char s = BT.read();
            if(s == '1') 
            {
              start = true;
              Serial.print(s);
            }
          }
        }
      }
    }
    else
    {
    R1 = digitalRead(r1); 
    R2 = digitalRead(r2);
    M = digitalRead(m);
    L2 = digitalRead(l2);
    L1 = digitalRead(l1);
   switch(state)
   {
    case initial: Tracking(R1, R2, M, L2, L1, state);/*read_card();*/ break;
    case waiting:
      wait(state);
      //read_card();
      break;
    case righting: 
      if(L1 && L2 && M && R2 && R1)
      {
        MotorWriting(75, 255);
      }
      else
      {
        MotorWriting(0, 255);
        delay(350);
        MotorWriting(75, 255);
        delay(150);
        state = initial;
      }
      
      break;
    case lefting:
      if(L1 && L2 && M && R2 && R1)
      {
        MotorWriting(255, 75);
      }
      else
      {
        MotorWriting(255, 0);
        delay(350);
        MotorWriting(255, 75);
        delay(150);
        state = initial;
      }
      
      break;
    case straighting:
      MotorWriting(240, 255);
      delay(800);
      state = initial;
      /*counter++;
      if(counter > times)
      {
        state = initial;
        counter = 0;
      }*/
      break;
    case U_turning:
      //Serial.println('b');
      while(!read_card()){MotorWriting(255, 255);}
      MotorWriting(-255, 255);
      delay(700);
      if(L1 || L2 || M || R2 || R1)  state = initial;
      break;
    case pausing:
      while(!read_card()){MotorWriting(255, 255);}
      MotorWriting(0, 0);
      delay(20000);
      break;
   }
    }

    
}

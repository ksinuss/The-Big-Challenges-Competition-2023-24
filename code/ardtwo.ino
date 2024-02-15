// инициализация для узд
#define PIN_TRIG 10 //10 - 16
#define PIN_ECHO 11 //11
int vl; 
int kop;
long duration, cm;
int n = 15; // расположение накопителя

// инициализация для сервопривода
#include <Servo.h>
Servo servo; // create servo object to control a servo
int pos; // variable to store the servo position

// инициализация переменных для дальнейшего получения информации о грузах (распознавание)
String data = "";

void setup() 
{
  Serial.begin(9600);
  servo.attach(12);
  servo.write(110);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

void loop() 
{
  data = receive();
  if (data == "1")
  {
    kop = 1;
    delay(5500);
    delay(1500);
    servo.write(95);
    delay(100);
    data = "";
  }

  if (kop==1)
  { vl = katusha();}

  if (vl <= n+2 and vl >= n-2 and kop == 1)
  { 
    delay(4000);
    servo.write(110);
    delay(200);
    kop = 0;
    vl = 0;
  }
}

int katusha() // ultrasonic distance sensor 
{
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG, HIGH);
  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  // Время задержки акустического сигнала на эхолокаторе
  duration = pulseIn(PIN_ECHO, HIGH);
  // Преобразование времени в расстояние
  cm = (duration / 2) / 29.1;
  delay(300); // Задержка между измерениями для корректной работы скеча
  return cm;
}

String receive() // получение сигнала о заказе или распознавшихся грузах
{
  if (Serial.available())
  {
    String data = Serial.readString();
    return data;
  }
  else
  { return "";}
}

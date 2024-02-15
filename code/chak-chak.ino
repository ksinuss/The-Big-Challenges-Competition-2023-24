// be happy

// инициализация для дисплея
#include <OLED_I2C.h>
extern uint8_t SmallFont[];
OLED myOLED(SDA, SCL, 8);
String del = "                       ";
String scan = "SCANNING... ";
String fnsh = "FINISH! ";
String gruz = " ";
int flagol;

// инициализация для шаговых моторов
#include <Stepper_28BYJ_48.h>
Stepper_28BYJ_48 stepperin (11, 10, 9, 8);
Stepper_28BYJ_48 stepperup (7, 6, 5, 4);
int flagin;
int flagup;
int back;

// инициализация для узд
#define PIN_TRIG 3
#define PIN_ECHO 2
#define PIN_TRIG2 14 // 16
#define PIN_ECHO2 15
int penki[] = {7, 15, 23, 31, 39, 46, 54, 61}; // пеньки - расположение ячеек для остановки по датчику расстояния точно напротив каждой
long duration, cm;
long duration2, cm2;
int cmp, cmb;
byte i = 0;
int cell;

// инициализация для икд
#define sensor A0
int vl, kop;
int n = 11; // расположение накопителя

// инициализация переменных для дальнейшего получения информации о грузах (распознавание)
String data = "";
int take;
int drop;
int flagrcv = 1;
int finish = -1;
int count; // подсчет распознанных грузов
int flag; // количество грузов в заказе

void setup()
{
  Serial.begin(9600);
  
  myOLED.begin();
  myOLED.setFont(SmallFont);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  pinMode(PIN_TRIG2, OUTPUT);
  pinMode(PIN_ECHO2, INPUT);
}

void loop()
{
  delay(100); // start yield
  
  // принимаем данные с raspberry (начало, конец, распознавание)
  if (flagrcv == 1)
  { data = receive();}

  // начинаем работу устройства
  if (data == "2" or data == "3")
  { 
    if (data == "2") // два груза в заказе
    { flag = 2;}
    if (data == "3") // три груза в заказе
    { flag = 3;}
    finish = 0;
    cell = penki[i];
    vl = 0;
    flagin = 1;
    flagol = 2;
    delay(100);
    flagol = 0;
    flagrcv = 1;
    data = "";
  }
  // завершаем работу устройства
  if (data == "0")
  { 
    int y = 25;
    int x = CENTER;
    myOLED.print(del, x, y);
    myOLED.update();
    flagol = 0;
    flagin = 0;
    flagup = 0;
    flagrcv = 0;
  }

  // груз распознан
  if (data.length() > 1)
  { 
    gruz = data;
    flagol = 1;
    delay(100);
    flagrcv = 0;
    flagol = 0;
    data = "";
    vl = 0;
    take = 1;
  }

  cm = kabanchik(); // считываем показания узд
  if (kop == 1)
  { vl = katusha();} // считываем показания узд 2 (для перемещения к накопителю)
  
  // перемещаемся от ячейки до ячейки
  if (cm >= cell-1 and cm <= cell+1 and back == 0 and kop == 0 and take == 0 and finish == 0)
  {
    if (i < 9)
    { 
      flagin = 0;
      flagrcv = 1;
      delay(2000);
      i++;
      cell = penki[i];
      flagin = 1;
    }
    if (i == 8)
    {
      delay(100);
      flagin = 0;
      back = 1;
      i = 0;
      flagrcv = 0;
      flagin = 2;
    }
  }
  
  // приехали к накопителю
  if (vl <= n+1 and vl >= n-1 and kop == 1 and back == 0)
  {
    delay(1800);
    flagin = 0;
    delay(100);
    kop = 0;
    vl = 0;
    delay(100);
    drop = 1;
  }

  // приехали на (базу) к месту, где остановились/распознали последний груз
  if ((cm > 100 or (cmb-2 <= cm and cm <= cmb+2)) and back == 1)
  {
    flagin = 0;
    back = 0;
    cmb = 0;
    delay(2000);
    cell = penki[i];
    flagrcv = 1;
    flagin = 1;
  }

  if (take == 1) // захват груза
  {
    flagin = 0;
    flagin = 2;
    delay(600);
    flagin = 0;
    cmb = kabanchik();
    // forward paw
    flagup = 1;
    delay(4000);
    flagup = 0;
    delay(1000);
    // back paw
    flagup = 2;
    delay(3850);
    flagup = 0;
    flagrcv = 0;
    take = 0;
    data = "";
    flagin = 1;
    kop = 1;
    delay(100);
  }

  if (drop == 1) // скидываем груз
  {
    flagup = 1;
    delay(3500);
    flagup = 0;
    delay(1000);
    flagup = 2;
    delay(3500);
    flagup = 0;
    drop = 0;
    kop = 0;
    count++;
    if (count == flag)
    { 
      flagol = 3;
      delay(100);
      flagol = 0;
      flagin = 0;
      back = 0;
    }
    else
    {
      flagin = 2;
      back = 1;
    }
  }
}

void yield() // выполнение функций независимо от loop
{  
  oleg();
  stepin();
  stepup();
}

void oleg() // display
{
  int y = 25;
  int x = CENTER;
  if (flagol == 1)
  {
    myOLED.print(del, x, y);
    myOLED.update();
    myOLED.print(gruz, x, y);
    myOLED.update();
  }
  if (flagol == 2)
  {
    myOLED.print(del, x, y);
    myOLED.update();
    myOLED.print(scan, x, y);
    myOLED.update();
  }
  if (flagol == 3)
  {
    myOLED.print(del, x, y);
    myOLED.update();
    myOLED.print(fnsh, x, y);
    myOLED.update();
  }
}

void stepin() // step motor in cart
{
  if (flagin == 1) 
  { stepperin.step(1);}
  if (flagin == 2) 
  { stepperin.step(-1);}
}

void stepup() // step motor on top of cart
{
  if (flagup == 1) 
  { stepperup.step(1);}
  if (flagup == 2) 
  { stepperup.step(-1);}
}

int kabanchik() // ultrasonic distance sensor 
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

int katusha() // ultrasonic distance sensor 2
{
  digitalWrite(PIN_TRIG2, LOW);
  delayMicroseconds(5);
  digitalWrite(PIN_TRIG2, HIGH);
  // Выставив высокий уровень сигнала, ждем около 10 микросекунд. В этот момент датчик будет посылать сигналы с частотой 40 КГц
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG2, LOW);
  // Время задержки акустического сигнала на эхолокаторе
  duration2 = pulseIn(PIN_ECHO2, HIGH);
  // Преобразование времени в расстояние
  cm2 = (duration2 / 2) / 29.1;
  delay(300); // Задержка между измерениями для корректной работы скеча
  return cm2;
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

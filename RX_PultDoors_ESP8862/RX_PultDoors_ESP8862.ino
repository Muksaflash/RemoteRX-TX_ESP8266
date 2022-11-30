#include <ESP8266WiFi.h>
#include <espnow.h>

#define OPEN_DOOR_TIME 2000
#define SMOKE_TIME 5000
#define DEUTCH_SHOOT_TIME 25000
#define WIN_TIME 3000
#define SHOOT_TIME 5000

//Таймеры дверей
bool func1;
bool func2;
bool func3;
bool func4;

bool allButtonsFlag;
bool buttonFlag5;
bool isShiftClick;

unsigned long buttonHoldTimer;
unsigned long smokeTimer;
unsigned long shiftOffTimer;
unsigned long shootingTimer;
unsigned long deutchlandShootingTimer;
unsigned long winTimer;

unsigned long doorTimer1;
unsigned long doorTimer2;
unsigned long doorTimer3;
unsigned long doorTimer4;
unsigned long doorTimer5;

// ЗАМЕНИТЕ МАС-АДРЕС платы, на которую отправляем данные.
uint8_t broadcastAddress[] = {0x58, 0xBF, 0x25, 0xDA, 0x59, 0xA9};

// Вводим переменные для хранения отправляемых данных
float door1;
float door2;
float door3;
float door4;
float door5;

// Вводим переменные для хранения принимаемых данных
float incomingDoor1;
float incomingDoor2;
float incomingDoor3;
float incomingDoor4;
float incomingDoor5;

bool isRecieve;

// Переменная для хранения состояния отправки
String success;

typedef struct struct_message {
  float d1;
  float d2;
  float d3;
  float d4;
  float d5;
} struct_message;

// Создаем переменную для хранения отправляемого сообщения
struct_message AgentDoorConditions;

// То же, но для принимаемого сообщения
struct_message incomingReadings;

int timerOff = 20000; // таймер самоотключения

// Callback-функция для получения состояния отправки
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");

  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

// То же, для индикации состояния приема данных
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  //Serial.print("Bytes received: ");
  //Serial.println(len);
  isRecieve = 1;
  incomingDoor1 = incomingReadings.d1;
  incomingDoor2 = incomingReadings.d2;
  incomingDoor3 = incomingReadings.d3;
  incomingDoor4 = incomingReadings.d4;
  incomingDoor5 = incomingReadings.d5;
}

void getReadings() {
  // Снимаем значения кнопок дерей
  door1 = digitalRead(D1);
  door2 = digitalRead(D2);
  door3 = digitalRead(D5);
  door4 = digitalRead(D6);
  door5 = digitalRead(D7);
}

void printIncomingReadings() {
  // Отображаем показания в мониторе порта
  Serial.println("INCOMING READINGS");
  Serial.print("Door1 condition: ");
  Serial.print(incomingDoor1);
  Serial.println(" !");
  Serial.print("Door 2 condition: ");
  Serial.print(incomingDoor2);
  Serial.println(" !");
}

void setup() {
  pinMode(D4, OUTPUT);
  pinMode(D3, OUTPUT);

  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);

  digitalWrite(D0, HIGH);
  digitalWrite(D1, HIGH);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  digitalWrite(D4 , HIGH);
  digitalWrite(D5, HIGH);
  digitalWrite(D6, HIGH);
  digitalWrite(D7, HIGH);

  // Запускаем монитор порта
  Serial.begin(115200);
  // Выставляем режим работы Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Инициализируем протокол ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Указываем роль платы в сети
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Регистрируем callback-функцию для получения статуса отправки
  esp_now_register_send_cb(OnDataSent);

  // Регистрируем пиры
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Регистрируем callback-функцию для получения статуса приема
  esp_now_register_recv_cb(OnDataRecv);
}

unsigned int switchClick(bool incomingDoor, uint8_t pin, unsigned int timer, int workTime)
{
  if (incomingDoor && allButtonsFlag)
  {
    digitalWrite(pin, LOW);
    timer = millis();
    Serial.println("Success 1");
    allButtonsFlag = 0;
  }
  if (millis() - timer > workTime && timer != 0)
  {
    Serial.println("Success 2");
    digitalWrite(pin, HIGH);
    timer = 0;

  }
  return timer;
}

void loop() {
  // printConditionDoors();

  if (!isRecieve)
  {
    incomingDoor1 = 0;
    incomingDoor2 = 0;
    incomingDoor3 = 0;
    incomingDoor4 = 0;
    incomingDoor5 = 0;
  }

  if (!incomingDoor5)
  {
    buttonFlag5 = true;
  }
  if (!incomingDoor1 && !incomingDoor2 && !incomingDoor3 && !incomingDoor4 && !incomingDoor5 && !allButtonsFlag)
  {
    allButtonsFlag = true;
  }

  if (incomingDoor5 && !isShiftClick && buttonFlag5 && shiftOffTimer == 0)
  {
    shiftOffTimer = millis();
    Serial.println("like");
    isShiftClick = true;
    buttonFlag5 = false;
  }
  Serial.println(isShiftClick);

  if (isShiftClick)
  {
    Serial.println("in if");
    func1 = incomingDoor1;
    func2 = incomingDoor2;
    func3 = incomingDoor3;
    func4 = incomingDoor4;
  }
  else
  {
    func1 = 0;
    func2 = 0;
    func3 = 0;
    func4 = 0;
    doorTimer1 = switchClick(incomingDoor1, D1, doorTimer1, OPEN_DOOR_TIME);
    doorTimer2 = switchClick(incomingDoor2, D5, doorTimer2, OPEN_DOOR_TIME);
    doorTimer3 = switchClick(incomingDoor3, D6, doorTimer3, OPEN_DOOR_TIME);
    doorTimer4 = switchClick(incomingDoor4, D7, doorTimer4, OPEN_DOOR_TIME);
  }
  deutchlandShootingTimer = switchClick(func1, D3, deutchlandShootingTimer, DEUTCH_SHOOT_TIME);
  smokeTimer = switchClick(func2, D2, smokeTimer, SMOKE_TIME);
  shootingTimer = switchClick(func3, D0, shootingTimer, SHOOT_TIME);
  winTimer = switchClick(func4, D4, winTimer, WIN_TIME);

  if ((incomingDoor1 || incomingDoor2 || incomingDoor3 || incomingDoor4 || incomingDoor5) && isShiftClick && buttonFlag5)
  {

    buttonFlag5 = false;
    Serial.println("off click");
    isShiftClick = false;
  }
  if (millis() - shiftOffTimer > 2000 && shiftOffTimer != 0)
  {
    shiftOffTimer = 0;
    isShiftClick = false;
  }

  isRecieve = 0;
  delay(100);
}

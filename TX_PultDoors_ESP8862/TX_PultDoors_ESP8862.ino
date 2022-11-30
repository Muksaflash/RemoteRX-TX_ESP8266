#include <ESP8266WiFi.h>
#include <espnow.h>

// ЗАМЕНИТЕ МАС-АДРЕС платы, на которую отправляем данные.
uint8_t broadcastAddress[] = {0x58, 0xBF, 0x25, 0xD7, 0x47, 0x6B};

// Вводим переменные для хранения отправляемых данных
float door1;
float door2;
float door3;
float door4;
float door5;

// Переменная для хранения состояния отправки
String success;

//Пример структуры для отправки
//Должна совпадать со структурой на плате-приемнике
typedef struct struct_message {
  float d1;
  float d2;
  float d3;
  float d4;
  float d5;
} struct_message;

// Создаем переменную для хранения отправляемого сообщения
struct_message AgentDoorConditions;

int timerOff = 2000; // таймер самоотключения

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

void getReadings() {
  // Снимаем значения кнопок дерей
  door1 = !digitalRead(D1);
  door2 = !digitalRead(D2);
  door3 = !digitalRead(D5);
  door4 = !digitalRead(D6);
  door5 = !digitalRead(D7);
}

void setup() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW);
  
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  
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
}

void loop() {
  {
    //Запрашиваем показания кнопок
    getReadings();

    //Записываем их в переменные
    AgentDoorConditions.d1 = door1;
    AgentDoorConditions.d2 = door2;
    AgentDoorConditions.d3 = door3;
    AgentDoorConditions.d4 = door4;
    AgentDoorConditions.d5 = door5;

    // Отправляем сообщение
    esp_now_send(broadcastAddress, (uint8_t *) &AgentDoorConditions, sizeof(AgentDoorConditions));
  }
  delay(100);
  if(millis() > timerOff) ESP.deepSleep(0);
}

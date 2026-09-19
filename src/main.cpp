#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// --- BIBLIOTEKI SUPLA ---
#include <SuplaDevice.h>
#include <supla/network/esp_wifi.h>
#include <supla/sensor/virtual_thermometer.h>
#include <supla/sensor/general_purpose_measurement.h>

// --- BIBLIOTEKI DO TRYBU KONFIGURACYJNEGO ---
#include <supla/network/esp_web_server.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>

// --- PAMIĘĆ DLA ESP32 ---
#include <supla/storage/littlefs_config.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_PIN 0 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_BMP280 bmp;

Supla::LittleFsConfig configStore;
Supla::ESPWifi wifi;
Supla::EspWebServer suplaServer;

Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;


Supla::Sensor::VirtualThermometer *termometr;
Supla::Sensor::GeneralPurposeMeasurement *cisnieniomierz;

unsigned long ostatniPomiar = 0;
unsigned long czasPobudki = 0;
bool ekranWlaczony = true;

bool przyciskWcisniety = false;
unsigned long czasWcisniecia = 0;
bool trybKonfig = false; 

void setup() {
  setCpuFrequencyMhz(80);

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 10);
  display.println("Start Supla...");
  display.display();

  if (!bmp.begin(0x76)) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Blad BMP280!");
    display.display();
    while (1) delay(10);
  }

  termometr = new Supla::Sensor::VirtualThermometer();
  cisnieniomierz = new Supla::Sensor::GeneralPurposeMeasurement();

  SuplaDevice.begin();

  WiFi.setSleep(true); 

  czasPobudki = millis();
}

void loop() {
  SuplaDevice.iterate();

  // --- 1. OBSŁUGA PRZYCISKU ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (!przyciskWcisniety) {
      przyciskWcisniety = true;
      czasWcisniecia = millis();
      czasPobudki = millis();

      if (!ekranWlaczony && !trybKonfig) {
        display.ssd1306_command(SSD1306_DISPLAYON);
        ekranWlaczony = true;
      }
    } else {
      if (millis() - czasWcisniecia > 5000 && !trybKonfig) {
        trybKonfig = true;
        SuplaDevice.enterConfigMode();
      }
    }
  } else {
    przyciskWcisniety = false;
  }

  // --- 2. ODCZYTY Z CZUJNIKA ---
  if (millis() - ostatniPomiar > 2000) {
    ostatniPomiar = millis();

    float temperatura = bmp.readTemperature();
    float cisnienie = bmp.readPressure() / 100.0F;

    termometr->setValue(temperatura);
    cisnieniomierz->setValue(cisnienie);

    // --- 3. EKRAN ---
    if (trybKonfig) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("TRYB KONFIGURACJI");
      display.setCursor(0, 20);
      display.println("Polacz sie z Wi-Fi:");
      display.println("SUPLA-ESP32...");
      display.setCursor(0, 45);
      display.println("IP: 192.168.4.1");
      display.display();
    }
    else if (millis() - czasPobudki < 15000) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("--- SUPLA METEO ---");
      display.setCursor(0, 20);
      display.print("Temp: ");
      display.print(temperatura);
      display.println(" C");
      display.setCursor(0, 40);
      display.print("Cisn: ");
      display.print(cisnienie);
      display.println(" hPa");
      display.display();
    }
    else if (ekranWlaczony) {
      display.clearDisplay();
      display.display();
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      ekranWlaczony = false;
    }
  }

  delay(10); // oddaje CPU na chwilę zamiast kręcić pętlą na pełnych obrotach
}
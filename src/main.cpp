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
#include <supla/control/button.h>
#include <supla/sensor/therm_press_meter.h>

// --- BIBLIOTEKI DO TRYBU KONFIGURACYJNEGO ---
#include <supla/network/esp_web_server.h>
#include <supla/network/html/custom_parameter.h>
#include <supla/network/html/device_info.h>
#include <supla/network/html/protocol_parameters.h>
#include <supla/network/html/wifi_parameters.h>

// --- PAMIĘĆ DLA ESP32 ---
#include <supla/storage/littlefs_config.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADRES 0x3C
#define BUTTON_PIN 0

#define NAZWA_URZADZENIA "SUPLA-METEO"
#define WERSJA_PROGRAMU "2.0.0"
#define PARAM_WYSOKOSC "wysokosc"

const uint32_t CZAS_WYGASZANIA_EKRANU_MS = 15000;
const uint32_t ODSWIEZANIE_EKRANU_MS = 2000;
const int ODCZYT_CZUJNIKA_MS = 30000;

const uint8_t ADRESY_CZUJNIKA[] = {0x76, 0x77};
const uint8_t ID_UKLADOW[] = {BMP280_CHIPID, 0x60 /* BME280 */};

class CzujnikBMP280 : public Supla::Sensor::ThermPressMeter {
 public:
  CzujnikBMP280() {
    setRefreshIntervalMs(ODCZYT_CZUJNIKA_MS);
  }

  void ustawWysokosc(int32_t metry) {
    wysokosc = metry;
    odczytaj();
  }

  bool czyDziala() const {
    return dziala;
  }

  void onInit() override {
    polacz();
    odczytaj();
  }

  void iterateAlways() override {
    if (millis() - lastReadTime > refreshIntervalMs) {
      lastReadTime = millis();
      odczytaj();
    }
  }

 protected:
  bool polacz() {
    for (uint8_t adres : ADRESY_CZUJNIKA) {
      for (uint8_t idUkladu : ID_UKLADOW) {
        if (bmp.begin(adres, idUkladu)) {
          this->adres = adres;
          bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                          Adafruit_BMP280::SAMPLING_X1,
                          Adafruit_BMP280::SAMPLING_X1,
                          Adafruit_BMP280::FILTER_OFF);
          Serial.printf("BMP280: znaleziono pod adresem 0x%02X\n", adres);
          dziala = true;
          return true;
        }
      }
    }
    Serial.println("BMP280: nie znaleziono czujnika (0x76 / 0x77)");
    dziala = false;
    return false;
  }

  bool czujnikOdpowiada() {
    Wire.beginTransmission(adres);
    return Wire.endTransmission() == 0;
  }

  void odczytaj() {
    double temperatura = TEMPERATURE_NOT_AVAILABLE;
    double cisnienie = PRESSURE_NOT_AVAILABLE;

    if (!dziala || !czujnikOdpowiada()) {
      polacz();
    }

    if (dziala && bmp.takeForcedMeasurement()) {
      float t = bmp.readTemperature();
      float p = bmp.readPressure() / 100.0F;
      if (!isnan(t) && !isnan(p) && p > 0) {
        temperatura = t;
        cisnienie = bmp.seaLevelForAltitude(wysokosc, p);
      }
    }

    channel.setNewValue(temperatura);
    pressureChannel.setNewValue(cisnienie);
  }

  Adafruit_BMP280 bmp;
  uint8_t adres = ADRESY_CZUJNIKA[0];
  int32_t wysokosc = 0;
  bool dziala = false;
};

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Supla::LittleFsConfig configStore;
Supla::ESPWifi wifi;
Supla::EspWebServer suplaServer;

Supla::Html::DeviceInfo htmlDeviceInfo(&SuplaDevice);
Supla::Html::WifiParameters htmlWifi;
Supla::Html::ProtocolParameters htmlProto;
// Tworzony w setup(), bo jako zmienna globalna wywala ESP32 przy starcie.
Supla::Html::CustomParameter *htmlWysokosc;

CzujnikBMP280 *czujnik;

bool ekranDziala = false;
bool ekranWlaczony = true;
bool bylTrybKonfig = false;
unsigned long czasPobudki = 0;
unsigned long ostatnieOdswiezenie = 0;
bool przyciskWcisniety = false;

bool trybKonfiguracji() {
  return SuplaDevice.getDeviceMode() == Supla::DEVICE_MODE_CONFIG;
}

const char *opisStatusuSupli() {
  switch (SuplaDevice.getCurrentStatus()) {
    case STATUS_REGISTERED_AND_READY:
      return "Supla: polaczono";
    case STATUS_NETWORK_DISCONNECTED:
      return "Brak Wi-Fi";
    case STATUS_REGISTRATION_DISABLED:
      return "Wlacz rejestracje!";
    case STATUS_CHANNEL_CONFLICT:
      return "Konflikt kanalow!";
    case STATUS_BAD_CREDENTIALS:
    case STATUS_INVALID_AUTHKEY:
      return "Zle dane logowania";
    default:
      return "Laczenie z Supla...";
  }
}

void rysujTrybKonfiguracji() {
  display.setCursor(0, 0);
  display.println("TRYB KONFIGURACJI");
  display.setCursor(0, 16);
  display.println("Polacz sie z Wi-Fi:");
  display.println(WiFi.softAPSSID());
  display.setCursor(0, 40);
  display.println("IP: 192.168.4.1");
  display.setCursor(0, 54);
  display.println("Klik BOOT = wyjscie");
}

void rysujPomiary() {
  double temperatura = czujnik->getChannel()->getValueDouble();
  double cisnienie = czujnik->getSecondaryChannel()->getValueDouble();

  display.setCursor(0, 0);
  display.println("--- SUPLA METEO ---");

  display.setCursor(0, 18);
  display.print("Temp: ");
  if (temperatura > TEMPERATURE_NOT_AVAILABLE) {
    display.print(temperatura, 1);
    display.print(' ');
    display.write(248);
    display.println("C");
  } else {
    display.println("--");
  }

  display.setCursor(0, 32);
  display.print("Cisn: ");
  if (cisnienie > 0) {
    display.print(cisnienie, 1);
    display.println(" hPa");
  } else {
    display.println("--");
  }

  display.setCursor(0, 54);
  display.println(czujnik->czyDziala() ? opisStatusuSupli() : "Blad czujnika!");
}

void rysujEkran() {
  display.clearDisplay();
  if (trybKonfiguracji()) {
    rysujTrybKonfiguracji();
  } else {
    rysujPomiary();
  }
  display.display();
}

void wlaczEkran() {
  if (!ekranWlaczony) {
    display.ssd1306_command(SSD1306_DISPLAYON);
    ekranWlaczony = true;
  }
  rysujEkran();
  ostatnieOdswiezenie = millis();
}

void wylaczEkran() {
  display.clearDisplay();
  display.display();
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  ekranWlaczony = false;
}

void setup() {
  setCpuFrequencyMhz(80);

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ekranDziala = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADRES);
  if (ekranDziala) {
    display.cp437(true);
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Start Supla...");
    display.display();
  } else {
    Serial.println("OLED: nie znaleziono ekranu (0x3C)");
  }

  htmlWysokosc = new Supla::Html::CustomParameter(PARAM_WYSOKOSC,
                                                  "Wysokość n.p.m. [m]",
                                                  0, -500, 9000);

  czujnik = new CzujnikBMP280();

  auto przyciskKonfig = new Supla::Control::Button(BUTTON_PIN, true, true);
  przyciskKonfig->configureAsConfigButton(&SuplaDevice);

  SuplaDevice.setName(NAZWA_URZADZENIA);
  SuplaDevice.setSwVersion(WERSJA_PROGRAMU);
  SuplaDevice.begin();

  czujnik->ustawWysokosc(htmlWysokosc->getParameterValue());

  WiFi.setSleep(true);

  czasPobudki = millis();
}

void loop() {
  SuplaDevice.iterate();

  if (!ekranDziala) {
    delay(10);
    return;
  }

  // --- 1. PRZYCISK: WYBUDZANIE EKRANU ---
  bool wcisniety = digitalRead(BUTTON_PIN) == LOW;
  if (wcisniety && !przyciskWcisniety) {
    czasPobudki = millis();
    wlaczEkran();
  }
  przyciskWcisniety = wcisniety;

  // --- 2. EKRAN ---
  bool konfig = trybKonfiguracji();
  if (konfig != bylTrybKonfig) {
    bylTrybKonfig = konfig;
    czasPobudki = millis();
    wlaczEkran();
  }

  if (konfig || millis() - czasPobudki < CZAS_WYGASZANIA_EKRANU_MS) {
    if (millis() - ostatnieOdswiezenie > ODSWIEZANIE_EKRANU_MS) {
      wlaczEkran();
    }
  } else if (ekranWlaczony) {
    wylaczEkran();
  }

  delay(10);  // oddaje CPU na chwilę zamiast kręcić pętlą na pełnych obrotach
}

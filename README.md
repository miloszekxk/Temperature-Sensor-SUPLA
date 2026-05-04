<div align="center">

# 🌡️ Stacja Meteo ESP32 + Supla


![Hardware](https://img.shields.io/badge/Hardware-ESP32-blue?style=for-the-badge&logo=espressif)
![IoT](https://img.shields.io/badge/IoT-Supla-brightgreen?style=for-the-badge)
![IDE](https://img.shields.io/badge/IDE-PlatformIO-orange?style=for-the-badge&logo=platformio)
![Language](https://img.shields.io/badge/Language-C++-00599C?style=for-the-badge&logo=c%2B%2B)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

> Inteligentna, bezprzewodowa stacja pogodowa mierząca temperaturę oraz ciśnienie atmosferyczne. Dane wyświetlane są na lokalnym ekranie OLED oraz przesyłane bezpośrednio do chmury **Supla**.

</div>

---

## 📌 O projekcie
Projekt wyróżnia się zastosowaniem **Trybu Konfiguracyjnego (Captive Portal)** oraz zaawansowanego systemu plików **LittleFS**. Oznacza to, że **nie musisz wpisywać haseł Wi-Fi bezpośrednio w kodzie źródłowym**. Urządzenie samo generuje tymczasową sieć po wciśnięciu przycisku, a Ty konfigurujesz je wygodnie z poziomu przeglądarki na telefonie.

## ✨ Główne funkcje
* **🌡️ Precyzyjny pomiar:** Temperatura i ciśnienie atmosferyczne dzięki czujnikowi BMP280.
* **📺 Interfejs OLED:** Czytelne wyświetlanie danych w czasie rzeczywistym.
* **🔋 Eco Mode:** Automatyczne wygaszanie ekranu po 15 sekundach (wybudzanie sprzętowym przyciskiem BOOT).
* **⚙️ Web Server:** Konfiguracja sieci Wi-Fi i parametrów chmury Supla przez smartfon.
* **💾 Pamięć Nielotna:** Bezpieczne przechowywanie danych logowania w trwałej pamięci układu ESP32.

---

## 🛠️ Wymagany sprzęt

| Element | Opis |
| :--- | :--- |
| **Mikrokontroler** | Płytka z układem **ESP32** (np. klasyczny DevKit V1) |
| **Czujnik** | Moduł **BMP280** (komunikacja I2C) |
| **Wyświetlacz** | Ekran **OLED SSD1306** 128x64px (komunikacja I2C) |
| **Zasilanie** | Standardowy przewód Micro USB / USB-C i ładowarka 5V |

---

## 🔌 Schemat połączeń (Magistrala I2C)

Zarówno ekran, jak i czujnik korzystają z magistrali I2C, co pozwala na ich równoległe podłączenie do tych samych pinów mikrokontrolera:

| Pin ESP32 | ➡️ | Moduł BMP280 | Moduł OLED |
| :---: | :---: | :---: | :---: |
| **3.3V** | ➡️ | VCC | VCC |
| **GND** | ➡️ | GND | GND |
| **GPIO 21** | ➡️ | SDA | SDA |
| **GPIO 22** | ➡️ | SCL | SCL |

---

## 🚀 Instalacja i wgrywanie kodu

### 1. Środowisko programistyczne
Projekt został przygotowany dla środowiska **Visual Studio Code** ze skonfigurowanym rozszerzeniem **PlatformIO**.

### 2. Edycja pliku konfiguracyjnego (Wymagane)
Ze względu na obecność interfejsu WWW, skompilowany kod zajmuje więcej miejsca w pamięci ESP32. Upewnij się, że w Twoim pliku `platformio.ini` znajduje się modyfikacja układu partycji:
```ini
board_build.partitions = huge_app.csv
```

### 3. Wgrywanie (Krok po kroku)
1. Podłącz ESP32 do komputera za pomocą przewodu obsługującego transmisję danych.
2. Zdecydowanie zaleca się całkowite wyczyszczenie układu przed pierwszym wgraniem, używając opcji **`Erase Flash`** w menu PlatformIO.
3. Skompiluj i wgraj oprogramowanie przyciskiem **`Upload`**.

---

## ☁️ Konfiguracja Chmury Supla (Przed uruchomieniem)

Zanim połączysz stację ze swoim domowym Wi-Fi, musisz przygotować serwer na przyjęcie nowego sprzętu.

1. Zaloguj się na swoje konto przez przeglądarkę na stronie [cloud.supla.org](https://cloud.supla.org).
2. Na stronie głównej lub w zakładce *Moja Supla* **włącz "Rejestrację urządzeń"** (odpowiedni przełącznik/suwak musi zaświecić się na zielono). **To kluczowy krok – bez tego serwer odrzuci połączenie z nową stacją!**
3. Zlokalizuj i skopiuj **Adres serwera**, do którego przypisane jest Twoje konto (np. `svr12.supla.org`). Będzie on potrzebny w kolejnym kroku.

---

## 📱 Pierwsza konfiguracja urządzenia (Bez kabli)

Gdy urządzenie ma wgrany kod i serwer Supli jest gotowy, przejdź do podłączenia stacji:

1. Włącz ESP32 do zasilania.
2. Wciśnij i przytrzymaj wbudowany przycisk **BOOT** (GPIO 0) przez ok. **5 sekund**.
3. Gdy na ekranie OLED pojawi się komunikat **"TRYB KONFIGURACJI"**, układ zacznie nadawać własną sieć Wi-Fi.
4. Na swoim smartfonie połącz się z nową, otwartą siecią o nazwie zaczynającej się od **`SUPLA-ESP32...`**
5. Otwórz przeglądarkę internetową (najlepiej w trybie Incognito / Karcie prywatnej) i wpisz adres panelu sterowania:
   👉 **`http://192.168.4.1`**
6. Wypełnij wyświetlony formularz:
   * Wybierz lub wpisz nazwę swojej **domowej sieci Wi-Fi** i podaj do niej hasło.
   * Wpisz **adres e-mail** powiązany z Twoim kontem Supla.
   * Podaj zlokalizowany wcześniej **adres serwera** (np. `svrX.supla.org`).
7. Kliknij **`Save & Restart`**. 

Gotowe! Twoja stacja zrestartuje się, połączy z siecią domową i po chwili automatycznie pojawi w aplikacji Supla na Twoim smartfonie. Od tego momentu możesz wyłączyć "Rejestrację urządzeń" w chmurze Supla.

---

## ⚠️ Rozwiązywanie problemów (Troubleshooting)

* **Nie widzę paneli do wpisania haseł pod adresem 192.168.4.1:** Upewnij się, że otwierasz stronę w trybie Incognito. Telefony często zapisują stary wygląd strony w pamięci Cache.
* **Urządzenie nie łączy się z domowym routerem po konfiguracji:** Pamiętaj, że układ ESP32 obsługuje **wyłącznie sieci Wi-Fi w standardzie 2.4 GHz**. Upewnij się w ustawieniach swojego domowego routera, że nadaje on sygnał w tym paśmie (sieci 5 GHz są dla ESP32 niewidoczne).
* **Brak miejsca podczas wgrywania kodu:** Upewnij się, że poprawnie dodałeś linijkę `board_build.partitions = huge_app.csv` do pliku `platformio.ini`.

---
## 📝 Licencja
Projekt udostępniony na licencji MIT. Możesz go dowolnie modyfikować i używać w swoich rozwiązaniach domowych.
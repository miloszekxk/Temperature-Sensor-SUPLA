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
Projekt wyróżnia się zastosowaniem **Trybu Konfiguracyjnego (Captive Portal)** oraz systemu plików **LittleFS**. Oznacza to, że **nie musisz wpisywać haseł Wi-Fi bezpośrednio w kodzie źródłowym**. Urządzenie samo generuje tymczasową sieć Wi-Fi (przy pierwszym uruchomieniu albo po przytrzymaniu przycisku), a Ty konfigurujesz je wygodnie z poziomu przeglądarki na telefonie.

## ✨ Główne funkcje
* **🌡️ Pomiar temperatury i ciśnienia:** czujnik BMP280 (lub BME280), wykrywany automatycznie pod adresem 0x76 albo 0x77.
* **🌍 Ciśnienie jak w prognozie pogody:** po podaniu wysokości n.p.m. ciśnienie jest przeliczane do poziomu morza.
* **📺 Interfejs OLED:** aktualne pomiary oraz stan połączenia z Wi-Fi i Supla.
* **🔋 Eco Mode:** ekran gaśnie po 15 sekundach (wybudzanie przyciskiem BOOT), czujnik mierzy tylko co 30 s i śpi między pomiarami, procesor pracuje z obniżonym taktowaniem (80 MHz).
* **⚙️ Web Server:** konfiguracja Wi-Fi, chmury Supla i wysokości n.p.m. przez smartfon.
* **💾 Pamięć Nielotna:** dane logowania i ustawienia przechowywane są w trwałej pamięci ESP32.
* **🛡️ Odporność na awarie:** brak ekranu nie zatrzymuje urządzenia, a odłączony czujnik jest wykrywany ponownie po podłączeniu.

---

## 🛠️ Wymagany sprzęt

| Element | Opis |
| :--- | :--- |
| **Mikrokontroler** | Płytka z układem **ESP32** (np. klasyczny DevKit V1) |
| **Czujnik** | Moduł **BMP280** lub **BME280** (komunikacja I2C, adres 0x76 lub 0x77) |
| **Wyświetlacz** | Ekran **OLED SSD1306** 128x64px (komunikacja I2C, adres 0x3C) |
| **Zasilanie** | Standardowy przewód Micro USB / USB-C i ładowarka 5V |

> 💡 **Dokładność pomiaru:** BMP280 mierzy temperaturę z dokładnością ok. ±1 °C. Pracujący ESP32 się nagrzewa, więc umieść czujnik z dala od płytki (np. na kilkunastocentymetrowym przewodzie). Ewentualne przesunięcie temperatury można skorygować w ustawieniach kanału w chmurze Supla.

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
Projekt został przygotowany dla środowiska **Visual Studio Code** ze skonfigurowanym rozszerzeniem **PlatformIO**. Wszystkie biblioteki (w tym [supla-device](https://github.com/SUPLA/supla-device) w wersji **26.9.2**) PlatformIO pobierze samo przy pierwszej kompilacji.

### 2. Plik konfiguracyjny `platformio.ini`
Plik jest już gotowy, nie trzeba go zmieniać. Warto wiedzieć, co w nim jest:
* `board_build.partitions = huge_app.csv` – większa partycja na program, bo interfejs WWW zajmuje sporo miejsca.
* **Przypięte wersje** platformy i bibliotek – dzięki temu projekt za rok skompiluje się tak samo jak dziś. Aktualizację bibliotek najlepiej robić świadomie, zmieniając numer wersji.

### 3. Wgrywanie (Krok po kroku)
1. Podłącz ESP32 do komputera za pomocą przewodu obsługującego transmisję danych.
2. **Tylko przy pierwszym wgraniu na nową płytkę** zaleca się wyczyszczenie układu opcją **`Erase Flash`** w menu PlatformIO.
   > ⚠️ `Erase Flash` kasuje też konfigurację i identyfikator urządzenia w Supli – po nim stacja pojawi się w chmurze jako nowe urządzenie. Przy aktualizacji działającej stacji pomiń ten krok.
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

1. Włącz ESP32 do zasilania. Przy pierwszym uruchomieniu (brak konfiguracji) **tryb konfiguracji włączy się sam**. Później możesz go włączyć, przytrzymując przycisk **BOOT** (GPIO 0) przez ok. **5 sekund**.
2. Na ekranie OLED pojawi się komunikat **"TRYB KONFIGURACJI"** wraz z nazwą sieci Wi-Fi, którą nadaje stacja.
3. Na swoim smartfonie połącz się z tą otwartą siecią – jej nazwa zaczyna się od **`SUPLA-METEO-`**.
4. Otwórz przeglądarkę internetową (najlepiej w trybie Incognito / Karcie prywatnej) i wpisz adres panelu sterowania:
   👉 **`http://192.168.4.1`**
5. Wypełnij wyświetlony formularz:
   * Wybierz z listy lub wpisz nazwę swojej **domowej sieci Wi-Fi** i podaj do niej hasło.
   * Wpisz **adres e-mail** powiązany z Twoim kontem Supla.
   * Podaj zlokalizowany wcześniej **adres serwera** (np. `svrX.supla.org`).
   * Wpisz **wysokość n.p.m.** miejsca, w którym stoi stacja (w metrach, można ją sprawdzić np. w Mapach Google lub na geoportal.gov.pl). Zostaw `0`, jeśli chcesz ciśnienie bezwzględne, bez przeliczania.
6. Kliknij **`Save & Restart`**.

Gotowe! Twoja stacja zrestartuje się, połączy z siecią domową i po chwili automatycznie pojawi w aplikacji Supla na Twoim smartfonie. Od tego momentu możesz wyłączyć "Rejestrację urządzeń" w chmurze Supla.

> 🔒 **Bezpieczeństwo:** sieć konfiguracyjna jest otwarta (bez hasła). Działa tylko wtedy, gdy stacja jest w trybie konfiguracji, więc po skończonej konfiguracji nie zostawiaj jej w tym trybie.

### Co pojawi się w Supli

| Kanał | Typ | Jednostka |
| :---: | :--- | :--- |
| **0** | Termometr | °C |
| **1** | Czujnik ciśnienia | hPa |

---

## 🔘 Obsługa przycisku BOOT

| Gdy stacja… | Akcja | Efekt |
| :--- | :--- | :--- |
| działa normalnie | krótkie naciśnięcie | wybudza ekran na 15 s |
| działa normalnie | przytrzymanie ~5 s | wejście w tryb konfiguracji |
| jest w trybie konfiguracji | krótkie naciśnięcie | wyjście z trybu konfiguracji i restart |
| jest w trybie konfiguracji | przytrzymanie ~5 s | ⚠️ **reset do ustawień fabrycznych** (kasuje Wi-Fi i dane Supli) |

---

## 🔄 Aktualizacja z wersji 1.x

Wersja 2.0 zmienia typ kanału ciśnienia z „pomiaru ogólnego” na właściwy **czujnik ciśnienia** (z jednostką hPa i przeliczaniem do poziomu morza). Po wgraniu nowej wersji:

1. Na ekranie może pojawić się **"Konflikt kanalow!"** – chmura Supla widzi, że kanał nr 1 zmienił typ.
2. Wejdź na [cloud.supla.org](https://cloud.supla.org), otwórz urządzenie i potwierdź zmianę kanału zgodnie z komunikatem w chmurze. Historia starego kanału ciśnienia nie przejdzie na nowy kanał. Kanał temperatury się nie zmienia.
3. Przy pierwszym wejściu w tryb konfiguracji uzupełnij **wysokość n.p.m.**
4. Nazwa sieci konfiguracyjnej zmieniła się z `SUPLA-ESP32-…` na `SUPLA-METEO-…`.

Poprzednia wersja jest zachowana w repozytorium na gałęzi `oszczedzanie-energii`.

---

## ⚠️ Rozwiązywanie problemów (Troubleshooting)

* **Nie widzę paneli do wpisania haseł pod adresem 192.168.4.1:** Upewnij się, że otwierasz stronę w trybie Incognito. Telefony często zapisują stary wygląd strony w pamięci Cache.
* **Urządzenie nie łączy się z domowym routerem po konfiguracji:** Pamiętaj, że układ ESP32 obsługuje **wyłącznie sieci Wi-Fi w standardzie 2.4 GHz**. Upewnij się w ustawieniach swojego domowego routera, że nadaje on sygnał w tym paśmie (sieci 5 GHz są dla ESP32 niewidoczne).
* **Na ekranie "Blad czujnika!" i "--" zamiast pomiarów:** sprawdź przewody SDA/SCL i zasilanie czujnika. Stacja sama spróbuje go wykryć ponownie (co 30 s). W monitorze portu szeregowego (115200) zobaczysz, czy i pod jakim adresem czujnik został znaleziony.
* **Na ekranie "Wlacz rejestracje!":** włącz "Rejestrację urządzeń" w chmurze Supla (patrz wyżej).
* **Na ekranie "Konflikt kanalow!":** patrz sekcja [Aktualizacja z wersji 1.x](#-aktualizacja-z-wersji-1x).
* **Ekran nic nie pokazuje:** sprawdź, czy ekran ma adres 0x3C. Stacja działa i wysyła pomiary do Supli także bez ekranu.
* **Wgrywanie kończy się błędem połączenia:** niektóre płytki nie radzą sobie z szybkim wgrywaniem. Zmień w `platformio.ini` wartość `upload_speed` z `921600` na `115200`.

---
## 📝 Licencja
Projekt udostępniony na licencji MIT – możesz go dowolnie używać, modyfikować i rozpowszechniać, także w zastosowaniach komercyjnych, pod warunkiem zachowania informacji o autorze i licencji. Szczegóły w pliku [LICENSE](LICENSE).

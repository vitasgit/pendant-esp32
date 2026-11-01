/*
 * Тест платы 01Space ESP32-C3FH4-RGB (5x5 матрица)
 * -------------------------------------------------
 * Матрица: 25 шт. WS2812B (5x5), подключена к GPIO8
 * Порядок цветов: GRB
 * Нумерация: 0..24, построчно; USB-C сверху -> 0 = верхний левый,
 *            24 = нижний правый (проверено сообществом fivebyfive)
 *
 * Настройки Arduino IDE:
 *   Board:  "ESP32C3 Dev Module" (ядро esp32 3.3.8 уже установлено)
 *   Tools -> USB CDC On Boot: "Enabled"   (обязательно!)
 *   Остальное: по умолчанию
 *
 * Первая прошивка (если авто-сброс не сработает):
 *   зажать B (BOOT) -> нажать и отпустить R (RESET) -> отпустить B -> Upload
 *
 * Библиотека: FastLED 3.10.3 (уже установлена; на ESP32 использует RMT-драйвер)
 */
#include <WiFi.h>
#include <FastLED.h>

#define DATA_PIN     8
#define NUM_LEDS     25
#define LED_TYPE     WS2812B
#define COLOR_ORDER  GRB

CRGB leds[NUM_LEDS];

inline uint8_t xy(uint8_t x, uint8_t y) { return y * 5 + x; }

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-C3FH4-RGB: test start");

  // Экономия батареи: радио ОБЯЗАТЕЛЬНО выключить
  // (с включённым WiFi плата ест 80-350 мА вместо ~20 мА)
  WiFi.mode(WIFI_OFF);
  btStop();
  setCpuFrequencyMhz(80);          // 160 -> 80 МГц, ещё ~10 мА экономии; RMT не пострадает

  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(20);                       // 0-255, тестовой яркости хватает
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 100);  // жёсткий лимит тока матрицы
  FastLED.clear(true);
}

void loop() {
  // ТЕСТ 1: бегущий пиксель 0..24
  // Главная проверка после истории с бракованной платой: гореть должны ВСЕ 25!
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    FastLED.clear();
    leds[i] = CHSV(i * 10, 255, 255);
    FastLED.show();
    delay(150);
  }

  // ТЕСТ 2: заливка R / G / B — проверка порядка цветов
  // (если вместо красного горит зелёный — сменить COLOR_ORDER на RGB)
  const CRGB primaries[] = {CRGB::Red, CRGB::Green, CRGB::Blue};
  for (auto c : primaries) {
    fill_solid(leds, NUM_LEDS, c);
    FastLED.show();
    delay(500);
  }
  FastLED.clear(true);

  // ТЕСТ 3: углы + центр — проверка ориентации матрицы
  leds[xy(0, 0)] = CRGB::White;    // верх-лево
  leds[xy(4, 0)] = CRGB::Red;      // верх-право
  leds[xy(0, 4)] = CRGB::Green;    // низ-лево
  leds[xy(4, 4)] = CRGB::Blue;     // низ-право
  leds[xy(2, 2)] = CRGB::Purple;   // центр
  FastLED.show();
  delay(1500);
}

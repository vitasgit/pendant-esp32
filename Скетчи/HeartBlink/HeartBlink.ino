/*
 * Кулон: СЕРДЦЕ с пульсацией + максимум энергосбережения
 * Плата: 01Space ESP32-C3FH4-RGB (матрица 5x5 WS2812B на GPIO8, GRB)
 * IDE:   "ESP32C3 Dev Module", USB CDC On Boot: Enabled
 *
 * Энергопрофиль:
 *   - WiFi и Bluetooth выключены полностью (главные пожиратели)
 *   - CPU 80 МГц вместо 160 МГц
 *   - между кадрами — light sleep: ~5 мА вместо ~25 мА (экономия ~3x)
 *   - в паузе сердце «тлеет» на 18% — читается в темноте, почти не ест
 *
 * ВАЖНО про отладку: с USE_SLEEP=true плата во время сна отключает USB —
 * порт /dev/ttyACM0 будет periodically пропадать, а esptool может выдать
 * "Protocol error". Варианты:
 *   1) на время разработки поставь USE_SLEEP false, облучи код, потом true;
 *   2) при ошибке загрузки: зажать B -> тапнуть R -> отпустить B -> Upload.
 */
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include "esp_sleep.h"

#define LED_PIN    8            // матрица на этой плате — GPIO8 (не 10!)
#define NUM_LEDS   25
#define USE_SLEEP  true
#define PEAK       12           // макс. яркость красного 0..255.
// 40 — видно при дневном свете;
// 12-15 — достаточно для тёмного кулона (ещё ~3x экономии)

Adafruit_NeoPixel px(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* HEART[5] = {
  ".#.#.",
  "#####",
  "#####",
  ".###.",
  "..#.."
};

static void idle(uint32_t ms) {
  if (USE_SLEEP) {
    esp_sleep_enable_timer_wakeup((uint64_t)ms * 1000);
    esp_light_sleep_start();    // USB на это время отваливается — это нормально
  } else {
    delay(ms);
  }
}

static void drawHeart(uint8_t level) {          // level 0..100
  uint8_t r = (uint16_t)PEAK * level / 100;
  uint32_t c = px.Color(r, 0, r / 6);           // лёгкий холодный подтон, «глиняное» сердце
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      px.setPixelColor(y * 5 + x, HEART[y][x] == '#' ? c : 0);
    }
  }
  px.show();
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);          // выключить WiFi (включён по умолчанию не бывает,
  // но после wake перестраховка стоит копейки)
  btStop();                     // Bluetooth HAL — если когда-то использовался, точно вырубим
  setCpuFrequencyMhz(80);

  px.begin();
  px.clear();
  px.show();
  Serial.println("heart: alive");
}

void loop() {
  // двойной удар «тум-тум»: быстрый разгон яркости, как настоящее сердце
  const uint8_t beat[] = {50, 100, 40, 85, 25};
  for (uint8_t i = 0; i < sizeof(beat); i++) {
    drawHeart(beat[i]);
    idle(90);                   // 90 мс между импульсами — в сне
  }
  drawHeart(18);                // тлеющая поддержка ~1300 мс (85% цикла — сон)
  idle(1300);
}

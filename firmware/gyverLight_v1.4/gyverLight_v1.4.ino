/*
  Скетч к проекту "Эффектный светильник"
*/

/*
   Управление кнопкой/сенсором
  - 1х тап - вкл. воспроизведения голосового сообщения. Смена режима работы световых индикаторов 
*/

// ************************** НАСТРОЙКИ ***********************
#define CURRENT_LIMIT 2000 // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define AUTOPLAY_TIME 10   // время проигрывания голосового сообщения
#define SPEED_EFFECT 10    // Время отображения секущего эффекта

#define NUM_LEDS 9   // количество светодиодов в одном отрезке ленты
#define NUM_STRIPS 3 // количество отрезков ленты (в параллели)
#define LED_PIN 6    // пин ленты
#define BTN_PIN 2    // пин кнопки/сенсора

#define IDS1820_PIN 5 // пин isd1820

#define BRIGHTNESS 255 // яркость

#include "GyverButton.h"
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);

#include <FastLED.h>
CRGB leds[NUM_LEDS];
CRGBPalette16 gPal;

#include "GyverTimer.h"
GTimer_ms autoplayTimer((long)AUTOPLAY_TIME * 1000);

static int brightness = BRIGHTNESS;
static boolean autoplay = true;

enum ModeWork
{
    Static = 1,
    Dynamic
};

static ModeWork modeWork = Static;

// Задать всем светодиодам один цвет
void fillAll(CRGB newcolor)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = newcolor;
    }
}

// ****************************** ЦВЕТА ******************************
void effectColors()
{
    static byte hue = 0;
    hue += 2;
    CRGB thisColor = CHSV(hue, 255, 255);
    fillAll(CHSV(hue, 255, 255));
}

// ****************************** РАДУГА ******************************
void effectRainbow()
{
    static byte hue = 0;
    hue += 2;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CHSV((byte)(hue + i * float(255 / NUM_LEDS)), 255, 255);
    }
}

void setup()
{
    Serial.begin(9600);
    pinMode(IDS1820_PIN, OUTPUT);
    FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    if (CURRENT_LIMIT > 0)
    {
        FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
    }
    FastLED.setBrightness(brightness);
    FastLED.show();

    touch.setTimeout(300);
    touch.setStepTimeout(50);
}

void loop()
{
    touch.tick();

    // Управление режимами
    if (touch.hasClicks())
    {
        // Включаем воспроизведение голосовой записи
        digitalWrite(IDS1820_PIN, HIGH);
        // Переходим в измененный режим работы светодиодов
        modeWork = Dynamic;
        FastLED.show();
    }

    switch (modeWork)
    {
    case Static:
        effectColors();
        break;

    case Dynamic:
        effectRainbow();
        break;

    default:
        break;
    }

    if (autoplayTimer.isReady() && autoplay)
    {
        modeWork = Static;
        digitalWrite(IDS1820_PIN, LOW);
    }
}

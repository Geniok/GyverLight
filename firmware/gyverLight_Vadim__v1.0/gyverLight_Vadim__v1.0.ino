/*
  Скетч к проекту "Эффектный светильник" переработанный согласно пожеланиям Вадима
*/

/*
   Управление кнопкой/сенсором
  - 1х тап - вкл. воспроизведения голосового сообщения. Смена режима работы световых индикаторов 
*/

#include "GyverButton.h"
#include <FastLED.h>
#include "GyverTimer.h"

// ************************** НАСТРОЙКИ ***********************
#define CURRENT_LIMIT 2000 // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define PLAY_TIME 10       // время проигрывания голосового сообщения (в сек.)
#define EFFECT_1_SPEED 20  // Скорость отображения эффекта  1 (20 ед. примерно соотвествует времени в 5 сек.)
#define EFFECT_2_SPEED 40  // Скорость отображения эффекта  2 (40 ед. примерно соотвествует времени в 10 сек.)

#define NUM_LEDS 9   // количество светодиодов в одном отрезке ленты
#define NUM_STRIPS 3 // количество отрезков ленты (в параллели)

#define LED_PIN 6     // пин ленты
#define BTN_PIN 2     // пин кнопки/сенсора
#define IDS1820_PIN 5 // пин isd1820

static GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
static CRGB leds[NUM_LEDS];
static CRGBPalette16 gPal;

static GTimer_ms playTimer((uint32_t)PLAY_TIME * 1000);
static GTimer_ms effectTimer((uint32_t)EFFECT_1_SPEED);

enum ModeWork
{
    Static = 1,
    Dynamic
};

static int brightness = 255; // яркость свечения светодиодов
static bool play = false;
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
    hue += 1;
    CRGB thisColor = CHSV(hue, 255, 255);
    fillAll(thisColor);
}

// ****************************** РАДУГА ******************************
void effectRainbow()
{
    static byte hue = 0;
    hue += 1;
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = CHSV((byte)(hue + i * float(255 / NUM_LEDS)), 255, 255);
    }
}

void setup()
{
    Serial.begin(9600);
    pinMode(IDS1820_PIN, OUTPUT);
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    if (CURRENT_LIMIT > 0)
    {
        FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
    }
    FastLED.setBrightness(brightness);
    FastLED.show();

    touch.setTimeout(300);
    touch.setStepTimeout(50);

    effectTimer.setInterval(EFFECT_1_SPEED);
    effectTimer.start();
}

void loop()
{
    touch.tick();

    // Управление режимами
    if ((touch.hasClicks()) && (!play))
    {
        // Включаем воспроизведение голосовой записи
        digitalWrite(IDS1820_PIN, HIGH);
        // Переходим в измененный режим работы светодиодов
        modeWork = Dynamic;
        effectTimer.setInterval(EFFECT_2_SPEED);
        playTimer.start();
        play = true;
    }

    if (effectTimer.isReady())
    {
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
    }

    if (playTimer.isReady())
    {
        playTimer.stop();
        effectTimer.setInterval(EFFECT_1_SPEED);
        modeWork = Static;
        digitalWrite(IDS1820_PIN, LOW);
        play = false;
    }
    FastLED.show();
}

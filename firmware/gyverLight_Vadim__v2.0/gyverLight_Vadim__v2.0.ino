/*
  Скетч к проекту "Светящаяся лампа" версия 2.0
*/

/*
   Управление кнопкой/сенсором
  - 1-й тап - вкл. запуск эффекта
  - 2-й тап - запуск следующего эффекта
*/

#include "GyverButton.h"
#include <FastLED.h>
#include "GyverTimer.h"

// ************************** НАСТРОЙКИ ***********************
#define CURRENT_LIMIT 2000 // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит
#define LIGHT_TIME 60      // время до выключения, если не было касания (60 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_1_SPEED 240 // Скорость отображения эффекта  1 (240 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_2_SPEED 240 // Скорость отображения эффекта  2 (240 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_3_SPEED 240 // Скорость отображения эффекта  3 (240 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_4_SPEED 240 // Скорость отображения эффекта  4 (240 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_5_SPEED 240 // Скорость отображения эффекта  5 (240 ед. примерно соотвествует времени в 60 сек.)
#define EFFECT_6_SPEED 240 // Скорость отображения эффекта  6 (240 ед. примерно соотвествует времени в 60 сек.)

#define TRACK_STEP 50

#define NUM_LEDS 9   // количество светодиодов в одном отрезке ленты
#define NUM_STRIPS 3 // количество отрезков ленты (в параллели)

#define LED_PIN 6 // пин ленты
#define BTN_PIN 2 // пин кнопки/сенсора

static GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);
static CRGB leds[NUM_LEDS];
static CRGBPalette16 gPal;

static GTimer_ms lightTimer((uint32_t)LIGHT_TIME * 1000);
static GTimer_ms effectTimer((uint32_t)EFFECT_1_SPEED);

static uint8_t brightness = 255; // яркость свечения светодиодов
static uint8_t numberEffects = 0;
static const uint8_t countEffects = 6;
static boolean powerActive = false;

// Задать всем светодиодам один цвет
void fillAll(CRGB newcolor)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i] = newcolor;
    }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(int thisPixel)
{
    return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8) | (long)leds[thisPixel].b);
}

void fade()
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        if ((uint32_t)getPixColor(i) == 0)
        {
            continue;
        }
        leds[i].fadeToBlackBy(TRACK_STEP);
    }
}

// ****************************** ОГОНЁК ******************************
void effectLighter()
{
    static int16_t position = 0;
    static boolean direction = false;

    FastLED.clear();
    if (direction)
    {
        position++;
        if (position > NUM_LEDS - 2)
        {
            direction = false;
        }
    }
    else
    {
        position--;
        if (position < 1)
        {
            direction = true;
        }
    }
    leds[position] = CRGB::White;
}

// ****************************** СВЕТЛЯЧКИ ******************************
void effectLightBugs()
{
    static const uint8_t maxSpeed = 30;
    static const uint8_t bugsAmount = 3;
    static int16_t speed[bugsAmount];
    static int16_t pos[bugsAmount];
    static CRGB bugColors[bugsAmount];
    static boolean loadingFlag = true;

    if (loadingFlag)
    {
        loadingFlag = false;
        for (int i = 0; i < bugsAmount; i++)
        {
            bugColors[i] = CHSV(random(0, 9) * 28, 255, 255);
            pos[i] = random(0, NUM_LEDS);
            speed[i] += random(-5, 6);
        }
    }
    FastLED.clear();
    for (int i = 0; i < bugsAmount; i++)
    {
        speed[i] += random(-5, 6);
        if (speed[i] == 0)
        {
            speed[i] += (-5, 6);
        }
        else if (abs(speed[i]) > maxSpeed)
        {
            speed[i] = 0;
        }

        pos[i] += speed[i] / 10;

        if (pos[i] < 0)
        {
            pos[i] = 0;
            speed[i] = -speed[i];
        }
        else if (pos[i] > NUM_LEDS - 1)
        {
            pos[i] = NUM_LEDS - 1;
            speed[i] = -speed[i];
        }

        leds[pos[i]] = bugColors[i];
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

// ****************************** КОНФЕТТИ ******************************
void effectSparkles()
{
    byte thisNum = random(0, NUM_LEDS);
    if (getPixColor(thisNum) == 0)
    {
        leds[thisNum] = CHSV(random(0, 255), 255, 255);
    }
    fade();
}

// ****************************** ОГОНЬ ******************************
void effectFire()
{
    random16_add_entropy(random());
    Fire2012WithPalette();
}

void Fire2012WithPalette()
{
    static uint8_t COOLING = 55;
    static uint8_t SPARKING = 120;
    // Array of temperature readings at each simulation cell
    static byte heat[NUM_LEDS];
    static bool gReverseDirection = false;

    // Step 1.  Cool down every cell a little
    for (int i = 0; i < NUM_LEDS; i++)
    {
        heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (int k = NUM_LEDS - 1; k >= 2; k--)
    {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if (random8() < SPARKING)
    {
        int y = random8(7);
        heat[y] = qadd8(heat[y], random8(160, 255));
    }

    // Step 4.  Map from heat cells to LED colors
    for (int j = 0; j < NUM_LEDS; j++)
    {
        // Scale the heat value from 0-255 down to 0-240
        // for best results with color palettes.
        byte colorindex = scale8(heat[j], 240);
        CRGB color = ColorFromPalette(gPal, colorindex);
        int pixelnumber;
        if (gReverseDirection)
        {
            pixelnumber = (NUM_LEDS - 1) - j;
        }
        else
        {
            pixelnumber = j;
        }
        leds[pixelnumber] = color;
    }
}

void setup()
{
    Serial.begin(9600);
    FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    if (CURRENT_LIMIT > 0)
    {
        FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT / NUM_STRIPS);
    }
    FastLED.setBrightness(brightness);
    FastLED.show();

    touch.setTimeout(300);

    lightTimer.reset();
    lightTimer.stop();
}

void loop()
{
    touch.tick();

    // Управление режимами
    if (touch.isPress() && !powerActive)
    {
        powerActive = true;
        nextEffect();
    }
    else if (touch.isPress() && powerActive)
    {
        nextEffect();
    }

    if (powerActive)
    {
        if (effectTimer.isReady())
        {
            switch (numberEffects)
            {
            case 1:
                effectLighter();
                break;
            case 2:
                effectLightBugs();
                break;
            case 3:
                effectColors();
                break;
            case 4:
                effectRainbow();
                break;
            case 5:
                effectSparkles();
                break;
            case 6:
                effectFire();
                break;
            }
        }

        FastLED.show();
    }
    else
    {
        // FastLED.clear(true);
        fillAll(CRGB::Black);
    }

    if (lightTimer.isReady())
    {
        lightTimer.stop();
        effectTimer.stop();
        powerActive = false;
        numberEffects = 0;
    }
}

void nextEffect()
{
    numberEffects++;
    if (numberEffects > countEffects)
    {
        numberEffects = 1;
    }

    switch (numberEffects)
    {
    case 1:
        effectTimer.setInterval(EFFECT_1_SPEED);
        break;
    case 2:
        effectTimer.setInterval(EFFECT_2_SPEED);
        break;
    case 3:
        effectTimer.setInterval(EFFECT_3_SPEED);
        break;
    case 4:
        effectTimer.setInterval(EFFECT_4_SPEED);
        break;
    case 5:
        effectTimer.setInterval(EFFECT_5_SPEED);
        break;
    case 6:
        effectTimer.setInterval(EFFECT_6_SPEED);
        break;
    }

    effectTimer.reset();
    effectTimer.start();
    lightTimer.reset();
    lightTimer.start();
    FastLED.clear();
}

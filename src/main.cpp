#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <esp_pthread.h>

#include <atomic>
#include <sstream>
#include <thread>

#define TEXT_HEIGHT 16
#define CNUMBER 40

// Structure to hold circle plotting parameters
typedef struct circle_t
{
    int16_t cx[CNUMBER] = {0};    // x coordinate of centre
    int16_t cy[CNUMBER] = {0};    // y coordinate of centre
    int16_t cr[CNUMBER] = {0};    // radius
    uint16_t col[CNUMBER] = {0};  // colour
    int16_t dx[CNUMBER] = {0};    // x movement & direction
    int16_t dy[CNUMBER] = {0};    // y movement & direction
} circle_param;

void drawUpdate(TFT_eSprite& spr, uint16_t sel, circle_t& circle)
{
    spr.fillSprite(TFT_BLACK);
    for (uint16_t i = 0; i < CNUMBER; i++)
    {
        // Draw (Note sprite 1 datum was moved, so coordinates do not need to be
        // adjusted
        spr.fillCircle(circle.cx[i], circle.cy[i], circle.cr[i], circle.col[i]);
        spr.drawCircle(circle.cx[i], circle.cy[i], circle.cr[i], TFT_WHITE);
        spr.setTextColor(TFT_BLACK, circle.col[i]);
        spr.drawNumber(i + 1, 1 + circle.cx[i], circle.cy[i], 2);
    }

    // Update circle positions after bottom half has been drawn
    if (sel == 1)
    {
        for (uint16_t i = 0; i < CNUMBER; i++)
        {
            circle.cx[i] += circle.dx[i];
            circle.cy[i] += circle.dy[i];
            if (circle.cx[i] <= circle.cr[i])
            {
                circle.cx[i] = circle.cr[i];
                circle.dx[i] = -circle.dx[i];
            }
            else if (circle.cx[i] + circle.cr[i] >= TFT_WIDTH - 1)
            {
                circle.cx[i] = TFT_WIDTH - circle.cr[i] - 1;
                circle.dx[i] = -circle.dx[i];
            }
            if (circle.cy[i] <= circle.cr[i])
            {
                circle.cy[i] = circle.cr[i];
                circle.dy[i] = -circle.dy[i];
            }
            else if (circle.cy[i] + circle.cr[i] >= TFT_HEIGHT - 1)
            {
                circle.cy[i] = TFT_HEIGHT - circle.cr[i] - 1;
                circle.dy[i] = -circle.dy[i];
            }
        }
    }
}

#ifdef HC_SR04
const int TRIG = 22;
const int ECHO = 21;
std::atomic_uint32_t duration;

void ultrasound_meter()
{
    for (;;)
    {
        digitalWrite(TRIG, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG, LOW);
        duration = pulseIn(ECHO, HIGH);
        delayMicroseconds(100);
    }
}
#endif  // HC_SR04

extern "C" void app_main()
{
    initArduino();
    Serial.begin(115200);

#ifdef HC_SR04
    pinMode(ECHO, INPUT);
    pinMode(TRIG, OUTPUT);
    auto cfg = esp_pthread_get_default_config();
    cfg.pin_to_core = 1;
    esp_pthread_set_cfg(&cfg);
    std::thread thread1(ultrasound_meter);
#endif  // HC_SR04

    TFT_eSPI tft = TFT_eSPI();
    TFT_eSprite spr[2] = {TFT_eSprite(&tft), TFT_eSprite(&tft)};
    uint16_t* sprPtr[2];
    uint16_t counter = 0;
    unsigned long startMillis = millis();
    uint16_t interval = 100;
    String fps = "xx.xx fps";

    circle_t circle;
    tft.init();
    tft.initDMA();
    tft.setRotation(0);
    tft.setSwapBytes(false);
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    sprPtr[0] = (uint16_t*)spr[0].createSprite(TFT_WIDTH, TFT_HEIGHT / 2 - 8);
    sprPtr[1] = (uint16_t*)spr[1].createSprite(TFT_WIDTH, TFT_HEIGHT / 2 - 8);
    spr[1].setViewport(0, -TFT_HEIGHT / 2 + 8, TFT_WIDTH, TFT_HEIGHT - 16);
    spr[0].setTextDatum(MC_DATUM);
    spr[1].setTextDatum(MC_DATUM);

    randomSeed(analogRead(A0));

    for (uint16_t i = 0; i < CNUMBER; i++)
    {
        circle.cr[i] = random(12, 24);
        circle.cx[i] = random(circle.cr[i], TFT_WIDTH - circle.cr[i]);
        circle.cy[i] =
            random(circle.cr[i] + 16, TFT_HEIGHT - circle.cr[i] - 16);

        circle.col[i] = 0xaaaa;
        circle.dx[i] = random(1, 5);
        if (random(2)) circle.dx[i] = -circle.dx[i];
        circle.dy[i] = random(1, 5);
        if (random(2)) circle.dy[i] = -circle.dy[i];
    }
    tft.startWrite();
    startMillis = millis();
    for (;;)
    {
        drawUpdate(spr[0], 0, circle);
#ifdef HC_SR04
        uint32_t distance = duration.load();
        distance = double(distance) * 0.01715;
        std::stringstream ss;
        ss << " Distance: " << distance;

        spr[0].setTextColor(TFT_BLUE, TFT_WHITE, false);
        spr[0].setTextDatum(BC_DATUM);
        spr[0].drawString(ss.str().c_str(), spr[0].width() / 2, spr[0].height(),
                          4);
#endif  // HC_SR04
        tft.pushImageDMA(0, 16, TFT_WIDTH, TFT_HEIGHT / 2 - 8, sprPtr[0]);
        drawUpdate(spr[1], 1, circle);
        tft.pushImageDMA(0, TFT_HEIGHT / 2 + 8, TFT_WIDTH, TFT_HEIGHT / 2 - 8,
                         sprPtr[1]);
        counter++;
        if (counter == interval)
        {
            unsigned long millisSinceUpdate = millis() - startMillis;
            fps = String((interval * 1000.0 / (millisSinceUpdate))) + " fps";
            startMillis = millis();
            tft.drawCentreString(" Serial Terminal - " + fps, 120, 0, 2);
            counter = 1;
        }
    }
}

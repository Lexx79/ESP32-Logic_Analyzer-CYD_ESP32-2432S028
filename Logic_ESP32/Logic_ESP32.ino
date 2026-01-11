// ----------------- Библиотеки -----------------
#include <SPI.h>
#include <TFT_eSPI.h>

// ----------------- Настройки -----------------
#define CHANNEL_1_PIN 22
#define CHANNEL_2_PIN 27 // Изменено пользователем на GPIO 27 для CN1
#define CAPTURE_BUFFER_SIZE 2048
#define WAVE_START_X 50

// =======================================================================
// НОРМАЛЬНАЯ ВЕРСИЯ
// =======================================================================

// ----------------- Глобальные объекты и переменные -----------------
TFT_eSPI tft = TFT_eSPI();
uint16_t calData[5] = { 320, 3424, 356, 3385, 1 };

byte data_ch1[CAPTURE_BUFFER_SIZE];
byte data_ch2[CAPTURE_BUFFER_SIZE];

enum TrigMode { T_AUTO, T_FALL, T_RISE, T_SINGLE };
TrigMode current_trig_mode = T_SINGLE;

bool hold = true;

int time_scale_idx = 5;
const int time_delays[] = {0, 1, 5, 10, 25, 50, 100, 200, 500, 1000, 2000, 4000};

int view_offset = 0;
const float display_divisors[] = {0.125, 0.25, 0.5, 1.0, 2.0, 4.0, 8.0};
int current_display_idx = 3;

const int MAIN_BTN_Y = 200;
const int SHIFT_BTN_Y = 155;
const int BTN_W = 70;
const int BTN_H = 35;
const int BTN_X1 = 10, BTN_X2 = 90, BTN_X3 = 170, BTN_X4 = 250;
unsigned long last_touch_time = 0;

// ----------------- Информация о приложении -----------------
#define APP_NAME "Logic Analyzer"
#define APP_VERSION "1.2"
#define APP_AUTHOR "Gemini"

// ----------------- Константы для экрана -----------------
const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 240;

// =======================================================================
// ЗАСТАВКА
// =======================================================================
void showSplashScreen() {
  tft.fillScreen(TFT_BLACK);
  
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(4);
  tft.drawString(APP_NAME, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 10);
  
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("v" + String(APP_VERSION), SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 25);

  tft.setTextDatum(BC_DATUM);
  tft.drawString("by " + String(APP_AUTHOR), SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10);

  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(1);
}

// =======================================================================
// SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  pinMode(CHANNEL_1_PIN, INPUT_PULLUP);
  pinMode(CHANNEL_2_PIN, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.setTouch(calData);

  // --- Показываем заставку ---
  showSplashScreen();
  delay(2500);
  
  updateUI();
  drawData();
}

// =======================================================================
// LOOP
// =======================================================================
void loop() {
  checkTouch();

  if (!hold) {
    if (current_trig_mode == T_AUTO) {
      readData(320);
      view_offset = 0;
      current_display_idx = 3;
      drawData();
    } else { // FALL или RISE
        if(waitForTrigger(100000)) {
            readData(320);
            view_offset = 0;
            current_display_idx = 3;
            drawData();
        }
    }
  }
  delay(10);
}

// =======================================================================
// ОСНОВНЫЕ ФУНКЦИИ
// =======================================================================

bool waitForTrigger(unsigned long timeout_us) {
  unsigned long startTime = micros();
  
  TrigMode effective_trig = current_trig_mode;
  if (current_trig_mode == T_SINGLE) {
      effective_trig = T_FALL;
  }

  while (micros() - startTime < timeout_us) {
    bool state1 = digitalRead(CHANNEL_1_PIN);
    bool state2 = digitalRead(CHANNEL_2_PIN);
    delayMicroseconds(1);
    bool nextState1 = digitalRead(CHANNEL_1_PIN);
    bool nextState2 = digitalRead(CHANNEL_2_PIN);

    if (effective_trig == T_RISE) {
        if ((!state1 && nextState1) || (!state2 && nextState2)) return true;
    }
    if (effective_trig == T_FALL) {
        if ((state1 && !nextState1) || (state2 && !nextState2)) return true;
    }
  }
  return false;
}


void readData(int samples) {
  int delay_us = time_delays[time_scale_idx];
  int start_idx = 0;
  if (current_trig_mode == T_AUTO) {
    start_idx = WAVE_START_X;
    samples = 320;
  }
  for (int i = start_idx; i < samples; i++) {
    if (i >= CAPTURE_BUFFER_SIZE) break;
    
    // Возвращаем чтение с физического пина
    data_ch1[i] = digitalRead(CHANNEL_1_PIN);
    data_ch2[i] = digitalRead(CHANNEL_2_PIN); 

    if (delay_us > 0) delayMicroseconds(delay_us);
  }
}

void drawData() {
  tft.fillRect(WAVE_START_X, 29, 320 - WAVE_START_X, 125, TFT_BLACK);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("CH1", 5, 45);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("CH2", 5, 105);

  const int ch1_base_y = 80;
  const int ch2_base_y = 125;
  const int amplitude = 30;
  float divisor = display_divisors[current_display_idx];

  for (int x = WAVE_START_X; x < 320; x++) {
    int data_idx = view_offset + (int)((x - WAVE_START_X) / divisor);
    int data_idx_prev = view_offset + (int)((x - 1 - WAVE_START_X) / divisor);
    
    if (data_idx >= CAPTURE_BUFFER_SIZE || data_idx_prev < 0) continue;

    int y1_prev = ch1_base_y - (data_ch1[data_idx_prev] * amplitude);
    int y1_curr = ch1_base_y - (data_ch1[data_idx] * amplitude);
    
    int y2_prev = ch2_base_y - (data_ch2[data_idx_prev] * amplitude);
    int y2_curr = ch2_base_y - (data_ch2[data_idx] * amplitude);

    if (divisor >= 1) {
        tft.drawFastHLine(x - 1, y1_prev, 2, TFT_YELLOW);
        tft.drawFastHLine(x - 1, y2_prev, 2, TFT_CYAN);
        if (y1_prev != y1_curr) tft.drawFastVLine(x, min(y1_prev, y1_curr), abs(y1_prev - y1_curr), TFT_YELLOW);
        if (y2_prev != y2_curr) tft.drawFastVLine(x, min(y2_prev, y2_curr), abs(y2_prev - y2_curr), TFT_CYAN);
    } else {
        tft.drawFastVLine(x, min(y1_prev, y1_curr), abs(y1_prev-y1_curr)+1, TFT_YELLOW);
        tft.drawFastVLine(x, min(y2_prev, y2_curr), abs(y2_prev-y2_curr)+1, TFT_CYAN);
    }
  }
}

// =======================================================================
// ИНТЕРФЕЙС
// =======================================================================

void updateUI() {
  tft.fillRect(0, 0, 320, 29, TFT_BLACK);
  tft.fillRect(0, 155, 320, 240, TFT_BLACK);

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  String mode_str = "AUTO";
  if (current_trig_mode == T_FALL) mode_str = "FALL";
  if (current_trig_mode == T_RISE) mode_str = "RISE";
  if (current_trig_mode == T_SINGLE) mode_str = "SINGLE";
  tft.setCursor(5, 5);
  tft.print(mode_str);
  
  float base_us = time_delays[time_scale_idx];
  float divisor = display_divisors[current_display_idx];
  float effective_us = hold ? base_us / divisor : base_us;
  
  tft.setCursor(180, 5);
  tft.printf("T:%.1fus", effective_us);

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  
  const char* btn1_text;
  uint16_t btn1_color;
  if (current_trig_mode == T_SINGLE) {
      btn1_color = TFT_GREEN;
      btn1_text = "ARM";
  } else {
      if (hold) {
          btn1_color = TFT_GREEN;
          btn1_text = "RUN";
      } else {
          btn1_color = TFT_RED;
          btn1_text = "STOP";
      }
  }
  tft.drawRoundRect(BTN_X1, MAIN_BTN_Y, BTN_W, BTN_H, 5, btn1_color);
  tft.setTextColor(btn1_color);
  tft.drawString(btn1_text, BTN_X1 + BTN_W / 2, MAIN_BTN_Y + BTN_H / 2);

  tft.drawRoundRect(BTN_X2, MAIN_BTN_Y, BTN_W, BTN_H, 5, TFT_CYAN);
  tft.setTextColor(TFT_CYAN);
  tft.drawString("T-", BTN_X2 + BTN_W / 2, MAIN_BTN_Y + BTN_H / 2);

  tft.drawRoundRect(BTN_X3, MAIN_BTN_Y, BTN_W, BTN_H, 5, TFT_CYAN);
  tft.drawString("T+", BTN_X3 + BTN_W / 2, MAIN_BTN_Y + BTN_H / 2);

  tft.drawRoundRect(BTN_X4, MAIN_BTN_Y, BTN_W, BTN_H, 5, TFT_MAGENTA);
  tft.setTextColor(TFT_MAGENTA);
  tft.drawString("TRIG", BTN_X4 + BTN_W / 2, MAIN_BTN_Y + BTN_H / 2);
  
  if (hold) {
    tft.drawRoundRect(BTN_X2, SHIFT_BTN_Y, BTN_W, BTN_H, 5, TFT_ORANGE);
    tft.setTextColor(TFT_ORANGE);
    tft.drawString("<-", BTN_X2 + BTN_W / 2, SHIFT_BTN_Y + BTN_H / 2);

    tft.drawRoundRect(BTN_X3, SHIFT_BTN_Y, BTN_W, BTN_H, 5, TFT_ORANGE);
    tft.drawString("->", BTN_X3 + BTN_W / 2, SHIFT_BTN_Y + BTN_H / 2);
  }
  
  tft.setTextDatum(TL_DATUM);
}

void checkTouch() {
  uint16_t x, y;
  if (tft.getTouch(&x, &y) && (millis() - last_touch_time > 200)) {
    last_touch_time = millis();
    bool ui_needs_update = true;
    bool data_needs_update = false;

    if (y > MAIN_BTN_Y && x > BTN_X4 && x < BTN_X4 + BTN_W) { 
        current_trig_mode = (TrigMode)((current_trig_mode + 1) % 4);
        hold = (current_trig_mode == T_SINGLE);
    }
    else if (y > MAIN_BTN_Y && x > BTN_X1 && x < BTN_X1 + BTN_W) {
        if (current_trig_mode == T_SINGLE) {
            tft.setTextDatum(TC_DATUM);
            tft.setTextSize(1);
            tft.setTextColor(TFT_RED);
            tft.drawString("WAITING...", 130, 5);
            delay(10);

            if (waitForTrigger(3000000)) {
                readData(CAPTURE_BUFFER_SIZE);
                view_offset = 0;
                current_display_idx = 3;
                data_needs_update = true;
            }
            hold = true;
        } else {
            hold = !hold;
            if(!hold) current_display_idx = 3;
        }
    }
    else if (y > MAIN_BTN_Y && x > BTN_X2 && x < BTN_X3 + BTN_W){
       if (x > BTN_X2 && x < BTN_X2 + BTN_W) { // T-
           if(hold) { if (current_display_idx > 0) current_display_idx--; data_needs_update = true; } 
           else { if (time_scale_idx > 0) time_scale_idx--; }
       }
       if (x > BTN_X3 && x < BTN_X3 + BTN_W) { // T+
           if(hold) { if (current_display_idx < 6) current_display_idx++; data_needs_update = true; }
           else { if (time_scale_idx < (sizeof(time_delays) / sizeof(int)) - 1) time_scale_idx++; }
       }
    }
    else if (hold && y > SHIFT_BTN_Y && y < SHIFT_BTN_Y + BTN_H) {
        float divisor = display_divisors[current_display_idx];
        int shift_amount = (int)(80 / divisor);
        if(shift_amount < 1) shift_amount = 1;
        
        if (x > BTN_X2 && x < BTN_X2 + BTN_W) { // <-
            view_offset -= shift_amount;
            if (view_offset < 0) view_offset = 0;
            data_needs_update = true;
        }
        if (x > BTN_X3 && x < BTN_X3 + BTN_W) { // ->
            view_offset += shift_amount;
            int max_offset = CAPTURE_BUFFER_SIZE - (int)(320.0f / divisor);
            if (view_offset > max_offset) view_offset = max_offset;
            data_needs_update = true;
        }
    }
    
    if (ui_needs_update) updateUI();
    if (data_needs_update && hold) drawData();
  }
}
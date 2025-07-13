#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <esp_sleep.h>
#include "driver/touch_sensor.h"
#include "Consts.h"
#include "Env.h"

// Entire circuit takes about 32mA

/* Constructor */  //Display takes about 12mA of current when on
//U8G2_SSD1306_128X64_NONAME_1_SW_I2C My_u8g_Panel(U8G2_R0, /* clock=*/GPIO_NUM_13, /* data=*/GPIO_NUM_15, /* reset=*/GPIO_NUM_14);
U8G2_SSD1306_128X64_NONAME_F_SW_I2C My_u8g_Panel(U8G2_R0, /* clock=*/GPIO_NUM_13, /* data=*/GPIO_NUM_15, /* reset=*/GPIO_NUM_14);

// Values for soil moisture
const uint16_t AirValue = 896;  //3140;
const uint16_t MiddleValue = 482;
const uint16_t WaterValue = 50;  //1700;
uint16_t intervals = (AirValue - WaterValue) / 5;
uint16_t soilMoistureValue;

// consts for deep sleep time and up time for esp32
RTC_DATA_ATTR int wakeup_time_sec = 3600;  // seconds (3600s = 1 hour)
// Switch
gpio_num_t button = GPIO_NUM_26;
bool buttonWakeUp = false;  // variable for reading the pushbutton status
long button_wake_up_time = 0;

// ************************************************
// Interval selector variables

long buttonTimer = 0;
long longPressTime = 1000;

bool buttonActive = false;
bool longPressActive = false;

RTC_DATA_ATTR int interval = 0;

// ************************************************

/* My_u8g_Panel.begin() is required and will sent the setup/init sequence to the display */
void setup(void) {

#ifdef DEBUG
  Serial.begin(115200);  // open serial port, set the baud rate to 9600 bps
  Serial.println("Begin Plant Sensor Test");
  delay(1000);
#endif

  // This is used for the button
  pinMode(button, INPUT_PULLUP);
  // Begin panel init
  clear_u8g(); //This will clear the buffer and display so the display does not flicker when returning from deepsleep
  My_u8g_Panel.begin();
  u8g_prepare();

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      handle_buttonpress();
      break;

    case ESP_SLEEP_WAKEUP_TIMER:
      handle_timer();
      break;

    default:
      handle_firstwake();
      break;
  }
}


void loop(void) {
  if (buttonWakeUp == true) {
    wakeupButtonPressLogic();
  } else {
    menuLogic();
  }
}

// ************************************************

// init display settings
void u8g_prepare(void) {
  My_u8g_Panel.setFont(u8g_font_6x10);
  My_u8g_Panel.setFontPosTop();
  My_u8g_Panel.setContrast(0x01);
}

void clear_u8g(void){
  My_u8g_Panel.clearBuffer();					// clear the internal memory
  My_u8g_Panel.clearDisplay();
}
// Handle wake up button press. Scan and then sleep
void handle_buttonpress() {
  #ifdef DEBUG
    Serial.println("button pressed");
  #endif
  //debounce button press
  delay(25);
  showSensorResponse();
  buttonWakeUp = true;
  button_wake_up_time = millis();
}
// Handle timer wake up. Scan and then sleep
void handle_timer() {
  showSensorResponse();
  sleepNow();
}
// Handle first wake. We display the interval selector screen
void handle_firstwake() {
  displayInterval(false);
}

// ************************************************

void wakeupButtonPressLogic() {
  if (digitalRead(button) == LOW) {
    // Debounce
    delay(25);
    // We scan again if the user presses button
    showSensorResponse();
    // Reset the wake up time so the display doesn't turn off in the middle of the
    // scan.
    button_wake_up_time = millis();
  } else {
    // If the user hasn't long pressed then after 5s we sleep again
    if (millis() - button_wake_up_time >= 5000) {
      sleepNow();
    }
  }
}

// ************************************************

void menuLogic() {
  // As long as the user has not set the interval then we let them choose the interval
  if (longPressActive == false) {
    intervalSelectorMenu();
  } else {  // Used to start the scan after the user selects their interval
    firstScanClick();
  }
}

void intervalSelectorMenu() {
  if (digitalRead(button) == LOW) {
    // Debounce
    delay(25);
    // Set the interval
    onIntervalLongPressClick();
  } else {
    if (buttonActive == true) {
      delay(25);
      // increase interval
      onIntervalClick();
    }
  }
}

void firstScanClick() {
  if (digitalRead(button) == LOW) {
    //debounce button press
    delay(25);
#ifdef DEBUG
    Serial.println("Button Pressed!");
#endif
    // Scan now that the user is ready and has hit the button after setting the interval
    showSensorResponse();
    //delay 5s
    delay(5000);
    sleepNow();
  }
}

// This function will handle a long press. If the user does a long press then
// we set the interval and move on to waiting for the user to start the first
// scan.
void onIntervalLongPressClick() {
  // We set buttonActive to true so the onClick function will called if the
  // longPressTime as not been met (checked in the next if statement)
  if (buttonActive == false) {
    buttonActive = true;
    buttonTimer = millis();
  }
  // Check if longPressTime has been met
  if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
    longPressActive = true;
    // Set wakeup (need to add 1 because interval starts at 0 since we use it as an index for the numbers_with_hours array)
    wakeup_time_sec = (interval + 1) * 3600;
    //long press
    displayInterval(longPressActive);
  }
}

// This function will call the function that will increase the interval and display the
// current interval.
void onIntervalClick() {
  if (longPressActive == true) {
    longPressActive = false;
  } else {
    //short press
    increaseInterval(longPressActive);
  }

  buttonActive = false;
}

// Increase interval by 1 and call display function
void increaseInterval(bool longPress) {
  interval = interval + 1;
  if (interval >= 24) {
    interval = 0;
  }
  displayInterval(longPress);
}


void displayInterval(bool longPress) {
  My_u8g_Panel.firstPage();
  do {
    drawInterval(longPress);
  } while (My_u8g_Panel.nextPage());
}

void drawInterval(bool longPress) {
  if (longPress == true) {
    My_u8g_Panel.drawStr(0, 0, "Press Button to Scan!");
    #ifdef DEBUG
        Serial.println("Press Button to Scan!");
    #endif
  } else {
    My_u8g_Panel.drawStr(30, 0, "Scan Every: ");
    My_u8g_Panel.drawStr(30, 20, intervalHelper());
  }
}

const char* intervalHelper() {
  return numbers_with_hours[interval];
}

// ************************************************

void showSensorResponse() {
  #ifdef DEBUG
    Serial.println("Scanning");
  #endif

  char* soilStatus = readSensor();

  My_u8g_Panel.firstPage();
  do {
    draw(soilStatus);
  } while (My_u8g_Panel.nextPage());
}

char* readSensor(void) {
  touch_pad_init();
  touch_pad_set_voltage(TOUCH_HVOLT_2V4, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
  touch_pad_config(TOUCH_PAD_NUM2, 0);

  //do measurement
  touch_pad_read(TOUCH_PAD_NUM2, &soilMoistureValue);
  //soilMoistureValue = analogRead(A0);  //put Sensor insert into soil

#ifdef DEBUG
  Serial.println(soilMoistureValue);
#endif
  //delay(1000);
  return setStatus(soilMoistureValue);
}

char* setStatus(uint16_t soilMoistureValue) {
  if (soilMoistureValue > WaterValue && soilMoistureValue < (WaterValue + intervals))  // 2223 to 2406
  {
#ifdef DEBUG
    Serial.println("Very Wet");
#endif
    return "Very Wet";
  } else if (soilMoistureValue > (WaterValue + intervals) && soilMoistureValue < (MiddleValue - intervals))  // 2406 to 2497
  {
#ifdef DEBUG
    Serial.println("Wet");
#endif
    return "Wet";
  } else if (soilMoistureValue > (MiddleValue - intervals) && soilMoistureValue < (MiddleValue + intervals))  // 2497 to 2864
  {
#ifdef DEBUG
    Serial.println("Good");
#endif
    return "Good";
  } else if (soilMoistureValue > (MiddleValue + intervals) && soilMoistureValue < (AirValue - intervals))  // 2864 to 2957
  {
#ifdef DEBUG
    Serial.println("Kinda Dry");
#endif
    return "Kinda Dry";
  } else if (soilMoistureValue < AirValue && soilMoistureValue > (AirValue - intervals))  // 2957 to 3140
  {
#ifdef DEBUG
    Serial.println("Water Now");
#endif
    return "Water Now";
  }
}

void draw(char* soilStatus) {
  My_u8g_Panel.drawStr(0, 0, "Interval:");
  My_u8g_Panel.drawStr(57, 0, intervalHelper());
  My_u8g_Panel.drawStr(0, 15, soilStatus);
  if (soilStatus == "Very Wet" || soilStatus == "Wet" || soilStatus == "Good") {
    My_u8g_Panel.drawXBMP(30, 25, 47, 38, happyPlant);
  } else if (soilStatus == "Kinda Dry" || soilStatus == "Water Now") {
    My_u8g_Panel.drawXBMP(30, 25, 62, 43, sadPlant);
  }
}



// ************************************************

// 15s deepsleep
void sleepNow() {
  #ifdef DEBUG
    Serial.println("Sleeping");
    Serial.println(wakeup_time_sec);
  #endif
  clear_u8g(); //This will clear the buffer and display so the display does not flicker when returning from deepsleep
  gpio_deep_sleep_hold_en();
  esp_sleep_enable_ext0_wakeup(button, 0);
  esp_sleep_enable_timer_wakeup(1000000ULL * wakeup_time_sec);  // param is in uS so need to multiply by 10^6

  esp_deep_sleep_start();
}
// ************************************************

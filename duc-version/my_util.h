#pragma once

#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h" // sử dụng hàm checkForBeat() cho hàm đọc sensor cũ (readSensor())
#include "heartRate.h" // sử dụng hàm maxim_heart_rate_and_oxygen_saturation() cho hàm đọc sensor mới (newReadSensor()), chính xác hơn

// Các cờ global
extern volatile int mode;
extern volatile bool enableInterrupt;

// Biến global cho hàm đọc sensor cũ (readSensor())
extern float beatsPerMinute;
extern int beatAvg;

// Biến global cho hàm đọc sensor mới (newReadSensor())
extern int8_t validSPO2;
extern int8_t validHeartRate;

extern TaskHandle_t read_sensor_task_handle; // Biến để điều khiển luồng, vd gọi ngắt luồng từ 1 hàm khác

/*
  KHAI BÁO NGUYÊN MẪU CÁC HÀM CẦN SỬ DỤNG.
  ĐỊNH NGHĨA HÀM (TRIỂN KHAI HÀM) SẼ Ở FILE .CPP.
*/

// Tạo riêng 1 luồng chỉ để đọc dữ liệu từ cảm biến, sau đó ghi giá trị vào các biến global
void TaskReadSensor( void* pvParameters);

// Các hàm làm việc cho LCD
void showStartScreen();
void enableMode1();
void enableMode2();
void enableMode3();
void enableMode4();

// Các hàm làm việc cho cảm biến
void initSensor();
void newInitSensor();
void readSensor();
void newReadSensor();
void turnOffSensor();


#include "my_util.h"
#include "env.h"
#include "Adafruit_ILI9341.h"

// Các biến cờ
volatile int mode = 0; 
volatile bool enableInterrupt = false; // cờ để chống nhảy mode khi bấm liên tiếp phím, đảm bảo các mode được thực hiện tuần tự, mode này xong rồi mới đến mode khác

/*
  Các cờ này có mục đích cho hàm main loop chỉ gọi các mode 1 lần,
  các lần loop sau kiểm tra xem nếu được gọi rồi thì sẽ không gọi lại nữa
*/
bool isMode1Started = false; 
bool isMode2Started = false;
bool isMode3Started = false;
bool isMode4Started = false;

// Hàm xử lý ngắt
void IRAM_ATTR handleButtonPress(); // hàm này sẽ được đặt trong bộ nhớ IRAM của ESP32 để thực hiện nhanh chóng

void setup() {
  Serial.begin(115200);

  showStartScreen();
  enableInterrupt = true;

  // Cấu hình chân cho nút kích hoạt ngắt
  pinMode(BUTTON_1, INPUT_PULLUP);

  // Gắn ngắt cho chân nút kích hoạt ngắt
  attachInterrupt(digitalPinToInterrupt(BUTTON_1), handleButtonPress, FALLING);
}

void loop() {
  switch(mode){
    case 1: { // vẽ đồ thị nhịp tim
      enableInterrupt = false;

      if (isMode1Started) {
        enableInterrupt = true;
        break;
      }

      enableMode1();    // hàm thực tế

      isMode1Started = true;
      isMode2Started = false;

      enableInterrupt = true; // để khi chạy vào case 2, nó sẽ không chạy vào if, mà sẽ gọi hàm enableMode2() truóc
      break;
    }
    case 2: {
      enableInterrupt = false;
      // Serial.println("mode 2"); 

      if (isMode2Started) {
        enableInterrupt = true;
        break;
      }

      enableMode2();

      isMode2Started = true;
      isMode3Started = false;

      enableInterrupt = true;
      break;
    }
    case 3: {
      enableInterrupt = false;
      // Serial.println("mode 3");

      if (isMode3Started) {
        enableInterrupt = true;
        break;
      }

      enableMode3();

      isMode3Started = true;
      isMode4Started = false; 

      enableInterrupt = true;
      break;
    }
    case 4: {
      enableInterrupt = false;
      // Serial.println("mode 4"); 

      if (isMode4Started) {
        enableInterrupt = true;
        break;
      }

      enableMode4();

      isMode4Started = true;
      isMode1Started = false;

      enableInterrupt = true;
      break;
    }
  }
  // delay(500);
}

void IRAM_ATTR handleButtonPress() { // triển khai hàm ngắt
  if (enableInterrupt) {
      mode++;
      if (mode >= 5) {
        mode = 1;
      }
    enableInterrupt = false;
  }
}
#include <Arduino.h>
#include "my_util.h"
#include <Adafruit_ILI9341.h>
#include "Adafruit_GFX.h"
#include "env.h"  
#include "Graph.h"
#include <utility>

// Khởi tạo cảm biến
MAX30105 particleSensor;
// Khởi tạo màn hình
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// For old readSensor()
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
long lastGetAvg = 0;
float beatsPerMinute;
int beatAvg;

// For newReadSensor()
uint32_t irBuffer[100]; // mảng lưu giá trị của Led hồng ngoại, đo nhịp tim
uint32_t redBuffer[100];  // mảng lưu giá trị của Led đỏ, đo nồng độ O2
int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

// Đèn led mặc định của Esp32 sẽ sáng với mỗi lần đọc giá trị từ cảm biến
byte readLED = 2; // Lưu ý led sẽ chỉ nhấp nháy khi cảm biến bắt đầu đọc được giá trị từ tay người dùng (trước đó người dùng phải đặt tay vào cảm biến)


TaskHandle_t read_sensor_task_handle;

void showStartScreen() {
  tft.begin();
  tft.setRotation(2);
  // Vẽ nền
  tft.fillRect(0, 0, tft.width(), tft.height(), ILI9341_BLACK);
  // Vẽ tiêu đề
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(5);
  tft.setCursor(18, 20);
  tft.println("WELCOME");
  // Thêm văn bản chào mừng
  tft.setTextColor(ILI9341_CYAN);
  tft.setTextSize(2);
  tft.setCursor(20, 100);
  tft.println("Press any key to");
  tft.setCursor(20, 120);
  tft.println("start....");

  tft.setTextSize(5);
}
void enableMode1() {
  Serial.println("mode 1"); 
  // tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  //tft.setRotation(3);
  // tft.setTextColor(ILI9341_YELLOW);
  // tft.println("mode1");
  
  std::pair<int, int> keyRange = std::make_pair(0, 10);
  std::pair<int, int> valueRange = std::make_pair(200, 500);
  std::pair<int, int> pointNum = std::make_pair(11, 6);

  // Khởi tạo đối tượng Graph
  Graph myGraph("Luu Duc",
                10, 10, 
                220, 150, 
                ILI9341_BLUE, ILI9341_WHITE, ILI9341_GREEN);
  myGraph.setPrinter(&tft);
  myGraph.drawBase();
  myGraph.drawBorder(ILI9341_RED);

  myGraph.drawRulerX(keyRange, pointNum.first);
  myGraph.drawRulerY(valueRange, pointNum.second);
  
  myGraph.enableLine(true);
  myGraph.enablePoint(true);
  
  std::vector<std::pair<int,int>> vectorData = myGraph.generateData(keyRange, valueRange, 11);
  std::queue<std::pair<int,int>> queueData;
  for (int i = 0; i< vectorData.size(); i++) queueData.push(vectorData[i]);
  
  myGraph.drawChart(queueData);

  // Bật ngắt để cho phép chuyển mode, người dùng ấn nút thì sẽ thoát vòng while của hàm enableMode1()
  enableInterrupt = true;

  // Khởi tạo cảm biến
  newInitSensor();
  
  // Bắt đầu chạy luồng để lấy dữ liệu từ cảm biến
  xTaskCreate(TaskReadSensor, "Task Read Sensor", 2048, NULL, 1, &read_sensor_task_handle);

  while(1) { // Logic chạy liên tục của mode 1
    if (mode != 1) { // Nếu mode được chuyển đổi
      enableInterrupt = false; // Tắt ngắt, để tránh nhảy mode khi ấn liên tiếp nút
      turnOffSensor(); // Tắt sensor trước khi xóa task
      vTaskDelete(read_sensor_task_handle); // Xóa luồng
      read_sensor_task_handle = NULL; // Vì luồng đã được xóa, set = null để tránh việc hàm vTaskDelete() được gọi cho 1 Task không còn tồn tại
      digitalWrite(readLED, LOW); // Tắt đèn LED mặc định của ESP32
      return;
    }
    delay(400);
    myGraph.deleteChart();

    keyRange = std::make_pair(keyRange.first + 1, keyRange.second + 1);
    myGraph.deleteRulerX();
    myGraph.drawRulerX(keyRange, pointNum.first);
    
    int value = valueRange.first + std::rand() % (valueRange.second - valueRange.first + 1);
    queueData.push(std::make_pair(keyRange.second, value));
    queueData.pop();
    
    myGraph.drawChart(queueData);
    
  }
}
void enableMode2() {
  Serial.println("mode 2"); 
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("mode2");
}
void enableMode3() {
  Serial.println("mode 3"); 
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("mode3");
}
void enableMode4() {
  Serial.println("mode 4"); 
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_YELLOW);
  tft.println("mode4");
}
void initSensor() {
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}
void newInitSensor() {
  pinMode(readLED, OUTPUT);
  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  Serial.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  // while (Serial.available() == 0) ; //wait until user presses a key
  // Serial.read();

  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}
void turnOffSensor() {
  particleSensor.shutDown(); 
  // particleSensor.setPulseAmplitudeRed(0x00);
  // particleSensor.setPulseAmplitudeIR(0x00);
  // particleSensor.setPulseAmplitudeGreen(0x00);
}
void readSensor() {
  long irValue = particleSensor.getIR(); 

  if (checkForBeat(irValue) == true)// chỉ để kiểm tra tim đập hay không, nếu có sẽ nhận 1 tín hiệu, dựa trên thời gian nhận để tính nhịp tim
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
      // Serial.println("beatAvg:" + String(beatAvg));
    }
  }

  // Serial.print("IR=");
  // Serial.print(irValue);
  // Serial.print(", BPM=");
  // Serial.print(beatsPerMinute);
  // Serial.print(", Avg BPM=");
  // Serial.print(beatAvg);

  // if (irValue < 50000)
  //   Serial.print(" No finger?");

  // Serial.println();
  long currentTime = millis();
  if (currentTime - lastGetAvg >= MY_INTERVAL) {
    // Serial.println(beatAvg);
    lastGetAvg = currentTime;
  }
}
void newReadSensor() {
  // lấy mẫu kích thước 32 bit, Arduino không đủ SRAM để lưu trữ 100 mẫu 32 bit, Esp32 thì có
  // Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
  int32_t bufferLength = 100; //buffer length of 100 stores 4 seconds of samples running at 25sps

  //read the first 100 samples, and determine the signal range
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample

    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }

  //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
  while (1)
  {
    //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }

    //take 25 sets of samples before calculating the heart rate.
    for (byte i = 75; i < 100; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      digitalWrite(readLED, !digitalRead(readLED)); //Blink onboard LED with every data read

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample

      //send samples and calculation result to terminal program through UART
      Serial.print(F("red="));
      Serial.print(redBuffer[i], DEC);
      Serial.print(F(", ir="));
      Serial.print(irBuffer[i], DEC);

      Serial.print(F(", HR="));
      Serial.print(heartRate, DEC);

      Serial.print(F(", HRvalid="));
      Serial.print(validHeartRate, DEC);

      Serial.print(F(", SPO2="));
      Serial.print(spo2, DEC);

      Serial.print(F(", SPO2Valid="));
      Serial.println(validSPO2, DEC);
    }

    //After gathering 25 new samples recalculate HR and SP02
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }
}
void TaskReadSensor(void *pvParameters) {
  for(;;) {
    newReadSensor();
  }
}
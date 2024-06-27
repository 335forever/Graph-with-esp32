#include "Graph.h"
#include <math.h>
#include <ctime>
#include <algorithm>

Graph::Graph(const char* name,
             int positionX, int positionY, 
             int width, int height,
             uint16_t graphColor, uint16_t backgroundColor, uint16_t lineColor) :
             graphName(name),
             positionX(positionX), positionY(positionY), 
             width(width), height(height), 
             graphColor(graphColor), backgroundColor(backgroundColor), lineColor(lineColor),  tft(nullptr) {

  // Tính toán textSize dựa trên độ dài đường chéo của màn hình
  float diagonal_length = sqrt(pow(width, 2) + pow(height, 2)); // Độ dài đường chéo của màn 320x240
  if (diagonal_length < 200) {
      textSize = 1;
  } else if (diagonal_length < 300) {
      textSize = 2;
  } else {
      textSize = 3;
  };

  // Tính tọa độ vùng vẽ Chart
  startDrawZoneX = positionX + 5 + (textSize - 1) * 6 * 3 + 5;
  startDrawZoneY = positionY + 10 + textSize * 8 + 10;
  endDrawZoneX   = positionX + width - 10;
  endDrawZoneY   = positionY +  height - 10 - (textSize - 1) * 8;
}

// Chỉ định công cụ vẽ
void Graph::setPrinter(Adafruit_ILI9341* tft) {
  this->tft = tft;
}

// Vẽ khung đồ thị
void Graph::drawBase() {
  if (!tft) return;
    
  tft->fillRect(positionX, positionY, width, height, backgroundColor); // Đổ nền
  tft->setCursor(positionX + 10, positionY + 10);
  tft->setTextColor(ILI9341_RED);
  tft->setTextSize(textSize);
  tft->println(graphName); // In tiêu đề

  tft->drawLine(startDrawZoneX, startDrawZoneY, startDrawZoneX, endDrawZoneY, graphColor); // Vẽ trục tung
  tft->drawLine(startDrawZoneX, endDrawZoneY, endDrawZoneX, endDrawZoneY, graphColor); // Vẽ trục hoành
}

// Vẽ thước
void Graph::drawRuler(std::pair<int,int> keyRange, std::pair<int,int> valueRange, std::pair<int,int> pointNum) {
  if (!tft) return;
  drawRulerX(keyRange, pointNum.first);
  drawRulerY(keyRange, pointNum.second);
}

// Vẽ thước ngang
void Graph::drawRulerX(std::pair<int,int> keyRange, int pointNum) {
  if (!tft) return;
  
  // Nạp chia độ (theo đơn vị thực)
  std::vector<int> keys;
  for (int i = 0; i < pointNum - 1; i++) keys.push_back(keyRange.first + (keyRange.second - keyRange.first) / (pointNum - 1) * i); keys.push_back(keyRange.second);
  this->keys = keys;

  // Tính độ chia (theo pixel)
  int minorSizeX = (endDrawZoneX - startDrawZoneX - 10) / (pointNum - 1);
  
  // Vẽ vạch
  for (int i = 0; i < pointNum; i++) {
    tft->drawLine(10 + startDrawZoneX + minorSizeX * i, endDrawZoneY + 1, 10 + startDrawZoneX + minorSizeX * i, endDrawZoneY + 5, graphColor);
  }
  
  // In thanh giá trị 
  tft->setTextColor(graphColor);
  tft->setTextSize(textSize - 1);
  if (textSize != 1) 
    for(int i = 0; i < pointNum; i++) {
      tft->setCursor(10 + startDrawZoneX + minorSizeX * i - (textSize - 1) * 3, endDrawZoneY + 10);
      tft->println(keys[i]);
    }
}

// Vẽ thước dọc
void Graph::drawRulerY(std::pair<int,int> valueRange, int pointNum) {
  if (!tft) return;
  
  // Nạp chia độ (theo đơn vị thực)
  std::vector<int> values;
  for (int i = 0; i < pointNum - 1; i++) values.push_back(valueRange.first + (valueRange.second - valueRange.first) / (pointNum - 1) * i); values.push_back(valueRange.second);
  this->values = values;
  
  // Tính độ chia (theo pixel)
  int minorSizeY = (endDrawZoneY - startDrawZoneY - 10) / (pointNum - 1);
  
  // Vẽ vạch
  for(int i = 0; i < pointNum; i++) {
    tft->drawLine(startDrawZoneX - 1, endDrawZoneY - minorSizeY * i - 10, startDrawZoneX - 5, endDrawZoneY - minorSizeY * i - 10, graphColor);
  }
  
  // In thanh giá trị 
  tft->setTextColor(graphColor);
  tft->setTextSize(textSize - 1);
  if  (textSize != 1) 
    for(int i = 0; i < pointNum; i++) {
      tft->setCursor(startDrawZoneX - 10 - (textSize - 1) * 3 * 6, endDrawZoneY - minorSizeY * i - (textSize - 1) * 4 - 10);
      tft->println(values[i]);
    }
}

// Xóa thước ngang
void Graph::deleteRulerX()  {
  if (!tft) return;
  
  // Tính độ chia (theo pixel)
  int minorSizeX = (endDrawZoneX - startDrawZoneX - 10) / (keys.size() - 1);
  
  // Vẽ đè vạch bằng màu nền
  for(int i = 0; i < keys.size(); i++) {
    tft->drawLine(10 + startDrawZoneX + minorSizeX * i, endDrawZoneY + 1, 10 + startDrawZoneX + minorSizeX * i, endDrawZoneY + 5, backgroundColor);
  }

  // In đè thanh giá trị bằng màu nền
  tft->setTextColor(backgroundColor);
  tft->setTextSize(textSize - 1);
  if  (textSize != 1) 
    for(int i = 0; i < keys.size(); i++) {
      tft->setCursor(10 + startDrawZoneX + minorSizeX * i - (textSize - 1) * 3, endDrawZoneY + 10);
      tft->println(keys[i]);
    }
}

// Xóa thước dọc
void Graph::deleteRulerY()  {
  if (!tft) return; 

  // Tính độ chia (theo pixel)
  int minorSizeY = (endDrawZoneY - startDrawZoneY - 10) / (values.size() - 1);
  
  // Vẽ đè vạch bằng màu nền
  for(int i = 0; i < values.size(); i++) {
    tft->drawLine(startDrawZoneX - 1 , endDrawZoneY - minorSizeY * i - 10, startDrawZoneX - 5, endDrawZoneY - minorSizeY * i - 10, backgroundColor);
  }
  
  // In đè thanh giá trị bằng màu nền
  tft->setTextColor(backgroundColor);
  tft->setTextSize(textSize - 1);
  if  (textSize != 1) 
    for(int i = 0; i < values.size(); i++) {
      tft->setCursor(startDrawZoneX - 10 - (textSize - 1) * 3 * 6, endDrawZoneY - minorSizeY * i - (textSize - 1) * 4 - 10);
      tft->println(values[i]);
    }
}

// Vẽ đồ thị điểm
void Graph::drawPoints()  {
  if (!tft) return; 

  for (int i = 0; i < pointPositions.size(); i++) {
    tft->fillCircle(pointPositions[i].first, pointPositions[i].second, textSize, ILI9341_RED);
  }
}

// Xóa đồ thị điểm
void Graph::deletePoints()  {
  if (!tft) return;

  for (int i = 0; i < pointPositions.size(); i++) 
    tft->fillCircle(pointPositions[i].first, pointPositions[i].second, textSize, backgroundColor);
}

// Vẽ đồ thị đường
void Graph::drawLines()  {
  if (!tft) return;

  for (int i = 0; i < pointPositions.size() - 1; i++) {
    tft->drawLine(pointPositions[i].first, pointPositions[i].second, pointPositions[i+1].first, pointPositions[i+1].second, lineColor);
  }
}

// Xóa đồ thị đường
void Graph::deleteLines()  {
  if (!tft) return;

  for (int i = 0; i < pointPositions.size() - 1; i++) {
    tft->drawLine(pointPositions[i].first, pointPositions[i].second, pointPositions[i+1].first, pointPositions[i+1].second, backgroundColor);
  }
}

// Vẽ đồ thị
void Graph::drawChart(std::vector<std::pair<int,int>> data)  {
  if (!tft) return;
  if (!pointOn && !lineOn) return;
  
  // Chuyển giá trị (key,value) sang tọa độ pixel (x,y)
  std::vector<std::pair<int,int>> positions;
  for (int i = 0; i < data.size(); i++) {
    if (data[i].first < keys[0] || keys[keys.size() - 1] < data[i].first || data[i].second < values[0] || values[values.size() - 1] < data[i].second) continue; // Bỏ điểm ngoài range
    
    float t = (float)(data[i].first - keys[0]) / (keys[keys.size() - 1] - keys[0]); // Tỉ số độ dài
    float tt = (endDrawZoneX - startDrawZoneX - 10) * t; // Khoảng cách so với gốc tọa độ (pixel)
    int x = startDrawZoneX + 10 + static_cast<int>(round(tt)); // Tọa độ thực trên màn hình
    
    float p = (float)(data[i].second - values[0]) / (values[values.size() - 1] - values[0]);  // Tỉ số độ dài
    float pp = (endDrawZoneY - startDrawZoneY - 10) * (1 - p); // Khoảng cách so với gốc tọa độ (pixel)
    int y = startDrawZoneY + static_cast<int>(round(pp)); // Tọa độ thực trên màn hình
    
    positions.push_back(std::make_pair(x,y));
  }
  pointPositions = positions;  // Lưu lại danh sách tọa độ
  
  if (pointOn && lineOn) {
    drawLines();
    drawPoints();
  }
  if (!pointOn && lineOn) drawLines();
  if (pointOn && !lineOn) drawPoints();
}

// Xóa đồ thị
void Graph::deleteChart()  {
  if (!tft) return;

  deleteLines();
  deletePoints();
}

// Vẽ khung đồ thị
void Graph::drawBorder(uint16_t borderColor) {
  tft->drawRect(positionX - 1, positionY - 1, width + 2, height + 2, borderColor);
}

// Sinh bộ data ngẫu nhiên (theo chiều tăng trục x)
std::vector<std::pair<int, int>> Graph::generateData(std::pair<int, int> keyRange, std::pair<int, int> valueRange, int numPairs) {
  std::vector<std::pair<int, int>> data;
  data.reserve(numPairs);

  std::srand(std::time(nullptr)); // Seed cho hàm rand()

  for (int i = 0; i < numPairs; ++i) {
      int key = keyRange.first + std::rand() % (keyRange.second - keyRange.first + 1);
      int value = valueRange.first + std::rand() % (valueRange.second - valueRange.first + 1);
      data.push_back(std::make_pair(key, value));
  }

  // Sắp xếp data theo key tăng dần
  std::sort(data.begin(), data.end(), [](const std::pair<int, int>& left, const std::pair<int, int>& right) { 
    return left.first < right.first; 
  });

  return data;
}

// Bật chế độ vẽ đồ thị điểm
void Graph::enablePoint(bool status) {
  pointOn = status;
}

// Bật chế độ vẽ đồ thị đường
void Graph::enableLine(bool status) {
  lineOn = status;
}
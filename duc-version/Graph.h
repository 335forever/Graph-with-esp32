#pragma once

#include <vector>
#include <Adafruit_ILI9341.h>
#include <queue>

class Graph {
private:
    const char* graphName;
    
    int positionX;
    int positionY;
    int width;
    int height;
    
    uint16_t graphColor;
    uint16_t backgroundColor;
    uint16_t lineColor;
    uint16_t pointColor;
    
    Adafruit_ILI9341* tft;
    
    uint8_t textSize;
  
    int startDrawZoneX;
    int startDrawZoneY;
    int endDrawZoneX;
    int endDrawZoneY;

    bool pointOn = false;
    bool lineOn = true;

    std::vector<int> keys;
    std::vector<int> values;
    
    std::vector<std::pair<int,int>> pointPositions;
public:
    Graph(const char* name,
          int positionX, int positionY, 
          int width, int height,
          uint16_t graphColor, uint16_t backgroundColor, uint16_t lineColor);
    void setPrinter(Adafruit_ILI9341* tft);
    
    void drawBase();
    
    void drawRuler(std::pair<int,int> keyRange, std::pair<int,int> valueRange, std::pair<int,int> pointNum);
    void drawRulerX(std::pair<int,int> keyRange, int pointNum);
    void drawRulerY(std::pair<int,int> valueRange, int pointNum);
    
    void deleteRulerX();
    void deleteRulerY();
    
    void enablePoint(bool status);
    void enableLine(bool status);
    
    void drawPoints();
    void deletePoints();
    void drawLines();
    void deleteLines();

    void drawChart(std::vector<std::pair<int,int>> data);
    void deleteChart();
    
    void drawBorder(uint16_t borderColor);

    std::vector<std::pair<int, int>> generateData(std::pair<int, int> keyRange, std::pair<int, int> valueRange, int numPairs);
};

#ifndef WATERING_H
#define WATERING_H
#include "arduino.h"

#define SENSORS_NUM 4
#define PUMPS_NUM 5


struct _sensor {
  int Pin;
  int16_t Value;
};

struct _pump {
  // TODO finish automatation
  int Pin;
  bool State;
  int s_hour,s_min,s_sec; // s for start
  int e_hour,e_min,e_sec; //
  bool automatic_mode;
};

class WateringSystem {
  private:
    bool automatic_watering = false;
    
  public:
    struct _sensor sensor[SENSORS_NUM];
    struct _pump pump[PUMPS_NUM];
    bool is_watering = false;

    int sensorPin[SENSORS_NUM] = { 33, 32, 35, 34};
    int pumpPin[PUMPS_NUM] = {15, 5, 19, 21, 22};
    // auciliar watering functions

    void update_watering_time(int, int , int , int, int, int , int);
    void SerialPrintWaterTimings();
    void init_watering();

    void TimeChecker(struct tm *timeinfo);

    void water_plant(int pumpIndex);

    void update_sensores();
};

#endif // WATERING_H

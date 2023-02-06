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
  int Pin;
  int State;
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


    void init_watering();
    bool set_automatic_watering_on();
    bool set_automatic_watering_off();


    void water_plant(int pumpIndex);

    void update_sensores();
};

#endif // WATERING_H

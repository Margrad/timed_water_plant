#include "watering.h"


void WateringSystem::init_watering() {
  // Set Pumps pins
  for (int i = 0; i < SENSORS_NUM; i++) {
    sensor[i].Pin = sensorPin[i];
    sensor[i].Value = 0;
  }

  for (int i = 0; i < PUMPS_NUM; i++) {
    pump[i].Pin = pumpPin[i];
    pump[i].State = 0;
    pinMode(pump[i].Pin, OUTPUT);
    digitalWrite(pump[i].Pin, HIGH);
  }
}

void WateringSystem::water_plant(int pumpIndex) {
  if (pump[pumpIndex].State) {
    digitalWrite(pump[pumpIndex].Pin, LOW);
  } else {
    digitalWrite(pump[pumpIndex].Pin, HIGH);
  }
}

void WateringSystem::update_sensores()
{
  for (int i = 0; i < SENSORS_NUM; i++) {
    sensor[i].Value = analogRead(sensor[i].Pin);
  }
}


    bool WateringSystem::set_automatic_watering_on(){
      return automatic_watering = true;
    }
    bool WateringSystem::set_automatic_watering_off(){
      return automatic_watering = false;
    }

#include "watering.h"


void WateringSystem::init_watering() {
  // Set Pumps pins
  for (int i = 0; i < SENSORS_NUM; i++) {
    sensor[i].Pin = sensorPin[i];
    sensor[i].Value = 0;
  }

  for (int i = 0; i < PUMPS_NUM; i++) {
    pump[i].Pin = pumpPin[i];
    pump[i].State = false;
    pinMode(pump[i].Pin, OUTPUT);
    digitalWrite(pump[i].Pin, HIGH);
    pump[i].s_hour = 10;
    pump[i].s_min = 0;
    pump[i].s_sec = 0;
    pump[i].e_hour = 10;
    pump[i].e_min = 0;
    pump[i].e_sec = 15;
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

void WateringSystem::TimeChecker(struct tm *timeinfo)
{
  for (int i = 0; i < PUMPS_NUM; i++) // Check every pump connected to see if they are set to automatic watering
  {
    if (pump[i].automatic_mode)     // Automatic mode is on (true)
    {
      if (!pump[i].State)       // The pump is NOT on (Not true)
      {
        // Check it is time to turn the pump on, but not to late to keep it of
        if ((timeinfo->tm_hour >= pump[i].s_hour) && (timeinfo->tm_hour <= pump[i].e_hour) &&         //check hours
            (timeinfo->tm_min >= pump[i].s_min) && (timeinfo->tm_min <= pump[i].e_min) &&
            ( timeinfo->tm_sec <= pump[i].s_sec) &&   (timeinfo->tm_sec <= pump[i].s_sec)) {
          pump[i].State = true;
          water_plant(i);
        }
      }
    }
    else // The pum is on, need to check if it's time to turn it off
    {
      if((timeinfo->tm_hour >= pump[i].e_hour) &&  (timeinfo->tm_min >= pump[i].e_min) &&  (timeinfo->tm_sec >= pump[i].s_sec)){
          pump[i].State = false;
          water_plant(i);        
      }
    }
  }
}

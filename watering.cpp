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
    pump[i].s_hour = 9;
    pump[i].s_min = 50+i;
    pump[i].s_sec = 0;
    pump[i].e_hour = 9;
    pump[i].e_min = 50+i;
    pump[i].e_sec = 3;
    pump[i].automatic_timer_mode = true;
    pump[i].label[0]='P';
    pump[i].label[1]='0'+i;
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
    sensor[i].Value = 0;
    for (int j = 0; j < 2; j++)
      sensor[i].Value += analogRead(sensor[i].Pin);
    sensor[i].Value /= 2;
  }
}

void WateringSystem::TimeChecker(struct tm *timeinfo)
{
  for (int i = 0; i < PUMPS_NUM; i++) // Check every pump connected to see if they are set to automatic watering
  {
    if (pump[i].automatic_timer_mode)     // Automatic mode is on (true)
    {
      if (!pump[i].State)       // The pump is NOT on (Not true)
      {
        // Check it is time to turn the pump on, but not to late to keep it of
        if ((timeinfo->tm_hour >= pump[i].s_hour) && (timeinfo->tm_hour <= pump[i].e_hour) &&         //check hours
            (timeinfo->tm_min >= pump[i].s_min) && (timeinfo->tm_min <= pump[i].e_min) &&
            ( timeinfo->tm_sec >= pump[i].s_sec) &&   (timeinfo->tm_sec < pump[i].e_sec)) {
          pump[i].State = true;
          water_plant(i);
          Serial.print("Pump "); Serial.print(i); Serial.println(": ON");
        }
      }

      else // The pum is on, need to check if it's time to turn it off
      {
        if ((timeinfo->tm_hour >= pump[i].e_hour) &&  (timeinfo->tm_min >= pump[i].e_min) &&  (timeinfo->tm_sec >= pump[i].e_sec)) {
          pump[i].State = false;
          water_plant(i);
          Serial.print("Pump "); Serial.print(i); Serial.println(": OFF");
        }
      }
    }
  }
}

void WateringSystem::watering_sensores_tick(struct tm *timeinfo){
/* struct _pump_sensor_config{
  bool sensores[SENSORS_NUM];
  bool is_watering;
  bool is_waiting; 
  u_int min_val[SENSORS_NUM]; 
  u_int max_val[SENSORS_NUM];
  int next_seconds_check;
  };*/

  int pump_index;
  for (pump_index = 0; pump_index<PUMPS_NUM; pump_index++){
    // Check if is_watering and time != next_seconds_check OR is_waiting and time != next_seconds_check
      // Continue
    // Check if is_watering and time == next_seconds_check
      // is_watering = false; 
      // turn off pump
      // reset timer
      // is_waiting = true
      // continue
    // is_waiting and time == next_seconds_check
      // is_waiting = false
      // check if sensores < max_value
        // if true
          // is_watering = true
          // turn on pump
          // set timer for +3 seconds
          // continue
        // if false (sensor >= max)
          // is_watering = false
          // continue
    // Check if any sensor is lower then the minimal value and no sensor > max
      // is_waterin = true
      // turn on pump
      // set timer for 3 seconds
    }
  }


void WateringSystem::update_watering_time(int pump_index, int sh, int sm, int ss, int eh, int em, int es){
  pump[pump_index].s_hour = sh;
  pump[pump_index].s_min = sm;
  pump[pump_index].s_sec = ss;
  pump[pump_index].e_hour = eh;
  pump[pump_index].e_min = em;
  pump[pump_index].e_sec = es;
  }

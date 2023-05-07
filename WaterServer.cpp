#include "WaterServer.h"



void WaterServer::process_header(String header, WateringSystem *WS) {
  /*
    This function processes the header and updates the Watering system accordingly
    - Get timers for watering updated per pump
    - Turn Automatic watering on/off per pump or all at 1 time
    - Turn On/Off a pump that is not in auto mode, or all at the same time
    Safeties:
    checks if pump number is in use
    when changing to/from Auto mode, turns off the pump by default
  */
  int pump_index;
  int sh, sm, ss; // S for Start, E for End :  h ->hour, m ->minutes, s -> seconds
  int eh, em, es;

  int index = header.indexOf("?st");
  if ( index >= 2 && index < 25) {
    pump_index = (int)header[index + 3] - '0';
    sh = (int)(header[index + 5] - '0') * 10 + header[index + 6] - '0';
    sm = (int)(header[index + 10] - '0') * 10 + header[index + 11] - '0';
    ss = (int)(header[index + 15] - '0') * 10 + header[index + 16] - '0';
    eh = (int)(header[index + 22] - '0') * 10 + header[index + 23] - '0';
    em = (int)(header[index + 27] - '0') * 10 + header[index + 28] - '0';
    es = (int)(header[index + 32] - '0') * 10 + header[index + 33] - '0';

    //protect from incorrect time used or pump
    if (pump_index < 0 || pump_index >= PUMPS_NUM)
      return;
    if (sh < 0 || sh >= 24 || eh < 0 || eh >= 24 )
      return;
    if (sm < 0 || sm >= 60 || em < 0 || em >= 60 )
      return;
    if (ss < 0 || ss >= 60 || es < 0 || es >= 60 )
      return;

    WS->update_watering_time( pump_index, sh,  sm,  ss,  eh,  em,  es);
    return;
  }
  index = header.indexOf("/P");
  if ( index >= 0 && index < 25)
  {
    pump_index = (int)header[index + 2] - '0';
    if (pump_index < 0 || pump_index >= PUMPS_NUM) return; // checks for correct pump number
    if (WS->pump[pump_index].automatic_mode ) return; // if Automatic mode is on, ignore this command
    if (header[index + 4] == 'o' && header[index + 5] == 'n')
    {
      WS->pump[pump_index].State = 1;
      WS->water_plant(pump_index);
      return;
    }
    else if (header[index + 4] == 'o' && header[index + 5] == 'f' && header[index + 6] == 'f')
    {
      WS->pump[pump_index].State = 0;
      WS->water_plant(pump_index);
      return;
    }
  }

  index = header.indexOf("/ALL");
  Serial.println(index);
  if ( index >= 0 && index < 25)
  {
    if (header[index + 5] == 'o' && header[index + 6] == 'n')
    {
      for (int i = 0; i < PUMPS_NUM ; i++)
      {
        Serial.print(i);
        if (WS->pump[i].automatic_mode ) continue; // if Automatic mode is on, jump this pump
        Serial.println(i);
        WS->pump[i].State = 1;
        WS->water_plant(i);
      }
      return;
    }
    else if (header[index + 5] == 'o' && header[index + 6] == 'f' && header[index + 7] == 'f')
    {
      for (int i = 0; i < PUMPS_NUM ; i++)
      {
        Serial.print(i);
        if (WS->pump[i].automatic_mode ) continue;// if Automatic mode is on, jump this pump
        Serial.println(i);
        WS->pump[i].State = 0;
        WS->water_plant(i);
      }
      return;
    }
  }

  index = header.indexOf("/Auto");
  if ( index >= 0 && index < 25)
  {
    if (header[index + 5] == 'A' )
    {
      if (header[index + 7] == 'o' && header[index + 8] == 'n')
      {
        for (int i = 0; i < PUMPS_NUM ; i++)
        {
          WS->pump[i].automatic_mode = 1;
          WS->pump[i].State = 0;
          WS->water_plant(i);
        }
        return;
      }
      else if (header[index + 7] == 'o' && header[index + 8] == 'f' && header[index + 9] == 'f')
      {
        for (int i = 0; i < PUMPS_NUM ; i++)
        {
          WS->pump[i].automatic_mode = 0;
          WS->pump[i].State = 0;
          WS->water_plant(i);
        }
        return;
      }
    }
    pump_index = (int)header[index + 5] - '0';
    if (pump_index < 0 || pump_index >= PUMPS_NUM) return;
    if (header[index + 7] == 'o' && header[index + 8] == 'n')
    {
      WS->pump[pump_index].automatic_mode = 1;
      WS->pump[pump_index].State = 0;
      WS->water_plant(pump_index);
    }
    else if (header[index + 7] == 'o' && header[index + 8] == 'f' && header[index + 9] == 'f')
    {
      WS->pump[pump_index].automatic_mode = 0;
      WS->pump[pump_index].State = 0;
      WS->water_plant(pump_index);
    }
  }
}

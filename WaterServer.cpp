#include "WaterServer.h"



void WaterServer::process_header(String header, WateringSystem *WS) {
  int pump_index;
  int sh, sm, ss; // S for Start, E for End :  h ->hour, m ->minutes, s -> seconds
  int eh, em, es;

  int index = header.indexOf("?st");
  if ( index >= 2 ) {
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
  if ( index >= 0 )
  {
    pump_index = (int)header[index + 2] - '0';
    if (header[index + 4] == 'o' && header[index + 5] == 'n')
    {
      WS->pump[pump_index].State = 1;
      WS->water_plant(pump_index);
    }
    else if (header[index + 4] == 'o' && header[index + 5] == 'f' && header[index + 6] == 'f')
    {
      WS->pump[pump_index].State = 0;
      WS->water_plant(pump_index);
    }
  }

  index = header.indexOf("/ALL");
  if ( index >= 0 )
  {
    if (header[index + 5] == 'o' && header[index + 6] == 'n')
    {
      for (int i = 0; i < PUMPS_NUM ; i++)
      {
        WS->pump[i].State = 1;
        WS->water_plant(i);
      }
    }
    else if (header[index + 5] == 'o' && header[index + 6] == 'f' && header[index + 7] == 'f')
    {
      for (int i = 0; i < PUMPS_NUM ; i++)
      {
        WS->pump[i].State = 0;
        WS->water_plant(i);
      }
    }
  }

  index = header.indexOf("/Auto");
  if ( index >= 0 )
  {
    pump_index = (int)header[index + 5] - '0';
    if (header[index + 7] == 'o' && header[index + 8] == 'n')
    {
      WS->pump[pump_index].automatic_mode = 1;
    }
    else if (header[index + 7] == 'o' && header[index + 8] == 'f' && header[index + 9] == 'f')
    {
      WS->pump[pump_index].automatic_mode = 0;
    }
  }

}

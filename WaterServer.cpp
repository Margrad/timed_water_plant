#include "WaterServer.h"



void WaterServer::process_header(String header, WateringSystem *WS) {
  int index = header.indexOf("?st");
  if ( index < 2 )
    return;
  else {
    int pump_index;
    int sh, sm, ss; // S for Start, E for End :  h ->hour, m ->minutes, s -> seconds
    int eh, em, es;
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


}

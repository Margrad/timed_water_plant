#include "WaterServer.h"



void WaterServer::process_header(String header) {        
        String portbuffer = String(PORT_NUMBER);
        int pump_index = header.indexOf("/P") + 2;
        if( pump_index < 2 ) return; 
        Serial.println(pump_index);

        Serial.println(header[pump_index]);
        
      }

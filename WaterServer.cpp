#include "WaterServer.h"



void WaterServer::process_header(String header, WateringSystem WS) {        
        //String portbuffer = String(PORT_NUMBER);
        //int pump_index = header.indexOf("/P") + 2;
        //int pump_num = (int)(header[pump_index]-'0');
        //for (int i = 0; i< 9; i++)
        //  Serial.print(header[pump_index+i]);
        //Serial.println(pump_num );
        int index = header.indexOf("?st");
        if( index < 2 ) return; 
        Serial.println((int)header[index+3]-'0'); // pump
        Serial.println((int)(header[index+5]-'0')*10+header[index+6]-'0');
        Serial.println((int)(header[index+10]-'0')*10+header[index+11]-'0');
        Serial.println((int)(header[index+15]-'0')*10+header[index+16]-'0');
        Serial.println((int)(header[index+22]-'0')*10+header[index+23]-'0');
        Serial.println((int)(header[index+27]-'0')*10+header[index+28]-'0');
        Serial.println((int)(header[index+32]-'0')*10+header[index+33]-'0');
        //Serial.println(header[index+22]);
        //Serial.println(header[index+23]);
        //Serial.println(header[index+27]);
        //Serial.println(header[index+28]);
        //Serial.println(header[index+32]);
        //Serial.println(header[index+33]);
        WS.update_watering_time((int)header[index+3]-'0',
                        (int)(header[index+5]-'0')*10+header[index+6]-'0',
                        (int)(header[index+10]-'0')*10+header[index+11]-'0',
                        (int)(header[index+15]-'0')*10+header[index+16]-'0',
                        (int)(header[index+22]-'0')*10+header[index+23]-'0',
                        (int)(header[index+27]-'0')*10+header[index+28]-'0',
                        (int)(header[index+32]-'0')*10+header[index+33]-'0');
      }

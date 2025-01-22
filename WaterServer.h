#ifndef WATERSERVER

#include <WiFi.h>
#include "watering.h"
#include "log.h"

#define PORT_NUMBER 7531

// Derived class
class WaterServer: public WiFiServer {

   public:
      WaterServer(int x) : WiFiServer(x){
        }
      void process_POST(String Post, WateringSystem *WS); 
      void process_GET(String header, WateringSystem *WS,MyLog *Logger);
      void process_METHODE(String header, WateringSystem *WS);
};


#endif 

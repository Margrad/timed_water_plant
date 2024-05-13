#ifndef WATERSERVER

#include <WiFi.h>
#include "watering.h"


#define PORT_NUMBER 7531

// Derived class
class WaterServer: public WiFiServer {

   public:
      WaterServer(int x) : WiFiServer(x){
        }
      void process_POST(String Post, WateringSystem *WS); 
      void process_GET(String header, WateringSystem *WS);
};


#endif 

#ifndef LOG_H
#define LOG_H

#include <ESP_Mail_Client.h>
#include "watering.h"
#include "pass.h"

#define SMTP_PORT 465
#define SMTP_HOST  "smtp.gmail.com"
#define LOG_SIZE 36

class MyLog {
  private:
    SMTPSession smtp;
  public:
    int i = 0;
    bool rotated = false;
    int16_t sensorLog[LOG_SIZE][SENSORS_NUM];
    struct tm timeLog[LOG_SIZE];

  
    MyLog() {
      for (int i = 0; i < LOG_SIZE; i++ )
      {
        for (int j = 0; j < SENSORS_NUM; j++)
          sensorLog[i][j] = 0;
      }
    }


    void save_to_log(struct tm, struct _sensor*);
    void send_email(String);
    void print_from_log();

};

#endif //LOG_H

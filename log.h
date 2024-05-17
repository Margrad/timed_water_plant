#ifndef LOG_H
#define LOG_H

#include <ESP_Mail_Client.h>
#include "watering.h"
#include "pass.h"

#define SMTP_PORT 465
#define SMTP_HOST  "smtp.gmail.com"
#define LOG_SIZE 128

class MyLog {
  private:
    SMTPSession smtp;
    // time when the Logger will be called to log sensors data
    int log_hour, log_min;
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
    
    int get_log_hour(){return log_hour;};
    int get_log_min(){return log_min;};
    void set_next_log_time(int interval_in_minutes);
    void init_log_time(struct tm *timeinfo);

    String prepare_log_to_email();
    void save_to_log(struct tm, struct _sensor*);
    void send_email(String);
    void print_from_log();

};

#endif //LOG_H

/*
  Water the plants
  Create a server to monitor plants watering sensores and pumps
*/

#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include "watering.h" // set sensores

//#include "log.h"      // sets the logging files, needs to be set after the waterings, as it uses some of the watering data
//#include "mail_me.h"

/* ***********************************************
 * *Set up Email
*/
#define LOG_INTERVAL_MINS 10
#define LOG_SIZE 36
#define WATER_HOUR 11
#define WATER_MINUTES 45
#define WATER_DURATION_S 5
#define  SMTP_PORT 465

#define SMTP_HOST  "smtp.gmail.com"
///* The sign in credentials */
#define AUTHOR_EMAIL  "marcogranadaesp@gmail.com"
#define AUTHOR_PASSWORD  "uyudiggeebkeraud"
#define RECIPIENT_EMAIL  "marcogranada@gmail.com"

/* The SMTP Session object used for Email sending */
class MyLog {
  private:

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

    void print_from_log();

};
/* Callback function to get the Email sending status */
SMTPSession smtp;
void send_email(String);


void webserver(void * pvParameters);
void google_graph(WiFiClient client);

/* ***********************************************
 * * WIFI Set up
*/

const char* ssid = "VM7265119";         //"REPLACE_WITH_YOUR_SSID";
const char* password = "yqwvnmJZp5rr";  //"REPLACE_WITH_YOUR_PASSWORD";

WateringSystem WS;
MyLog Logger;
WiFiServer server(7531);


// Set web server port number to 80 (http)
String header;           // Variable to store the HTTP request
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // 2s


/* ***********************************************
 * *  SET-UP Watering system
*/

// Time variables
struct tm timeinfo;
bool Flag_log = true;
int cur_year, cur_month, cur_day, cur_hour, cur_min, cur_s;
char date_buffer[30];

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  WS.init_watering();


  delay(1);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("!");
  WiFi.enableIpV6();
  Serial.println("Getting IPv6");
  delay(2000);
  // Print local IP address and start web server
  Serial.println("WiFi connected.");
  Serial.println("IPv4 address:");
  Serial.println(WiFi.localIP());
  Serial.println("IPv6 address:");
  Serial.println(WiFi.localIPv6());
  Serial.print("AP IPv6: ");
  Serial.println(WiFi.softAPIPv6());
  server.begin();

  //Get real time
  Serial.println("Getting current time:");
  configTime(0, 3600, "time.google.com");

  //char strftime_buf[20];
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Could not obtain time info");
    configTime(0, 3600, "time.google.com");
  }
  cur_year = timeinfo.tm_year;
  cur_month = timeinfo.tm_mon;
  
  if (timeinfo.tm_hour >= WATER_HOUR && timeinfo.tm_min > WATER_MINUTES )
    cur_day = timeinfo.tm_mday + 1;
  else
      cur_day = timeinfo.tm_mday;
      
  cur_hour = timeinfo.tm_hour;
  cur_min = timeinfo.tm_min + LOG_INTERVAL_MINS;
  if (cur_min >= 60 ) {
    cur_min -= 60;
    cur_hour++;
  }
  if (cur_hour > 23 ) cur_hour = 0;
  cur_s = timeinfo.tm_sec;

  strftime(date_buffer, sizeof(date_buffer), "%Y/%m/%d %H:%M:%S", &timeinfo);
  Serial.println(date_buffer);
  WS.update_sensores();
  Logger.save_to_log(timeinfo, WS.sensor);
}



void loop() {

  webserver(NULL);


  getLocalTime(&timeinfo);
  // check for auto watering time
  if (timeinfo.tm_mon > cur_month)
  {
    cur_month++;
    cur_day = 1;
  }

  if (WS.is_watering  == false)
  {
    if (timeinfo.tm_hour == WATER_HOUR && timeinfo.tm_min == WATER_MINUTES)
    {
      if (timeinfo.tm_mday == cur_day)
      {
        for (int i = 0; i < PUMPS_NUM; i++)
        {
          WS.pump[i].State = 1;
          WS.water_plant(i);
        }
        WS.is_watering  = true;
      }
    }
  }

  if (WS.is_watering == true)
  {
    if (timeinfo.tm_sec >= WATER_DURATION_S)
    {
      for (int i = 0; i < PUMPS_NUM; i++)
      {
        WS.pump[i].State = 0;
        WS.water_plant(i);
      }
      WS.is_watering  = false;
      cur_day++;
    }
  }
  // TODO Check month and reset day

    // Log stuff
    if (timeinfo.tm_min == cur_min && timeinfo.tm_hour == cur_hour)
    {
      strftime(date_buffer, sizeof(date_buffer), "%Y%m%d%H%M%S", &timeinfo);
      Serial.println(date_buffer);
      /// Update next time to log
      WS.update_sensores();
      cur_hour = timeinfo.tm_hour;
      cur_min = timeinfo.tm_min + LOG_INTERVAL_MINS;
      if (cur_min >= 60 ) {
        cur_min -= 60;
        cur_hour++;
      }
      if (cur_hour >= 23 ) cur_hour = 0;
      //
      Logger.save_to_log(timeinfo, WS.sensor);
      cur_hour = timeinfo.tm_hour;
      cur_min = timeinfo.tm_min + LOG_INTERVAL_MINS;
      if (cur_min >= 60 ) {
        cur_min -= 60;
        cur_hour++;
      }
      if (cur_hour > 23 ) cur_hour = 0;
    }

}


void webserver( void * pvParameters) {
  //char num_buffer[17];
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    bool button_pressed = false;
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      WS.update_sensores();
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the pumps on and off

            if (header.indexOf("GET /P1/on") >= 0) {      // Check Pump1
              Serial.println("P1 on");
              button_pressed = true;
              WS.pump[0].State = 1;
              WS.water_plant(0);
            } else if (header.indexOf("GET /P1/off") >= 0) {
              Serial.println("P1 off");
              button_pressed = true;
              WS.pump[0].State = 0;
              WS.water_plant(0);
            } else if (header.indexOf("GET /P2/on") >= 0) {       // Check Pump 2
              Serial.println("P2 on");
              button_pressed = true;
              WS.pump[1].State = 1;
              WS.water_plant(1);
            } else if (header.indexOf("GET /P2/off") >= 0) {
              Serial.println("P2 off");
              button_pressed = true;
              WS.pump[1].State = 0;
              WS.water_plant(1);
            } else if (header.indexOf("GET /P3/on") >= 0) {       //Check Pump  P3
              Serial.println("P3 on");
              button_pressed = true;
              WS.pump[2].State = 1;
              WS.water_plant(2);
            } else if (header.indexOf("GET /P3/off") >= 0) {
              Serial.println("P3 off");
              button_pressed = true;
              WS.pump[2].State = 0;
              WS.water_plant(2);
            } else if (header.indexOf("GET /P4/on") >= 0) {       //Check Pump  P4
              Serial.println("P4 on");
              button_pressed = true;
              WS.pump[3].State = 1;
              WS.water_plant(3);
            } else if (header.indexOf("GET /P5/on") >= 0) {       //Check Pump  P4
              Serial.println("P5 on");
              button_pressed = true;
              WS.pump[4].State = 1;
              WS.water_plant(4);
            } else if (header.indexOf("GET /P4/off") >= 0) {
              Serial.println("P4 off");
              button_pressed = true;
              WS.pump[3].State = 0;
              WS.water_plant(3);
            } else if (header.indexOf("GET /P5/off") >= 0) {
              Serial.println("P5 off");
              button_pressed = true;
              WS.pump[4].State = 0;
              WS.water_plant(4);
            } else if (header.indexOf("GET /ALL/on") >= 0) {       //Check Pump  P4
              Serial.println("ALL on");
              button_pressed = true;
              for (int i = 0; i < PUMPS_NUM ; i++)
              {
                WS.pump[i].State = 1;
                WS.water_plant(i);
              }
            } else if (header.indexOf("GET /ALL/off") >= 0) {
              Serial.println("ALL off");
              button_pressed = true;
              for (int i = 0; i < PUMPS_NUM ; i++)
              {
                WS.pump[i].State = 0;
                WS.water_plant(i);
              }
            } else if (header.indexOf("GET /mail") >= 0) {
              Serial.println("sending mail...");
              button_pressed = true;
              send_email(String("Still workin on this part"));
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<script src=\"https://www.gstatic.com/charts/loader.js\"> </script>");

            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style>");
            if (button_pressed)
              client.println("<meta http-equiv=\"Refresh\" content=\"0; URL='http://192.168.0.55:7531'\" /></head>");
            // Web Page Heading
            client.println("<body><h1>Watering the plants</h1>");
            // Display current Pump state, and ON/OFF buttons
            // If the Pump is off, it displays the ON button
            if (WS.pump[0].State ) {
              client.print("<p>Pump 1 - State ON ");
              client.println("</p>");
              client.println("<p><a href=\"/P1/off\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.print("<p>Pump 1 - State OFF ");
              client.println("</p>");
              client.println("<p><a href=\"/P1/on\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (WS.pump[1].State ) {
              client.print("<p>Pump 2 - State ON ");
              client.println("</p>");
              client.println("<p><a href=\"/P2/off\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.print("<p>Pump 2 - State OFF ");
              client.println("</p>");
              client.println("<p><a href=\"/P2/on\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (WS.pump[2].State ) {
              client.print("<p>Pump 3 - State ON ");
              client.println("</p>");
              client.println("<p><a href=\"/P3/off\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.print("<p>Pump 3 - State OFF ");
              client.println("</p>");
              client.println("<p><a href=\"/P3/on\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (WS.pump[3].State ) {
              client.print("<p>Pump 4 - State ON ");
              client.println("</p>");
              client.println("<p><a href=\"/P4/off\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.print("<p>Pump 4 - State OFF ");
              client.println("</p>");
              client.println("<p><a href=\"/P4/on\"><button class=\"button button2\">ON</button></a></p>");
            }
            if (WS.pump[4].State ) {
              client.print("<p>Pump 5 - State ON ");
              client.println("</p>");
              client.println("<p><a href=\"/P5/off\"><button class=\"button\">OFF</button></a></p>");
            } else {
              client.print("<p>Pump 5 - State OFF ");
              client.println("</p>");
              client.println("<p><a href=\"/P5/on\"><button class=\"button button2\">ON</button></a></p>");
            }
            client.println("<h1>Set all Pumps</h1>");
            client.println("<p><a href=\"/ALL/off\"><button class=\"button button2\">Set all OFF</button></a></p>");
            client.println("<p><a href=\"/ALL/on\"><button class=\"button button2\"> Set all ON</button></a></p>");
            client.println("<p><a href=\"/mail\"><button class=\"button button2\"> Send email</button></a></p>");

            client.println("<h1>Sensors:</h1>");
            for (int i = 0 ; i < SENSORS_NUM; i++) {
              client.print("<p>Sensor ");
              client.print(i + 1);
              client.print(" = ");
              client.print(WS.sensor[i].Value);
              client.println("</p>");
            }
            client.println("<div id=\"myChart\" style=\"width:100%; max-width:600px; height:500px;\"></div> ");
            google_graph(client);

            client.println("<p> WIFI Strength");
            client.println(WiFi.RSSI());
            client.println("</p>");
            strftime(date_buffer, sizeof(date_buffer), "%Y/%m/%d %H:%M:%S", &timeinfo);
            client.println("<p>");
            client.println(date_buffer);
            client.println("</p>");
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}


void google_graph(WiFiClient client) {
  char date_buffer[16];
  client.println("<script>");
  client.println("google.charts.load('current',{packages:['corechart']});");
  client.println("google.charts.setOnLoadCallback(drawChart);");
  client.println("function drawChart() {");
  client.println("var data = google.visualization.arrayToDataTable([");
  client.print("['tempo',");
  for (int s = 0; s < SENSORS_NUM ; s++) {
    client.print("'Sensor ");
    client.print(s + 1);
    client.print("'");
    if (s < SENSORS_NUM - 1) {
      client.print(",");
    }
  }
  client.println("],");
  if (Logger.rotated) {
    for (int t = Logger.i; t < LOG_SIZE; t++) {
      client.print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client.print(date_buffer);
      client.print(",");
      for (int s = 0; s < SENSORS_NUM ; s++) {
        client.print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client.print(",");
        }
      }
      client.println("],");
    }
    for (int t = 0 ; t < Logger.i; t++) {
      client.print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client.print(date_buffer);
      client.print(",");
      for (int s = 0; s < SENSORS_NUM ; s++) {
        client.print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client.print(",");
        }
      }
      client.print("]");
      if (t < Logger.i - 1) {
        client.println(",");
      }
    }
  } else {
    for (int t = 0 ; t < Logger.i; t++) {
      client.print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client.print(date_buffer);
      client.print(",");
      for (int s = 0; s < SENSORS_NUM ; s++) {
        client.print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client.print(",");
        }
      }
      client.print("]");
      if (t < Logger.i - 1) {
        client.println(",");
      }
    }
  }

  client.println("]);");
  client.println("var options = {");
  client.println("title: 'Soil Humidy Sensors',");
  client.println("hAxis: {title: 'Time'},");
  client.println("vAxis: {title: 'Humidity'},");
  client.println("legend: 'none'};");
  client.println("var chart = new google.visualization.LineChart(document.getElementById('myChart'));");
  client.println("chart.draw(data, options);}");
  client.println("</script>");
}

void send_email(String textMsg) {
  // Set the email server
  MailClient.networkReconnect(true);
  smtp.debug(0);
  //smtp.callback(smtpCallback);
  ESP_Mail_Session session;
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "";
  session.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  session.time.gmt_offset = 0;
  session.time.day_light_offset = 0;

  /* Declare the message class 109 670 124*/
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("Watering System"); // This witll be used with 'MAIL FROM' command and 'From' header field.
  message.sender.email = AUTHOR_EMAIL; // This witll be used with 'From' header field.
  message.subject = F("Water System Report");
  message.addRecipient("Marco", RECIPIENT_EMAIL); // This will be used with RCPT TO command and 'To' header field.

  //String textMsg = "This is simple plain text message";
  message.text.content = textMsg;

  /** If the message to send is a large string, to reduce the memory used from internal copying  while sending,
     you can assign string to message.text.blob by cast your string to uint8_t array like this

     String myBigString = "..... ......";
     message.text.blob.data = (uint8_t *)myBigString.c_str();
     message.text.blob.size = myBigString.length();

     or assign string to message.text.nonCopyContent, like this

     message.text.nonCopyContent = myBigString.c_str();

     Only base64 encoding is supported for content transfer encoding in this case.
  */

  /** The Plain text message character set e.g.
     us-ascii
     utf-8
     utf-7
     The default value is utf-8
  */
  message.text.charSet = F("us-ascii");

  /** The content transfer encoding e.g.
     enc_7bit or "7bit" (not encoded)
     enc_qp or "quoted-printable" (encoded)
     enc_base64 or "base64" (encoded)
     enc_binary or "binary" (not encoded)
     enc_8bit or "8bit" (not encoded)
     The default value is "7bit"
  */
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_base64;

  // If this is a reply message
  // message.in_reply_to = "<parent message id>";
  // message.references = "<parent references> <parent message id>";

  /** The message priority
     esp_mail_smtp_priority_high or 1
     esp_mail_smtp_priority_normal or 3
     esp_mail_smtp_priority_low or 5
     The default value is esp_mail_smtp_priority_low
  */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  // message.response.reply_to = "someone@somemail.com";
  // message.response.return_path = "someone@somemail.com";

  /** The Delivery Status Notifications e.g.
     esp_mail_smtp_notify_never
     esp_mail_smtp_notify_success
     esp_mail_smtp_notify_failure
     esp_mail_smtp_notify_delay
     The default value is esp_mail_smtp_notify_never
  */
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  //message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  // For Root CA certificate verification (ESP8266 and ESP32 only)
  // session.certificate.cert_data = rootCACert;
  // or
  // session.certificate.cert_file = "/path/to/der/file";
  // session.certificate.cert_file_storage_type = esp_mail_file_storage_type_flash; // esp_mail_file_storage_type_sd
  // session.certificate.verify = true;

  // The WiFiNINA firmware the Root CA certification can be added via the option in Firmware update tool in Arduino IDE

  /* Connect to server with the session config */

  // Library will be trying to sync the time with NTP server if time is never sync or set.
  // This is 10 seconds blocking process.
  // If time synching was timed out, the error "NTP server time synching timed out" will show via debug and callback function.
  // You can manually sync time by yourself with NTP library or calling configTime in ESP32 and ESP8266.
  // Time can be set manually with provided timestamp to function smtp.setSystemTime.

  /* Connect to the server */
  if (!smtp.connect(&session /* session credentials */))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());

  // to clear sending result log
  // smtp.sendingResult.clear();

  ESP_MAIL_PRINTF("Free Heap: %d\n", MailClient.getFreeHeap());
}

void MyLog::save_to_log(struct tm tempo, struct _sensor *sensores) {
  char date_buffer[10];
  timeLog[i] = tempo;
  for (int s = 0 ; s < SENSORS_NUM; s++)
    sensorLog[i][s] = sensores[s].Value;
  i++;
  if (i == LOG_SIZE) {
    rotated = true;
    i = 0;
    String LOG = "[";
    LOG += "['tempo',";
    for (int s = 0; s < SENSORS_NUM ; s++) {
      LOG += "'Sensor ";
      LOG += String(s + 1);
      LOG += "'";
      if (s < SENSORS_NUM - 1) {
        LOG += ",";
      }
    }

    LOG += "],\n";
    for (int t = 0 ; t < LOG_SIZE; t++) {
      LOG += "[";
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &timeLog[t]);
      LOG += date_buffer;
      LOG += ",";
      for (int s = 0; s < SENSORS_NUM ; s++) {
        LOG += String(sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          LOG += ",";
        }
      }
      LOG += "]";
      if (t < LOG_SIZE - 1) {
        LOG += ",";
      }
      LOG += "\n";
    }
    LOG += "]";

    send_email(LOG);
  }
};

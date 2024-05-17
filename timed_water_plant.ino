/*
  Water the plants
  Create a server to monitor plants watering sensores and pumps
*/
//#define __DEBUG__
//#define __DEBUG__PLOT__
//#include <WiFi.h>

#include "WaterServer.h" // Get the server with extra methods made for the watering system
#include "watering.h" // set sensores and pumps
#include "pass.h"     // where the passwords are you will need to create this file yourselfe
#include "log.h"      // sets the logging files, needs to be set after the waterings, as it uses some of the watering data

/* The sign in credentials present in pass.h
  #define AUTHOR_EMAIL "email_esp_uses@to_send.emails"
  #define AUTHOR_PASSWORD  "Password used for the esp32 email"
  #define RECIPIENT_EMAIL  "email_you_send@emails.to"
  #define WIFI_SSID  "REPLACE_WITH_YOUR_SSID"
  #define WIFI_PASSWORD "REPLACE_WITH_YOUR_PASSWORD";
*/


// Define the time between data logging in minutes
#define LOG_INTERVAL_MINS 10


void webserver();
void google_graph(WiFiClient *client);
void displaytime(WiFiClient *client, int time);
/* ***********************************************
 * * WIFI server Set up
*/
// Set web server port number to 7531 (easier to use a different port from 80 for port forwarding)
WaterServer server(PORT_NUMBER);
unsigned long currentTime = millis();
unsigned long previousTime = 0;
const long timeoutTime = 2000; // 2s

/*******************************************************
   Setting both classes created for this project
*/
WateringSystem WS;
MyLog Logger;



// Time variables
struct tm timeinfo;
bool Flag_log = true;

char date_buffer[30];
char up_time[30];


// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
 // Initialize the Watering system
  WS.init_watering();
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID );
  WiFi.begin(WIFI_SSID , WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("!");
  // Print local IP address and start web server
  Serial.println("WiFi connected.");
  Serial.println("IPv4 address:");
  Serial.println(WiFi.localIP());
  server.begin();

  //Get current time information
  Serial.println("Getting current time:");
  configTime(0, 3600, "time.google.com");

  //char strftime_buf[20];
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Could not obtain time info");
    configTime(0, 3600, "time.google.com");
  }

  // Display current time
  strftime(date_buffer, sizeof(date_buffer), "%Y/%m/%d %H:%M:%S", &timeinfo);
  strftime(up_time, sizeof(up_time), "%Y/%m/%d %H:%M:%S", &timeinfo);
  Serial.println(date_buffer);
  WS.update_sensores();


  // Set the next time to log the sensors data
  Logger.init_log_time(&timeinfo);
  // Log starting data
  Logger.save_to_log(timeinfo, WS.sensor);
  Logger.set_next_log_time(LOG_INTERVAL_MINS);

  WS.update_sensores();
#ifdef __DEBUG__
#ifdef __DEBUG__PLOT__
  struct _sensor debug_sensor[SENSORS_NUM];
  struct tm debug_time;
  for (int i=0;i<SENSORS_NUM;i++)
    debug_sensor[i].Value=i;
    
  for(int j=0;j<37;j++)
    {
      debug_time.tm_hour=j;
      Logger.save_to_log(debug_time, debug_sensor);
    }
#endif //__DEBUG__PLOT_
#endif // __DEBUG__  

}



void loop() {

  webserver();
  getLocalTime(&timeinfo);
  WS.TimeChecker(&timeinfo);


  // Log stuff
  if (timeinfo.tm_min == Logger.get_log_min() && timeinfo.tm_hour == Logger.get_log_hour())
  {
    // strftime(date_buffer, sizeof(date_buffer), "%Y%m%d%H%M%S", &timeinfo);
    // Serial.println(date_buffer);
    /// Update next time to log
    WS.update_sensores();
    Logger.save_to_log(timeinfo, WS.sensor);
    Logger.set_next_log_time(LOG_INTERVAL_MINS);
  }
}



void webserver() {
  
  String header;           // Variable to store the HTTP request
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String POST_line = "";                  // make a string to holde the POST information
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

            //Check if it's a POST 
            if (header.indexOf("POST") == 0){
                int index = header.indexOf("Content-Length:")+16;
                char temp = header[index];
                String number = "";
                while (temp != '\n'){
                  number += temp;
                  index++;
                  temp = header[index];
                  }
                int Content_Length = number.toInt();
            Serial.println(Content_Length);
                POST_line = "";
                for(int i = 0; i< Content_Length ; i++){
                  POST_line += char (client.read());
                  } 
            Serial.println(POST_line);
            server.process_POST(POST_line, &WS);
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
                        
            client.println("<!DOCTYPE html><html><head><meta http-equiv=\"refresh\" content=\"1\"></head></html>");
            client.println();
            header = "";
            client.stop();  
            return;     
            }


            
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Proceess the header to 
            server.process_GET(header, &WS);


            if (header.indexOf("GET /mail") >= 0) {
              Serial.println("sending mail...");
              Logger.send_email(Logger.prepare_log_to_email());
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
            client.println(".button2 {background-color: #555555;}");
            client.println(".division {display: inline-block;padding: 0rem 1rem;}");
            client.println(".hide {display: none;}");
            client.println("</style>");
            // Web Page Heading
            client.println("<body><h1>Watering the plants</h1>");


            /*
               Print Pump config
            // Display current Pump state, and ON/OFF buttons
            // If the Pump is off, it displays the ON button
            */
            for (int pump = 0; pump < PUMPS_NUM; pump++)
            {

              // Creates buttons to turns the auto watering on or off
              client.print("<p><div  class='division'>"); // Div with button, pump name and label
              client.print("<p>Pump ");
              client.print(pump);
              client.print(": ");
              client.print("<form method = \"POST\"><input type=\"text\" name=\"P");
              client.print(pump);
              client.print("lab\" value=\"");
              client.print(WS.pump[pump].label);
              client.print("\" size=\"10\" ></form>");

              client.print(" - Auto ");
              (WS.pump[pump].automatic_timer_mode ) ? client.print("ON") : client.print("OFF");
              client.println("</p>");
              client.print("<p><a href = \"/Auto");
              client.print(pump);
              (WS.pump[pump].automatic_timer_mode ) ? client.print("/off") : client.print("/on");
              client.print("\"><button class=\"button");
              (WS.pump[pump].automatic_timer_mode ) ? client.print("\">OFF") : client.print(" button2\">ON");
              client.println("</button></a></p>");              
              client.println("</div>");  // End of button
              
              // Creates buttons to turn a pump on of off
              client.print("<div  class='division"); // Div with button
              (WS.pump[pump].automatic_timer_mode ) ? client.print(" hide'>") : client.print("'>");
              client.print("<p>Pump ");
              client.print(pump);
              client.print(" - State ");
              (WS.pump[pump].State ) ? client.print("ON") : client.print("OFF");
              client.println("</p>");
              client.print("<p><a href = \"/P");
              client.print(pump);
              (WS.pump[pump].State ) ? client.print("/off") : client.print("/on");
              client.print("\"><button class=\"button");
              (WS.pump[pump].State ) ? client.print("\">OFF") : client.print(" button2\">ON");
              client.println("</button></a></p>");     
              client.println("</div>");  // End of button
               


              // Creates time controllers
              client.print("<div  class='division'>"); // Div with form
              client.println("<form  method=\"GET\">");
              client.print("<div><label for=\"start-time\">");
              client.println("Watering Start:</label>");
              client.print("<input id=\"st"); client.print(pump); client.print("\" type=\"time\" value=\"");
              displaytime(&client, WS.pump[pump].s_hour); client.print(":"); displaytime(&client, WS.pump[pump].s_min); client.print(":"); displaytime(&client, WS.pump[pump].s_sec); client.print("\" ");
              client.print("name=\"st"); client.print(pump); client.println("\" step=\"1\"/></div><div>");
              client.print("<label for=\"stop-time\">");
              client.println("Watering End:</label>");
              client.print("<input id=\"et"); client.print(pump); client.print("\" type=\"time\" value=\"");
              displaytime(&client, WS.pump[pump].e_hour); client.print(":"); displaytime(&client, WS.pump[pump].e_min); client.print(":"); displaytime(&client, WS.pump[pump].e_sec); client.print("\" ");
              client.print("name=\"et"); client.print(pump); client.println("\" step=\"1\"/></div>");
              client.println("<div><input type=\"submit\" value=\"Submit times\" /></div>");
              client.println("</form></div></p>");  // End of formn

            }

            client.println("<h1>Set all Pumps</h1>");
            client.println("<p><a href=\"/ALL/off\"><button class=\"button button2\">Set all OFF</button></a></p>");
            client.println("<p><a href=\"/ALL/on\"><button class=\"button button2\"> Set all ON</button></a></p>");
            client.println("<p><a href=\"/AutoA/off\"><button class=\"button button2\">Turn OFF ALL Auto watering</button></a></p>");
            client.println("<p><a href=\"/AutoA/on\"><button class=\"button button2\">Turn ON ALL Auto watering</button></a></p>");
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
            google_graph(&client);

            client.println("<p> WIFI Strength");
            client.println(WiFi.RSSI());
            client.println("</p>");
            strftime(date_buffer, sizeof(date_buffer), "%Y/%m/%d %H:%M:%S", &timeinfo);
            client.println("<p>Current time:");
            client.println(date_buffer);
            client.println("</p>");
            client.println("<p>Starting time:");
            client.println(up_time);
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


void google_graph(WiFiClient *client) {
  //char date_buffer[16];
  int t;
  int s;
  client->println("<script>");
  client->println("google.charts.load('current',{packages:['corechart']});");
  client->println("google.charts.setOnLoadCallback(drawChart);");
  client->println("function drawChart() {");
  client->println("var data = google.visualization.arrayToDataTable([");
  client->print("['tempo',");
  for (s = 0; s < SENSORS_NUM ; s++) {
    client->print("'Sensor ");
    client->print(s + 1);
    client->print("'");
    if (s < SENSORS_NUM - 1) {
      client->print(",");
    }
  }
  client->println("],");
  if (Logger.rotated) {
    for (t = Logger.i; t < LOG_SIZE; t++) {
      client->print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client->print(date_buffer);
      client->print(",");
      for (s = 0; s < SENSORS_NUM ; s++) {
        client->print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client->print(",");
        }
      }
      client->println("],");
    }
    for (t = 0 ; t < Logger.i; t++) {
      client->print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client->print(date_buffer);
      client->print(",");
      for (s = 0; s < SENSORS_NUM ; s++) {
        client->print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client->print(",");
        }
      }
      client->print("]");
      if (t < Logger.i - 1) {
        client->println(",");
      }
    }

  } else {
    for (t = 0 ; t < Logger.i; t++) {
      client->print("[");
      strftime(date_buffer, sizeof(date_buffer), "%d%H%M", &Logger.timeLog[t]);
      client->print(date_buffer);
      //client.print(t);
      client->print(",");
      for (s = 0; s < SENSORS_NUM ; s++) {
        client->print(Logger.sensorLog[t][s]);
        if (s < SENSORS_NUM - 1) {
          client->print(",");
        }
      }
      client->print("]");
      if (t < Logger.i - 1) {
        client->println(",");
      }
    }
  }

  client->println("]);");
  client->println("var options = {");
  client->println("title: 'Soil Humidy Sensors',");
  client->println("hAxis: {title: 'Time'},");
  client->println("vAxis: {title: 'Humidity'},");
  client->println("legend: 'none'};");
  client->println("var chart = new google.visualization.LineChart(document.getElementById('myChart'));");
  client->println("chart.draw(data, options);}");
  client->println("</script>");
}

void displaytime(WiFiClient *client, int time) {
  if (time < 10) {
    client->print("0");
  }
  client->print(time);
}

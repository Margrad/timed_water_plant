#include "log.h"

void MyLog::send_email(String textMsg) {
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
        LOG += "\"" + String(sensorLog[t][s]) + "\"";
        if (s < SENSORS_NUM - 1) {
          LOG += ",";
        }
      }
      LOG += "]";
      if (t < LOG_SIZE - 1) {
        LOG += ",\n";
      }
      // LOG += "";
    }
    LOG += "]";

    send_email(LOG);
  }
};

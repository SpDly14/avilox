#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#ifndef APSSID
#define APSSID "Avilox"
#define APPSK "1234567890"
#endif

const char *dssid = APSSID;
const char *dpass = APPSK;

char ssid[] = "SpDly";
char pass[] = "Shivaprasad";

Servo myservo;

// Time offset for IST (Indian Standard Time) in seconds
const long IST_OFFSET = 19800; // 5 cthour 30 ctmin in seconds

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", IST_OFFSET);


ESP8266WebServer server(80);

int pillhour = 0;  // Default pill reminder time
int pillmin = 0;
int pos = 0;

void setup() {
  myservo.attach(D4);
  int timeout = 0;
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  
  while (WiFi.status() != WL_CONNECTED && timeout < 10) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    timeout++;
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  
//  WiFi.softAP(dssid, dpass);

//  IPAddress myIP = WiFi.softAPIP();
//  Serial.print("AP IP address: ");
//  Serial.println(myIP);

  timeClient.begin();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/store", HTTP_POST, handleStore);
  
  server.begin();
}

void loop() {
  bool state = true;
  int cthour, ctmin, seconds;
  server.handleClient();

  timeClient.update();

  cthour = timeClient.getHours();
  ctmin = timeClient.getMinutes();
/*
  Serial.print(cthour);
  Serial.print(":");
  Serial.println(ctmin);
  Serial.print(pillhour);
  Serial.print(":");
  Serial.println(pillmin);
*/
  

  if(cthour == pillhour) {
    while(ctmin == pillmin) {
      if(state) {
        for (pos = 0; pos <= 180; pos += 1) {  // goes from 0 degrees to 180 degrees
          // in steps of 1 degree
          myservo.write(pos);  // tell servo to go to position in variable 'pos'
          delay(15);           // waits 15ms for the servo to reach the position
        }
        for (pos = 180; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
          myservo.write(pos);                  // tell servo to go to position in variable 'pos'
          delay(15);                           // waits 15ms for the servo to reach the position
        }
        state = false;
      }
      timeClient.update();

      cthour = timeClient.getHours();
      ctmin = timeClient.getMinutes();
    }
  }
  state = true;
}

void handleRoot() {
  String html = "<!DOCTYPE html>"
                "<html lang='en'>"
                "<head>"
                "<meta charset='UTF-8'>"
                "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<style>"
                "body{font-family:Arial,sans-serif;margin:0;padding:0;background-color:#f4f4f4;display:flex;justify-content:center;align-items:center;min-height:100vh}.container{background-color:#fff;padding:20px;border-radius:10px;box-shadow:0 0 10px rgba(0,0,0,.1);width:80%;max-width:400px}label{display:block;margin-bottom:8px;font-weight:bold}input[type='number']{width:100%;padding:10px;border:1px solid #ccc;border-radius:5px;margin-bottom:15px}button{background-color:#007bff;color:#fff;border:none;padding:10px 20px;border-radius:5px;cursor:pointer}button:hover{background-color:#0056b3}"
                "</style>"
                "<title>Reminder Form</title>"
                "</head>"
                "<body>"
                "  <div class='container'>"
                "    <h2>Set a Reminder</h2>"
                "    <form action='/store' method='POST'>"
                "      <label for='hours'>Hours (24-hour format)</label>"
                "      <input type='number' id='hours' name='hour' min='0' max='23' required>"
                "      <label for='minutes'>Minutes</label>"
                "      <input type='number' id='minutes' name='minute' min='0' max='59' required>"
                "      <button type='submit'>Set Reminder</button>"
                "    </form>"
                "  </div>"
                "</body>"
                "</html>";

  server.send(200, "text/html", html);
}

void handleStore() {
  pillhour = server.arg("hour").toInt();
  pillmin = server.arg("minute").toInt();


  String response = "Reminder set for " + String(pillhour) + ":" + String(pillmin);
  server.send(200, "text/plain", response);
}

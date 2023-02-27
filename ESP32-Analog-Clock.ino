#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include "time.h"
#include "sntp.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

int hrs=0;
int mins=0;
int secs=0;

Adafruit_SSD1306 display(128, 64, &Wire, -1);

const int NUM_POINTS = 60;
const int RADIUS = 28;

int pointsX[NUM_POINTS];
int pointsY[NUM_POINTS];

void setup() {
  Serial.begin(115200);
  // set notification call-back function
  sntp_set_time_sync_notification_cb(timeavailable);
  sntp_servermode_dhcp(1);  // (optional)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  /***************************************************/

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  //initialize the display with I2C address 0x3C
  display.clearDisplay();                     //clear the display buffer
  display.display();                          //update the display

  for (int i = 0; i < NUM_POINTS; i++) {
    pointsX[i] = 64 + RADIUS * cos(i * 6.28 / NUM_POINTS);
    pointsY[i] = 32 + RADIUS * sin(i * 6.28 / NUM_POINTS);
  }
}

void loop() {
  setLocalTime();
  //calculate the angle for each hand
  float secAngle = map(secs, 0, 60, 0, 360);
  float minAngle = map(mins, 0, 60, 0, 360);
  float hrAngle = map(hrs, 1, 12, 30, 360);

  //calculate the positions of the hands
  int hrX = 64 + (RADIUS - 11) * cos((hrAngle - 90) * PI / 180);
  int hrY = 32 + (RADIUS - 11) * sin((hrAngle - 90) * PI / 180);
  int minX = 64 + (RADIUS - 6) * cos((minAngle - 90) * PI / 180);
  int minY = 32 + (RADIUS - 6) * sin((minAngle - 90) * PI / 180);
  int secX = 64 + (RADIUS)*cos((secAngle - 90) * PI / 180);
  int secY = 32 + (RADIUS)*sin((secAngle - 90) * PI / 180);

  display.clearDisplay();  //clear the display buffer

  //draw the clock face
  display.drawCircle(64, 32, 32, WHITE);  //Draw a Center Point
  //Draw 12 Clock points
  for (int i = 0; i < NUM_POINTS; i += 5) {
    display.fillCircle(pointsX[i], pointsY[i], 1, WHITE);
  }

  //Display Numbers 12-3-6-9
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  for (int i = 12; i > 1; i -= 3) {
    float angle = map(i, 1, 12, 30, 360);
    int xPos = 64 + (RADIUS - 7) * cos((angle - 90) * PI / 180) - 3;
    int yPos = 32 + (RADIUS - 7) * sin((angle - 90) * PI / 180) - 3;
    display.setCursor(xPos, yPos);
    display.print(i);
  }

  //draw the hour hand
  display.drawLine(64, 32, hrX, hrY, WHITE);

  //draw the minute hand
  display.drawLine(64, 32, minX, minY, WHITE);

  //draw the Second hand
  //display.drawLine(64, 32, secX, secY, WHITE);
  display.drawCircle(secX, secY, 2, WHITE);

  //update the display
  display.display();
  delay(1000);
}


void setLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  hrs = timeinfo.tm_hour;
  mins = timeinfo.tm_min;
  secs = timeinfo.tm_sec;

}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval* t) {
  Serial.println("Got time adjustment from NTP!");
  setLocalTime();
}
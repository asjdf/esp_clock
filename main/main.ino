#include <ESP8266WebServer.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "heltec.h"  // alias for `#include "SSD1306Wire.h"

#define APSSID "ESP_clock"
#define APPSK ""

const char* ssid = APSSID;
const char* password = APPSK;

ESP8266WebServer server(80);

int timeSet = 0;

void setup() {
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Serial Enable*/);
    // IPAddress local_IP(192, 168, 1, 1);
    // IPAddress gateway(192, 168, 1, 1);
    // IPAddress subnet(255, 255, 255, 0);
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.on("/set", timeHandler);
    server.onNotFound(rootHandler);  //其他功能暂时不想做 所以就这样吧
    server.begin();
    Heltec.display->clear();
    Heltec.display->setLogBuffer(2, 30);
    Heltec.display->drawString(0, 0, "Wait for connection");
    // Heltec.display->drawLogBuffer(0, 0);
    Heltec.display->display();
    delay(100);
}
void loop() {
    if (timeSet) {
        drawTime();
    }

    server.handleClient();
}

void rootHandler() {
    const char msg[] = {R"rawliteral(Time:%s
<script>
  var myDate = new Date();
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("demo").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/set?year=" + myDate.getFullYear() + "&mon=" + (myDate.getMonth()+1) + "&day=" + myDate.getDate() + "&hour=" + myDate.getHours() + "&min=" + myDate.getMinutes() + "&sec=" + myDate.getSeconds(), true);
  xhttp.send();
</script>)rawliteral"};
    char the_page[1000];

    time_t rawtime;
    struct tm* info;
    time(&rawtime);
    info = localtime(&rawtime);
    sprintf(the_page, msg, asctime(info));
    server.send_P(200, "text/html", the_page);
}
void timeHandler() {
    struct tm tmGet;
    tmGet.tm_year = server.arg("year").toInt() - 1900;
    tmGet.tm_mon = server.arg("mon").toInt();
    tmGet.tm_mday = server.arg("day").toInt();
    tmGet.tm_hour = server.arg("hour").toInt();
    tmGet.tm_min = server.arg("min").toInt();
    tmGet.tm_sec = server.arg("sec").toInt();
    time_t t = mktime(&tmGet);
    printf("Setting time: %s", asctime(&tmGet));
    timeval tmSet = {.tv_sec = t};
    settimeofday(&tmSet, NULL);
    timeSet = 1;  //时间设置完了
    server.send(200, "text/html", "Done");
}

void drawTime() {
    struct tm time0;
    time0.tm_year = 2021 - 1900;
    time0.tm_mon = 6;
    time0.tm_mday = 7;
    time0.tm_hour = 0;
    time0.tm_min = 0;
    time0.tm_sec = 0;
    time_t t = mktime(&time0);
    Serial.println(t);
    time_t rawtime;
    time(&rawtime);
    Serial.println(rawtime);
    int diffTime = t - rawtime + (8 * 60 * 60);  //一键解决时差问题
    int diffDay = diffTime / (60 * 60 * 24);
    int diffHour = diffTime % (60 * 60 * 24) / (60 * 60);
    int diffMin = diffTime % (60 * 60) / (60);
    int diffSec = diffTime % 60;
    Heltec.display->clear();
    char str1[30];
    sprintf(str1, "Left: %dD %02dH %02dM %02dS", diffDay, diffHour, diffMin,
            diffSec);
    Heltec.display->setLogBuffer(2, 20);
    Heltec.display->println(str1);
    Heltec.display->drawLogBuffer(0, 0);
    Heltec.display->display();
}
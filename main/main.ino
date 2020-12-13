#include <ESP8266WebServer.h>
#include <avr/pgmspace.h>
#include <sys/time.h>
#include <time.h>
#include "heltec.h"  // alias for `#include "SSD1306Wire.h"

#define APSSID "ESP_clock"
#define APPSK ""
#define TIMEZONE "CST-8"

const char* ssid = APSSID;
const char* password = APPSK;

ESP8266WebServer server(80);

int timeSet = 0;

void setup() {
    setenv("TZ", TIMEZONE, 1);  // mountain time zone from #define at top
    tzset();
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
}
void loop() {
    server.handleClient();
    // time_t rawtime;
    // struct tm* info;
    // time(&rawtime);
    // info = localtime(&rawtime);
    // Serial.println(asctime(info));
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
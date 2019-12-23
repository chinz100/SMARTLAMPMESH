#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
//#include <StackArray.h>
#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN 0x10
#endif

#define BUTTON_A_PIN 35 /// interupt Defual reset

#define Relay0 17
#define Relay1 22

#include "EmonLib.h"
#include "IPAddress.h" //MESH
#include <WiFi.h>
#include <EEPROM.h>
#include <ESPmDNS.h>           ///DNS for wifi.h
#include <AsyncTCP.h>          /// TCP
#include <ESPAsyncWebServer.h> //Async Webserver
#include "painlessMesh.h"
#include "Button2.h"
#define MAX_LED_COUNT 10
#define MAXNODE 20
#define LAMP_ON HIGH
#define LAMP_OFF LOW
#define DelayMs 500
/////////////////////////
EnergyMonitor emon0;
EnergyMonitor emon1;

bool PINCHECKstatus0 = false; // 33
bool PINCHECKstatus1 = false; // 32
double Irms0 = 0.00;
double Irms1 = 0.00;
Button2 buttonA = Button2(BUTTON_A_PIN);
TaskHandle_t Buttonrealtime;
unsigned int timerbutton = 0;

bool scanstate = false,
     check = false,
     checkbutton = false,
     setupserver = false,
     stop_it = false;

unsigned long milibuttonchange = 0;
unsigned long down_time = 0;
/////////////////////////

TaskHandle_t serverpage1, logServerTask, asynchandle;
///////////////////////////
TFT_eSPI tft = TFT_eSPI(280, 190);
const char *HOSTNAME = "esp32local",
           *MESH_PREFIX = "SmartC1",
           *MESH_PASSWORD = "123456789";
const int MESH_PORT = 5556;
#define Switchprogram 13
byte numbernode;

String timenow,

    timeON0 = "99:99",
    timeON1 = "99:99",
    timeOFF0 = "99:99",
    timeOFF1 = "99:99";

byte LEDon[MAX_LED_COUNT]; //= 18; 2;

size_t logServerId = 0;
size_t MynodeId[MAXNODE];
String choosenNode;
String devmode = "";
String rssirec;
/// web Meshrecive
byte SendOK = 0;
String Clientstatus0 = "false"; ///server
String Clientstatus1 = "false";

String mobilecode PROGMEM = R"=====(<meta name="viewport" content="width=device-width, initial-scale=1.0">)=====",
                  csslamp PROGMEM = R"=====(<style>ul.ks-cboxtags{list-style: none; padding: 20px;}ul.ks-cboxtags li{display: inline;}ul.ks-cboxtags li label{display: inline-block; background-color: rgba(255, 255, 255, .9); border: 2px solid rgba(139, 139, 139, .3); color: #adadad; border-radius: 25px; white-space: nowrap; margin: 3px 0px; -webkit-touch-callout: none; -webkit-user-select: none; -moz-user-select: none; -ms-user-select: none; user-select: none; -webkit-tap-highlight-color: transparent; transition: all .2s;}ul.ks-cboxtags li label{padding: 8px 12px; cursor: pointer;}ul.ks-cboxtags li label::before{display: inline-block; font-style: normal; font-variant: normal; text-rendering: auto; -webkit-font-smoothing: antialiased; font-family: "Font Awesome 5 Free"; font-weight: 900; font-size: 12px; padding: 2px 6px 2px 2px; content: "\f067"; transition: transform .3s ease-in-out;}ul.ks-cboxtags li input[type="checkbox"]:checked + label::before{content: "\f00c"; transform: rotate(-360deg); transition: transform .3s ease-in-out;}ul.ks-cboxtags li input[type="checkbox"]:checked + label{border: 2px solid #1bdbf8; background-color: #12bbd4; color: #fff; transition: all .2s;}ul.ks-cboxtags li input[type="checkbox"]{display: absolute;}ul.ks-cboxtags li input[type="checkbox"]{position: absolute; opacity: 0;}ul.ks-cboxtags li input[type="checkbox"]:focus + label{border: 2px solid #e9a1ff;}.onoffswitch{position: relative;width: 90px;-webkit-user-select: none;-moz-user-select: none;-ms-user-select: none;}.onoffswitch-checkbox{display: none;}.onoffswitch-label{display: block;overflow: hidden;cursor: pointer;border: 2px solid #999999;border-radius: 20px;}.onoffswitch-inner{display: block;width: 200%;margin-left: -100%;transition: margin 0.3s ease-in 0s;}.onoffswitch-inner:before,.onoffswitch-inner:after{display: block;float: left;width: 50%;height: 30px;padding: 0;line-height: 30px;font-size: 14px;color: white;font-family: Trebuchet, Arial, sans-serif;font-weight: bold;box-sizing: border-box;}.onoffswitch-inner:before{content: "ON";padding-left: 10px;background-color: #34A7C1;color: #FFFFFF;}.onoffswitch-inner:after{content: "OFF";padding-right: 10px;background-color: #EEEEEE;color: #999999;text-align: right;}.onoffswitch-checkbox:checked+.onoffswitch-label .onoffswitch-inner{margin-left: 0;}.onoffswitch-checkbox:checked+.onoffswitch-label .onoffswitch-switch{right: 0px;}.navbar{display: none;}.navbar + .docs-section{border-top-width: 0;}.navbar, .navbar-spacer{display: block; width: 100%; height: 6.5rem; background: #fff; z-index: 99; border-top: 1px solid #eee; border-bottom: 1px solid #eee;}.navbar-spacer{display: none;}.navbar > .container{width: 100%;}.navbar-list{list-style: none; margin-bottom: 0;}.navbar-item{position: relative; float: left; margin-bottom: 0;}.navbar-link{text-transform: uppercase; font-size: 11px; font-weight: 600; letter-spacing: .2rem; margin-right: 35px; text-decoration: none; line-height: 6.5rem; color: #222;}.navbar-link.active{color: #33C3F0;}.has-docked-nav .navbar{position: fixed; top: 0; left: 0;}.has-docked-nav .navbar-spacer{display: block;}.has-docked-nav .navbar > .container{width: 80%;}</style>)=====",
                  style1 PROGMEM = R"=====(<style>#file-input,input{width: 100%;height: 44px;border-radius: 4px;margin: 10px auto;font-size: 15px}input{background: #b0a9a9;border: 0;padding: 0 15px}body{background: #DFBE95;font-family: sans-serif;font-size: 14px; color: #777; text-align: center;}#file-input{padding: 0;border: 1px solid #ddd;line-height: 44px;text-align: left;display: block;cursor: pointer;}#bar,#prgbar{background-color: #f1f1f1;border-radius: 10px}#bar{background-color: #FF9B34;width: 0%;height: 10px}form{background: #fff;max-width: 400px;margin: 100px auto;padding: 10px;border-radius: 5px;text-align: center;}div{background: #fff;max-width: 400px;margin: 3px auto;border-radius: 10px; text-align: center; display: inline-block;}.divonoff{width: 400;}.w3-btn,.w3-button{border: none;display: inline-block;padding: 8px 16px;vertical-align: middle;overflow: hidden;text-decoration: none;color: inherit;background-color: inherit;text-align: center;cursor: pointer;white-space: nowrap;}.w3-btn,.w3-button{-webkit-touch-callout: none;-webkit-user-select: none;-khtml-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;}.w3-disabled,.w3-btn:disabled,.w3-button:disabled{cursor: not-allowed;opacity: 0.3;}.w3-disabled *,:disabled *{pointer-events: none;}.w3-dropdown-hover:hover>.w3-button:first-child,.w3-dropdown-click:hover>.w3-button:first-child{background-color: #ccc;color: #000;}.w3-bar-block .w3-dropdown-hover .w3-button,.w3-bar-block .w3-dropdown-click .w3-button{width: 100%;text-align: left;padding: 8px 16px;}.w3-bar .w3-button{white-space: normal;}.w3-dropdown-hover.w3-mobile,.w3-dropdown-hover.w3-mobile .w3-btn,.w3-dropdown-hover.w3-mobile .w3-button,.w3-dropdown-click.w3-mobile,.w3-dropdown-click.w3-mobile .w3-btn,.w3-dropdown-click.w3-mobile .w3-button{width: 100%;}.w3-button:hover{color: #000!important;background-color: #ccc!important;}.w3-deep-orange,.w3-hover-deep-orange:hover{color: #fff!important;background-color: #ff5722!important;}.btn{background: #3498db;color: #fff; cursor: pointer;}</style>)=====",
                  handindex PROGMEM = R"=====(<head> <title>Wellcome 56027288</title></head><body> <div class="divonoff"> <h1>Control Lamp!</h1> <a href='/scanfon' class='w3-button w3-deep-orange'>SCAN WIFI</a> <p></p><a href='/listnode' class='w3-button w3-deep-orange'>Control</a> <p></p><a href='/lampconfig' class='w3-button w3-deep-orange'>ALL Control</a> <p></p><a href='/ip' class='w3-button w3-deep-orange'>Check IP</a> <p></p><a href='/reset' class='w3-button w3-deep-orange'>Reset Mesh</a> <p></p><h3>Mesh Lamp Control 56027288</h3> </div>)=====",
                  javaNode PROGMEM = R"=====(<script>function onoff(e){var n=e.name+"="+e.checked,t=new XMLHttpRequest;t.open("POST","Sendstatnode",!1),t.setRequestHeader("Content-Type","application/x-www-form-urlencoded"),t.send(n)}function getData(e){var n=new XMLHttpRequest;n.onreadystatechange=function(){if(4==this.readyState&&200==this.status)for(var e=JSON.parse(this.responseText),n=0;n<e.length;n++)j=e[n],document.getElementById(j.name)&&("led0"===j.name?document.getElementById(j.name).checked=j.val:document.getElementById(j.name).innerHTML=j.val),document.getElementById(j.name)&&("led1"===j.name?document.getElementById(j.name).checked=j.val:document.getElementById(j.name).innerHTML=j.val)},n.open("GET",e,!1),n.send()}setInterval(function(){getData("info")},3e3);</script>)=====",
                  allJSnode PROGMEM = R"=====(<script>function allmesh(e){var n="?"+e.name+"="+e.checked,t=new XMLHttpRequest;t.open("GET",n,!1),t.send(n)}   </script>)=====",
                  selecttime PROGMEM = R"=====(<form id="formname" action="#" method="post"> <div class="navbar"> <font size="2"><b>Change node name : </b><font> <input type="text" name="nodename" placeholder="Enter name zone.." style="width: min-content;background: #bff9ff;border: 1px solid #999999;" maxlength="20"> <input type="submit" value="Submit"> </form> <ul class="ks-cboxtags"><form id="form1" action="#" method="post"><h2> --- Lamp Timer --- </h2> <li><input type="checkbox" id="LED0" value="LED0" name="LED0"><label for="LED0">LAMP 0</label> </li><li><input type="checkbox" id="LED1" name="LED1" value="LED1"><label for="LED1">LAMP 1</label> </li><br><div id="Hourdropdown"> <select id="Hourdropdown" name="HourSelect" class="HourSelect"> <option value="0">--Select Hour--</option> </select> <select id="Mindropdown" name="MinSelect" class="MinSelect"> <option value="0">--Select Minutes--</option> </select> <br><input name="TurnOn" type="submit" value="TimerON"> <input name="TurnOff" type="submit" value="TimerOFF"> <input name="Cleartime" type="submit" value="ClearTimer"> </div></form> </ul> </div>)=====",
                  javatime PROGMEM = R"=====(<script>window.onload=function(){for(var e=document.getElementById("Hourdropdown").getElementsByTagName("select")[0],o=0;o<=23;o++){(t=new Option).text=o+" HH",t.value=o,e.options[o]=t}var n=document.getElementById("Mindropdown");for(o=0;o<=59;o++){var t;(t=new Option).text=o+" MM",t.value=o,n.options[o]=t}};</script>)=====",
                  javaS PROGMEM = R"=====(<script>function findnode(e){var n=e.name+"="+e.checked,o=new XMLHttpRequest;o.open("POST","choosenode",!1),o.setRequestHeader("Content-Type","application/x-www-form-urlencoded"),o.send(n)}</script>)=====",
                  sclip PROGMEM = R"=====(<script>function onoff(e){var n=e.name+"="+e.checked,o=new XMLHttpRequest;o.open("POST","onoff",!1),o.setRequestHeader("Content-Type","application/x-www-form-urlencoded"),o.send(n)}</script>)=====",
                  htmljava PROGMEM = R"=====(<h1>Please waiting...ScanNetworks !!</h1><script>location.replace("/joinwifi")</script>)=====",
                  Scriptlist PROGMEM = R"=====(<script>location.replace("/listshow")</script>)=====",
                  gohomepage PROGMEM = R"=====(<script>location.replace("/")</script>)=====";
IPAddress getlocalIP();
// All Mesh Function
Scheduler userScheduler; // to control your personal task
painlessMesh mesh;

String nodeName = "";   ////readname form web
String EEPROMnode = ""; /////readname form node

AsyncWebServer server(80);
IPAddress myIP(0, 0, 0, 0);
IPAddress myAPIP(0, 0, 0, 0);

const char *PARAM_MESSAGE = "message",
           *ssidAP = "SmartC",
           *passwordAP = "123456789",
           *domain = "esp32local";
////////////////////////////////////////////////// EEPROM
String Essid = "", //EEPROM Network SSID
    Epass = "",    //EEPROM Network Password
    sssid = "",    //Read SSID From Web Page
    passs = "";    //Read Password From Web Page
///////////////////////////////
#define ARRAYSIZE 20
int selectwifi;
String ssidconnect,
    inPass = "",
    findwifi[ARRAYSIZE],
    teststr[ARRAYSIZE],
    ssid_scan;

// Prototypes
void receivedCallback(size_t from, String &msg); // client
////// MeshClient Function sendtoserver
//void sendMessage();
//Task taskSendMessage(1000, TASK_FOREVER, &sendMessage);
//void sendMessage()
//{
//client send
//}

void setup()
{
  Serial.begin(115200);
  emon0.current(33, 66.6); // pin0
  emon1.current(32, 66.6); // pin1
  EEPROM.begin(512);
  LEDon[0] = Relay0;
  LEDon[1] = Relay1;
  tft.init();
  tft.fillScreen(TFT_RED);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  pinMode(LEDon[0], OUTPUT);
  pinMode(LEDon[1], OUTPUT);
  pinMode(Switchprogram, INPUT); // input down = master
  EEPROMwifi();
  readeepromnode();
  buttonResetDefault();

  if (digitalRead(Switchprogram) == 1)
  {
    ClientMesh();
    devmode = "Mesh mode";
    tft.drawString(devmode, 40, 65, 2);
    setupserver = true;
  }
  else if (digitalRead(Switchprogram) == 0)
  {
    MeshServersetup();

    xTaskCreatePinnedToCore(
        asyncPV,      /* Task function. */
        "asyncN",     /* name of task. */
        (1024 * 6),   /* Stack size of task */
        NULL,         /* parameter of the task */
        9,            /* priority of the task */
        &asynchandle, /* Task handle to keep track of created task */
        0);           /* pin task to core 0 */
    // Add the task to the your scheduler

    xTaskCreatePinnedToCore(
        PVlogServerTask,  /* Task function. */
        "NlogServerTask", /* name of task. */
        (1024 * 3),       /* Stack size of task */
        NULL,             /* parameter of the task */
        9,                /* priority of the task */
        &logServerTask,   /* Task handle to keep track of created task */
        0);
    ///////////////
  }

  tft.drawString("Smart Lamp control", 40, 51, 2);

  xTaskCreatePinnedToCore(
      serverpagePV,  /* Task function. */
      "serverpageN", /* name of task. */
      (1024 * 4),    /* Stack size of task */
      NULL,          /* parameter of the task */
      5,             /* priority of the task */
      &serverpage1,  /* Task handle to keep track of created task */
      0);            /* pin task to core 0 */
  // Add the task to the your scheduler
}

void loop()
{

  mesh.update();
  void clear();
}
void serverpagePV(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(200);

    Irms0 = emon0.calcIrms(1480); // Calculate Irms only
    Irms1 = emon1.calcIrms(1480); // Calculate Irms only
                                  ///// pin 0 sensor
    if (Irms0 > 60.00)
    {
      PINCHECKstatus0 = true;
    }
    else
      PINCHECKstatus0 = false;
    Serial.printf("Irms0 = %0.2f \n", Irms0); // Irms
    ///// pin 1 sensor
    if (Irms1 > 60.00)
    {
      PINCHECKstatus1 = true;
    }
    else
      PINCHECKstatus1 = false;
    Serial.printf("Irms1 = %0.2f \n", Irms1); // Irms

    if (down_time > 3000)
    {

      vTaskDelete(NULL);
    }
    if (setupserver == true)
    {
      buttonA.loop();

      vTaskDelay(500);
      tft.drawString("AP is " + myAPIP.toString(), 140, 65, 2);
      tft.drawString(Led0Status() + "  ", 180, 51, 2);
      tft.drawString(Led1Status() + "  ", 240, 51, 2);
      tft.drawString("ALL node = " + String(mesh.getNodeList().size() + 1) + "   ", 40, 170, 2);
      //   tmp_rssi = String(getPowerPercentage(WiFi.RSSI(0));
      tft.drawString(devmode, 40, 65, 2);
      if (timeON0 != "99:99")
        tft.drawString("LAMP 0 ON = " + timeON0 + "     ", 160, 125, 1.85);
      else
        tft.drawString("                                                 ", 160, 125, 1.85);
      if (timeON1 != "99:99")
        tft.drawString("LAMP 1 ON = " + timeON1 + "     ", 160, 140, 1.85);
      else
        tft.drawString("                                                 ", 160, 140, 1.85);
      if (timeOFF0 != "99:99")
        tft.drawString("LAMP 0 OFF = " + timeOFF0 + "    ", 160, 155, 1.85);
      else
        tft.drawString("                                                 ", 160, 155, 1.85);
      if (timeOFF1 != "99:99")
        tft.drawString("LAMP 1 OFF = " + timeOFF1 + "    ", 160, 170, 1.85);
      else
        tft.drawString("                                                 ", 160, 170, 1.85);

      //(WiFi.encryptionType(0) == WIFI_AUTH_OPEN) ? " " : "*";

      rssirec = getPowerPercentage(WiFi.RSSI());
      tft.drawString("Signal " + WiFi.SSID() + " RSSI : " + rssirec + "  %  ", 40, 105, 2);
      tft.drawString("Time = " + timenow + "   ", 40, 120, 2);
      //////interupt Hard reset
    }
    ////////////////////////
  }
}

IPAddress getlocalIP()
{
  return IPAddress(mesh.getStationIP());
}

////server Recive
void receivedCallbackserver(uint32_t from, String &msg)
{ //Server Recive
  //Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  Serial.printf("Server: Received from %u msg=%s\n", from, msg.c_str());
  DynamicJsonDocument doc(1024 * 4);
  deserializeJson(doc, msg);
  JsonObject obj = doc.as<JsonObject>();
  if (obj.containsKey("IDnodename"))
  {
    if (String("MyClient").equals(obj["IDnodename"].as<String>()))
    {
      /*
        byte n = 0;

        for (auto nodes = mesh.getNodeList().begin(); nodes != mesh.getNodeList().end(); nodes++)
        {
        ++n;
        MynodeId[n] = ("%s", *nodes);
        }
      */
      for (int i = 1; i < mesh.getNodeList().size() + 1; i++)
      {
        if (doc[String(MynodeId[i])].as<String>() == "null")
        {
          //Serial.println("0");
        }
        else
        {
          teststr[i] = doc[String(MynodeId[i])].as<String>();
        }
      }
    }
  }
  if (obj.containsKey("topic"))
  {
    if (String("MyClient").equals(obj["topic"].as<String>()))
    {
      Clientstatus0 = doc["Statusled0"].as<String>();
      Clientstatus1 = doc["Statusled1"].as<String>();
      SendOK = doc[String("SendOK")];
    }
  }
  void clear();
}

void receivedCallback(uint32_t from, String &msg)
{ //Client Recive
  Serial.printf("Client: Received from %u msg=%s\n", from, msg.c_str());
  // Saving logServerID
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, msg);
  JsonObject obj = doc.as<JsonObject>();

  if (obj.containsKey("topic"))
  { // topic = Find serverID
    if (String("MyServer").equals(obj["topic"].as<String>()))
    {
      logServerId = obj["nodeId"];
      String Jsontimenow = obj["Times"];
      timenow = Jsontimenow;
      //timer
      if (timenow == timeON0) ///LED 0 ON
      {
        if (Led0Status() == "false")
        {
          if (digitalRead(LEDon[0]) == LOW)
            digitalWrite(LEDon[0], HIGH);
          else if (digitalRead(LEDon[0]) == HIGH)
            digitalWrite(LEDon[0], LOW);
        }
      }
      if (timenow == timeON1) ///LED 1 ON
      {
        if (Led1Status() == "false")
        {
          if (digitalRead(LEDon[1]) == LOW)
            digitalWrite(LEDon[1], HIGH);
          else if (digitalRead(LEDon[1]) == HIGH)
            digitalWrite(LEDon[1], LOW);
        }
      }
      if (timenow == timeOFF0) // led 0 off
      {
        if (Led0Status() == "true")
        {
          if (digitalRead(LEDon[0]) == LOW)
            digitalWrite(LEDon[0], HIGH);
          else if (digitalRead(LEDon[0]) == HIGH)
            digitalWrite(LEDon[0], LOW);
        }
      }
      if (timenow == timeOFF1) // led 1 off
      {
        if (Led1Status() == "true")
        {
          if (digitalRead(LEDon[1]) == LOW)
            digitalWrite(LEDon[1], HIGH);
          else if (digitalRead(LEDon[1]) == HIGH)
            digitalWrite(LEDon[1], LOW);
        }
      }
    }
    Serial.println("Server detected!!! ServerId =" + String(logServerId) + ",Recive time = " + String(timenow));
    /// Find server
  }
  //Client send status to server
  if (obj.containsKey("CMD"))
  { //CMD = controlnode
    if (String("NameStatus").equals(obj["Status"].as<String>()))
    {
      String idmesh = String(mesh.getNodeId());
      DynamicJsonDocument jsonBuffer(1024);
      JsonObject stat = jsonBuffer.to<JsonObject>();
      stat["IDnodename"] = "MyClient";
      stat[String(idmesh)] = nodeName.c_str();
      String output;
      serializeJson(stat, output);
      mesh.sendSingle(logServerId, output);
      output = "";
      void clear();
    }
    if (String("Callstatus").equals(obj["Status"].as<String>()))
    {
      DynamicJsonDocument jsonBuffer(1024);
      JsonObject stat = jsonBuffer.to<JsonObject>();
      stat["topic"] = "MyClient";
      stat["nodeId"] = mesh.getNodeId();
      stat["Statusled0"] = Led0Status();
      stat["Statusled1"] = Led1Status();
      stat["SendOK"] = "1";
      String output;
      serializeJson(stat, output);
      mesh.sendSingle(logServerId, output);
      void clear();
    }
    if (String("ResetALL").equals(obj["Status"].as<String>()))
    {
      ESP.restart();
    }

    ////Client control lamp
    if (String("ControlLAMP").equals(obj["Status"].as<String>()))
    {
      String ARGname = obj["ARGname"]; ////LED0arg //LED1arg  to 0 or 1
      String ARGstat = obj["ARGstat"]; ////true //false
      //     Serial.println(ARGname+","+ARGstat);
      ARGname = ARGname.substring(3, 4);
      // Serial.println(ARGname + "," + ARGstat);
      //uint8_t cmd = (ARGstat == "true") ? LAMP_ON : LAMP_OFF;
      if (digitalRead(LEDon[ARGname.toInt()]) == LOW)
        digitalWrite(LEDon[ARGname.toInt()], HIGH);
      else if (digitalRead(LEDon[ARGname.toInt()]) == HIGH)
        digitalWrite(LEDon[ARGname.toInt()], LOW);
      // digitalWrite(LEDon[ARGname.toInt()], cmd);
    }
    /////// broadcastlamp
    if (String("ALLcontrol").equals(obj["Status"].as<String>()))
    {
      String ARGname = obj["ARGname"]; ////LED0arg //LED1arg  to 0 or 1
      String ARGstat = obj["ARGstat"]; ////true //false
      //     Serial.println(ARGname+","+ARGstat);
      ARGname = ARGname.substring(3, 4);
      // Serial.println(ARGname + "," + ARGstat);
      //uint8_t cmd = (ARGstat == "true") ? LAMP_ON : LAMP_OFF;
      if (ARGname == "0")
      {
        if (ARGstat == "true")
        {
          if (Led0Status() == "false") // LED 0 on
          {
            if (digitalRead(LEDon[0]) == LOW)
              digitalWrite(LEDon[0], HIGH);
            else if (digitalRead(LEDon[0]) == HIGH)
              digitalWrite(LEDon[0], LOW);
          }
        }
        else if (ARGstat == "false")
        {
          if (Led0Status() == "true") // LED 0 off
          {
            if (digitalRead(LEDon[0]) == LOW)
              digitalWrite(LEDon[0], HIGH);
            else if (digitalRead(LEDon[0]) == HIGH)
              digitalWrite(LEDon[0], LOW);
          }
        }
      }
      else if (ARGname == "1")
      {
        if (ARGstat == "true")
        {
          if (Led1Status() == "false") // LED 1 on
          {
            if (digitalRead(LEDon[1]) == LOW)
              digitalWrite(LEDon[1], HIGH);
            else if (digitalRead(LEDon[1]) == HIGH)
              digitalWrite(LEDon[1], LOW);
          }
        }
        else if (ARGstat == "false")
        {
          if (Led1Status() == "true") // LED 1 OFF
          {
            if (digitalRead(LEDon[1]) == LOW)
              digitalWrite(LEDon[1], HIGH);
            else if (digitalRead(LEDon[1]) == HIGH)
              digitalWrite(LEDon[1], LOW);
          }
        }
      }

      // digitalWrite(LEDon[ARGname.toInt()], cmd);
    }
    ///Client timer

    if (String("TimeLamp").equals(obj["Status"].as<String>()))
    {
      byte LAMP0 = obj["LAMP0"];
      byte LAMP1 = obj["LAMP1"];
      byte HourSelect = obj["HourSelect"];
      byte MinSelect = obj["MinSelect"];
      byte TurnOn = obj["TurnOn"];
      byte TurnOff = obj["TurnOff"];
      byte Cleartime = obj["Cleartime"];

      //Serial.println(LAMP0+","+LAMP1+","+HourSelect+","+MinSelect+","+TurnOn+","+TurnOff);
      /* Gobal var
        timeON0
        timeON1
        timeOFF0
        timeOFF
      */

      if (LAMP0 == 1 && TurnOn == 1)
        timeON0 = String(HourSelect) + ":" + String(MinSelect);
      else if (LAMP0 == 1 && TurnOff == 1)
        timeOFF0 = String(HourSelect) + ":" + String(MinSelect);
      else if (LAMP0 == 1 && Cleartime == 1)
        timeOFF0 = "99:99", timeON0 = "99:99";
      if (LAMP1 == 1 && TurnOn == 1)
        timeON1 = String(HourSelect) + ":" + String(MinSelect);
      else if (LAMP1 == 1 && TurnOff == 1)
        timeOFF1 = String(HourSelect) + ":" + String(MinSelect);
      else if (LAMP1 == 1 && Cleartime == 1)
        timeOFF1 = "99:99", timeON1 = "99:99";

      Serial.println(timenow);
      Serial.println(timeON0);
      Serial.println(timeON1);
      Serial.println(timeOFF0);
      Serial.println(timeOFF1);
    }

    if (String("ChangeName").equals(obj["Status"].as<String>()))
    {
      EEPROMnode = obj["NewName"].as<String>(); ////LED0arg //LED1arg  to 0 or 1
      Serial.println(EEPROMnode);
      nodeeeprom();
      ESP.restart();
    }
  }
  void clear();
}

String Led0Status()
{
  return (PINCHECKstatus0 == LAMP_OFF) ? "false" : "true";
}

String Led1Status()
{
  return (PINCHECKstatus1 == LAMP_OFF) ? "false" : "true";
}

void vchoosenode(AsyncWebServerRequest *request)
{
  //can't use node1 node2   Arg
  //use nod1  Arg

  //Serial.println(request->argName(0));
  // nod1 to choosenNode = 1
  // nod0 to choosenNode = 0
  choosenNode = request->argName(0).substring(3);
  //Serial.println("Choosen Node = " + String(MynodeId[choosenNode.toInt()]));
  request->send(200);
}

void vSendstatnode(AsyncWebServerRequest *request)
{

  int args = request->args();
  String ledweb;
  for (int i = 0; i < args; i++)
  {
    DynamicJsonDocument jsonBuffer(256);
    JsonObject msg = jsonBuffer.to<JsonObject>();
    msg["CMD"] = "ServerCMD";
    msg["Status"] = "ControlLAMP";
    msg["ARGname"] = request->argName(i).c_str();
    msg["ARGstat"] = request->arg(i).c_str();
    String str;
    serializeJson(msg, str);
    mesh.sendSingle(MynodeId[choosenNode.toInt()], str);
    void clear();
    //Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }
  DynamicJsonDocument jsonBuffer(256);
  JsonObject sta = jsonBuffer.to<JsonObject>();
  sta["CMD"] = "ServerCMD";
  sta["Status"] = "Callstatus";
  String output;
  serializeJson(sta, output);
  mesh.sendSingle(MynodeId[choosenNode.toInt()], output);
  void clear();
  request->send(200);
}

void HandleOnOff(AsyncWebServerRequest *request)
{

  Serial.println(request->argName(0));
  //uint8_t cmd = (server.arg(0) == "true") ? LOW : HIGH;
  String poiter = request->argName(0).substring(3, 4); //// LED 0 arg
  int args = request->args();
  for (int i = 0; i < args; i++)
  {
    if ((request->argName(i) == ("LED" + poiter + "arg")))
    {
      //uint8_t cmd = (request->arg(i) == "true") ? LAMP_ON : LAMP_OFF;
      if (digitalRead(LEDon[poiter.toInt()]) == LOW)
        digitalWrite(LEDon[poiter.toInt()], HIGH);
      else if (digitalRead(LEDon[poiter.toInt()]) == HIGH)
        digitalWrite(LEDon[poiter.toInt()], LOW);
      // Serial.println(poiter+String(request->arg(i)));
    }
    // Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }
  request->send(200);
}

void HandleInfo(AsyncWebServerRequest *request)
{

  String data = "[";
  data += "{\"name\": \"nodeid\",\"val\":" + String(MynodeId[choosenNode.toInt()]) + "}";
  data += ",{\"name\":\"led0\",\"val\":" + Clientstatus0 + "}";
  data += ",{\"name\":\"led1\",\"val\":" + Clientstatus1 + "}";
  data += "]";
  request->send(200, "application/json", data);

  StaticJsonDocument<256> sta;
  sta["CMD"] = "ServerCMD";
  sta["Status"] = "Callstatus";
  String output;
  serializeJson(sta, output);
  mesh.sendSingle(MynodeId[choosenNode.toInt()], output);
  sta = "";
  output = "";
  void clear();
}

void EEPROMwifi()
{

  /////////// EEPROM  wifi ////////////////
  for (int i = 0; i < 32; ++i) //Reading SSID EEPROM
  {
    Essid += char(EEPROM.read(i));
  }
  for (int i = 32; i < 96; ++i) //Reading Password EEPROM
  {
    Epass += char(EEPROM.read(i));
  }
  if (Essid.length() > 1)
  {
    Serial.println(Essid.c_str()); //Print SSID
    Serial.println(Epass.c_str()); //Print Password
  }
  return;
}
void MeshServersetup()
{
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE | DEBUG ); // all types on
  //mesh.setDebugMsgTypes( ERROR | CONNECTION | SYNC | S_TIME );  // set before init() so that you can see startup messages
  //mesh.setDebugMsgTypes( ERROR | CONNECTION | S_TIME );  // set before init() so that you can see startup messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&receivedCallbackserver);
  mesh.stationManual(Essid.c_str(), Epass.c_str());
  mesh.setHostname(HOSTNAME);
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  mesh.onChangedConnections(&changedConnectionCallback);
  myAPIP = IPAddress(mesh.getAPIP());
  tft.drawString("AP is " + myAPIP.toString(), 140, 65, 2);

  mesh.onNewConnection([](size_t nodeId) {
    Serial.printf("HiNew Connection %u\n", nodeId);
    //addstk.push(nodeId);
  });

  mesh.onDroppedConnection([](size_t nodeId) {
    Serial.printf("Dropped Connection %u\n", nodeId);
  });

  //userScheduler.addTask(logServerTask);
  //logServerTask.enable();
  return;
}
/////// Slave Mesh////
void ClientMesh()
{
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  // userScheduler.addTask(taskSendMessage);
  // taskSendMessage.enable();
  MynodeId[0] = mesh.getNodeId();
}

void timeshow()
{
  String years;
  static const char default_format[] = "%a %b %d %Y";
  int timezone = 7 * 3600;
  int dst = 0;
  configTime(timezone, dst, "time.cloudflare.com", "time.navy.mi.th");
  while (!time(nullptr))
  {
    vTaskDelay(200);
  }
  do
  {
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    // Serial.println(String(p_tm->tm_hour)+":"+String(p_tm->tm_min));
    timenow = String(p_tm->tm_hour) + ":" + String(p_tm->tm_min);
  } while (years == "1970");
  return;
}

void nodeeeprom()
{
  //First Clear Eeprom
  for (int i = 100; i < 150; ++i)
  {
    EEPROM.write(i, 0);
  }
  for (int i = 0; i < EEPROMnode.length(); ++i)
  {
    EEPROM.write(100 + i, EEPROMnode[i]);
  }
  EEPROM.commit();
  return;
}

void readeepromnode()
{
  for (int i = 100; i < 150; ++i) //Reading SSID EEPROM
  {
    nodeName += char(EEPROM.read(i));
  }
  nodeName += "\0";
  return;
}

void buttonResetDefault()
{
  buttonA.setPressedHandler(pressed);
  buttonA.setReleasedHandler(released);
  buttonA.setTapHandler(tap);
  xTaskCreatePinnedToCore(
      PVbutton,        /* Task function. */
      "Tbutton",       /* name of task. */
      (2048),          /* Stack size of task */
      NULL,            /* parameter of the task */
      10,              /* priority of the task */
      &Buttonrealtime, /* Task handle to keep track of created task */
      1);
  return;
}

void pressed(Button2 &btn)
{
  Serial.println("pressed");
  checkbutton = true;
  return;
}
void released(Button2 &btn)
{
  Serial.print("released: ");
  Serial.println(btn.wasPressedFor());
  return;
}
void tap(Button2 &btn)
{
  Serial.println("tap");
  checkbutton = false;
  return;
}
void PVbutton(void *pvParameters)
{
  while (1)
  {
    if (myIP != getlocalIP())
    {
      myIP = getlocalIP();
      tft.fillScreen(TFT_WHITE);
      tft.drawString(nodeName + "        ", 40, 51, 2);
      tft.drawString("My STA IP is " + myIP.toString(), 40, 80, 2);
    }
    vTaskDelay(100);
    if (checkbutton == true)
    {
      vTaskDelay(100);
      down_time = millis() - milibuttonchange;
      if (down_time > 3500)
      {
        tft.fillScreen(TFT_RED);
        tft.fillScreen(TFT_WHITE);
        tft.drawString("Reset EEPROM   ", 40, 90, 4);
        tft.drawString("to Default... ", 40, 120, 4);
        vTaskDelay(1000);

        Serial.println("ok");
        sssid = "";
        passs = "";
        EEPROMnode = "NodeName";
        ClearEeprom();
        vTaskDelay(500);
        nodeeeprom();
        vTaskDelay(500);
        //First Clear Eeprom
        ESP.restart();
        down_time = 0;
      }
      else
      {
        Serial.print(".");
      }
    }
    else
    {
      milibuttonchange = millis();
    }
    vTaskDelay(100);
  }
}

String showlistnodefunc(String listnode)
{
  String findmesh = "";
  for (int i = 0; i < numbernode; i++)
  {
    if (i == 0)
    {
      findmesh += "<a href='/lampcontrol' class='w3-button w3-deep-orange'> (" + String(MynodeId[i]) + ")  Lamp " + String(nodeName.c_str()) + "</a><br><br>";
    }
    else
    {
      findmesh += "<a href='/lampcontrolnode' name='nod" + String(i) + "' class='w3-button w3-deep-orange' onclick='findnode(this)'> (" + String(MynodeId[i]) + ") Lamp " + String(teststr[i]) + "</a><br><br>";
    }

    listnode = "<div class=\"divonoff\"><h1>LAMP All Control!</h1><body>";
    listnode += findmesh;
    listnode += "<br><br><a href =\"/\"><button>Back</button></a>";
    listnode += "</body></div>";
  }
  return listnode;
}
void asyncPV(void *pvParameters)
{
  setupserver = false;
  ////server
  if (!MDNS.begin(domain))
  { //http://esp32local
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(DelayMs);
    }
  }
  MDNS.addService("http", "tcp", 80);
  devmode = "Server mode";
  tft.drawString(devmode, 40, 65, 2);
  MynodeId[0] = mesh.getNodeId();
  teststr[0] = nodeName.c_str();
  server.on("/", vhandleRoot);
  ///serupwifi
  server.on("/joinwifi", vjoinwifi);
  server.on("/a", vwifiscana);
  //// control lamp
  server.on("/listnode", vlistnode);
  server.on("/scanfon", HTTP_GET, [](AsyncWebServerRequest *request) {
    void clear();
    request->send(200, "text/html", style1 + htmljava);
  });
  server.on("/listshow", HTTP_GET, [](AsyncWebServerRequest *request) {
    String listnode = "";
    request->send(200, "text/html", mobilecode + showlistnodefunc(listnode) + javaS + style1);
  });
  server.on("/lampcontrol", vlampcontrol);
  server.on("/lampcontrolnode", vlampcontrolnode);
  server.on("/lampconfig", vlampallnode);
  server.on("/reset", vreset);
  server.on("/ip", vcheckip);

  ///javaHTML
  server.on("/choosenode", vchoosenode);     /// listnode
  server.on("/onoff", HandleOnOff);          /// control Server on off
  server.on("/info", HandleInfo);            /// node status
  server.on("/Sendstatnode", vSendstatnode); ///send onoff to node
  server.onNotFound(notFound);

  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request) {
    String tex1web;
    tex1web = String("Free Memory is ") + String(ESP.getFreeHeap());
    tex1web += "<br><br>";
    tex1web += String("FlashChipSize is ") + String(ESP.getFlashChipSize());
    request->send(200, "text/html", tex1web);
  });

  server.begin();
  setupserver = true;
  vTaskDelete(NULL);
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}
void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}
void nodeTimeAdjustedCallback(int32_t offset)
{
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}
//////end MeshClient Function

// Send myServer ID every 10 seconds to inform others
void PVlogServerTask(void *pvParameters)
{
  while (1)
  {
    vTaskDelay(100);
    if (setupserver == true)
    {
      vTaskDelay(4000);
      timeshow();
      vTaskDelay(500);
      String nummesh = String(mesh.getNodeId());
      StaticJsonDocument<256> msg;
      msg["topic"] = "MyServer";
      msg["nodeId"] = nummesh;
      msg["Times"] = String(timenow);
      String str;
      serializeJson(msg, str);
      mesh.sendBroadcast(str);
      msg = "";
      str = "";
      void clear();
      //timer
      if (timenow == timeON0)
        if (Led0Status() == "false")
        {
          if (digitalRead(LEDon[0]) == LOW)
            digitalWrite(LEDon[0], HIGH);
          else if (digitalRead(LEDon[0]) == HIGH)
            digitalWrite(LEDon[0], LOW);
        }
      if (timenow == timeON1)
      {
        if (Led1Status() == "false")
        {
          if (digitalRead(LEDon[1]) == LOW)
            digitalWrite(LEDon[1], HIGH);
          else if (digitalRead(LEDon[1]) == HIGH)
            digitalWrite(LEDon[1], LOW);
        }
      }
      if (timenow == timeOFF0)
        if (Led0Status() == "true")
        {
          if (digitalRead(LEDon[0]) == LOW)
            digitalWrite(LEDon[0], HIGH);
          else if (digitalRead(LEDon[0]) == HIGH)
            digitalWrite(LEDon[0], LOW);
        }
      if (timenow == timeOFF1)
        if (Led1Status() == "true")
        {
          if (digitalRead(LEDon[1]) == LOW)
            digitalWrite(LEDon[1], HIGH);
          else if (digitalRead(LEDon[1]) == HIGH)
            digitalWrite(LEDon[1], LOW);
        }
      //////interupt Hard reset
      if (down_time > 3000)
      {
        vTaskDelete(NULL);
      }

      ////////////////////////
    }
  }
}

void notFound(AsyncWebServerRequest *request)
{
  Serial.printf("NOT_FOUND: ");
  if (request->method() == HTTP_GET)
    Serial.printf("GET");
  else if (request->method() == HTTP_POST)
    Serial.printf("POST");
  else if (request->method() == HTTP_DELETE)
    Serial.printf("DELETE");
  else if (request->method() == HTTP_PUT)
    Serial.printf("PUT");
  else if (request->method() == HTTP_PATCH)
    Serial.printf("PATCH");
  else if (request->method() == HTTP_HEAD)
    Serial.printf("HEAD");
  else if (request->method() == HTTP_OPTIONS)
    Serial.printf("OPTIONS");
  else
    Serial.printf("UNKNOWN");
  Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

  if (request->contentLength())
  {
    Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
    Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
  }

  int headers = request->headers();
  int i;
  for (i = 0; i < headers; i++)
  {
    AsyncWebHeader *h = request->getHeader(i);
    Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
  }

  int params = request->params();
  for (i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile())
    {
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    }
    else if (p->isPost())
    {
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    else
    {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  request->send(404);
}

void vwifiscana(AsyncWebServerRequest *request)
{
  String thtml = "<h2>Join new wifi success,Please reconnect....</h2>";
  request->send(200, "text/html", thtml + style1);
  int i;
  int params = request->params();
  for (i = 0; i < params; i++)
  {
    AsyncWebParameter *p = request->getParam(i);
    if (p->isFile())
    {
      Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    }
    else if (p->isPost())
    {
      Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      if (i == 0)
      {
        selectwifi = p->value().toInt();
        Serial.println(selectwifi);
      }
      else if (i == 1)
      {
        inPass = String(p->value().c_str());
        Serial.println(inPass);
      }
    }
    else
    {
      Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }
  Serial.print(F("Connect Mode Connecting to "));
  ssidconnect = findwifi[selectwifi];

  Serial.println(ssidconnect.c_str());
  Serial.println(inPass.c_str());
  sssid = ssidconnect.c_str();
  passs = inPass.c_str();
  if (sssid.length() > 1 && passs.length() > 1)
  {
    ClearEeprom(); //First Clear Eeprom
    delay(DelayMs);
    for (int i = 0; i < sssid.length(); ++i)
    {
      EEPROM.write(i, sssid[i]);
    }

    for (int i = 0; i < passs.length(); ++i)
    {
      EEPROM.write(32 + i, passs[i]);
    }
    EEPROM.commit();
  }

  ESP.restart();
}

void ClearEeprom()
{
  for (int i = 0; i < 96; ++i)
  {
    EEPROM.write(i, 0);
  }
  return;
}

void vhandleRoot(AsyncWebServerRequest *request)
{
  request->send(200, "text/html", mobilecode + handindex + style1);
  if (request->hasArg("BROADCAST"))
  {
    String msg = request->arg("BROADCAST");
    mesh.sendBroadcast(msg);
  }
}

void vlistnode(AsyncWebServerRequest *request)
{
  numbernode = mesh.getNodeList().size() + 1;
  byte n = 0;
  if (check == false)
  {
    check = true;
    //////callnamenode///
    StaticJsonDocument<256> sta;
    sta["CMD"] = "ServerCMD";
    sta["Status"] = "NameStatus";
    String output;
    serializeJson(sta, output);
    mesh.sendBroadcast(output);
    sta = "";
    output = "";
    void clear();
    /////////////
    if (numbernode > 1)
    {
      for (auto nodes = mesh.getNodeList().begin(); nodes != mesh.getNodeList().end(); nodes++)
      {
        ++n;
        MynodeId[n] = ("%s", *nodes);
      }
    }
    delay(200);
    request->send(200, "text/html", Scriptlist);
    check = false;
  }
}

void vlampcontrolnode(AsyncWebServerRequest *request)
{
  String controlnode;
  String htmoop = "";

  htmoop += "<div class=\"onoffswitch\"> <div class=\"onoffswitch__info\">  <input type=\"checkbox\" name=\"LED0arg\" class=\"onoffswitch-checkbox\" id=\"led0\" onclick=\"onoff(this)\" > <label class=\"onoffswitch-label\" for=\"led0\">Lamp 0<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div></div>";
  htmoop += "<div class=\"onoffswitch\"> <div class=\"onoffswitch__info\">  <input type=\"checkbox\" name=\"LED1arg\" class=\"onoffswitch-checkbox\" id=\"led1\" onclick=\"onoff(this)\" > <label class=\"onoffswitch-label\" for=\"led1\">Lamp 1<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div></div>";

  controlnode = "<div class='divonoff'><h1>Node " + String(teststr[choosenNode.toInt()]) + " Control!</h1><body>";
  //lampcontrol += "<form action='/'>";
  controlnode += htmoop;
  controlnode += "<br><br><a href =\"/listnode\"><button>Back</button></a>";
  controlnode += selecttime;
  controlnode += "</body></div>";
  request->send(200, "text/html", mobilecode + controlnode + javaNode + javatime + csslamp + style1);
  /////Timer/////
  StaticJsonDocument<256> sta;
  sta["CMD"] = "ServerCMD";
  sta["Status"] = "Callstatus";
  String output;
  serializeJson(sta, output);
  mesh.sendSingle(MynodeId[choosenNode.toInt()], output);
  sta = "";
  output = "";
  void clear();
  byte statz = 0, LED0 = 0, LED1 = 0, HourSelect = 99, MinSelect = 99, TurnOn = 0, TurnOff = 0, Cleartime = 0;
  int args = request->args();
  for (int i = 0; i < args; i++)
  {
    if ((request->argName(i) == ("nodename")))
    {
      String sendname = request->arg(i).c_str();
      Serial.println(sendname);
      StaticJsonDocument<256> msg;
      msg["CMD"] = "ServerCMD";
      msg["Status"] = "ChangeName";
      msg["NewName"] = sendname;
      String str;
      serializeJson(msg, str);
      mesh.sendSingle(MynodeId[choosenNode.toInt()], str);
      str = "";
      msg = "";
      void clear();
    }
    if ((request->argName(i) == ("LED0")))
      LED0 = 1;
    if ((request->argName(i) == ("LED1")))
      LED1 = 1;
    if ((request->argName(i) == ("HourSelect")))
      HourSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("MinSelect")))
      MinSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("TurnOn")))
      TurnOn = 1, statz = 1;
    if ((request->argName(i) == ("TurnOff")))
      TurnOff = 1, statz = 1;
    if ((request->argName(i) == ("Cleartime")))
      Cleartime = 1, statz = 1;
    // Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }
  /*
    Serial.println("===========================");
    Serial.println(LED0);
    Serial.println(LED1);
    Serial.println(HourSelect);
    Serial.println(MinSelect);
    Serial.println(TurnOn);
    Serial.println(TurnOff);
    Serial.println(Cleartime);
    Serial.println("===========================");
  */
  if (statz == 1)
  {
    DynamicJsonDocument jsonBuffer(256);
    JsonObject msg = jsonBuffer.to<JsonObject>();
    msg["CMD"] = "ServerCMD";
    msg["Status"] = "TimeLamp";
    msg["LAMP0"] = LED0;
    msg["LAMP1"] = LED1;
    msg["HourSelect"] = HourSelect;
    msg["MinSelect"] = MinSelect;
    msg["TurnOn"] = TurnOn;
    msg["TurnOff"] = TurnOff;
    msg["Cleartime"] = Cleartime;
    String str;
    serializeJson(msg, str);
    mesh.sendSingle(MynodeId[choosenNode.toInt()], str);
    statz = 0;
    void clear();
  }
}

void vlampcontrol(AsyncWebServerRequest *request)
{
  String lampcontrol;
  String htmloop = "";

  if (Led0Status() == "true")
  {
    htmloop += "<div class=\"onoffswitch\"> <input type=\"checkbox\" name=\"LED0arg\" class=\"onoffswitch-checkbox\" id=\"myonoffswitch0\" onclick=\"onoff(this)\" checked> <label class=\"onoffswitch-label\" for=\"myonoffswitch0\">Lamp 0<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  }
  else if (Led0Status() == "false")
  {
    htmloop += "<div class=\"onoffswitch\"> <input type=\"checkbox\" name=\"LED0arg\" class=\"onoffswitch-checkbox\" id=\"myonoffswitch0\" onclick=\"onoff(this)\" > <label class=\"onoffswitch-label\" for=\"myonoffswitch0\">Lamp 0<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  }
  if (Led1Status() == "true")
  {
    htmloop += "<div class=\"onoffswitch\"> <input type=\"checkbox\" name=\"LED1arg\" class=\"onoffswitch-checkbox\" id=\"myonoffswitch1\" onclick=\"onoff(this)\" checked> <label class=\"onoffswitch-label\" for=\"myonoffswitch1\">Lamp 1<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  }
  else if (Led1Status() == "false")
  {
    htmloop += "<div class=\"onoffswitch\"> <input type=\"checkbox\" name=\"LED1arg\" class=\"onoffswitch-checkbox\" id=\"myonoffswitch1\" onclick=\"onoff(this)\" > <label class=\"onoffswitch-label\" for=\"myonoffswitch1\">Lamp 1<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  }

  lampcontrol = "<div><h1>MyNode " + String(teststr[0]) + " Control!</h1><body>";
  //lampcontrol += "<form action='/'>";
  lampcontrol += htmloop;
  lampcontrol += "<br><br><a href =\"/listnode\"><button>Back</button></a>";
  lampcontrol += selecttime;
  lampcontrol += "</body></div>";
  request->send(200, "text/html", mobilecode + javatime + sclip + lampcontrol + csslamp + style1);
  /////Timer/////
  byte statz = 0, LED0 = 0, LED1 = 0, HourSelect = 99, MinSelect = 99, TurnOn = 0, TurnOff = 0, Cleartime = 0;
  int args = request->args();
  for (int i = 0; i < args; i++)
  {
    if ((request->argName(i) == ("nodename")))
    {
      EEPROMnode = request->arg(i).c_str();
      nodeeeprom();
      ESP.restart();
    }
    if ((request->argName(i) == ("LED0")))
      LED0 = 1;
    if ((request->argName(i) == ("LED1")))
      LED1 = 1;
    if ((request->argName(i) == ("HourSelect")))
      HourSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("MinSelect")))
      MinSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("TurnOn")))
      TurnOn = 1, statz = 1;
    if ((request->argName(i) == ("TurnOff")))
      TurnOff = 1, statz = 1;
    if ((request->argName(i) == ("Cleartime")))
      Cleartime = 1, statz = 1;
    // Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }

  /////control
  if (statz == 1)
  {
    if (LED0 == 1 && TurnOn == 1)
      timeON0 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED0 == 1 && TurnOff == 1)
      timeOFF0 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED0 == 1 && Cleartime == 1)
      timeOFF0 = "99:99", timeON0 = "99:99";
    if (LED1 == 1 && TurnOn == 1)
      timeON1 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED1 == 1 && TurnOff == 1)
      timeOFF1 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED1 == 1 && Cleartime == 1)
      timeOFF1 = "99:99", timeON1 = "99:99";

    Serial.println(timenow);
    Serial.println(timeON0);
    Serial.println(timeON1);
    Serial.println(timeOFF0);
    Serial.println(timeOFF1);

    statz = 0;
  }
}
void vlampallnode(AsyncWebServerRequest *request)
{
  String lampconfig, htmlall;
  lampconfig = "<div class=\"onoffswitch\">   <input type=\"checkbox\" name=\"LED0arg\" class=\"onoffswitch-checkbox\" id=\"led0\" onclick=\"allmesh(this)\" > <label class=\"onoffswitch-label\" for=\"led0\">Lamp 0<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  lampconfig += "<div class=\"onoffswitch\">   <input type=\"checkbox\" name=\"LED1arg\" class=\"onoffswitch-checkbox\" id=\"led1\" onclick=\"allmesh(this)\" > <label class=\"onoffswitch-label\" for=\"led1\">Lamp 1<span class=\"onoffswitch-inner\"></span><span class=\"onoffswitch-switch\"></span> </label></div> ";
  htmlall = "<div class=\"divonoff\"><h1>BROADCAST Control!</h1><body>";
  htmlall += lampconfig;
  htmlall += "<br><br><a href =\"/\"><button>Back</button></a>";
  htmlall += selecttime;
  htmlall += "</body></div>";

  request->send(200, "text/html", mobilecode + htmlall + javatime + allJSnode + csslamp + style1);
  /////Timer/////
  byte statz = 0, LED0 = 0, LED1 = 0, HourSelect = 99, MinSelect = 99, TurnOn = 0, TurnOff = 0, Cleartime = 0;
  int args = request->args();
  for (int i = 0; i < args; i++)
  {
    if ((request->argName(i) == ("nodename")))
    {
      String sendname = request->arg(i).c_str();
      Serial.println(sendname);
      StaticJsonDocument<256> msg;
      msg["CMD"] = "ServerCMD";
      msg["Status"] = "ChangeName";
      msg["NewName"] = String(sendname);
      String str;
      serializeJson(msg, str);
      mesh.sendBroadcast(str);
      delay(500);
      EEPROMnode = sendname;
      nodeeeprom();
      ESP.restart();
    }
    if ((request->argName(i) == ("LED0")))
      LED0 = 1;
    if ((request->argName(i) == ("LED1")))
      LED1 = 1;
    if ((request->argName(i) == ("HourSelect")))
      HourSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("MinSelect")))
      MinSelect = request->arg(i).toInt();
    if ((request->argName(i) == ("TurnOn")))
      TurnOn = 1, statz = 1;
    if ((request->argName(i) == ("TurnOff")))
      TurnOff = 1, statz = 1;
    if ((request->argName(i) == ("Cleartime")))
      Cleartime = 1, statz = 1;
    // Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
  }
  if (statz == 1)
  {
    if (LED0 == 1 && TurnOn == 1)
      timeON0 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED0 == 1 && TurnOff == 1)
      timeOFF0 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED0 == 1 && Cleartime == 1)
      timeOFF0 = "99:99", timeON0 = "99:99";
    if (LED1 == 1 && TurnOn == 1)
      timeON1 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED1 == 1 && TurnOff == 1)
      timeOFF1 = String(HourSelect) + ":" + String(MinSelect);
    else if (LED1 == 1 && Cleartime == 1)
      timeOFF1 = "99:99", timeON1 = "99:99";

    Serial.println(timenow);
    Serial.println(timeON0);
    Serial.println(timeON1);
    Serial.println(timeOFF0);
    Serial.println(timeOFF1);

    DynamicJsonDocument jsonBuffer(256);
    JsonObject msg = jsonBuffer.to<JsonObject>();
    msg["CMD"] = "ServerCMD";
    msg["Status"] = "TimeLamp";
    msg["LAMP0"] = LED0;
    msg["LAMP1"] = LED1;
    msg["HourSelect"] = HourSelect;
    msg["MinSelect"] = MinSelect;
    msg["TurnOn"] = TurnOn;
    msg["TurnOff"] = TurnOff;
    msg["Cleartime"] = Cleartime;
    String str;
    serializeJson(msg, str);
    mesh.sendBroadcast(str);
    statz = 0;
    void clear();
  }
  ////// BROADCAST onoff
  if (request->hasArg("LED0arg"))
  {
    String stats = request->arg("LED0arg");
    String msg = "{\"CMD\":\"ServerCMD\",\"Status\":\"ALLcontrol\",\"ARGname\":\"LED0arg\",\"ARGstat\":\"" + stats + "\"}";
    if (stats == "true")
    {
      if (Led0Status() == "false") // LED 0 on
      {
        if (digitalRead(LEDon[0]) == LOW)
          digitalWrite(LEDon[0], HIGH);
        else if (digitalRead(LEDon[0]) == HIGH)
          digitalWrite(LEDon[0], LOW);
      }
    }
    else if (stats == "false")
    {
      if (Led0Status() == "true") // LED 0 off
      {
        if (digitalRead(LEDon[0]) == LOW)
          digitalWrite(LEDon[0], HIGH);
        else if (digitalRead(LEDon[0]) == HIGH)
          digitalWrite(LEDon[0], LOW);
      }
    }
    Serial.println(msg);
    mesh.sendBroadcast(msg);
  }
  else if (request->hasArg("LED1arg"))
  {
    String stats = request->arg("LED1arg");
    String msg = "{\"CMD\":\"ServerCMD\",\"Status\":\"ALLcontrol\",\"ARGname\":\"LED1arg\",\"ARGstat\":\"" + stats + "\"}";
    if (stats == "true")
    {
      if (Led1Status() == "false") // LED 1 on
      {
        if (digitalRead(LEDon[1]) == LOW)
          digitalWrite(LEDon[1], HIGH);
        else if (digitalRead(LEDon[1]) == HIGH)
          digitalWrite(LEDon[1], LOW);
      }
    }
    else if (stats == "false")
    {
      if (Led1Status() == "true") // LED 1 OFF
      {
        if (digitalRead(LEDon[1]) == LOW)
          digitalWrite(LEDon[1], HIGH);
        else if (digitalRead(LEDon[1]) == HIGH)
          digitalWrite(LEDon[1], LOW);
      }
    }
    Serial.println(msg);
    mesh.sendBroadcast(msg);
  }
}

void vjoinwifi(AsyncWebServerRequest *request)
{

  String ssid_scan = "";
  int Tnetwork = 0, i = 0, len = 0;
  String st = "", out = "";
  if (scanstate == false)
  {
    scanstate = true;
    Tnetwork = WiFi.scanNetworks();
    st = "<select name=\"ssid\">";
    for (int i = 0; i < Tnetwork && i < 10; ++i)
    {
      //st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
      ssid_scan = WiFi.SSID(i);
      if (ssid_scan != MESH_PREFIX)
      {
        void clear();
        st += "<option value=" + String(i) + ">" + ssid_scan + "( " + getPowerPercentage(WiFi.RSSI(i)) + "\% )</option>";
      }
      findwifi[i] = ssid_scan;
      // Serial.println(findwifi[i]);
    }
    st += "</select>";
    IPAddress ip = WiFi.softAPIP();
    out = "<div><h1>Scan Wifi!</h1> ";
    out += "<form method='post' action='a'><label>SSID: </label>" + st + "<p><label>PASSWORD : </label><input name='pass' length=64 placeholder='Ex : Pass1234'></p><input type='submit'></form>";
    out += "<a href =\"/\"><button>Back</button></a><div>";
    scanstate = false;
  }
  request->send(200, "text/html", mobilecode + out + style1);
}
void vcheckip(AsyncWebServerRequest *request)
{
  String checkip = "", youip = "";
  youip = myIP.toString();
  checkip = "<h1>Check Your IP...</h1> ";
  checkip += "Your IP in home : ";
  checkip += " " + youip + "<br><br>\n\n";
  // }
  checkip += "<a href =\"/\"><button>Back</button></a>";

  request->send(200, "text/html", mobilecode + checkip + style1);
}
void vreset(AsyncWebServerRequest *request)
{
  String html = "", youip = "";
  html = "<h1>Reset ALL MESH OK....</h1> ";
  html += "<a href =\"/\"><button>Back</button></a>";

  StaticJsonDocument<256> msg;
  msg["CMD"] = "ServerCMD";
  msg["Status"] = "ResetALL";
  String str;
  serializeJson(msg, str);
  mesh.sendBroadcast(str);
  request->send(200, "text/html", mobilecode + html + style1 + gohomepage);
}
int getPowerPercentage(int power)
{
  int quality;
  // dBm to Quality:
  if (power <= -100)
    quality = 0;
  else if (power >= -50)
    quality = 100;
  else
    quality = 2 * (power + 100);

  return quality;
}

// HEADER FILES: *****************************************************
#include <nodemcu_pins.h>
#include <SerialComm.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

#include "keypad.h"
#include "menu.h"
// END HEADER: *****************************************************
#define DEBUGMODE 0

Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);
#define displayPin A0

bool arduinoInit = false;
bool saveEEPROM = false;
bool notifScreen = false;
bool refreshMenu = false;
long timeSinceInput = 0;

const char* ssid = "Rumen";
const char* password = "87651412";
bool serverStarted = false;
IPAddress ip;
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  pinMode(pin_keypad, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  
  EEPROM.begin(512);
  stats[NASTROIKI][ITEMB] = EEPROM.read(1);
  Serial.println(EEPROM.read(1), DEC);
  EEPROM.end();
  
  display_start();
}

void loop()
{
  checkSerial();
  if (initSystems() == false)
    return;
  wifi_handle();
  menu_refresh();
  data_refresh();
  buttons_handle();

  delay(100);
}

void sleep_handle()
{
  if (millis() - timeSinceInput >= 300000)
    return;
  Serial.println("SLEEP");
  
}

void data_refresh()
{
  static long lastDataRefresh = 0;
  
  if ( (millis() - lastDataRefresh) >= 500 )
  {
    Serial.println("STATS");
    lastDataRefresh = millis();
    if (menu_current == DANNI)
      refreshMenu = true;
  }
}

void wifi_handle()
{
  WiFiClient client = server.available();  
  if (!client || !client.available()) {
    return;
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();
  
  // Match the request
  int val;
  if (req.indexOf("/gpio/0") != -1)
    val = 0;
  else if (req.indexOf("/gpio/1") != -1)
    val = 1;
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  // digitalWrite(2, val);
  
  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += "</html>\n";

  // Send the response to the client
  client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected 
  // when the function returns and 'client' object is detroyed
}

bool initSystems()
{
  static bool firstRun = true;
  static unsigned long tprevious = millis();  
  unsigned long tpassed = millis();
  
  if ( ((arduinoInit == false) && (DEBUGMODE == 0))
      || (WiFi.status() != WL_CONNECTED))
  {
    if  ((abs(tpassed - tprevious) > 5000) || firstRun)
    {      
      tprevious = tpassed;
      Serial.println("INIT");    
      display.clearDisplay();
      display.setCursor(0, 5);
      display.setTextSize(1);
      display.setTextColor(BLACK);
      display.println("wkl\\`wane...");
      display.display();
      notifScreen = true;
    }
    firstRun = false;
    delay(250);
    return false;
  }
  else
  {
    // RETURN TO MENU
    if (notifScreen)
    {
      notifScreen = false;
      refreshMenu = true;
    }
    // START SERVER
    if (serverStarted == false)
    {
      server.begin();
      ip = WiFi.localIP();
      serverStarted = true;      
    }
    return true;
  }
}

void checkSerial()
{
  char* input = processSerial();
  
  if (strcmp(input,"0") != NULL)
  {
    arduinoInit = true;
    if (strstr(input,"STATS") != NULL)
    {
      char* p = input;
      int param[3] = {0};
      byte i = 0;
      
      while (*p)
      {
          if (*p++ == ':')
          {
            param[i] = atoi(p);
            i++;
          }
      }
      stats[ param[0] ][ param[1] ] = param[2];
    }
    else if (strcmp(input,"READY") == NULL) {}
  }
}

void buttons_handle()
{    
  byte btn = button_get();
  
  if (btn == 0)
  {
    return;
  }
  
  if ( isMenu() )
  {
    switch (btn)
    {
    case WHITETOP:
      Serial.println("BFEED1");
      break;
    case WHITEMID:
      break;
    case WHITEBOT:
      menu_draw(MAIN);
      if (saveEEPROM = true)
      {        
        EEPROM.begin(512);
        EEPROM.write(1, stats[NASTROIKI][ITEMB]);
        EEPROM.end();
        saveEEPROM = false;
      }
      break;
    case BLUELEFT:      
      if (statActive)
      {
        switch(menu_current)
        {
        case NASTROIKI:
          if(menu_selection = ITEMB)
          {
            if ( (stats[menu_current][menu_selection] - 10) >= 0 )
              stats[menu_current][menu_selection] = stats[menu_current][menu_selection] - 10;
            else
              stats[menu_current][menu_selection] = 0;
            display.setTextSize(1);
            menu_refreshCurrent(true);
            saveEEPROM = true;
          }
          break;
        }
      }
      break;
    case BLUERIGHT:
      if (statActive)
      {
        switch(menu_current)
        {
        case NASTROIKI:
          if(menu_selection = ITEMB)
          {
            if ( (stats[menu_current][menu_selection] + 10) <= 250 )
              stats[menu_current][menu_selection] = stats[menu_current][menu_selection] + 10;
            else
              stats[menu_current][menu_selection] = 250;
            display.setTextSize(1);
            menu_refreshCurrent(true);
            saveEEPROM = true;
          }
          break;
        }
      }
      break;
    case BLUETOP:
      menu_select(menu_selection - 1);
      break;
    case BLUEBOT:
      menu_select(menu_selection + 1);
      break;
    case GREENMID:
      menu_activate();
      break;
    default:
      break;
    }
  }
  else
  {
    switch (btn)
    {
      case WHITETOP:
        Serial.println("BFEED1");
        break;
      case WHITEMID:
        break;
      case WHITEBOT:
        menu_draw(MAIN);
        break;
      case BLUELEFT:
        break;
      case BLUERIGHT:
        break;
      case BLUETOP:
        break;
      case BLUEBOT:
        break;
      case GREENMID:
        menu_activate();
        break;
      default:
        break;
    }
  }
}

void display_start()
{
  pinMode(SK, OUTPUT);

  digitalWrite(SK, LOW);

  display.begin();
  display.setContrast(55);
  display.setRotation(2);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.clearDisplay();

  menu_draw(MAIN);
}

void display_stop()
{
  display.clearDisplay();
  digitalWrite(SK, HIGH);
}

void menu_refresh()
{  
  if (refreshMenu)
  {
    menu_draw(menu_current);
    refreshMenu = false;
  }
}

void menu_activate()
{
  switch (menu_current)
  {
    case MAIN:
      switch (menu_selection)
      {
        case ITEMA:
          menu_draw(PLAN);
          break;
        case ITEMB:
          menu_draw(DANNI);
          break;
        case ITEMC:
          menu_draw(NASTROIKI);
          break;
        case ITEMD:
          menu_draw(VRUZKA);
          break;
        default:
          break;
      }
      break;
    case PLAN:
      switch (menu_selection)
      {
        case ITEMA:
          break;
        case ITEMB:
          break;
        case ITEMC:
          break;
        default:
          break;
      }
      break;
    case DANNI:
      break;
    case NASTROIKI:
      switch (menu_selection)
      {
        case ITEMA:
          
          break;
        case ITEMB:
          break;
        case ITEMC:
          Serial.println("<TARE");
          menu_draw(MAIN);
          break;
        case ITEMD:
          Serial.println(">TARE");
          menu_draw(MAIN);
          break;
        default:
          break;
      }
      break;
    case VRUZKA:
      break;
  }
}

void menu_select(byte item)
{
  if ((item < MENU_ITEMS) && (item > TITLE))
  {
    if (menu_current == NASTROIKI && item == ITEMB)
    {
      display.setTextSize(1);

      menu_unselectCurrent(false);
      menu_selection = item;
      // clear row
      menu_unselectCurrent(true);
      // write value instead
      menu_selectCurrent(true);
      display.display();      
    }
    else if (menu_text[menu_current][item][0] != '\0') //check if first byte of string is \0 thus empty string
    {
      display.setTextSize(1);

      menu_unselectCurrent(false);
      menu_selection = item;
      menu_selectCurrent(false);
    }
  }
}

void menu_refreshCurrent(bool replaceWithValue)
{  
  menu_unselectCurrent(false);
  menu_unselectCurrent(true);
  menu_selectCurrent(replaceWithValue);
}

void menu_selectCurrent(bool replaceWithValue)
{  
  display.setCursor(0, (8 + (8 * menu_selection)));
  display.setTextColor(WHITE, BLACK);
  if (replaceWithValue)
  {
    display.print(stats[menu_current][menu_selection]);
    statActive = true;
  }
  else    
  {
    display.print(menu_text[menu_current][menu_selection]);
    statActive = false;
  }
  display.display();
}

void menu_unselectCurrent(bool deleteInstead)
{  
  if (deleteInstead)
    display.setTextColor(WHITE, WHITE);
  else 
    display.setTextColor(BLACK, WHITE);
  statActive = false;
  display.setCursor(0, (8 + (8 * menu_selection)));
  display.print(menu_text[menu_current][menu_selection]);
  display.display();
}

void menu_draw(byte menu)
{
  menu_current = menu;

  display.clearDisplay();
  display.setTextColor(BLACK);

  if (menu == MAIN)
  {
    display.setCursor(0, 0);
    display.setTextSize(2);
  }
  else
  {
    display.setCursor(0, 5);
    display.setTextSize(1);
  }

  display.println(menu_text[TITLE][menu]);
  display.drawLine(0, 13, display.width() - 1, 13, BLACK);

  display.setCursor(0, 16);
  display.setTextSize(1);

  if (menu == DANNI)
  {
    for (int item = 1; item <= 4; item++)
    {
      if (menu_text[menu][item] != "")
      {
        display.print(menu_text[menu][item]);
        display.print(stats[menu][item]);
        switch (item)
        {
          case ITEMA:
            display.print("ml");
            break;
          case ITEMB:
            display.print("g");
            break;
          case ITEMC:
            display.print("g");
            break;
          default:
            break;
        }
      }
      display.println();
    }
  }
  else if (menu == VRUZKA)
  {
    display.println(ip);
    for (int item = 3; item <= 4; item++)
    {
      if (menu_text[menu][item] != "")
      {
        display.print(menu_text[menu][item]);
      }
      display.println();
    }
  }
  else
  {
    for (int item = 1; item <= 4; item++)
    {
      display.println(menu_text[menu][item]);
    }
  }

  display.display();

  if ( isMenu() ) menu_select(1);
}

bool isMenu()
{
  if (  (menu_current == MAIN)
      ||(menu_current == PLAN)
      ||(menu_current == NASTROIKI) )
  {
    return true;
  }
  return false;
}





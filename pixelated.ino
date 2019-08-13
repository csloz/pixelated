/*
 * 
 *   AUTHOR: L. Sheed
 *   
 *   64 x 64 P2.5 Panel Effect / tool
 *   Uses ESP32 and Custom PCB to connect to a HUB75E style Board with the RGB64x32MatrixPanel library (misnamed, as it supports other sizes!)
 *   
 *   On initial load will setup an AP called pixelated.  Configure mqtt server, wifi at 192.168.4.1 once connected, and save.
 *   mqtt_client_id = esp32 for testing
 *
 *
 *   MQTT Controlled On/ Off 
 *   
 *   /* Supported MQTT Params.
 *
 *  esp32/palette - Change color Palette to specific number or up/down  esp32/palette->0, esp32/palette->up, esp32/palette->down
 *  esp32/display - Display on / off  esp32/display->on, esp32/display->off
 *  esp32/effect - Effect specific number of up / down  + esp32/effect->0..  (See FX for list)
 *  esp32/gif - Gif to Display - specific number of up / down  esp32/gif->0,esp32/gif->up, esp32/gif->down
 *  esp32/brightness - Change display Brightness 1=off, 2=lowest, 64 = max brightness [panel brightness max = panel rows]
 *  esp32/factoryreset - Reset EEPROM Settings to Default
 *
 * EFFECT LIST - Status
 * PLASMA 1 - Working non blocking
 * PONG  2 - Working blocking
 * CLOCK 3 - Not implemented yet
 * GIF 4 - Working non blocking
 * SCROLLER 5 - Not implemented yet
 * CONWAY_GAME_OF_LIFE 6 - Working non blocking
 * SNEK 7 - Not implemented yet
 * COLOR_TEST 8 - Not implemented yet
 * STARFIELD 9 - Not implemented yet
 * STOCKS 10 - Not implemented yet
 * MAX_FX 11
 *
 *  Compile options 1M / 3M FATFS
 *  Stick img.ffat at  0x111000 - 
 *  esptool.py --chip esp32  --baud 921600 write_flash  0x111000 img.ffat
*/


//******************************************************************************************************************
// Library Includes
#include <FS.h>

#include <ESP32-RGB64x32MatrixPanel-I2S-DMA.h> //https://github.com/mrfaptastic/ESP32-RGB64x32MatrixPanel-I2S-DMA
#include <FastLED.h> //Fast LED library for LED effects
#include <Adafruit_GFX.h> //Ada Graphics library
//#include <WiFi.h> //Wifi
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>   //git clone https://github.com/tzapu/WiFiManager.git --branch development in library folder

#include <PubSubClient.h> //MQTT

#include <Fonts/Picopixel.h> //Tiny font
//#include <JPEGDecoder.h>

//GIF Functions (w/filesystem)
//#include <SPIFFS.h>
//FFAT instead of SPIFFS
#include <esp_partition.h>
#include "FFat.h"
#include <Arduino.h>
#include <Time.h>


#include <SD.h>
#include <SPI.h>
#include <simpleDSTadjust.h>

#include "GifDecoder.h"
#include "FilenameFunctions.h"

//Crashy libraries, so ifdef'd out for now.
#ifdef IR
  #include <IRrecv.h>
  #include <IRutils.h>
//#include <IRremoteESP8266.h>

  #include <IRremote.h> //https://github.com/z3t0/Arduino-IRremote as the other one is crashy.  Still need to patch as per https://forum.arduino.cc/index.php?topic=432276.0
#endif

//Include EEPROM Library
#include <EEPROM.h>

//******************************************************************************************************************
//DEFINE Print Debugging

#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)  Serial.print (x, DEC)
 #define DEBUG_PRINTHEX(x)  Serial.print (x,HEX)
 #define DEBUG_PRINTLN(x)   Serial.println (x)
 #define DEBUG_PRINTF(x,y)  Serial.printf( x,y)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTHEX(x)
 #define DEBUG_PRINTLN(x) 
 #define DEBUG_PRINTF(x,y) 
#endif

//******************************************************************************************************************
// Package Setup

#define WIFI_CONFIG_ADDRESS 10

struct package {

    //WIFI Manager Bits
    uint val=0;
    char str[20] = "";
    char WIFI_SSID[20] ="";
    char WIFI_PASS[20]="";
   
    //My own bits
    int brightness=0;
    int fx=0;
    int gif=0;
    int palette=0;
    int myDelay=0;

    //mqtt bits
    char mqtt_server[40]; 
    char mqtt_user[40];
    char mqtt_pass[40];
    char mqtt_client_id[32];
};

typedef struct package Package;
Package pixelated_config;


//******************************************************************************************************************
// IR Setup

//IR causing cache issues due to use of interrupts? commented out for now
//#define IR


#ifdef IR
//PIN 23
const int RECV_PIN = 23;

IRrecv irrecv(RECV_PIN);
decode_results IRcapture;

String lastIRreceived ="none";

#endif



//******************************************************************************************************************


//MQTT Config 
char mqtt_server[40]; 
char mqtt_user[40];
char mqtt_pass[40];
char mqtt_client_id[32];

char* mqtt_topic= "";
byte* mqtt_message[] = {0,0,0,0,0,0,0,0,0,0};
//******************************************************************************************************************

//SD Card Setup (still in alpha)

#define PIN_NUM_MISO 32 //
#define PIN_NUM_MOSI 23 //ok
#define PIN_NUM_CLK  22 //
#define PIN_NUM_CS   33 // Needs 10k pullup resistor??

SPIClass mySPI(HSPI);

//******************************************************************************************************************

// RGB Matrix Config

const byte ROWS=64;
const byte COLUMNS=64;
 //Redefine libary default of 32 -> 64 in ESP32-RGB64x32MatrixPanel-I2S-DMA.h
 //#define MATRIX_HEIGHT  COLUMNS //64

 #define A_PIN  26
 #define B_PIN  4
 #define C_PIN  27
 #define D_PIN  2
 #define E_PIN  21 //Only on modded board

 #define R1_PIN   5
 #define R2_PIN  19
 #define G1_PIN  17
 #define G2_PIN  16
 #define B1_PIN  18
 #define B2_PIN  25

 #define CLK_PIN  14
 #define LAT_PIN  15
 #define OE_PIN  13



//After PIN Defines.
RGB64x32MatrixPanel_I2S_DMA matrix;

int brightness = 10;

//#define setBrightness(x) fillScreen(0) // no-op, no brightness on this board
#define clear() fillScreen(0)
#define show() drawPixel(0, 0, 0)     // no show method in this GFX implementation
#define Color(x,y,z) color444(x/16,y/16,z/16)


void screenClearCallback(void) {
  matrix.fillScreen(matrix.color565(0,0,0));
}

void updateScreenCallback(void) {
 // matrix.flipDMABuffer();  don't support too slow..
}

void drawPixelCallback(int16_t x, int16_t y, uint8_t red, uint8_t green, uint8_t blue) {
    matrix.drawPixelRGB888(x, y, red, green, blue);
}



CRGBPalette16 currentPalette;
CRGB currentColor;
CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) {
 return ColorFromPalette(currentPalette, index, brightness, blendType);
}


//******************************************************************************************************************

// Pong Defines

#define X_MAX 63                         // Matrix X max LED coordinate (for 2 displays placed next to each other)
#define Y_MAX 63
#define BAT1_X 2                         // Pong left bat x pos (this is where the ball collision occurs, the bat is drawn 1 behind these coords)
#if (X_MAX == 63)
#define BAT2_X 60
#else
#define BAT2_X 28
#endif


//******************************************************************************************************************

// Clock / Time Related

char *dstAbbrev;
#define SHOWCLOCK 100
uint16_t showClock = SHOWCLOCK;  
time_t now;
struct tm * timeinfo; 

// Adjust according to your language
const String WDAY_NAMES[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

struct dstRule StartRule = {"CEST", Last, Sun, Mar, 2, 3600}; // Central European Summer Time = UTC/GMT +2 hours
struct dstRule EndRule = {"CET", Last, Sun, Oct, 2, 0};       // Central European Time = UTC/GMT +1 hour
#define UTC_OFFSET +8
#define NTP_SERVERS "0.ch.pool.ntp.org", "1.ch.pool.ntp.org", "2.ch.pool.ntp.org"


simpleDSTadjust dstAdjusted(StartRule, EndRule);

//******************************************************************************************************************

// GIF 

#define GIF_DIRECTORY "/gifs64"
#define DISPLAY_TIME_SECONDS 60
int file_index = 0;

// Gif sizes should match exactly that of the RGB Matrix display.
const uint8_t GIFWidth = 64;     
const uint8_t GIFHeight = 64;  
GifDecoder<64, 64, 12> decoder;
int num_files;

//******************************************************************************************************************

// Color Defines

#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

#define LED_RED_VERYLOW   (3 <<  11)
#define LED_RED_LOW     (7 <<  11)
#define LED_RED_MEDIUM    (15 << 11)
#define LED_RED_HIGH    (31 << 11)

#define LED_GREEN_VERYLOW (1 <<  5)   
#define LED_GREEN_LOW     (15 << 5)  
#define LED_GREEN_MEDIUM  (31 << 5)  
#define LED_GREEN_HIGH    (63 << 5)  

#define LED_BLUE_VERYLOW  3
#define LED_BLUE_LOW    7
#define LED_BLUE_MEDIUM   15
#define LED_BLUE_HIGH     31

#define LED_ORANGE_VERYLOW  (LED_RED_VERYLOW + LED_GREEN_VERYLOW)
#define LED_ORANGE_LOW    (LED_RED_LOW     + LED_GREEN_LOW)
#define LED_ORANGE_MEDIUM (LED_RED_MEDIUM  + LED_GREEN_MEDIUM)
#define LED_ORANGE_HIGH   (LED_RED_HIGH    + LED_GREEN_HIGH)

#define LED_PURPLE_VERYLOW  (LED_RED_VERYLOW + LED_BLUE_VERYLOW)
#define LED_PURPLE_LOW    (LED_RED_LOW     + LED_BLUE_LOW)
#define LED_PURPLE_MEDIUM (LED_RED_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_PURPLE_HIGH   (LED_RED_HIGH    + LED_BLUE_HIGH)

#define LED_CYAN_VERYLOW  (LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_CYAN_LOW    (LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_CYAN_MEDIUM   (LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_CYAN_HIGH   (LED_GREEN_HIGH    + LED_BLUE_HIGH)

#define LED_WHITE_VERYLOW (LED_RED_VERYLOW + LED_GREEN_VERYLOW + LED_BLUE_VERYLOW)
#define LED_WHITE_LOW   (LED_RED_LOW     + LED_GREEN_LOW     + LED_BLUE_LOW)
#define LED_WHITE_MEDIUM  (LED_RED_MEDIUM  + LED_GREEN_MEDIUM  + LED_BLUE_MEDIUM)
#define LED_WHITE_HIGH    (LED_RED_HIGH    + LED_GREEN_HIGH    + LED_BLUE_HIGH)

//******************************************************************************************************************

// FX Defines

#define PLASMA 1
#define PONG  2
#define CLOCK 3
#define GIF 4
#define SCROLLER 5
#define CONWAY_GAME_OF_LIFE 6
#define SNEK 7
#define COLOR_TEST 8
#define STARFIELD 9
#define STOCKS 10
#define MAX_FX 11


int fx=10; //Hard code to stocks for testing.


//******************************************************************************************************************

// Misc Setup Vars
int DELAY=0; //Slow down delay = 0
bool SHOWMENU=false;  //MENU
boolean screenPowered=true;  //Screen defaults to on

int myPalette=1; //RGB full color
int myTime=0; 
int cycles=0;

//Snake Vars
#define KEY_NONE 0
#define KEY_START 3
uint8_t uiKeyCode = KEY_NONE;




//******************************************************************************************************************

// Config Wifi


WiFiClient espClient;
PubSubClient mqttclient(espClient);



//******************************************************************************************************************
//******************************************************************************************************************
//******************************************************************************************************************

// Code Starts Here

//******************************************************************************************************************
//******************************************************************************************************************
//******************************************************************************************************************




void setup_conway(); //Defined elsewhere.

void setup_sd() {
  
  pinMode( PIN_NUM_CS , OUTPUT);
  
  
  mySPI.begin(PIN_NUM_CLK ,PIN_NUM_MISO ,PIN_NUM_MOSI, PIN_NUM_CS ); //sck, miso,  mosi,  ss
  
  if (!SD.begin(PIN_NUM_CS, mySPI, 24000000)){ 
      DEBUG_PRINTLN("Card Mount Failed");
  }
  else {
      DEBUG_PRINTLN ("SD mounted");
  }

}


void sprintdiv () {
 DEBUG_PRINTLN("/***************************************************************/");
}


void configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG_PRINTLN("[WIFI] Entered config mode");
  DEBUG_PRINT("[WIFI] ");
  DEBUG_PRINTLN(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DEBUG_PRINT("[WIFI] ");
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID());
}

void saveParamCallback(){
  DEBUG_PRINTLN ("[CALLBACK] saveParamCallback fired");
}


void setup_wifi() {
  
    //Attempt connection to EEPROM settings
    WiFi.begin (pixelated_config.WIFI_SSID, pixelated_config.WIFI_PASS);
    int timeout =0;

    //Try connect within 10 seconds
    DEBUG_PRINT ("Attempting wifi connection");
    
    while ((WiFi.status() != WL_CONNECTED) && (timeout < 100)) {
        DEBUG_PRINT(".");
        delay (100);
        timeout++;
    }

    DEBUG_PRINTLN();
    
    //Couldn't connect, lets setup an AP
    if (WiFi.status() != WL_CONNECTED) {
          WiFiManager wm;
          wm.setAPCallback(configModeCallback);
          wm.setSaveParamsCallback(saveParamCallback);
      
          //Config our extra params in WiFiManager
          WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
          WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqtt_user, 40);
          WiFiManagerParameter custom_mqtt_pass("pass", "mqtt pass", mqtt_pass, 40);
          WiFiManagerParameter custom_mqtt_client_id("client", "mqtt client id", mqtt_client_id, 32);
      
          wm.addParameter(&custom_mqtt_server);
          wm.addParameter(&custom_mqtt_user);
          wm.addParameter(&custom_mqtt_pass);
          wm.addParameter(&custom_mqtt_client_id);
      
      
          if (!wm.autoConnect("pixelated")) {
            DEBUG_PRINTLN("failed to connect to saved settings");
            delay(3000);
          
            // if we still have not connected restart and try all over again
            //ESP.restart();
            //delay(5000);
          }
      
          //read updated parameters back into our config array struct
          strcpy(pixelated_config.mqtt_server, custom_mqtt_server.getValue());
          strcpy(pixelated_config.mqtt_user, custom_mqtt_user.getValue());
          strcpy(pixelated_config.mqtt_pass, custom_mqtt_pass.getValue());
          strcpy(pixelated_config.mqtt_client_id, custom_mqtt_client_id.getValue());
          strncpy(pixelated_config.WIFI_SSID, WiFi.SSID().c_str(), 20);
          strncpy(pixelated_config.WIFI_PASS, WiFi.psk().c_str(), 20);
          
          //Save to EEPROM?
          SaveEEPROM();
          
          //Re-read EEPROM as it sets up the MQTT vars
          ReadEEPROM();
    }
}


void setup_mqtt(){
 
  mqttclient.setServer(mqtt_server,1883);
  mqttclient.setCallback(mqtt_callback);
  mqttclient.connect (mqtt_client_id, mqtt_user, mqtt_pass);
  sprintdiv();
  if (mqttclient.connected()) {
      DEBUG_PRINTLN("[MQTT] Connected");
      matrix.println ("[MQTT] Connected");
      if (mqttclient.connect(mqtt_client_id )) {
        mqttclient.subscribe("esp32/#");
        DEBUG_PRINTLN("[MQTT] Subscribed to esp32/#");
      }
  }
  else {
    DEBUG_PRINTLN("[MQTT] Couldn't connect");
  }
  sprintdiv();
}


void setup_gif() {

    sprintdiv();

    //Only open 1 file for ram purposes (12kb used)
    if(!FFat.begin (0,"",2)){ 
          DEBUG_PRINTLN("[FFAT] Mount Failed");
    }
    else {
        DEBUG_PRINTLN ("[FFAT] Mount Success");
        matrix.println ("[FFAT] Mount Success");
        DEBUG_PRINTLN("[FFAT] File system mounted");
        DEBUG_PRINTF("[FFAT] Total space: %10lu\n", FFat.totalBytes());
        DEBUG_PRINTF("[FFAT] Free space:  %10lu\n\n", FFat.freeBytes());
    }
   
    //map gif decoder to our own functions.
    
    decoder.setScreenClearCallback(screenClearCallback);
    decoder.setUpdateScreenCallback(updateScreenCallback);
    decoder.setDrawPixelCallback(drawPixelCallback);

    decoder.setFileSeekCallback(fileSeekCallback);
    decoder.setFilePositionCallback(filePositionCallback);
    decoder.setFileReadCallback(fileReadCallback);
    decoder.setFileReadBlockCallback(fileReadBlockCallback);

    // Determine how many animated GIF files exist
    num_files = enumerateGIFFiles(GIF_DIRECTORY, false);
    DEBUG_PRINTF ("[GIF] Number of files: %i", num_files);
    DEBUG_PRINTLN();
    
    sprintdiv();
    
}

void mqtt_callback(char* topic, byte* message, unsigned int length) {
  
  sprintdiv();
  DEBUG_PRINT("MQTT [Message arrived on topic: ");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] Message [ ");
  String messageValue;

  //Start Menu timer
  SHOWMENU=true;
  
  for (int i = 0; i < length; i++) {
    DEBUG_PRINT((char)message[i]);
    messageValue += (char)message[i];
  }
  DEBUG_PRINTLN("]");


  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/display") {
    if(messageValue == "on"){
      screenPowered=true;
    }
    else if(messageValue == "off"){
      screenPowered=false;
      matrix.clear();
    }
  }

  if (String(topic) == "esp32/factoryreset") {
      ClearEEPROM();
  }

  //Brightness MAX = panel ROWS
  if (String(topic) == "esp32/brightness") {
    
    if (messageValue == "up") { 
        brightness++;  
        if (brightness>ROWS){ 
          brightness= ROWS; //loop at max_fx
        }
    }
    else if (messageValue == "down") {
      brightness--;
      if (brightness<1) {
        brightness = 1; 
      }
    }
    else {
        int i=messageValue.toInt();
        if (i>0) brightness=i;
        if (brightness>ROWS) brightness=ROWS;
    }
    
    matrix.setPanelBrightness(brightness);
    DEBUG_PRINT("Changing brightness to ");
    DEBUG_PRINT (messageValue);
    DEBUG_PRINT (" ");
    DEBUG_PRINTLN (brightness);
  }
  

  if (String(topic) == "esp32/delay") {
    Serial.print("Changing delay to ");
    Serial.println (messageValue);
    DELAY = messageValue.toInt();
    
  }

  if (String(topic) == "esp32/startsnake") {
     uiKeyCode = KEY_START;  
  }

  if (String(topic) == "esp32/effect") {
   
    if (messageValue == "up") { 
        fx++;  
        if (fx==MAX_FX-1){ 
          fx= 1; //loop at max_fx
        }
    }
    else if (messageValue == "down") {
      fx--;
      if (fx<1) {
        fx = MAX_FX-1; //loop at zero
      }
    }
    else {
      fx = messageValue.toInt();
    }

    Serial.print("Changing Effect to: ");
    Serial.println (fx);
    
    if (fx == CONWAY_GAME_OF_LIFE) {
        setup_conway(); //random init game of life
    }
    //Clear screen as new effect will look funky otherwise
    matrix.fillScreen(BLACK);
  }

  
 if (String(topic) == "esp32/palette") {
    Serial.print("Changing Palette to: ");
    Serial.println (messageValue + " " + myPalette);
    if (messageValue == "up") { 
        myPalette++; 
    }
    else if (messageValue == "down") {
      myPalette--; 
    }
    else {
      myPalette = messageValue.toInt();
    }
       
    DoPalette();
  }

  if (String(topic) == "esp32/gif") {
    DEBUG_PRINT ("Changing GIF index to: ");
    DEBUG_PRINTLN (messageValue);
    
    if (messageValue =="up") { 
        file_index++; 
        if (file_index == num_files) file_index=0; //loop at max
    }
    else if (messageValue == "down") {
      file_index--;
      if (file_index<0) file_index=num_files-1; //loop at min  
    }
    else {
      file_index = messageValue.toInt();
    }
    DEBUG_PRINTLN (file_index);
    
    //Open GIF        
    if (openGifFilenameByIndex(GIF_DIRECTORY, file_index) >= 0) {
      // Can clear screen for new animation here, but this might cause flicker with short animations
      matrix.fillScreen(BLACK);
      // matrix.swapBuffers();
      decoder.startDecoding();
      //Set display to GIF
      fx=GIF;
    }
    else{ 
       DEBUG_PRINTLN("FATAL: failed to open file");
    }
    
  }
  
  sprintdiv();

}



void ReadEEPROM(){
  
  //Setup our alleged data package
  EEPROM.begin(sizeof (pixelated_config) + WIFI_CONFIG_ADDRESS);
  DEBUG_PRINT ("sizeof(package):");
  DEBUG_PRINTLN (sizeof(pixelated_config) + WIFI_CONFIG_ADDRESS);

  //Get our initial data
  EEPROM.get (WIFI_CONFIG_ADDRESS, pixelated_config);
  DEBUG_PRINTLN ("Old values are:" + String (pixelated_config.val) + "," + String (pixelated_config.str));

  //Setup values from EEPROM
  /*
  pixelated_config.mqtt_server;
  pixelated_config.mqtt_user;
  pixelated_config.mqtt_pass;
  pixelated_config.mqtt_client_id;
  pixelated_config.brightness;
  pixelated_config.fx;
  pixelated_config.gif;
  pixelated_config.palette;
  pixelated_config.myDelay;
  */

  sprintdiv ();
  DEBUG_PRINTLN (pixelated_config.mqtt_server);
  DEBUG_PRINTLN (pixelated_config.mqtt_user);
  DEBUG_PRINTLN (pixelated_config.mqtt_pass);
  DEBUG_PRINTLN (pixelated_config.mqtt_client_id);
  DEBUG_PRINTLN ("B: " + String (pixelated_config.brightness) +  " FX: " + String (pixelated_config.fx) + " GIF: "+String (pixelated_config.gif) + " P: " + String (pixelated_config.palette) + " D: " + String (pixelated_config.myDelay)) ;
  sprintdiv ();
  
  //Setup our variables
  strncpy (mqtt_server, pixelated_config.mqtt_server, 40);
  strncpy (mqtt_user, pixelated_config.mqtt_user, 40);
  strncpy (mqtt_pass, pixelated_config.mqtt_pass, 40);
  strncpy (mqtt_client_id, pixelated_config.mqtt_client_id, 32);
  
  
  
}

void ClearEEPROM() {
  DEBUG_PRINTLN ("[EEPROM] Erasing ");
  int j = sizeof (pixelated_config) + WIFI_CONFIG_ADDRESS;
  
  for (int i=WIFI_CONFIG_ADDRESS; i <  j ; i++) {
    EEPROM.write (i,0);  
    DEBUG_PRINT(".");
  }
  DEBUG_PRINTLN();
  EEPROM.commit(); 
  DEBUG_PRINTLN ("[EEPROM] Erased");
  DEBUG_PRINTLN ("[WIFI] Disconnect");
  WiFi.disconnect(false,true); //Erase existing WIFI credentials
}

void SaveEEPROM() {
//  EEPROM.write (0,ledState);
    EEPROM.put (WIFI_CONFIG_ADDRESS, pixelated_config);
    EEPROM.commit();
    DEBUG_PRINTLN ("Writing EEPROM");
    DEBUG_PRINTLN ("New Values are:" + String (pixelated_config.val) + "," + String (pixelated_config.str));
    
}


void setup() {
  //Setup Serial
  Serial.begin(115200);

  
  //Default to standard palette
  currentPalette = RainbowColors_p;
  myPalette = 1;

  //Setup EEPROM default settings
  ReadEEPROM();

  
  //Fix RGB as intitally setup incorrectly, doh!
  matrix.begin(R1_PIN,B1_PIN, G1_PIN, R2_PIN, B2_PIN,G2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN );  // setup the LED matrix
  matrix.setFont(&Picopixel);
  matrix.clear();
  matrix.setPanelBrightness(brightness);

 
  
  setup_wifi();
  
  //Setup MQTT
  setup_mqtt();

  //Setup SD card
  //setup_sd();

  //Setup GIF
  setup_gif();

  //Setup Conway random
  setup_conway();


  //Setup IR
  //irrecv.enableIRIn();  //runs on an interrupt
  
  
  delay (1000);
  
  screenPowered=true;

  
}



void DoPlasma() {
  
  for (int x = 0; x <  matrix.width(); x++) {
           for (int y = 0; y <  matrix.height(); y++) {
               int16_t v = 0;
               uint8_t wibble = sin8(myTime);
               v += sin16(x * wibble * 3 + myTime);
               v += cos16(y * (128 - wibble)  + myTime);
               v += sin16(y * x * cos8(-myTime) / 8);

       currentColor = ColorFromPalette(currentPalette, (v >> 8) + 127); //127, brightness, currentBlendType);
       matrix.drawPixelRGB888(x, y,  currentColor.r, currentColor.g,currentColor.b);

           }
       }
 
       
       cycles++;

       if (cycles >= 2048) {
           myTime = 0;
           cycles = 0;
       }
  
}

void DoColorTest() {
  //Black clear 
  matrix.clear();
  matrix.setCursor(2, 2);
  matrix.setTextColor(WHITE);
  //draw a box red;
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.color444(15, 0, 0));
  matrix.print("RED");
  delay(1000);
  
  // draw a box in green
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.color444(0, 15, 0));
  matrix.print("GREEN");
  delay(1000);
  
  //draw a box blue;
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.color444(0, 0, 15));
  matrix.print("BLUE");
  delay(1000);

  //yellow
  matrix.fillRect(0, 0, matrix.width(), matrix.height(), matrix.color444(15,15, 0));
  matrix.print("YELLOW");
  delay(1000);
  
  // fill a violet circle
  //matrix.fillCircle(40, 21, 10, matrix.color444(15, 0, 15));
  //delay(500);
}


int loaded = false;

void DoGIF() {
  
  static unsigned long futureTime;

   // int index = -1; //random(num_files);

    if(!loaded) {
        if (file_index >= num_files) {
            file_index = num_files;
        }

       if (openGifFilenameByIndex(GIF_DIRECTORY, file_index) >= 0) {
            // Can clear screen for new animation here, but this might cause flicker with short animations
            //matrix.fillScreen(BLACK);
            //matrix.flipDMABuffer();
            loaded=true;
            decoder.startDecoding();

            // Calculate time in the future to terminate animation
            futureTime = millis() + (DISPLAY_TIME_SECONDS * 1000);
        }
    }

    decoder.decodeFrame();
  
}

void DoScroller() {
  
    
    matrix.setTextWrap(false);  // we don't wrap text so it scrolls nicely
    matrix.setTextSize(1);
    matrix.setRotation(0);
    for (int8_t x=7; x>=-64; x--) {
      matrix.clear();
      matrix.setCursor(x,10);
      matrix.setTextColor(LED_GREEN_HIGH);
      matrix.print("Hello");
      
      matrix.setCursor(-20-x,64-7);
      matrix.setTextColor(LED_ORANGE_HIGH);
      matrix.print("World");
      delay (50);
  }
  

}



void DoPalette() {

  switch (myPalette) {
      case 0: //Full RGB Palette
        currentPalette = RainbowColors_p;
        break;
      case 1: //Ice Blue
        currentPalette = CRGBPalette16(CRGB::Black, CRGB::Blue, CRGB::Aqua, CRGB::White);
        break;
      case 2: //GreyScale
        currentPalette = CRGBPalette16(CRGB::Black, CRGB::White);  //GreyScale
        break;
      case 3: //RedScale
        currentPalette = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::White);
        break;
      case 4: //Blue Green
        currentPalette = CRGBPalette16( CRGB::Black, CRGB::Green, CRGB::Aqua,  CRGB::White);
        break;
       case 5: //Red Yellow
        currentPalette = CRGBPalette16( CRGB::Black, CRGB::Red, CRGB::Yellow, CRGB::White);
        break;
       case 6: 
        currentPalette = PartyColors_p; 
        break;
       case 7:
        currentPalette = ForestColors_p; 
        break;
       case 8:
        currentPalette = LavaColors_p; 
       break;
       case 9:
        currentPalette = CloudColors_p;  
       break;
       case 10:
        currentPalette = OceanColors_p;  
       break;
       case 11:
        currentPalette = RainbowStripeColors_p;
       break;
       case 12:  //random
             for( int i = 0; i < 16; i++) {
               currentPalette[i] = CHSV( random8(), 255, random8());
             }
       break;
       case 13: //B/W STRIPE
           // 'black out' all 16 palette entries...
          fill_solid( currentPalette, 16, CRGB::Black);
          // and set every fourth one to white.
          currentPalette[0] = CRGB::White;
          currentPalette[4] = CRGB::White;
          currentPalette[8] = CRGB::White;
          currentPalette[12] = CRGB::White; 
      break;
      default: //Reset to default
        myPalette=0;
        currentPalette = RainbowColors_p;
        break;
  }
  
}

void DoMenu() {
  static unsigned long menuTime;

  // Calculate time in the future to terminate animation
  if (SHOWMENU==true){
      menuTime = millis() + (2 * 1000); //2 second display.
      SHOWMENU=false;
      DEBUG_PRINTLN ("showmenu timer started");
      matrix.setTextSize(1);     // size 1 == 8 pixels high
      matrix.setTextWrap(false); // Don't wrap at end of line - will do ourselves
      matrix.setCursor(2,5);    // start at top left, with 8 pixel of spacing
      matrix.setTextColor(matrix.color565(255, 255, 255));
  }
    
  if(menuTime > millis()) { //Still have menu timer? 
    matrix.setCursor(2,5);    // start at top left, with 8 pixel of spacing 
    matrix.print("FX: ");
    matrix.print(fx);
    matrix.print("GIF:"); 
    matrix.print (file_index);
    matrix.print("CLR ");
    matrix.print(myPalette);
  } 

}

void cls(){
    matrix.fillScreen(matrix.color565(0,0,0));
    //matrix.clearDisplay(); //
}

unsigned long modeSwitch;




void DoStarField() {
  int a;
  int n=1000;
  int ld[n];
  float la[n];
  int lx[n],ly[n],lr[n];
  char wait;
  
  for(a=1;a<n;a++) {
      ld[a]=random(50);
      la[a]=(random(360)*6.2832)/360;
      lr[a]=random(3)+2;
  }
  
  
  for(a=1;a<n;a++)  {
      matrix.drawPixelRGB888(lx[a]+ROWS/2, ly[a]+COLUMNS/2, 0, 0, 0);
      ld[a]=ld[a]+lr[a];
      lx[a]=ld[a]*cos(la[a]);
      ly[a]=ld[a]*sin(la[a]);
  
    //for(int w=1;w<8000;w++){}
    
      if(lx[a]<160 && lx[a]>-160 && ly[a]<100 && ly[a]>-100)   
        matrix.drawPixelRGB888 (lx[a]+ROWS/2,ly[a]+COLUMNS/2, 255,255,255); //(ld[a]*13)/180+18
      if(ld[a]>180) {
        ld[a]=random(15);
        la[a]=(random(360)*6.2832)/360;
        lr[a]=random(3)+2;
      }
  }

}

void showGIF(int i){
    if (i<0) i=0;
    if (i > num_files) i= num_files; //Some basic safety.

    //Open GIF        
    if (openGifFilenameByIndex(GIF_DIRECTORY, i) >= 0) {
      // Can clear screen for new animation here, but this might cause flicker with short animations
      matrix.fillScreen(BLACK);
      // matrix.swapBuffers();
      decoder.startDecoding();
      //Set display to GIF
      fx=GIF;
    }
}

#ifdef IR
void  pollIR (){
//Poll IR Receiver
 if (irrecv.decode(&IRcapture)) {
    long int decCode = IRcapture.value;
    DEBUG_PRINT ("IR [");
    DEBUG_PRINT(decCode);
    DEBUG_PRINTLN("]");
    
    switch (IRcapture.value) {

      case 16754775:
        fx--;
        if (fx<1)  fx= MAX_FX;   //Loop on 0 to max_fx
        DEBUG_PRINT ("FX =");
        DEBUG_PRINTLN (fx);
        break;

      case 16736925:
        fx++;
        if (fx>MAX_FX) fx = 1; //Loop on max -> 1
        DEBUG_PRINT ("FX =");
        DEBUG_PRINTLN (fx);
        break;

      default:
       break;
    }
    
    
   
    if (lastIRreceived == "FF22DD") { //Left
      String message ="down";
      byte bytes[message.length() + 2];//Hacky
      message.getBytes (bytes, message.length()+1);
      mqtt_callback ("esp32/gif",bytes,message.length());  //Hard push to existing MQTT code for sheer laziness/ dual usage / don't rewrite the wheel.
      file_index--;
      if (file_index<0) file_index=num_files-1; //loop at min  
      //showGIF(file_index);
      fx=GIF;
    }
    
    if (lastIRreceived == "FFC23D") { //Right
      file_index++; 
      if (file_index == num_files) file_index=0; //loop at max
      //showGIF(file_index);  
      fx=GIF;
    }

    if (lastIRreceived == "FF6897") { fx=1; }
    if (lastIRreceived == "FF9867") { fx=2; }
    //irrecv.resume();
    //FFC23D = Right
    //FF22DD = Left
    //FF629D = Up
    //FFA857 = Down
    //FF02FD = OK 
    //FF6897 = 1
    //FF9867 = 2
    //FFB04F = 3
    //FF30CF = 4
    //FF18E7 = 5
    //FF7A85 = 6
    //FF10EF = 7
    //FF38C7 = 8
    //FF5AA5 = 9
    //FF4AB5 = 0
    //FF42BD = *
    //FF52AD = #

    irrecv.resume();
    
 }

}
#endif
  

int buttonState;

void loop() {
  /*
#define PLASMA 1
#define PONG  2
#define GIF 4
#define SCROLLER 5
#define CONWAY_GAME_OF_LIFE 6
#define SNEK 7
#define COLOR_TEST 8
#define STARFIELD 9
#define MAX_FX 10

*/
  


  //screenPowered=true;

  
  if (screenPowered) {
    switch (fx) {
      case PLASMA:
       // matrix.flipDMABuffer();
        
        DoPlasma();
        DoMenu();
       // matrix.showDMABuffer();
        break; 
        
      case PONG:
        // matrix.flipDMABuffer();
        //FillNoise();
        //standardNoiseSmearing();
        pong();
      //  DoMenu();
       // matrix.showDMABuffer();
        break;
      case COLOR_TEST:
       // matrix.flipDMABuffer();
        DoColorTest();
        DoMenu();
      //  matrix.showDMABuffer();
        break; 
        
      case SCROLLER:
        //matrix.flipDMABuffer();
        DoScroller();
        DoMenu();
        //matrix.showDMABuffer();
        break;
        
      case CONWAY_GAME_OF_LIFE:
       // matrix.flipDMABuffer();
        DoConway();
        DoMenu();
      //  matrix.showDMABuffer();
        break;
        
      case CLOCK:
     //   matrix.flipDMABuffer();
        //DoClock();
        DoMenu();
     //   matrix.showDMABuffer();    
           
      case SNEK:
        //matrix.flipDMABuffer();
   //     DoSnek(); //Snake
        DoMenu();
        //matrix.showDMABuffer();
        break;

      case GIF:
        DoGIF();
        break;

      case STARFIELD:
    //    DoStarField();
        DoMenu();

        
      default:
        break;  
    }
  }

  //Change FX
  if (millis() - modeSwitch > 120000UL) {  //Switch modes every 5 mins (12000UL)
      //Increment effect, reset to zero if at max
      //fx++;
      if (fx> MAX_FX) { fx=0;}
      //mode_changed = 1;
      modeSwitch = millis();
      DEBUG_PRINT ("Switch mode to ");
      DEBUG_PRINTLN (fx);
  }

 // Maybe move mqtt to 2nd cpu loop2?
 //Run mqtt check every 200ms
 if (myTime % 200) {
    //Poll MQTT
    mqttclient.loop();

    //Abuse the boot button to clear flash and crash so we can redo config
    buttonState = digitalRead(0);
    if (buttonState == LOW) {
      //do something
      DEBUG_PRINTLN ("Pressed, erasing flash");
      ClearEEPROM();
      esp_restart();
      
    }
    
    //Poll IR
    #ifdef IR
    pollIR();
    #endif
 }
 myTime++;   

 

 //delay(DELAY);
} 

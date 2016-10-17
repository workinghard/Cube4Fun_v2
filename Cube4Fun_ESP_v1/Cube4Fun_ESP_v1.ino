
#define DEBUG 1
#define ANIM_FILE_NAME "/ANIMS.FRM"
#define FILE_READ "r"
#define FILE_WRITE "w"
#define FBLENGTH 65
#define SERBFLENGTH 32
#define SERVER_PORT 80
#define MAX_SEND_RETRY 10
#define MAX_ANIMKEY_LENGTH 12
#define MY_CUBE_ADDR 2

//#include <ESP8266WiFi.h>
#include <FS.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
//#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>

#ifdef DEBUG
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_PRINT(x) Serial.print (x)
  #define DEBUG_PRINT2(x,y) Serial.print (x,y)
  #define DEBUG_PRINTLN_TXT(x)  Serial.println (F(x))
  #define DEBUG_PRINT_TXT(x) Serial.print (F(x))
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINT2(x,y)
  #define DEBUG_PRINTLN_TXT(x)
  #define DEBUG_PRINT_TXT(x)
#endif 

unsigned const char lc_slash = '/';
unsigned const char lc_question = '?';
unsigned const char lc_coma = ',';
unsigned const char lc_space = ' ';
unsigned const char lc_f = 'f';
unsigned const char lc_F = 'F';
unsigned const char lc_s = 's';
unsigned const char lc_S = 'S';

unsigned char myAnimationCount = 0;
unsigned int _animationSpeed = 0;
unsigned long _animationLength = 0;
unsigned long _animationStartPos = 0;
unsigned long _animationEndPos = 0;
unsigned int _animationActFrame = 0;
unsigned long _previousMillis = 0;        // will store last time animation frame was sent

unsigned char ga_buffer1[FBLENGTH];
unsigned char gi_buffer1_pos = 0;
unsigned char ga_buffer2[FBLENGTH];
unsigned char gi_buffer2_pos = 0;
unsigned char ga_sendBuffer[SERBFLENGTH];
unsigned char * gp_read_buffer;
//unsigned char * gi_read_buffer_pos;
unsigned char * gp_write_buffer;
//unsigned char * gi_write_buffer_pos;
int gi_status = 0; // 0 = 

unsigned long sendDelay = 1000;
unsigned long lastChangeTime = 0;

int gi_pos = 0;

// Initialize the WiFi server library
const char* ssid = "Enjoy";
const char* password = "iWLSpo8zZ3NS";
ESP8266WebServer server(SERVER_PORT);

const char* www_username = "admin";
const char* www_password = "esp8266";

void fill_buffer_random(unsigned char * pp_buff) {
  for (int i=0; i<FBLENGTH-1;i++){
     pp_buff[i] = (unsigned char)random(0,254);
     //pp_buff[i] = 1;
  }
  unsigned char li_check_digit = 0;
  // Simple check digit
  for (int i=0; i<FBLENGTH-1;i++){
     li_check_digit = li_check_digit + pp_buff[i]; 
     //DEBUG_PRINTLN(li_check_digit); 
  }
  pp_buff[FBLENGTH-1] = li_check_digit;
}

void fill_buffer_run1(unsigned char * pp_buff) {
  for (int i=0; i<FBLENGTH-1;i++){
    if ( i == gi_pos ) {
      pp_buff[i] = 128;
    }else{
      pp_buff[i] = 255;
    }
  }
  if ( gi_pos < 63 ) {
    gi_pos++;
  }else{
    gi_pos = 0; 
  }
}

void print_buffer(unsigned char * pp_buff) {
  DEBUG_PRINTLN("");
  for (int i=0; i<FBLENGTH-1;i++){
    DEBUG_PRINT(i);
    DEBUG_PRINT_TXT(":");
    DEBUG_PRINTLN(pp_buff[i]);
  }
}

boolean wireSendBytes(const uint8_t *data, size_t quantity) {
  boolean try_again = true;
  uint8_t error;
  unsigned char send_retries = 0;
  boolean success = false;
  while ( try_again == true ) {
    Wire.beginTransmission(MY_CUBE_ADDR);
    if ( quantity == 1 ) { 
      Wire.write(data[0]);
    }else{
      Wire.write(data, quantity);
    }
    error = Wire.endTransmission();
    if ( error == 0 || send_retries > MAX_SEND_RETRY ) {
      // everything went well or timeout 
      try_again = false;
      success = true;
    }else{
      // Something went wrong
      DEBUG_PRINTLN_TXT("Send failed"); 
    }
    delayMicroseconds(10);
  }
  send_retries++;
  return success;
}

void sendBufferedFrame() {
  boolean lb_send_success;
  // Send start key
  ga_sendBuffer[0] = lc_slash;
  ga_sendBuffer[1] = lc_slash;
  ga_sendBuffer[2] = lc_question;
  ga_sendBuffer[3] = lc_question;
  lb_send_success = wireSendBytes(ga_sendBuffer, 4);
  
  if (lb_send_success == true ) {

    // First half frame
    wireSendBytes(gp_read_buffer, 32);
    for (unsigned char i=0;i<32;i++) {  // Second half frame
      ga_sendBuffer[i] = gp_read_buffer[i+32];
    }
    wireSendBytes(ga_sendBuffer, 32);
  }  
  /*
  DEBUG_PRINTLN("");
  for (int i=0;i<FBLENGTH;i++) {  
    DEBUG_PRINT(gp_read_buffer[i]);
  }
  DEBUG_PRINTLN("");
  */
  
  // Send end key
  ga_sendBuffer[0] = lc_coma;
  ga_sendBuffer[1] = lc_coma;
  ga_sendBuffer[2] = lc_space;
  ga_sendBuffer[3] = lc_space;
  wireSendBytes(ga_sendBuffer, 4);  
}

//START:-------FILE handler ------------------//
void printFileContent() {
  File myProjectFile = SPIFFS.open(ANIM_FILE_NAME, FILE_READ);
  if (myProjectFile) {
    while (myProjectFile.available()) {
      unsigned char myC = myProjectFile.read();
      DEBUG_PRINTLN(myC);
    } 
  }
}
void checkAnimationSDCard() {
  // Open File for read
  File myProjectFile = SPIFFS.open(ANIM_FILE_NAME, FILE_READ);
  if (myProjectFile) {
    DEBUG_PRINTLN_TXT("Reading keys");
    
    unsigned char myReadStatus = 0; // 0 = expect key
    unsigned char myC = 0;
    unsigned char myKeyBuffer[MAX_ANIMKEY_LENGTH]; // maximum length for the key
    unsigned char myKeyLength = 0;
    unsigned char myIntBuffer[2]; // Byte to int
    unsigned char myIntBufferLength = 0;     
    
    while (myProjectFile.available() && myReadStatus < 20) {
      myC = myProjectFile.read();
     
      switch (myReadStatus) {
        case 0: // expect key
          if ( myC == lc_coma ) {
            myReadStatus = 1; // Startkey found  
          }
          break;
        case 1: // expect keyline confirmation
          if ( myC == lc_F ) {
            myReadStatus = 2; //  Key-line confirmed
          }else{
            myReadStatus = 0; // Reset 
          }
          break;
        case 2: // Reading animation key
          if ( myC == lc_coma ) {
            myReadStatus = 3; // change to read next element
          }else if ( myKeyLength < MAX_ANIMKEY_LENGTH ) { // Fill till max buffer
            myKeyBuffer[myKeyLength] = myC;
            myKeyLength++;
          }
          break;
        case 3: // Save playtime
          if ( myIntBufferLength < 2 ) { // Save playtime value
            myIntBuffer[myIntBufferLength] = myC;
            myIntBufferLength++;
          }
          if ( myIntBufferLength == 2 ) { // Buffer is full, set values
            unsigned int animationLength = word(myIntBuffer[1], myIntBuffer[0]);
            // Save the time the animation will end
            _animationLength = animationLength * 1000 + millis();
            // clear buffer
            myIntBufferLength = 0;
            // expect next value
            myReadStatus = 4;
          }
          break;
        case 4: // Save speed
          if ( myIntBufferLength < 2 ) { // Save speed value
            myIntBuffer[myIntBufferLength] = myC;
            myIntBufferLength++;
          }
          if ( myIntBufferLength == 2 ) { // Buffer is full, set values
            _animationSpeed = word(myIntBuffer[1], myIntBuffer[0]);
            // clear buffer
            myIntBufferLength = 0;
            // expect next value
            myReadStatus = 5;
          }
          break;
        case 5: // Save frames
          if ( myIntBufferLength < 2 ) { // Save speed value
            myIntBuffer[myIntBufferLength] = myC;
            myIntBufferLength++;
          }
          if ( myIntBufferLength == 2 ) { // Buffer is full, set values
            // set to complete read
            myReadStatus = 6;
            unsigned int animationFrames = word(myIntBuffer[1], myIntBuffer[0]);
            // Calculate start and end position
            _animationStartPos = myProjectFile.position() + 1;
            _animationEndPos = _animationStartPos + 65 * animationFrames;
            // Check if the key matches
            if(readBufferCompare2(reinterpret_cast<const char*>(myKeyBuffer), myKeyLength) > -1) {
              // We found animation
              DEBUG_PRINTLN_TXT("Found animation");
               myReadStatus = 20; // End search, we found our animation
            }else{
              DEBUG_PRINTLN_TXT("No animation found");
              if (_animationEndPos > _animationStartPos ) { 
                // Goto next frame 
                myProjectFile.seek(_animationEndPos, SeekSet);
                myReadStatus = 0;
              }
            }
            
            // clear buffer
            myIntBufferLength = 0;
          }
          break;
        case 6:
          if( myC == lc_newline ) { // uncomplete keyline or no animation found
            // check for values
            if ( _animationEndPos > 0 && _animationStartPos > 0 && _animationSpeed > 0 ) {
              DEBUG_PRINTLN_TXT("Anim key found");
            }else{
              DEBUG_PRINTLN_TXT("--keyline failed--");
              // Reset everything
              myReadStatus = 0; 
              myKeyLength = 0;
              myIntBufferLength = 0;
              clearSavedAnimation();
            }
          }            
          break;
      }
    } 
  }
  myProjectFile.close();
}

boolean readAnimationSD() {
  unsigned char myC;
  unsigned char myBytes = 0;
  // Read SD Card
  File myProjectFile = SPIFFS.open(ANIM_FILE_NAME, FILE_READ);
  // Goto start position
  if ( myProjectFile.available() ) {
    unsigned long _animationPos = _animationStartPos + _animationActFrame * 65; // startPos + offset
    if ( _animationPos < _animationEndPos ) { 
      myProjectFile.seek(_animationPos, SeekSet);
    }else{
      _animationActFrame = 0; // Start at the first frame again
      myProjectFile.seek(_animationStartPos, SeekSet);
    }
  }
  // Read one frame
  while ( myProjectFile.available() && myBytes < 64 ) { 
    myC = myProjectFile.read();
    myReceiveBuffer[myBytes] = myC;
    myBytes++; 
  } 

  // close the file:
  myProjectFile.close();
  
  // If we have complete frame, return true
  if ( myBytes > 63 ) { 
    // read successfull
    _animationActFrame++;
    return true;
  }else{
    return false;
  }
}
unsigned long byte2Long(unsigned char* byteArray) {
  // little endian conversion
  unsigned long retval;
  retval  = (unsigned long) byteArray[3] << 24 | (unsigned long) byteArray[2] << 16;
  retval |= (unsigned long) byteArray[1] << 8 | byteArray[0];  
  return retval;
}
void writeAnimationSDCard(WiFiClient client) {
  // Send answer
  unsigned char readBuffer[4];
  // First 10 bytes is a key, following by the length
  readBuffer[0] = myReadBuffer[10];
  readBuffer[1] = myReadBuffer[11];
  readBuffer[2] = myReadBuffer[12];
  readBuffer[3] = myReadBuffer[13];
  unsigned long fileSize = byte2Long(readBuffer);

//  DEBUG_PRINT_TXT("fileSizeBuffer: ");
  for (unsigned char i=0; i<4; i++ ) {
    client.write(readBuffer[i]);
//    DEBUG_PRINT(readBuffer[i]);
  }
  client.write(lc_return);
  client.write(lc_newline);
//  DEBUG_PRINTLN_TXT(" ");

  
  
//  DEBUG_PRINT_TXT("Filesize expected:");
//  DEBUG_PRINTLN(fileSize);
  
  // Blocking mode to write receiving data to file
  if (client) {
    // Remove file if exists
    if ( SPIFFS.exists(ANIM_FILE_NAME) ) {
      SPIFFS.remove(ANIM_FILE_NAME);
    }
    
    File myProjectFile = SPIFFS.open(ANIM_FILE_NAME, FILE_WRITE);
    unsigned long receivedBytes = 0;
    
    if (myProjectFile) {
      while (client.connected() && receivedBytes < fileSize) {
        if (client.available()) {
          unsigned char c = client.read();
          //writing bytes
          myProjectFile.write(c);
          receivedBytes++;
        } 
      }
    }
/*
    if ( receivedBytes == fileSize ) {
      DEBUG_PRINTLN_TXT("Complete data received\n");
    }else{
      DEBUG_PRINTLN_TXT("Data incomplete\n");
    }
    DEBUG_PRINT_TXT("Expected:");
    DEBUG_PRINTLN(fileSize);
    DEBUG_PRINT_TXT("Received:");
    DEBUG_PRINTLN(receivedBytes);
*/
    myProjectFile.close();
  }
}
void displaySavedAnimation() {
  if ( _animationLength > 0 ) { // Animation was set
    unsigned long currentMillis = millis();
    // Display animation set over the network
    if ( currentMillis < _animationLength ) { // __animationLength = startTime + animLengthTime
      if ( _animationStartPos > 0 && _animationEndPos > 0 && _animationSpeed > 0){ // validity check
        if ( _animationEndPos > _animationStartPos ) {
          //DEBUG_PRINTLN_TXT("Setting mode to 2");
          setServerMode(2); // Switch the server mode
          if ( currentMillis - _previousMillis >= _animationSpeed ) { // Sent data with the set speed
            // save the last time you sent frame
            _previousMillis = currentMillis;  
            // Fill Buffer
            if ( true == readAnimationSD() ) {
              // Send buffered frame
              sendBufferedFrame();
            } 
          }
        }     
      } 
    }else{
      // End the stream mode
      setServerMode(0);
    }
  }   
}
//END:---------FILE handler ------------------//

//START:-------HTTP handler ------------------//

void handleRoot() {
  if(!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }else{
    server.send(200, "text/plain", "Login OK");
  }
}

//END:--------HTTP handler -------------------//


//START:------------SETUP-----------------//
void setup() {
  Wire.begin();  //for ESP8266-12E SDA=D2 and SLC=D1
  Wire.setClockStretchLimit(1500); 

#ifdef DEBUG
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
#endif


  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();
  DEBUG_PRINT_TXT("SPIFFS opened: ");
  DEBUG_PRINTLN(result);

  // Start the WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() != WL_CONNECTED) {
    DEBUG_PRINTLN_TXT("WiFi Connect Failed! Rebooting...");
    delay(1000);
    ESP.restart();
  }
  //ArduinoOTA.begin();
  server.on("/", handleRoot);
  server.begin();

  DEBUG_PRINT_TXT("Server started http://");
  DEBUG_PRINTLN(WiFi.localIP());

  // Init the buffer
  gp_read_buffer = ga_buffer1;
  gp_write_buffer = ga_buffer2;

  // Random number generator
  randomSeed(123);
} 
//END:---------------SETUP--------------//

//START:-------------MAIN---------------// 
void loop() {

#ifdef DEBUG
  if ( Serial.available() > 0 ) {
    // get incoming byte:
     char inByte = Serial.read();    
     if ( inByte == '+' ) {
      if ( sendDelay < 10000 ) {
         sendDelay = sendDelay + 100; 
      }
     }
     if ( inByte == '-' ) {
       if ( sendDelay >= 100 ) {
         sendDelay = sendDelay - 100; 
       }
     }
     if ( inByte == '1' ) {
      gi_status = 1;
     }
     if ( inByte == '0' ) {
      gi_status = 0;
     }
     
     //DEBUG_PRINTLN(inByte);
     DEBUG_PRINT_TXT("Delay: ");
     DEBUG_PRINTLN(sendDelay);
     DEBUG_PRINT_TXT("Status: ");
     DEBUG_PRINTLN(gi_status);
  }
#endif

  unsigned long currentMillis = millis();

  if ( gi_status == 1 && currentMillis - lastChangeTime > sendDelay) {
    lastChangeTime = currentMillis;
  
    // Fill the buffer with random data
    //fill_buffer_random(gp_read_buffer);
    fill_buffer_run1(gp_read_buffer);
  
    // Send data
    sendBufferedFrame();

    //print_buffer(gp_read_buffer);

    //DEBUG_PRINT_TXT("Data sent. Check digit: ");
    //DEBUG_PRINTLN(gp_read_buffer[FBLENGTH-1]);

    //delay(sendDelay);
  }

  // handle the Webserver requests
  //ArduinoOTA.handle();
  server.handleClient();
}

//END:--------------MAIN----------------------//

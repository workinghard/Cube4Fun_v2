#include <Rainbowduino.h>
#include <Wire.h>

//#define DEBUG true
#define COLORSHIFTTIME 100
#define MAXSTACKSIZE 256
#define NEWFRAMETIMEOUT 5000

#ifdef DEBUG
  #define DEBUG_PRINTLN(x)  Serial.println (x)
  #define DEBUG_PRINT(x) Serial.print (x)
  #define DEBUG_PRINTLN_TXT(x)  Serial.println (F(x))
  #define DEBUG_PRINT_TXT(x) Serial.print (F(x))
#else
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN_TXT(x)
  #define DEBUG_PRINT_TXT(x)
#endif 

// New balanced colors
static unsigned char RED[256] = {255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                          255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,247,243,238,230,226,217,213,209,
                          200,196,188,183,179,171,166,158,154,145,141,137,128,124,115,111,107,98,94,85,81,77,68,64,56,51,47,39,34,
                          26,22,13,9,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,9,13,22,26,35,39,43,52,56,
                          64,68,77,81,85,94,98,107,111,115,124,128,136,141,145,153,158,166,171,179,183,188,196,200,209,213,217,226,
                          230,239,243,247,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                          255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0};
static unsigned char GREEN[256] = {0,0,7,17,22,30,34,39,47,51,60,64,68,77,81,90,94,98,107,111,119,124,132,136,141,149,153,162,166,170,179,
                          183,192,196,200,209,213,221,226,230,238,243,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                          255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                          255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                          255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,251,243,238,230,226,221,213,209,200,196,
                          192,183,179,170,166,158,154,149,141,136,128,124,120,111,107,98,94,90,81,77,68,64,56,52,47,39,34,26,22,18,9,
                          5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                          0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0};
static unsigned char BLUE[256] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,9,18,22,26,34,39,47,52,56,64,
                                  68,77,81,90,94,98,107,111,120,124,128,136,141,149,154,158,166,170,179,183,188,196,200,209,213,221,226,230,
                                  238,243,251,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
                                  255,255,255,255,255,255,255,255,251,243,239,234,226,221,213,209,200,196,192,183,179,171,166,162,153,149,141,
                                  136,132,124,120,111,107,103,94,90,81,77,68,64,60,52,47,39,35,30,22,17,255,0};

//unsigned char _buffer_2D[8][8];      // PlasmaMatrix = buffer_2D
unsigned char _buffer2_3D[4][4][4];  // 
unsigned char _buffer1_3D[4][4][4];  // 
unsigned char (* displayBuffer)[4][4][4];
unsigned char (* writeBuffer)[4][4][4];
int writeBuffer_top = 0;
unsigned char _receive_buffer[MAXSTACKSIZE];
unsigned char _receive_buffer_top = 0;
unsigned char _receive_buffer_bot = 0;
unsigned long streamStartedTime = 0;
unsigned long newFrameMillis = 0;
unsigned char z,x,y,colorshift=0;
long animation = 0;
unsigned long lastChangeTime = 0;
unsigned long lastColorShift = 0;
const long animInterval = 300000; // 5 minutes
int __state = 0; // 0 = waiting for cmd
int __stateEnd = 0; // 0 = start
boolean newFrame = false;

unsigned const char lc_slash = '/';
unsigned const char lc_question = '?';
unsigned const char lc_coma = ',';
unsigned const char lc_space = ' ';

// --------------- FUNCTIONS ---------------- //
void genPlasmaMatrix() {
  // Generate Plasma-Array
  for(x = 0; x < 8; x++) {
    for(y = 0; y < 8; y++) {
      uint32_t color = int(32.0 + (32.0 * sin(x / 4.0)) + 32.0 + (32.0 * sin(y / 4.0))) / 2;
      setByteColor2D(color);
    }
  }
}

void genPlasmaMatrix2() {
  // Generate Plasma2-Array
  for(x = 0; x < 8; x++) {
    for(y = 0; y < 8; y++) {
      uint32_t color = int(32.0 + (32.0 * sin(x / 1.0)) + 32.0 + (32.0 * sin(y / 1.0))) / 2;
      setByteColor2D(color);
    }
  }
}

void genPlasmaCube() {
  // Generate PlasmaCube-Array
  for(x = 0; x < 4; x++) {
    for(y = 0; y < 4; y++) {
      for(z = 0; z < 4; z++) {
        int color = int(32.0 + (32.0 * sin(x / 1.0))+ 32.0 + (32.0 * sin(y / 1.0)) + 32.0 + (32.0 * sin(z / 1.0))) / 3;
        setByteColor3D(color);      
      }   
    }
  }  
}

void genPlasmaCube2() {
  // Generate PlasmaCube2-Array
  for(x = 0; x < 4; x++) {
    for(y = 0; y < 4; y++) {
      for(z = 0; z < 4; z++) {
        int color = int(32.0 + (32.0 * sin(x / 4.0))+ 32.0 + (32.0 * sin(y / 4.0)) + 32.0 + (32.0 * sin(z / 4.0))) / 3;
        setByteColor3D(color);      
      }   
    }
  }
}

void genPlasmaCube3() {
  // Generate PlasmaCube3-Array
  for(x = 0; x < 4; x++) {
    for(y = 0; y < 4; y++) {
      for(z = 0; z < 4; z++) {
        int color = int(32.0 + (32.0 * sin(x / 8.0))+ 32.0 + (32.0 * sin(y / 8.0)) + 32.0 + (32.0 * sin(z / 8.0))) / 3;
        setByteColor3D(color);      
      }   
    }
  }
}

void setByteColor2D(uint32_t color) {  
  unsigned char color255 = (color * 4 + colorshift ) % 256; // Transform to 8 Bit color
  if ( color255 > 0 ) {
    if ( color255 > 253 ) { // 254 and 255 are reserved
      color255 = 253;
    }
  }else{
    color255 = 0;
  } // Range 0 ... 253
  
  Rb.setPixelXY(x,y,RED[color255],GREEN[color255],BLUE[color255]);
}

void setByteColor3D(uint32_t color) {  
  unsigned char color255 = (color * 4 + colorshift ) % 256; // Transform to 8 Bit color
  if ( color255 > 0 ) {
    if ( color255 > 253 ) { // 254 and 255 are reserved
      color255 = 253;
    }
  }else{
    color255 = 0;
  } // Range 0 ... 253
  
  Rb.setPixelZXY(z,x,y,RED[color255],GREEN[color255],BLUE[color255]);
}

void drawNewFrame() {
  for(x=0;x<4;x++) {
    for(y=0;y<4;y++) {
      for(z=0;z<4;z++) {
        Rb.setPixelZXY(z,x,y,(RED[(*displayBuffer)[x][y][z]]),(GREEN[(*displayBuffer)[x][y][z]]),(BLUE[(*displayBuffer)[x][y][z]])); //uses R, G and B color bytes
      }
    }
  }
  // Update the timeout timer
  streamStartedTime = millis();
}

void changeAnim() {
  animation = random(0,5);
  //animation = 0;
}


// Flip the write and display buffer
void flipBuffer() {
  unsigned char (* p_tmp)[4][4][4];
  p_tmp = displayBuffer;
  displayBuffer = writeBuffer;
  writeBuffer = p_tmp;

  // Remember the timestamp
  newFrameMillis = millis();
} 

void receiveEvent(int howMany) {
  while ( Wire.available() > 0 ) {
    if ( _receive_buffer_top < MAXSTACKSIZE ) {
      _receive_buffer[_receive_buffer_top] = (unsigned char)Wire.read();
      _receive_buffer_top ++;
    }else{
      DEBUG_PRINTLN_TXT("Buffer overflow");
      Wire.read(); // Empty the queue
    }
  }  
}

void fillBuffer(unsigned char pc_input, int p_pos) {
  int li_tmp = 0;
  for(int lx=0;lx<4;lx++) {
    for(int ly=0;ly<4;ly++) {
      for(int lz=0;lz<4;lz++) {
        if ( li_tmp == p_pos ) {
          (*writeBuffer)[lx][ly][lz] = pc_input;
          return;
        }
        li_tmp++;
      }
    }
  }   
}
unsigned char getBuffer(int p_pos) {
  int li_tmp = 0;
  for(int lx=0;lx<4;lx++) {
    for(int ly=0;ly<4;ly++) {
      for(int lz=0;lz<4;lz++) {
        if ( li_tmp == p_pos ) {
          return (*writeBuffer)[lx][ly][lz];
        }
        li_tmp++;
      }
    }
  }  
  return 0;
}

int checkEnd(unsigned char pc_input) {
  int isEnd = 1;
  switch (__stateEnd) {
    case 0:
      if ( pc_input == lc_coma) { 
        __stateEnd = __stateEnd + 1;
      }
      break;
    case 1:
      if ( pc_input == lc_coma ) {
        __stateEnd = __stateEnd + 1;
      }else{
        __stateEnd = 0; // Roll back
      }
      break;
    case 2:
      if ( pc_input == lc_space ) {
        __stateEnd = __stateEnd + 1;
      }else{
        __stateEnd = 0; // Roll back
      }
      break;
    case 3:
      if ( pc_input == lc_space ) {
        // End found
        isEnd = 0;
      }
      __stateEnd = 0; // Roll back

  }
  return isEnd;
}


void processIn(unsigned char pc_input) {
  switch (__state) {
    case 0:
      if ( checkEnd(pc_input) == 0 ) {
        // Reset the values
        //DEBUG_PRINTLN_TXT("End found.");
        //DEBUG_PRINT_TXT("Pos: ");
        //DEBUG_PRINTLN(gi_buffer2_pos);
        __state = 0;
        writeBuffer_top = 0;
      }
      if ( pc_input == lc_slash ) { 
        __state = __state + 1;
        //DEBUG_PRINTLN_TXT("0->1");
      }
      break;
    case 1:
      if ( pc_input == lc_slash ) {
        __state = __state + 1;
        //DEBUG_PRINTLN_TXT("1->2");
      }else{
        // something went wrong, roll back
        __state = 0;
        //DEBUG_PRINTLN_TXT("1->0");
      }
      break;
    case 2:
      if ( pc_input == lc_question ) {
        __state = __state + 1;
        //DEBUG_PRINTLN_TXT("2->3");
      }else{
        // something went wrong, roll back
        __state = 0;
        //DEBUG_PRINTLN_TXT("2->0");
      }
      break;
    case 3:
      if ( pc_input == lc_question ) {
        __state = __state + 1;
        //DEBUG_PRINTLN_TXT("3->4");
      }else{
        // something went wrong, roll back
        __state = 0;
      }
      break;
    case 4:
      //DEBUG_PRINT(writeBuffer_top);
      //DEBUG_PRINT_TXT(":");
      //DEBUG_PRINTLN(pc_input);
      if ( checkEnd(pc_input) == 0 ) {
        // Reset the values
        __state = 0;
        writeBuffer_top = 0;
        DEBUG_PRINTLN_TXT("ERR: End found.");
      }else{
        // Reading data
        fillBuffer(pc_input, writeBuffer_top);
        if ( writeBuffer_top < 64 ) { // Fill 64 chars
          writeBuffer_top++;
        }
        if ( writeBuffer_top == 64 ) {
          unsigned char lc_check = 0;
          for (int i=0; i<64;i++) {
            lc_check = lc_check + getBuffer(i); 
            //DEBUG_PRINT(i);
            //DEBUG_PRINT_TXT(":");
            //DEBUG_PRINTLN(getBuffer(i));
          }
          DEBUG_PRINT_TXT("Checksum: ");
          DEBUG_PRINTLN(lc_check);

          newFrame = true;     // Draw new frame
          flipBuffer();        // Change to the second buffer, so the first one can be displayed
          writeBuffer_top = 0; // Reset the writeBuffer
          __state = 0;         // Change state to listen
          __stateEnd = 0;      // Reset endState check
        }
      }   
  }
  
}

// ---------------- SETUP --------------- //
void setup() {
  Rb.init(); //initialize Rainbowduino driver
  Wire.begin(2); // initialize wire connection as slave #2
  Wire.onReceive(receiveEvent); // set function to be called
  
#ifdef DEBUG
  Serial.begin(57600);
#endif
  DEBUG_PRINTLN_TXT("Empfaenger 2");
  
  randomSeed(analogRead(0)); // Init randomizer

  displayBuffer = &_buffer2_3D;
  writeBuffer   = &_buffer1_3D;
  
  changeAnim(); // First random animation
}
// ---------------- SETUP --------------- //

// ---------------- MAIN --------------- //

void loop()  {
 
  if ( _receive_buffer_top != _receive_buffer_bot ) {
    while ( _receive_buffer_top != _receive_buffer_bot ) {
      //DEBUG_PRINT(_receive_buffer[_receive_buffer_bot]);
      processIn(_receive_buffer[_receive_buffer_bot]);
      _receive_buffer_bot++;  // Consume input
    }
    //DEBUG_PRINTLN_TXT("");
  }
  if ( _receive_buffer_top == _receive_buffer_bot && _receive_buffer_top > 0) {
    _receive_buffer_top = 0;
    _receive_buffer_bot = 0;
  }
  
  unsigned long currentMillis = millis();

  if ( newFrame == true ) {
    // Draw received frame
    drawNewFrame();
    // Check for the timeout
    if ( currentMillis - newFrameMillis > NEWFRAMETIMEOUT ) {
      newFrameMillis = currentMillis;
      newFrame = false;
    }
  }else{
      if ( currentMillis - lastChangeTime > animInterval) {
        lastChangeTime = currentMillis;
        changeAnim();
      }
  
      switch (animation) {
        case 0:  // Plasma1
          genPlasmaMatrix();
          break;
        case 1:  // Plasma2
          genPlasmaMatrix2();
          break;
        case 2: // Cube3
          genPlasmaCube3();
          break;
        case 3: // Cube1
          genPlasmaCube();
          break;
        case 4: // Cube2
          genPlasmaCube2();
          break;
        default:  
          genPlasmaCube();
      }

      if ( currentMillis - lastColorShift > COLORSHIFTTIME) { // 
        lastColorShift = currentMillis;
        colorshift = colorshift + 1; 
      }
      //delay(100);
  }         


  
}


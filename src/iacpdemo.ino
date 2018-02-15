/***************************************************
This is our Bitmap drawing example for the Adafruit ILI9341 Breakout and Shield
----> http://www.adafruit.com/products/1651

Check out the links above for our tutorials and wiring diagrams
These displays use SPI to communicate, 4 or 5 pins are required to
interface (RST is optional)
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution
****************************************************/


#include "SdFat.h"
#include <Adafruit_FT6206.h>

#if defined(PARTICLE)

#include "Adafruit_mfGFX.h"
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC D6
#define TFT_CS A1
#define TFT_MOSI A5
#define TFT_MISO A4
#define TFT_RST D7
#define TFT_CLK A3

#else
#include "SPI.h"
#include <Adafruit_mfGFX.h>
#include "Adafruit_ILI9341.h"

// For the Adafruit shield, these are the default.
#define TFT_DC 9
#define TFT_CS 10
#endif

#include <queue>

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

#define SD_CS A0

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

// Software SPI
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Capacitive Touchscreen object
Adafruit_FT6206 ts = Adafruit_FT6206();

SdFat SD;
File file;
volatile bool IsDmaReady = true;
bool resumeFlag = false;
std::queue<uint8_t*> bufferFillManager;
std::queue<uint8_t*> bufferSendManager;
uint16_t transferProgressIndex = 0;

void setup(void) {
  Serial.begin(9600);
  SPI.setClockSpeed(30, MHZ);

  tft.begin();
  if (!ts.begin(40)) {
    Serial.println("Unable to start touchscreen.");
  }
  else {
    Serial.println("Touchscreen started.");
  }
  tft.fillScreen(ILI9341_BLACK);

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS, SPI_FULL_SPEED)) {
    SD.errorPrint("initFail");

    Serial.println("Trying again...");

    waitFor(resetSPILines, 5000);
  }
  Serial.println("OK!");

  bmpDraw("unitUI1_F.bmp", 0, 0);
}

void loop() {
  static unsigned long timer = millis();
  static bool flip = false;
  if (ts.touched()) {

    if ((millis() - timer) > 100) {
      if (flip) {
        flip = false;
        //tft.writecommand(ILI9341_DISPOFF);
        bmpDraw("unitUI1_F.bmp", 0, 0);
        //tft.fillRedScreen(ILI9341_RED);
        //tft.writecommand(ILI9341_DISPON);
      } else {
        flip = true;
        //tft.writecommand(ILI9341_DISPOFF);
        bmpDraw("unitUI2_F.bmp", 0, 0);
        //  bmpDraw("purple.bmp", 0, 0);
        //tft.writecommand(ILI9341_DISPON);
      }
      timer = millis();
    }
  }
}

bool resetSPILines() {
  bool result = false;
  SPI.endTransaction();

  static unsigned long lineResetDebounce = millis();
  if ((millis() - lineResetDebounce) > 100) {
    if (SD.begin(SD_CS, SPI_FULL_SPEED)) {
      result = true;
    }
  }

  return result;
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 960
#define NUM_OF_ROWS 6

uint8_t* readptr = NULL;
volatile bool isBufferFillEmpty;

void bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;
  uint16_t pixSize = 3 * BUFFPIXEL;
  uint16_t maxReadBufferLen = 0;
  uint8_t  sdbuffer[pixSize]; // pixel buffer (R+G+B per pixel)
  uint16_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();

  uint16_t txBufferLen = 2 * 240 * NUM_OF_ROWS;
  static uint8_t txBuffer1[2 * 240 * 6];
  static uint8_t txBuffer2[2 * 240 * 6];
  static uint8_t txBuffer3[2 * 240 * 6];
  bufferFillManager.push((unsigned char*) txBuffer1);
  bufferFillManager.push((unsigned char*) txBuffer2);
  bufferFillManager.push((unsigned char*) txBuffer3);
  uint8_t* writeptr;

  if((x >= tft.width()) || (y >= tft.height())) return;

  Serial.println();
  Serial.print(F("Loading image '"));
  Serial.print(filename);
  Serial.println('\'');

  // Open requested file on SD card
  if ((bmpFile = SD.open(filename)) == NULL) {
    Serial.print(F("File not found"));
    return;
  }

  writeptr = bufferFillManager.front();
  bufferFillManager.pop();

  // Parse BMP header

  if(read16(bmpFile) == 0x4D42) { // BMP signature

    Serial.print(F("File size: ")); Serial.println(read32(bmpFile));
    (void)read32(bmpFile); // Read & ignore creator bytes
    bmpImageoffset = read32(bmpFile); // Start of image data
    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    Serial.print(F("Header size: ")); Serial.println(read32(bmpFile));
    bmpWidth  = read32(bmpFile);
    bmpHeight = read32(bmpFile);
    if(read16(bmpFile) == 1) { // # planes -- must be '1'
    bmpDepth = read16(bmpFile); // bits per pixel
    Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
    if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed

      goodBmp = true; // Supported BMP format -- proceed!
      Serial.print(F("Image size:"));
      Serial.print(bmpWidth);
      Serial.print('x');
      Serial.println(bmpHeight);

      // BMP rows are padded (if needed) to 4-byte boundary
      rowSize = (bmpWidth * 3 + 3) & ~3;

      // If bmpHeight is negative, image is in top-down order.
      // This is not canon but has been observed in the wild.
      if(bmpHeight < 0) {
        bmpHeight = -bmpHeight;
        flip      = false;
      }

      // Crop area to be loaded
      x2 = x + bmpWidth  - 1; // Lower-right corner
      y2 = y + bmpHeight - 1;
      if((x2 >= 0) && (y2 >= 0)) { // On screen?
        w = bmpWidth; // Width/height of section to load/display
        h = bmpHeight;
        bx1 = by1 = 0; // UL coordinate in BMP file
        if(x < 0) { // Clip left
          bx1 = -x;
          x   = 0;
          w   = x2 + 1;
        }
        if(y < 0) { // Clip top
          by1 = -y;
          y   = 0;
          h   = y2 + 1;
        }

        if(x2 >= tft.width())  w = tft.width()  - x; // Clip right
        if(y2 >= tft.height()) h = tft.height() - y; // Clip bottom

        // Set TFT address window to clipped image bounds
        //tft.startWrite(); // Requires start/end transaction now
        tft.setAddrWindow(x, y, w, h);

        uint16_t bufferTracker = 0;
        for (row=0; row<h - 4; row+=4) { // For each scanline...

          if (readptr) {
            pauseDmaTransfer();
          }
          // bmpFile.seek(pos);
          //Serial.printlnf("File Position: %d", bmpFile.position());

          pos = bmpImageoffset + (row + by1) * rowSize;
          pos += bx1 * 3; // Factor in starting column (bx1)
          //Serial.printlnf("Target File Position: %d\n", pos);
          bmpFile.seek(pos);
          buffidx = sizeof(sdbuffer); // Force buffer reload

          if (buffidx >= sizeof(sdbuffer)) { // Indeed
            //Serial.print("Pause for Reading: ");
            maxReadBufferLen = bmpFile.read(sdbuffer, sizeof(sdbuffer));
            //Serial.printlnf("%d ", maxReadBufferLen);
            //Serial.println("Complete");

            //for (int i = 0; i < quantRead; i++) {
            //  bmpFile.read(&sdbuffer[i * rowSize], rowSize);
            //  Serial.printf("%d ", i * rowSize);
            //}
            //Serial.println();
            //if (!sizeBuff) {
            //  bmpFile.read(&sdbuffer[sizeof(sdbuffer) - sizeBuff], sizeBuff);
            //}

            //pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            //pos += bx1 * 3; // Factor in starting column (bx1)
            //if (bmpFile.position() != pos) {
            //  bmpFile.seek(pos);
            //}
            buffidx = 0; // Set index to beginning
          }
          unpauseDmaTransfer(readptr, txBufferLen);
          //}

          while (buffidx < maxReadBufferLen) {
            //while (buffidx < sizeof(sdbuffer)) {
            //Serial.printf("%d-", buffidx);
            b = sdbuffer[buffidx++];
            //Serial.printf("%d-", buffidx);
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];

            uint16_t colorStore = tft.color565(r, g, b);

            if (!writeptr) {
              // Serial.println("Held up");

              while(isBufferFillEmpty) {
                isBufferFillEmpty = bufferFillManager.empty();
                SysCall::yield();
                // Serial.printlnf("Somethin's up %d", isBufferFillEmpty);
              }
              //Serial.println("Finished");
              writeptr = bufferFillManager.front();
              bufferFillManager.pop();
              bufferTracker = 0;
            }

            if (writeptr) {
              //Serial.printf("Write comp: %d | ", bufferTracker);
              writeptr[bufferTracker++] = colorStore >> 8;
              writeptr[bufferTracker++] = colorStore;
              //Serial.printlnf("COMPLETE: %d", bufferTracker);
            }

            if (writeptr && (bufferTracker > txBufferLen - 1)) {
              bufferTracker = 0;
              bufferSendManager.push((unsigned char*) writeptr);
              if (bufferFillManager.size() > 0) {
                writeptr = bufferFillManager.front();
                bufferFillManager.pop();
              } else {
                writeptr = NULL;
              }
            }
          } // end pixel

          //for (int i = 0; i < w; i++) {
          //  uint16_t r = i * 2;
          //  Serial.printf("%d %d ", writeptr[r], writeptr[r+1]);
          //}

          if (IsDmaReady && bufferSendManager.size() > 0) {
            IsDmaReady = false;
            readptr = bufferSendManager.front();
            bufferSendManager.pop();

            //Serial.println("startWrite may be bottleneck");
            tft.startWrite();
            //Serial.println("enter write");
            tft.dmaspiwrite(readptr, txBufferLen, resetDmaReadyFlag);
            //Serial.println("Leave write");
          }
          // end scanline
          // End last TFT transaction
        }
      } // end onscreen
      Serial.print(F("Loaded in "));
      Serial.print(millis() - startTime);
      Serial.println(" ms");
    } // end goodBmp
  }
}

readptr = NULL;
writeptr = NULL;
isBufferFillEmpty = true;
for (int i = 0; i < bufferSendManager.size(); i++) {
  bufferSendManager.pop();
}
for (int i = 0; i < bufferFillManager.size(); i++) {
  bufferFillManager.pop();
}
bmpFile.close();
if(!goodBmp) Serial.println(F("BMP format not recognized."));
}

void pauseDmaTransfer() {
  SPI.transferCancel();
  //Serial.print("Waiting...");
  while (!IsDmaReady) {
    SysCall::yield();
  }
  //Serial.println("  Finished!");
  if (transferProgressIndex < 2 * 240 * NUM_OF_ROWS) {
    resumeFlag = true;
  } else {
    Serial.printlnf("\n\nWe messsed up: %d \n\n", transferProgressIndex);
  }
}

void unpauseDmaTransfer(uint8_t* txBuffer, uint16_t bufflen) {
  if (!resumeFlag || !txBuffer) {
    return;
  }

  IsDmaReady = false;
  resumeFlag = false;
  uint16_t indexFromEnd = bufflen - transferProgressIndex;
  //Serial.printlnf("RESUME -- Remaining: %d", indexFromEnd);

  tft.startWrite();

  //  Somehow Particle knows what's already sent and appropriate handles unsent data.
  //    Unlikely.
  //tft.dmaspiwrite(txBuffer, bufflen, resetDmaReadyFlag);

  //  Data is sent from the end of the array.
  //tft.dmaspiwrite(txBuffer, indexFromEnd, resetDmaReadyFlag);

  // Data shifts out from the beginning of the array. -- Result: No go.
  tft.dmaspiwrite(&txBuffer[transferProgressIndex], indexFromEnd, resetDmaReadyFlag);
}

void resetDmaReadyFlag() {
  tft.endWrite();
  transferProgressIndex += SPI.available();
  if (transferProgressIndex == 0 || transferProgressIndex < 2 * 240 * NUM_OF_ROWS) {
    //Serial.printlnf("Transfer Cancelled: %d", transferProgressIndex);
  } else {
    bufferFillManager.push((unsigned char*) readptr);
    readptr = NULL;
    transferProgressIndex = 0;
  }
  isBufferFillEmpty = bufferFillManager.empty();
  IsDmaReady = true;
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

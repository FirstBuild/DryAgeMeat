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

#include "elapsedMicros.h"
#include "SdFat.h"
#if defined(PARTICLE)
#include <Adafruit_mfGFX.h>
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

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);

// TFT display and SD card will share the hardware SPI interface.
// Hardware SPI pins are specific to the Arduino board type and
// cannot be remapped to alternate pins.  For Arduino Uno,
// Duemilanove, etc., pin 11 = MOSI, pin 12 = MISO, pin 13 = SCK.

// Software SPI
//Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

// Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

#define SD_CS A0
SdFat SD;
File file;
const uint8_t chipSelect = A0;
bool isFinished = true;

void setup(void) {
  WiFi.listen();
  Serial.begin(9600);
  SPI.setClockSpeed(16, MHZ);

  tft.begin();

  yield();

  // Create or open the file.

  Serial.print("Initializing SD card...");
  if (!SD.begin(SD_CS)) {
    Serial.println("failed!");
  }
  Serial.println("OK!");

}

void loop() {
  //for(uint8_t r=0; r<4; r++) {
  //  tft.setRotation(r);
  //  tft.fillScreen(ILI9341_BLUE);
  //  for(int8_t i=-2; i<1; i++) {
  //    bmpDraw("purple.bmp",
  //      (tft.width()  / 2) + (i * 120),
  //      (tft.height() / 2) + (i * 160));
  //  }
  //}


  static unsigned long timer = millis();
  static bool flip = false;
  if ((millis() - timer) > 3000) {
    if (flip) {
      flip = false;
      //tft.writecommand(ILI9341_DISPOFF);
        bmpDraw("purple.bmp", 0, 0);

      //tft.fillRedScreen(ILI9341_RED);
      //tft.writecommand(ILI9341_DISPON);
    } else {
      flip = true;
      //tft.writecommand(ILI9341_DISPOFF);
      tft.fillScreen(ILI9341_BLUE);
      //tft.writecommand(ILI9341_DISPON);
    }
    timer = millis();
  }
}


// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

#define BUFFPIXEL 50

void bmpDraw(char *filename, int16_t x, int16_t y) {

  File     bmpFile;
  int      bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int      w, h, row, col, x2, y2, bx1, by1;
  uint8_t  r, g, b;
  uint32_t pos = 0, startTime = millis();
  uint16_t txBufferLen = 2 * 240;
  uint8_t txBuffer[txBufferLen];


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

  elapsedMicros usec;
  uint32_t us;
  uint32_t total_seek = 0;
  uint32_t total_read = 0;
  uint32_t total_parse = 0;
  uint32_t total_draw = 0;


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

          usec = 0;

          // Set TFT address window to clipped image bounds
          //tft.startWrite(); // Requires start/end transaction now
          tft.setAddrWindow(x, y, w, h);

          for (row=0; row<h; row++) { // For each scanline...

            // Seek to start of scan line.  It might seem labor-
            // intensive to be doing this on every line, but this
            // method covers a lot of gritty details like cropping
            // and scanline padding.  Also, the seek only takes
            // place if the file position actually needs to change
            // (avoids a lot of cluster math in SD library).
            if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
              pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
            else     // Bitmap is stored top-to-bottom
              pos = bmpImageoffset + (row + by1) * rowSize;
            pos += bx1 * 3; // Factor in starting column (bx1)
            if(bmpFile.position() != pos) { // Need seek?
              //tft.endWrite(); // End TFT transaction
              bmpFile.seek(pos);
              buffidx = sizeof(sdbuffer); // Force buffer reload
              //tft.startWrite(); // Start new TFT transaction
            }

            us = usec;
            usec -= us;
            total_seek += us;

            for (col=0; col<w; col++) { // For each pixel...
              // Time to read more pixel data?
              if (buffidx >= sizeof(sdbuffer)) { // Indeed
                usec -= us;
                total_parse += us;
                //tft.endWrite(); // End TFT transaction
                bmpFile.read(sdbuffer, sizeof(sdbuffer));
                buffidx = 0; // Set index to beginning
                //tft.startWrite(); // Start new TFT transaction
                us = usec;
                usec -= us;
                total_read += us;
              }
              // Convert pixel from BMP to TFT format, push to display
              int16_t txBuffInc = col * 2;

              b = sdbuffer[buffidx++];
              g = sdbuffer[buffidx++];
              r = sdbuffer[buffidx++];

              uint16_t colorStore = tft.color565(r, g, b);
              txBuffer[txBuffInc] = colorStore >> 8;
              txBuffer[txBuffInc + 1] = colorStore;

            } // end pixel

            us = usec;
            usec -= us;
            total_parse += us;
            tft.dmaspiwrite(txBuffer, txBufferLen, NULL);
            us = usec;
            usec -= us;
            total_draw += us;

          } // end scanline
          //tft.endWrite(); // End last TFT transaction
        } // end onscreen
        Serial.print(F("Loaded in "));
        Serial.print(millis() - startTime);
        Serial.println(" ms");
        Serial.print("Seek: ");
        Serial.println(total_seek);
        Serial.print("Read: ");
        Serial.println(total_read);
        Serial.print("Parse: ");
        Serial.println(total_parse);
        Serial.print("Draw: ");
        Serial.println(total_draw);
      } // end goodBmp
    }
  }

  bmpFile.close();
  if(!goodBmp) Serial.println(F("BMP format not recognized."));
}


void refillBufferFlag() {
  isFinished = true;

  Serial.println("\n\n COMPLETE \n\n");
}

//void oldRead() {
//tft.startWrite();
//for (row=0; row<h; row++) { // For each scanline...
//
//// Seek to start of scan line.  It might seem labor-
//// intensive to be doing this on every line, but this
//// method covers a lot of gritty details like cropping
//// and scanline padding.  Also, the seek only takes
//// place if the file position actually needs to change
//// (avoids a lot of cluster math in SD library).
//if(flip) // Bitmap is stored bottom-to-top order (normal BMP)
//pos = bmpImageoffset + (bmpHeight - 1 - (row + by1)) * rowSize;
//else     // Bitmap is stored top-to-bottom
//pos = bmpImageoffset + (row + by1) * rowSize;
//pos += bx1 * 3; // Factor in starting column (bx1)
//if(bmpFile.position() != pos) { // Need seek?
//bmpFile.seek(pos);
//buffidx = sizeof(sdbuffer); // Force buffer reload
////tft.startWrite(); // Start new TFT transaction
//}
//for (col=0; col<w; col++) { // For each pixel...
//// Time to read more pixel data?
//if (buffidx >= sizeof(sdbuffer)) { // Indeed
//bmpFile.read(sdbuffer, sizeof(sdbuffer));
//buffidx = 0; // Set index to beginning
////tft.startWrite(); // Start new TFT transaction
//}
//// Convert pixel from BMP to TFT format, push to display
//b = sdbuffer[buffidx++];
//g = sdbuffer[buffidx++];
//r = sdbuffer[buffidx++];
//tft.contWrite(tft.color565(r,g,b));
//} // end pixel
//} // end scanline
//tft.endWrite();
//
//}

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
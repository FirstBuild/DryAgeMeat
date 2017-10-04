#ifndef DRYAGEMEAT_H
#define DRYAGEMEAT_H

#include "PietteTech_DHT.h"
#include "neopixel.h"
#include "RBD_Timer.h"

#define DRYAGEMEAT_VERSION_MAJOR 0
#define DRYAGEMEAT_VERSION_MINOR 0
#define DRYAGEMEAT_VERSION_PATCH 5

// How many leds in your strip?
#define NUM_LEDS 19
#define DATA_PIN D7
#define BRIGHTNESS 155  /// 60% brightness -- Max value is 255

#define COMPRESSOR_RELAY D6   /// This is Arduino pin D4
#define FAN_RELAY A5          /// This is Arduin pin D8

#define S0 A4
#define S1 5
#define S2 2

#define DEBUG

// system defines
#define DHTTYPE  DHT11              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   2000  // Sample every two seconds

const int selectPins[3] = {S0, S1, S2}; //

struct MeatData {
  float humidityInPercent;
  float meatTempInFahren;
  float meatWeightInLbs;
};

struct OpenScaleData {
  long timeStamp;
  float weight;
  String units;
  float ambientTempInFahren;
};

class DryAgeMeat {
  public:
    DryAgeMeat(void) { }
    PietteTech_DHT* DHT;
    Adafruit_NeoPixel* strip;
    MeatData meatChunk;
    OpenScaleData shelfOne;
    RBD::Timer* refreshTimer;
    unsigned int DHTnextSampleTime;	    // Next time we want to start sample
    bool bDHTstarted;		                // flag to indicate we started acquisition
    int n;                              // counter

  public:
    void setup() {
      Serial.begin(9600);
      Serial1.begin(57600);

      setupMux();

      DHT = new PietteTech_DHT(DHTPIN, DHTTYPE);
      strip = new Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, WS2812B);
      refreshTimer = new RBD::Timer(1000);

      strip->begin();
      strip->setBrightness(BRIGHTNESS);
      strip->clear();
      strip->show();

      DHTnextSampleTime = 0;  // Start the first sample immediately

    }

    void loop() {

      refreshData();
    }

    void refreshData() {
      if (refreshTimer->onExpired()) {
        selectMuxPin(2);
        readProbeData(meatChunk);

        selectMuxPin(0);
        readScaleData(shelfOne);
        refreshTimer->restart();
      }
    }

// Access to Meat Probe and Weight Data
    void selectMuxPin(byte pin) {
      if (pin > 7) return; // Exit if pin is out of scope
      for (int i=0; i<3; i++)
      {
        if (pin & (1<<i))
          digitalWrite(selectPins[i], HIGH);
        else
          digitalWrite(selectPins[i], LOW);
      }
      delay(1);
    }

    void readProbeData(MeatData &newData) {
      Serial1.readStringUntil('\n'); /// Remove Junk Data;
      float probeTempInFahren = Serial1.parseFloat();

      newData.meatTempInFahren = probeTempInFahren;

      #ifdef DEBUG
       Serial.print("Probe Temp: ");
       Serial.println(newData.meatTempInFahren);
      #endif
    }

    void readScaleData(OpenScaleData &newData) {
      Serial1.readStringUntil('\n'); /// Remove Junk Data;
      String scaleData = Serial1.readStringUntil('\n');

      newData.timeStamp = commaToken(scaleData).toInt();
      newData.weight = commaToken(scaleData).toFloat();
      newData.units = commaToken(scaleData);
      newData.ambientTempInFahren = (commaToken(scaleData).toFloat() * 1.8) + 32;

    #ifdef DEBUG
      Serial.print("Time: ");
      Serial.println(newData.timeStamp);
      Serial.print("Weight: ");
      Serial.println(newData.weight);
      Serial.print("Units: ");
      Serial.println(newData.units);
      Serial.print("Ambient Temp: ");
      Serial.println(newData.ambientTempInFahren);
    #endif
    }

    String commaToken(String &start) {
      if (start.length() < 1) {
        return "";
      }
      int terminatingChar = start.indexOf(',');
      String result = start.substring(0, terminatingChar);
      start = start.substring(terminatingChar + 1);
      return result;
    }

    void setupMux() {
      for (int i = 0; i < 3; i++) {
        pinMode(selectPins[i], OUTPUT);
        digitalWrite(selectPins[i], HIGH);
      }
    }

// LED Control
    void turnOn() {
      for (int i = 0; i < NUM_LEDS; i++) {
        strip->setColor(i, 255, 255, 255);
      }
      strip->show();
    }

    void turnOff() {
      strip->clear();
    }

// Humidity Control
    void humiditySampler() {
      // Check if we need to start the next sample
      if (millis() > DHTnextSampleTime) {
        if (!bDHTstarted) {		// start the sample
          Serial.print("\n");
          Serial.print(n);
          Serial.print(": Retrieving information from sensor: ");
          DHT->acquire();
          bDHTstarted = true;
        }

        if (!DHT->acquiring()) {		// has sample completed?

          // get DHT status
          int result = DHT->getStatus();

          Serial.print("Read sensor: ");
          switch (result) {
            case DHTLIB_OK:
            Serial.println("OK");
            break;
            case DHTLIB_ERROR_CHECKSUM:
            Serial.println("Error\n\r\tChecksum error");
            break;
            case DHTLIB_ERROR_ISR_TIMEOUT:
            Serial.println("Error\n\r\tISR time out error");
            break;
            case DHTLIB_ERROR_RESPONSE_TIMEOUT:
            Serial.println("Error\n\r\tResponse time out error");
            break;
            case DHTLIB_ERROR_DATA_TIMEOUT:
            Serial.println("Error\n\r\tData time out error");
            break;
            case DHTLIB_ERROR_ACQUIRING:
            Serial.println("Error\n\r\tAcquiring");
            break;
            case DHTLIB_ERROR_DELTA:
            Serial.println("Error\n\r\tDelta time to small");
            break;
            case DHTLIB_ERROR_NOTSTARTED:
            Serial.println("Error\n\r\tNot started");
            break;
            default:
            Serial.println("Unknown error");
            break;
          }

          Serial.print("Humidity (%): ");
          Serial.println(DHT->getHumidity(), 2);

          Serial.print("Temperature (oC): ");
          Serial.println(DHT->getCelsius(), 2);

          Serial.print("Temperature (oF): ");
          Serial.println(DHT->getFahrenheit(), 2);

          Serial.print("Temperature (K): ");
          Serial.println(DHT->getKelvin(), 2);

          Serial.print("Dew Point (oC): ");
          Serial.println(DHT->getDewPoint());

          Serial.print("Dew Point Slow (oC): ");
          Serial.println(DHT->getDewPointSlow());

          n++;  // increment counter
          bDHTstarted = false;  // reset the sample flag so we can take another
          DHTnextSampleTime = millis() + DHT_SAMPLE_INTERVAL;  // set the time for next sample
        }
      }
    }
};

#endif /* DRYAGEMEAT_H */

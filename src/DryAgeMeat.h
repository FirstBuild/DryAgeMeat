#ifndef DRYAGEMEAT_H
#define DRYAGEMEAT_H

#include "PietteTech_DHT.h"
#include "neopixel.h"

#define DRYAGEMEAT_VERSION_MAJOR 0
#define DRYAGEMEAT_VERSION_MINOR 0
#define DRYAGEMEAT_VERSION_PATCH 3

// How many leds in your strip?
#define NUM_LEDS 19
#define DATA_PIN D7
#define BRIGHTNESS 155  /// 60% brightness -- Max value is 255

// system defines
#define DHTTYPE  DHT11              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   2000  // Sample every two seconds

class DryAgeMeat {
  public:
    DryAgeMeat(void) { }
    PietteTech_DHT* DHT;
    Adafruit_NeoPixel* strip;
    unsigned int DHTnextSampleTime;	    // Next time we want to start sample
    bool bDHTstarted;		                // flag to indicate we started acquisition
    int n;                              // counter

  public:
    void setup() {
      Serial.begin(9600);

      DHT = new PietteTech_DHT(DHTPIN, DHTTYPE);
      strip = new Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, WS2812B);
      strip->begin();
      strip->setBrightness(BRIGHTNESS);
      strip->show();
      DHTnextSampleTime = 0;  // Start the first sample immediately
    }

    void loop() {
    }

    void turnOn() {
      for (int i = 0; i < NUM_LEDS; i++) {
        strip->setColor(i, 255, 255, 255);
      }
      strip->show();
    }

    void turnOff() {
      strip->clear();
    }

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

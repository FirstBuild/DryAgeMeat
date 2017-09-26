#ifndef DRYAGEMEAT_H
#define DRYAGEMEAT_H

#include "PietteTech_DHT.h"

#define DRYAGEMEAT_VERSION_MAJOR 0
#define DRYAGEMEAT_VERSION_MINOR 0
#define DRYAGEMEAT_VERSION_PATCH 2

// system defines
#define DHTTYPE  DHT11              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A0           	    // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   2000  // Sample every two seconds

class DryAgeMeat {
  public:
    DryAgeMeat(void) { }
    PietteTech_DHT* DHT;
    unsigned int DHTnextSampleTime;	    // Next time we want to start sample
    bool bDHTstarted;		    // flag to indicate we started acquisition
    int n;                              // counter

  public:
    void setup() {
      Serial.begin(9600);

      DHT = new PietteTech_DHT(DHTPIN, DHTTYPE);
      DHTnextSampleTime = 0;  // Start the first sample immediately
    }
    void loop() {
      humiditySampler();
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

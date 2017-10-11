#ifndef DRYAGEMEAT_H
#define DRYAGEMEAT_H

#include "PietteTech_DHT.h"
#include "neopixel.h"
#include "RBD_Timer.h"
#include "RBD_Button.h"

#define DRYAGEMEAT_VERSION_MAJOR 0
#define DRYAGEMEAT_VERSION_MINOR 0
#define DRYAGEMEAT_VERSION_PATCH 6

// How many leds in your strip?
#define NUM_LEDS 19
#define DATA_PIN D7
#define BRIGHTNESS 155  /// 60% brightness -- Max value is 255

#define COMPRESSOR_RELAY D6   /// This is Arduino pin D4
#define FAN_RELAY A5          /// This is Arduino pin D8
#define DOOR_SWITCH A0        /// This is Arduino pin D3

#define S0 A4
#define S1 5
#define S2 2

#define DEBUG

// system defines
#define DHTTYPE  DHT11              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   A1           	    // Digital pin for communications

const int selectPins[3] = {S0, S1, S2}; //
const int targetOffset = 3;

struct EnvironmentData {
  long timeStamp;
  float humidityInPercent;
  float dewPoint;
  float averageAmbient;
  float ambientTemp_DHT11;
  float ambientTemp_scaleOne;
  float ambientTemp_scaleTwo;
  float targetTemp = 35;
};

struct MeatData {
  long timeStamp;
  float meatTempInFahren;
  String units;
  float meatWeight;
};

class DryAgeMeat {
public:
  DryAgeMeat(void) { }
  MeatData meatChunk;
  EnvironmentData environState;

private:
  RBD::Timer* refreshTimer;
  RBD::Timer* dataTransmitTimer;
  PietteTech_DHT* DHT;
  RBD::Button* doorButton;
  Adafruit_NeoPixel* strip;
  bool bDHTstarted = false;		                // flag to indicate we started acquisition

public:
  void setup() {
    Serial.begin(9600);
    Serial1.begin(57600);

    setupMux();

    pinMode(COMPRESSOR_RELAY, OUTPUT);
    pinMode(FAN_RELAY, OUTPUT);

    DHT = new PietteTech_DHT(DHTPIN, DHTTYPE);
    strip = new Adafruit_NeoPixel(NUM_LEDS, DATA_PIN, WS2812B);
    refreshTimer = new RBD::Timer(1000);
    dataTransmitTimer = new RBD::Timer(60000);
    doorButton = new RBD::Button(DOOR_SWITCH);

    strip->begin();
    strip->setBrightness(BRIGHTNESS);
    strip->clear();
    strip->show();
    Serial.println("BEGIN");
  }

  void loop() {
    publishMeatData();
    handleCompressor();
    handleFan();
    handleLights();
    refreshData();
  }

private:
  void updateAmbientTemperatureWithFilter() {
    float newAverageAmbientReading = (environState.ambientTemp_scaleOne + environState.ambientTemp_DHT11) / 2;
    if (environState.averageAmbient == 0) {
      environState.averageAmbient = newAverageAmbientReading;
    }  else {
      environState.averageAmbient = (environState.averageAmbient * .9) + (newAverageAmbientReading * .1);
    }
  }

  //  Handle compressor
  void handleCompressor() {
    updateAmbientTemperatureWithFilter();
    if (environState.averageAmbient > (environState.targetTemp + targetOffset)) {
      turnCompressorOn();
      #ifdef DEBUG

      Serial.println("COMPRESSOR -- ON");
      #endif
    } else if (environState.averageAmbient < (environState.targetTemp - targetOffset)) {
      turnCompressorOff();

      #ifdef DEBUG
      Serial.println("COMPRESSOR -- OFF");
      #endif
    }
  }

  //  Handle fan
  void handleFan() {
    if (doorButton->onReleased()) {
      turnFanOff();

      #ifdef DEBUG
      Serial.println("FAN -- OFF");
      #endif
    } else if (doorButton->onPressed()) {
      turnFanOn();

      #ifdef DEBUG
      Serial.println("FAN -- ON");
      #endif
    }
  }

  //  Handle light behavior
  void handleLights() {
    if (doorButton->onReleased()) {
      turnLightsOn();

      #ifdef DEBUG
      Serial.println("LIGHTS -- ON");
      #endif
    } else if (doorButton->onPressed()) {
      turnLightsOff();

      #ifdef DEBUG
      Serial.println("LIGHTS -- OFF");
      #endif
    }
  }

  //  Populate internal data structures with data
  void refreshData() {
    if (refreshTimer->onExpired()) {
      selectMuxPin(2);
      readProbeData(meatChunk);

      selectMuxPin(0);
      readScaleData(meatChunk);

      readHumidityData();
      refreshTimer->restart();
    }
  }

  //  Compressor Control
  void turnCompressorOn() {
      digitalWrite(COMPRESSOR_RELAY, HIGH);
  }

  void turnCompressorOff() {
      digitalWrite(COMPRESSOR_RELAY, LOW);
  }

  //  Fan Control
  void turnFanOn() {
      digitalWrite(FAN_RELAY, HIGH);
  }

  void turnFanOff() {
      digitalWrite(FAN_RELAY, LOW);
  }

  // Access to Meat Probe and Weight Data
  void setupMux() {
    for (int i = 0; i < 3; i++) {
      pinMode(selectPins[i], OUTPUT);
      digitalWrite(selectPins[i], HIGH);
    }
  }

  void selectMuxPin(byte pin) {
    if (pin > 7) return; // Exit if pin is out of scope
    for (int i=0; i<3; i++)
    {
      if (pin & (1<<i))
      digitalWrite(selectPins[i], HIGH);
      else
      digitalWrite(selectPins[i], LOW);
    }
  }

  void readProbeData(MeatData &newMeatData) {
    Serial1.readStringUntil('\n'); /// Remove Junk Data;
    float probeTempInFahren = Serial1.parseFloat();

    newMeatData.meatTempInFahren = probeTempInFahren;

    #ifdef DEBUG
    Serial.print("Probe Temp: ");
    Serial.println(newMeatData.meatTempInFahren);
    #endif
  }

  void readScaleData(MeatData &newMeatData) {
    Serial1.readStringUntil('\n'); /// Remove Junk Data;
    String scaleData = Serial1.readStringUntil('\n');

    newMeatData.timeStamp = commaToken(scaleData).toInt();
    newMeatData.meatWeight = commaToken(scaleData).toFloat();
    newMeatData.units = commaToken(scaleData);

    environState.timeStamp = newMeatData.timeStamp;
    environState.ambientTemp_scaleOne = (commaToken(scaleData).toFloat() * 1.8) + 32;

    #ifdef DEBUG
    Serial.print("Time: ");
    Serial.println(newMeatData.timeStamp);
    Serial.print("Weight: ");
    Serial.println(newMeatData.meatWeight);
    Serial.print("Units: ");
    Serial.println(newMeatData.units);
    Serial.print("Ambient Temp: ");
    Serial.println(environState.ambientTemp_scaleOne);
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

  //  Transmit data using webhook to AWS API Gateway Endpoint
  void publishMeatData() {
    if (dataTransmitTimer->onExpired()) {
      String transmitData = String::format("{\"hmdty_data\": \"%.2f\", \"mt_tmp_data\": \"%.2f\", \"mt_wt_data\": \"%.2f\", \"amb_tmp_data\": \"%.2f\"}",
        environState.humidityInPercent, meatChunk.meatTempInFahren, meatChunk.meatWeight, environState.averageAmbient);
      Particle.publish("dryage-tx-data", transmitData.c_str(), PRIVATE, WITH_ACK);
      dataTransmitTimer->restart();
    }
  }

  // LED Control
  void turnLightsOn() {
    for (int i = 0; i < NUM_LEDS; i++) {
      strip->setColor(i, 255, 255, 255);
    }
    strip->show();

    #ifdef DEBUG
    Serial.println("LIGHTS - ON");
    #endif
  }

  void turnLightsOff() {
    strip->clear();
    strip->show();

    #ifdef DEBUG
    Serial.println("LIGHTS - OFF");
    #endif
  }

  // Humidity Control
  void readHumidityData() {
    // Check if we need to start the next sample
    if (!bDHTstarted) {		// start the sample
      Serial.print("\nRetrieving information from sensor\n");
      DHT->acquire();
      bDHTstarted = true;
    } else {
      //  The following is to retry DHT->acquire if some timing error results.
      static unsigned long humidityTimeoutTracker = millis();
      if ((millis() - humidityTimeoutTracker) > 3000) {
        bDHTstarted = false;
        humidityTimeoutTracker = millis();
      }
    }

    if (!DHT->acquiring() && isDHTReady(DHT->getStatus())) {		// has sample completed?

      environState.humidityInPercent = DHT->getHumidity();
      environState.dewPoint = (DHT->getDewPoint() * 1.8) + 32;
      environState.ambientTemp_DHT11 = DHT->getFahrenheit();

      #ifdef DEBUG
      Serial.print("Humidity (%): ");
      Serial.println(environState.humidityInPercent);
      Serial.print("Temperature (oF): ");
      Serial.println(environState.ambientTemp_DHT11);
      Serial.print("Dew Point (oF): ");
      Serial.println(environState.dewPoint);
      #endif

      bDHTstarted = false;  // reset the sample flag so we can take another
    }
  }

  bool isDHTReady(int statusCode) {
      switch (statusCode) {
        case DHTLIB_OK:
        #ifdef DEBUG
          Serial.println("OK");
          return true;
        #endif
          break;

        case DHTLIB_ERROR_CHECKSUM:
        #ifdef DEBUG
          Serial.println("Error\n\r\tChecksum error");
        #endif
          break;

        case DHTLIB_ERROR_ISR_TIMEOUT:
        #ifdef DEBUG
          Serial.println("Error\n\r\tISR time out error");
        #endif
          break;

        case DHTLIB_ERROR_RESPONSE_TIMEOUT:
        #ifdef DEBUG
          Serial.println("Error\n\r\tResponse time out error");
        #endif
          break;

        case DHTLIB_ERROR_DATA_TIMEOUT:
        #ifdef DEBUG
          Serial.println("Error\n\r\tData time out error");
        #endif
          break;

        case DHTLIB_ERROR_ACQUIRING:
        #ifdef DEBUG
          Serial.println("Error\n\r\tAcquiring");
        #endif
          break;

        case DHTLIB_ERROR_DELTA:
        #ifdef DEBUG
          Serial.println("Error\n\r\tDelta time to small");
        #endif
          break;

        case DHTLIB_ERROR_NOTSTARTED:
        #ifdef DEBUG
          Serial.println("Error\n\r\tNot started");
        #endif
          break;

        default:
        #ifdef DEBUG
          Serial.println("Unknown error");
        #endif
          break;
      }
      return false;
  }
};

#endif /* DRYAGEMEAT_H */

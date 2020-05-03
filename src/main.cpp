#include <Arduino.h>
#include "ArduinoJson.h"
#include <SPI.h>
#include <Adafruit_ATParser.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <Adafruit_BLEMIDI.h>
#include <Adafruit_BLEBattery.h>
#include <Adafruit_BLEGatt.h>
#include <Adafruit_BLEEddystone.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_UART.h>
#include "../include/BluefruitConfig.h"
#include "../include/Music.h"
#include "../include/RideTracking.h"
#include "../include/Navigation.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
Music music("Track","Artist", "01:00", "00:00", false,0);
RideTracking ride("","","","","", false);
Navigation nav("","","","","", false);
char * time = "12:00";

void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  ble.verbose(false);  // debug info is a little annoying after this point!
  Serial.print("Connecting");
  while (! ble.isConnected()) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

}

void loop() {

  char n, inputs[BUFSIZE+1];

  if (Serial.available())
  {
    n = Serial.readBytes(inputs, BUFSIZE);
    inputs[n] = 0;
    // Send characters to Bluefruit
    Serial.print("Sending: ");
    Serial.println(inputs);

    // Send input data to host via Bluefruit
    ble.print(inputs);
  }
  // Echo received data
  char json[256];
  int brackets = 0, count = 0;
  while ( ble.available() )
  {
    int c = ble.read();
    Serial.print((char)c);
    if((char)c == '}'){

      brackets--;
      count++;

      json[count] = (char)c;

      if(brackets == 0) {

        // Capacity calc for the JsonDoc
        StaticJsonDocument<256> doc;

        // Deserialize
        DeserializationError error = deserializeJson(doc, json);

        // Error check
        if(error) {
          Serial.print("Error: ");
          Serial.println(error.c_str());
        }

        if(doc["music"]) {
          music.setTrack(doc["music"]["track"]);
          music.setArtist(doc["music"]["artist"]);
          music.setLength(doc["music"]["track_length"]);
          music.setPlaying(doc["music"]["playing"]);
          music.setPosition(doc["music"]["position"]);
          music.setProgressBar(doc["music"]["progressBar"]);
        } 

        if(doc["navigation"]["next_direction"]) {
          nav.setNextDir(doc["navigation"]["next_direction"]);
          nav.setNextRoad(doc["navigation"]["next_road"]);
          nav.setNextDistance(doc["navigation"]["next_step_distance"]);
          nav.setDistance(doc["navigation"]["distance_remaining"]);
          nav.setTime(doc["navigation"]["time_remaining"]);
          nav.setRunning(true);
        }

        if(doc["navigation"]["running"]) {
          nav.setRunning(false);
        }

        if(doc["ride_tracking"]["distance"]) {
          ride.setDistance(doc["ride_tracking"]["distance"]);
          ride.setTime(doc["ride_tracking"]["time_ride"]);
          ride.setSpeed(doc["ride_tracking"]["speed"]);
          ride.setAvg(doc["ride_tracking"]["avg_speed"]);
          ride.setGain(doc["ride_tracking"]["gain"]);
          ride.setRunning(true);
        }

        if(doc["ride_tracking"]["running"]) {
          ride.setRunning(false);
        }

        if(doc["time"]) {
          time = doc["time"];
        }
        
      }

    } else if((char)c == '{') {

      brackets++;
      count++;
      json[count] = (char)c;

    } else {

      count++;
      json[count] = (char)c;

    }
    
  }

}
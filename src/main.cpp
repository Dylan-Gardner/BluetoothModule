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

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

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
  char * track, * artist, * next_step_instruction, * curr_step_name;
  float next_step_distance, distance_remaining, time_remaining, distance, speed_ride, avg_speed, change_alt, speed_run;
  int track_length, position, time_ride;
  bool playing, running_nav, running_ride;

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
          track = doc["music"]["track"];
          artist = doc["music"]["artist"];
          track_length = doc["music"]["track_length"];
          playing = doc["music"]["playing"];
          position = doc["music"]["position"];
        } 

        if(doc["navigation"]["next_step_instruction"]) {
          next_step_instruction = doc["navigation"]["next_step_instruction"];
          next_step_distance = doc["navigation"]["next_step_distance"];
          curr_step_name = doc["navigation"]["curr_step_name"];
          distance_remaining = doc["navigation"]["distance_remaining"];
          time_remaining = doc["navigation"]["time_remaining"];
        }

        if(doc["navigation"]["running"]) {
          running_nav = doc["navigation"]["running"];
        }

        if(doc["ride_tracking"]["distance"]) {
          distance = doc["ride_tracking"]["distance"];
          time_ride = doc["ride_tracking"]["time_ride"];
          speed_ride = doc["ride_tracking"]["speed"];
          avg_speed = doc["ride_tracking"]["avg_speed"];
          change_alt = doc["ride_tracking"]["change_alt"];
        }

        if(doc["ride_tracking"]["running"]) {
          running_ride = doc["ride_tracking"]["running"];
          speed_run = doc["ride_tracking"]["speed"];
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
#include <Arduino.h>
/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2018 Sensnology AB
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Motion Sensor example using HC-SR501
 * http://www.mysensors.org/build/motion
 *
 */

// Enable debug prints
#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95

#define MY_PARENT_NODE_ID 0
#define MY_NODE_ID 21

#include <MySensors.h>

// important settings
#define SLEEP_TIME 2 // Sleep time between reports (in minutes)
#define WINDOW_SENSOR 3   // The digital input you attached your motion sensor.  (Only 2 and 3 generates interrupt!)
#define STATUS_LED 4
#define CHILD_ID_WINDOW 1   // Id of the sensor child
#define CHILD_ID_VOLTAGE 2   // Id of the sensor child
#define BATTERY_SENSE_PIN 16 // A0
#define VOLTS_PER_BIT 0.003363075 // if voltage divider with 1M - 470K
#define MAX_VOLTAGE 3 // max battery voltage (100%)
#define MIN_VOLTAGE 1.8 // min voltage possible (0%)

// calculate some variables from settings
const float difference = MAX_VOLTAGE - MIN_VOLTAGE;
const uint32_t sleep_time = SLEEP_TIME * 60 * 1000;

// initialize some variables
bool window_success, voltage_success, percent_success;

// Initialize window message
MyMessage msg_window(CHILD_ID_WINDOW, V_TRIPPED);
MyMessage msg_voltage(CHILD_ID_VOLTAGE, V_VOLTAGE);

void setup()
{
   // use the 1.1 V internal reference
  #if defined(__AVR_ATmega2560__)
      analogReference(INTERNAL1V1);
  #else
      analogReference(INTERNAL);
  #endif
	
  // pinMode declaration
  pinMode(WINDOW_SENSOR, INPUT);
  pinMode(STATUS_LED, OUTPUT);

  // trigger status LED after connecting
  digitalWrite(STATUS_LED, HIGH);
  sleep(500);
  digitalWrite(STATUS_LED, LOW);
}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("WindowSensor 1", "1.0");

	// Register all sensors to gw (they will be created as child devices)
	present(CHILD_ID_WINDOW, S_DOOR);
	present(CHILD_ID_VOLTAGE, S_MULTIMETER);
}

void loop()
{
  // maybe some sleep time to wake up properly
  // sleep(500);

	// get window state
	bool open = digitalRead(WINDOW_SENSOR) == HIGH;

  // get battery voltage
  float battery_voltage = analogRead(BATTERY_SENSE_PIN) * VOLTS_PER_BIT;

  // calculate battery percent
  uint8_t battery_percent = 100 * uint8_t((battery_percent - MIN_VOLTAGE) / difference);


	// DEBUG PRINT
  #ifdef MY_DEBUG
    Serial.print("Window state: ");
    Serial.println(open?"open":"closed");
    
    Serial.print("Battery Voltage: ");
    Serial.print(battery_voltage, 3);
    Serial.println(" V");

    Serial.print("Battery percent: ");
    Serial.print(battery_percent, 0);
    Serial.println(" %");
  #endif

  // SEND TO GATEWAY
	window_success = send(msg_window.set(open?"1":"0"));
  voltage_success = send(msg_voltage.set(&battery_voltage, sizeof(battery_voltage)));
  percent_success = sendBatteryLevel(battery_percent);

  // trigger status LED after sucessfully sending
  if (window_success && voltage_success && percent_success){
    digitalWrite(STATUS_LED, HIGH);
    sleep(100);
    digitalWrite(STATUS_LED, LOW);
  }
	
  // Sleep until interrupt from window sensor. Send update perdiodically
	sleep(digitalPinToInterrupt(WINDOW_SENSOR), CHANGE, SLEEP_TIME);
}
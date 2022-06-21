/*
 *  © 2022, Chuck Bade,  All rights reserved.
 *  Template originated from Neil McKechnie's code. All rights reserved.
 *  
 *  This file is part of DCC++EX API
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 *
 *  IO_MQTT by Chuck Bade 6/13/22
 *  This module connects to an MQTT server somewhere on the network, using 
 *  "DCC++EX." and the last part of the IP number, since each node connecting 
 *  to the MQTT broker needs a unique ID.  If the connection to the MQTT 
 *  broker is lost, the sketch will attempt to reconnect using a blocking 
 *  reconnect function.     
 *  Currently only works with ethernet.  Will develop for wifi when I receive 
 *  the wifi shield.    
 *  Has not been tested with withrottle.
 *  When the command station starts up, the "begin" function will subscribe 
 *  to a list of sensors, so if one of those sensors change, its status will 
 *  be sent to the callback function, where it will be stored in an array 
 *  where it can be accessed by the read function. 
 *  
 *  The write function publishes turnout values to the MQTT broker. 
 *  
 *  If a sensor is sent to MQTT, it will be published with the message 
 *  "ACTIVE" for a one or "INACTIVE" for 0, preceded by topic 
 *  "/trains/track/sensor/###", where ### is the sensor (Vpin) number. 
 */

#include <Arduino.h>
#include "IO_MQTT.h"
#include "MiniBitSet.h"

EthernetClient ethClient = EthernetServer(1883).available();
myPubSubClient mqttClient(ethClient);
MiniBitSet sensorData;
uint16_t firstSensor;

// Constructor
IO_MQTT::IO_MQTT(VPIN firstVpin, int nPins, int nTurnouts, int nSensors) {
  _firstVpin = firstVpin;
  _nPins = nPins;
  _nTurnouts = nTurnouts;
  _nSensors = nSensors;
  firstSensor = firstVpin + nTurnouts;
  addDevice(this);
}


// Static create method for one module.
void IO_MQTT::create(VPIN firstVpin, int nPins, int nTurnouts, int nSensors) {
  new IO_MQTT(firstVpin, nPins, nTurnouts, nSensors);
  sensorData.create(nSensors);  // create bit array
}


// Device-specific initialisation
void IO_MQTT::_begin() {

#if defined(DIAG_IO)
  _display();
#endif
  mqttClient.setServer(MQTTIP, 1883);
  mqttClient.setCallback(_callback);

  _reconnect();
  
  // publish an empty output message to clear any retained messages
  //mqttClient.publish(topic, "", true);   // doesn't work, causes 
                       // subscriptions to not be sent

}


// Device-specific write function.  Write a string in the form "#Wm,n#"
//  where m is the vpin number, and n is the value.
void IO_MQTT::_write(VPIN vpin, int value) {
  char topic[30];
  #ifdef DIAG_IO
  DIAG(F("IO_MQTT::_write Pin:%d Value:%d"), (int)vpin, value);
  #endif

  sprintf(topic, "%s%d", TURNOUTTOPIC, (int)vpin); 
  
  if (value == 1)
    _publish(topic, "THROWN");
  else if (value == 0)
    _publish(topic, "CLOSED");
}


// Device-specific read function.
int IO_MQTT::_read(VPIN vpin) {
  uint16_t bitIndex = vpin - firstSensor;
//  DIAG(F("IO_MQTT::_read vpin=%d bitIndex=%d"), (int)vpin, (int)bitIndex);
  if ((bitIndex < 0) || (bitIndex > _nSensors))
    return(0);
  uint8_t value = sensorData.get(bitIndex);
  DIAG(F("IO_MQTT::_read vpin=%d bitIndex=%d value=%d" )
    , (int)vpin, (int)bitIndex, int(value));
     
  return((int)value);
}


// Loop function to do background scanning of the input port.  State 
//  machine parses the incoming command as it is received.  Command
//  is in the form "#Nm,n#" where m is the index and n is the value.
void IO_MQTT::_loop(unsigned long currentMicros) {
  (void)currentMicros;  // Suppress compiler warnings

  // confirm still connected to mqtt server
  if (!mqttClient.connected())
    _reconnect();
  
  mqttClient.loop();
}


void IO_MQTT::_display() {
  DIAG(F("IO_MQTT::_display Configured on VPins:%d-%d"), (int)_firstVpin, 
    (int)_firstVpin+_nPins-1);
}


void IO_MQTT::_publish(char* topic, char* payload) {
  if (mqttClient.connected()) {
    DIAG(F("IO_MQTT::publish Publish topic=%s payload=%s"), topic, payload);
    mqttClient.publish(topic, payload, true);
  }
}


static void IO_MQTT::_callback(char* topic, byte* payload, unsigned int length) {
  /*
   * When a message is received that has been subscribed to, this function 
   * will be called.  
   */
  uint16_t inID;
  char* id;
  char* msg;
  // terminate the byte ar
  payload[length] = 0;

  DIAG(F("IO_MQTT::callback Message arrived, topic=%s payload=%s")
    , topic, payload);

  // copy the byte string to a char string
  msg = (char *) payload;

  // parse the first word from the incoming topic
  strtok(topic, "/");

  // parse until there are none left, outID will hold the ID number
  while (id = strtok(NULL, "/"))
    inID = atoi(id);

  // set the appropriate output

  DIAG(F("IO_MQTT::callback Sensor/Turnout number=%d %s"), (int)inID, payload);

  if (!strcmp(payload, "ACTIVE"))
    sensorData.set(inID - firstSensor);
  if (!strcmp(payload, "INACTIVE"))
    sensorData.clear(inID - firstSensor);
  
  DIAG(F("IO_MQTT::callback Saved bit value=%d for sensor %d")
    , sensorData.get(inID - firstSensor), inID);
}



long lastReconnectAttempt = 0;

void IO_MQTT::_reconnect() {
    char topic[30];

  // check that a connection isnt already in progress
  if (mqttClient.state() != MQTT_CONNECT_INPROGRESS) {
    long now = millis();
    if (now - lastReconnectAttempt < 5000)
      return;

    IPAddress ip = Ethernet.localIP(); // get the IP address
    char clientId[12];
    sprintf(clientId, "DCC++EX.%d", ip[3]);

    DIAG(F("IO_MQTT::connectMqtt Attempting MQTT connection with name=%s")
      , clientId);

    // Attempt to connect
    lastReconnectAttempt = now;
    mqttClient.beginConnect(clientId);
  }

  // check the connect status
  int ret = mqttClient.connectStatus();
  switch (ret) {
    case MQTT_CONNECTED:
      DIAG(F("IO_MQTT::connectMqtt connected to MQTT broker at %s"), MQTTIP);
      // DO WE NEED TO RESUBSCRIBE?
      lastReconnectAttempt = 0;
      DIAG(F("IO_MQTT::_begin Subscribe for %d sensors with topic %s")
        , _nSensors, SENSORTOPIC);

      for (int i = 0; i < _nSensors; i++) {    
        sprintf(topic, "%s%d", SENSORTOPIC, firstSensor + i); 
        mqttClient.subscribe(topic);
        DIAG(F("IO_MQTT::_begin subscribe, topic=%s"), topic);
      }

      break;
    case MQTT_CONNECT_INPROGRESS:
      return;
    default:
      Serial.print("failed! rc = "); Serial.print(ret);
      Serial.println(". Trying again in 5 seconds.");
      break;
  }
}


/*  
 *   
  MiniBitSet(<number of bits in array>)  // create array
  ~MiniBitSet() // delete array
  bool get(<bit number>)   // get a single bit
  void set(<bit number>)   // set a single bit to 1
  void clear(<bit number>) // set a single bit to 0
*/

/*
 *  © 2021, Neil McKechnie. All rights reserved.
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
 */

/*
 * To declare a device instance, 
 *    IO_MQTT myDevice(5000, 1000, 5100, 15);
 * or to create programmatically,
 *    IO_MQTT::create(5000, 300, 100, 10);
 * 
 * 
 * See IO_MQTT.cpp for the protocol used the network to a MQTT broker.
 * 
 */

#ifndef IO_MQTT_H
#define IO_MQTT_H

#include "IODevice.h"
#include <Ethernet.h>
#include "myPubSubClient.h"

class IO_MQTT : public IODevice {
public:
  IO_MQTT(VPIN firstVpin, int nPins, int nTurnouts, int nSensors);
  static void create(VPIN firstVpin, int nPins, int nTurnouts, int nSensors);  

protected:
  void _begin() override;
  void _loop(unsigned long currentMicros) override;
  void _write(VPIN vpin, int value) override;
  int  _read(VPIN vpin) override;
  void _display() override;
    
private:
  void _publish(char* topic, char* payload);
  static void _callback(char* topic, byte* payload, unsigned int length);
  void _reconnect();

  uint16_t _nTurnouts;
  uint16_t _nSensors;
};

#endif // IO_MQTT_H

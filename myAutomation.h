/* This is an automation example file.
 *  The presence of a file called "myAutomation.h" brings EX-RAIL code into
 *  the command station.
 *  The auotomation may have multiple concurrent tasks. 
 *  A task may 
 *  - Act as a ROUTE setup macro for a user to drive over 
 *  - drive a loco through an AUTOMATION 
 *  - automate some cosmetic part of the layout without any loco.
 *  
 *  
 *  Where the loco id is not known at compile time, a new task 
 *  can be creatd with the command:
 *  </ START [cab] route> 
 *  
 *  A ROUTE, AUTOMATION or SEQUENCE are internally identical in ExRail terms  
 *  but are just represented differently to a Withrottle user:
 *  ROUTE(n,"name") - as Route_n .. to setup a route through a layout
 *  AUTOMATION(n,"name") as Auto_n .. to send the current loco off along an automated journey
 *  SEQUENCE(n) is not visible to Withrottle.
 *  
 */
#include "IO_MQTT.h"
  //=======================================================================
  // The following directive defines an MQTT broker interface.
  //=======================================================================
  // The parameters are: 
  //   First Vpin=1000
  //   Total number of VPINs=300 (numbered 1000-1299)
  //   Number of outputs (turnouts) beginning at 1000
  //   Number of subscribed inputs (Sensors) beginning at 1100 
  HAL(IO_MQTT, 1000, 300, 100, 4)
  
  POWERON
  SIGNAL(1002, 1001, 1000)
  SIGNAL(1006, 1005, 1004)
  SIGNAL(1010, 1009, 1008)
  ALIAS(MAIN, 11)
  ALIAS(LEFT, 22)
  ALIAS(RIGHT, 33)
  ALIAS(MAINSIGNAL, 1002)
  ALIAS(LEFTSIGNAL, 1006)
  ALIAS(RIGHTSIGNAL, 1010)
  ALIAS(TURNOUT, 1012)
  ALIAS(MAINOCCUPIED, 1100)
  ALIAS(LEFTOCCUPIED, 1101)
  ALIAS(RIGHTOCCUPIED, 1102)
  SENDLOCO(7,1) // send loco 7 off along route 1
  DONE     // This just ends the startup thread, leaving 2 others running.

  SEQUENCE(1)    // initialize
    REV(40)      // go back to starting block
    AT(MAINOCCUPIED)     // wait until starting block is occupied
    STOP
    DELAY(1000)
    RESET(TURNOUT)  // close the turnout
  
  SEQUENCE(2)
    CALL(RIGHT)
    CALL(MAIN)
    CALL(LEFT)
    CALL(MAIN)
    FOLLOW(2)
     
  SEQUENCE(MAIN)  
    GREEN(MAINSIGNAL)
    DELAY(500)
    REV(35)
    AT(MAINOCCUPIED)
    DELAY(2800)
    AMBER(MAINSIGNAL)
    REV(25)
    DELAY(1500)
    RED(MAINSIGNAL)
    STOP
    DELAY(5000)
    RETURN

  SEQUENCE(LEFT)
    SET(TURNOUT)
    DELAY(500)
    GREEN(LEFTSIGNAL)
    DELAY(500)
    FON(0)
    FWD(35)
    AT(RIGHTOCCUPIED)     // CHANGE TO LEFT WHEN TURNOUT FIXED wait until left block occupied
    DELAY(800)  // go a litte farther under green
    AMBER(LEFTSIGNAL)
    FWD(25)
    DELAY(2000)
    RED(LEFTSIGNAL)
    STOP
    DELAY(5000)
    RETURN
     
  SEQUENCE(RIGHT)
    RESET(TURNOUT)
    DELAY(500)
    GREEN(RIGHTSIGNAL)
    DELAY(500)
    FON(0)
    FWD(35)
    AT(RIGHTOCCUPIED)     // wait until main block occupied
    DELAY(800)  // go a litte farther under green
    AMBER(RIGHTSIGNAL)
    FWD(25)
    DELAY(2000)
    RED(RIGHTSIGNAL)
    STOP
    DELAY(5000)
    RETURN

    
   

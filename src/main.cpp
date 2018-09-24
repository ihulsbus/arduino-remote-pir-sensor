#include <Arduino.h>
#include "RH_ASK.h"

#define NODO_PULSE_0                 500  // PWM: Tijdsduur van de puls bij verzenden van een '0' in uSec.
#define NODO_PULSE_MID              1000  // PWM: Pulsen langer zijn '1'
#define NODO_PULSE_1                1500  // PWM: Tijdsduur van de puls bij verzenden van een '1' in uSec. (3x NODO_PULSE_0)
#define NODO_SPACE                   500  // PWM: Tijdsduur van de space tussen de bitspuls bij verzenden van een '1' in uSec.
#define PIN_RF_TX_DATA                12
#define TRANSMITTER_STABLE_TIME        5
#define NODO_VERSION_MINOR            10  // Ophogen bij gewijzigde settings struct of nummering events/commando's.
#define NODO_VERSION_MAJOR             3  // Ophogen bij DataBlock en NodoEventStruct wijzigingen.
#define HOME_NODO                      1  // Home adres van Nodo's die tot Ã©Ã©n groep behoren (1..7). Heeft je buurman ook een Nodo, kies hier dan een ander Home adres
#define NODO_TYPE_EVENT                1
#define NODO_TYPE_COMMAND              2
#define VALUE_SOURCE_PLUGIN           39
#define CMD_VARIABLE_SET               5
#define RAW_BUFFER_SIZE              256  // Maximaal aantal te ontvangen 128 bits is voldoende voor capture meeste signalen.

// initialise different parts of the application
RH_ASK rf_driver;
int dataPin = 9; // Input for HC-S501
int dataValue; // Place to store read PIR Value
void Convert_2_RawSignal(struct DataBlockStruct DataBlock);
void RawSendRF(void);
void Convert_and_send_measurement(struct DataBlockStruct Measurement);

// Variabelen geplaatst in struct zodat deze later eenvoudig kunnen worden weggeschreven naar SDCard
struct RawSignalStruct{
  byte Source;                                                                  // Bron waar het signaal op is binnengekomen.
  int  Number;                                                                  // aantal bits, maal twee omdat iedere bit een mark en een space heeft.
  byte Repeats;                                                                 // Aantal maal dat de pulsreeks verzonden moet worden bij een zendactie.
  byte Delay;                                                                   // Pauze in ms. na verzenden van Ã©Ã©n enkele pulsenreeks
  byte Multiply;                                                                // Pulses[] * Multiply is de echte tijd van een puls in microseconden
  boolean RepeatChecksum;                                                       // Als deze vlag staat moet er eentweede signaal als checksum binnenkomen om een geldig event te zijn.
  unsigned long Time;                                                           // Tijdstempel wanneer signaal is binnengekomen (millis())
  byte Pulses[RAW_BUFFER_SIZE + 2];                                             // Tabel met de gemeten pulsen in microseconden gedeeld door RawSignal.Multiply. Dit scheelt helft aan RAM geheugen.
  // Om legacy redenen zit de eerste puls in element 1. Element 0 wordt dus niet gebruikt.
} RawSignal = {0, 0, 0, 0, 0, 0, 0L};

struct DataBlockStruct{
  byte Version = 10;        //  Version of nodo SW. Not important. (See nodo website for more options, NODO_VERSION_MINOR)
  byte SourceUnit = 7;      //  This value should be 1..9 but not 3 if you send data to domoticz. (See RF Link website for more options, plugin 90, or nodo website, UNIT_NODO)
  //                        //  ID in DOmoticz is build up of sourceunit value and par1 value.
  byte DestinationUnit = 0; //  Unitnummer waar de events naar toe gestuurd worden. 0=alle.
  byte Flags = 0;           //  Not used for this type of command.
  byte Type = 1;            //  This value should be 1 if you send data to domoticz. (See nodo website for more options, NODO_TYPE_EVENT)
  byte Command = 4;         //  This value should be 4 if you send data to domoticz. (See nodo website for more options, EVENT_VARIABLE)
  byte Par1 = 5;
  //                            Type of measurement
  //                             5 : Temperature
  //                             6 : Humidity           0 - 100
  //                             7 : Rain fall
  //                             8 : Wind speed
  //                             9 : Wind direction     0 - 15
  //                            10 : Wind gust
  //                            11, 12 en 13 : Temperature
  //                            14 : Humidity           0 - 100
  //                            15 : UV meter           0 - 1024
  //                            16 : Barometric pressure
  unsigned long Par2;       // Measurement data
  //
  byte Checksum;            // Checksum
};

///////////////////////////////////
//
//  This routine converts floats to a long int.
//
///////////////////////////////////
unsigned long float2ul(float f){
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(dataPin, INPUT);

  digitalWrite(ledPin, LOW);



}

void loop() {
  dataValue = digitalRead(dataPin);
  Serial.print("datapin: ");
  Serial.println(dataValue);
  digitalWrite(ledPin, dataValue);
}

/*

ATTiny85 UART Device.

Description:
UART attiny85 slave device. 
When the Tiny receiveds an 'f' character via UART, the connected stepper will move one step forward or
when the Tiny receives an 'r' character via UART, the connected stepper will move one step backwards.

Notes:
If too much data is sent from the Tiny, to the PC, the Tiny wont be able to receive data sent to it from the PC.
I suspect that the receive and transmit of the software UART for the Tiny share the same data buffer.
At the moment, the delay between each step is set to 3000 microseconds (3 milliseconds).

Settings used:
 - Baudrate: 9600,
 - Arduino IDE: ATTiny85 @ 8MHz (internal oscillator; BOD disabled).

Wiring:
RX hooked up to IC Pin 5.
TX hooked up to IC Pin 6.
Steppin1 & 2 are onnected to pin 4 of two HXJ8002 ics. 
The HXJ8002's are set up so they only need one pin (pin 4) to go high or low, to change the state of both the IC's output pins.

Features to add:
 1. Make sure the TRANSMIT pin is working.
 2. Complete instruction packets with CRC 16, which give the user the ability to: step forward, step backward, set the waittime between steps,
    retreive the waittimestored on the Tiny.
 3. Store the waittime in EEPROM, when the waittime is received via UART.
 4. Retreive the waittime from EEPROM, when the Tiny starts up.
 */

#include <SoftwareSerial.h>

// IC PIN = 5.
const byte rxpin = 0;
// IC PIN = 6.
const byte txpin = 1;

//  Output pins.
// IC PIN 2.
const byte steppin1 = 3;
//  IC PIN 3.
const byte steppin2 = 4;

// Keeps track of where (Throughout the 4, 90 degree stages of a step) the stepper is.
byte stepstage = 1;

// Holds the byte received over UART.
char data;

// The time (In millis) since the last UART data was sent.
unsigned long lastTransmit = 0;
// Only send UART data every this many milliseconds.
// Limiting the transmit interval, prevents the input/output UART buffer from filling up
// and freezing the ATTiny85's ability to receive data.
unsigned long intervalTransmit = 775;

SoftwareSerial TinySerial(rxpin, txpin); // RX, TX

void setup()
{
  // Make the Tiny 8MHz.
  CLKPR = 0x80;
  CLKPR = 0x00; 
  // Have not tried without these lines yet. Without these lines, the softwareserial might not work.
  pinMode(rxpin,INPUT);
  pinMode(txpin,OUTPUT);
  // Make both pins going to the steppers output pins.
  pinMode(steppin1, OUTPUT); 
  pinMode(steppin2, OUTPUT); 
  // Open serial communications and let us know we are connected.
  TinySerial.begin(9600);
  TinySerial.println("BEGIN");
}

void loop()
{
  // If there's a byte available.
  if(TinySerial.available() > 0){
    // Read it.
    data = TinySerial.read();
    // Step forward if the byte/char is 'f', or
    // step backward if the byte/char is 'r'.
    // The last parameter for the step functions, is the delay in microseconds between issueing a step instruction
    // and issueing the next step instruction.
    if(data == 'f'){
      stepForward(steppin1,steppin2,3000);}
    if(data == 'r'){
      stepBackward(steppin1,steppin2,3000);}
  }
  // The minimum wait time transmitInterval is somewhere between 775 and 550.
  checkSend(data,transmitInterval);
}

// If the minimum interval in milliseconds has passed since the last time
// the ATTiny85 send the data over UART, then send the new data.
void checkSend(int data, int interval){
  if((millis() - lastTransmit) > interval){
    TinySerial.println(data);
    lastTransmit = millis();
  }
}

void stepForward(byte ipin1, byte ipin2, int iwaittime){
  switch (stepstage){
    case 1:
      digitalWrite(ipin1, LOW);
      digitalWrite(ipin2, LOW);
      stepstage++;
      break;
    case 2:
      digitalWrite(ipin1, LOW);
      digitalWrite(ipin2, HIGH);
      stepstage++;
      break;
    case 3:
      digitalWrite(ipin1, HIGH);
      digitalWrite(ipin2, HIGH);
      stepstage++;
      break;
    case 4:
      digitalWrite(ipin1, HIGH);
      digitalWrite(ipin2, LOW);
      stepstage = 1;
      break;
  }
  delayMicroseconds(iwaittime);
}

void stepBackward(byte ipin1, byte ipin2, int iwaittime){
  switch (stepstage){
    case 4:
      digitalWrite(ipin1, LOW);
      digitalWrite(ipin2, HIGH);
      stepstage--;
      break;
    case 3:
      digitalWrite(ipin1, LOW);
      digitalWrite(ipin2, LOW);
      stepstage--;
      break;
    case 2:
      digitalWrite(ipin1, HIGH);
      digitalWrite(ipin2, LOW);
      stepstage--;
      break;
    case 1:
      digitalWrite(ipin1, HIGH);
      digitalWrite(ipin2, HIGH);
      stepstage = 4;
      break;
  }
  delayMicroseconds(iwaittime);
}



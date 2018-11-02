/*
	MicroView Current Monitor
	@author Mark Cocquio
	
	A simple sketch to monitor your ebike's current consumption and track it.
*/
#include <MicroView.h>
#include <Time.h>

uint16_t 	onDelay=5;		// this is the on delay in milliseconds, if there is no on delay, the erase will be too fast to clean up the screen.

int voltageSensorPin = A1; // not used unless you have a circuit to reduce battery voltage to a scaled 5v
int voltageSensorValue = 0;
int currentSensorPin = A0;
int currentSensorValue = 0;
float currentConsumed = 0;
const int numReadings = 25;     // number of readings to average over - sync this with INTERVAL below if you like using 1000/INTERVAL
float readings[numReadings];    // the readings from the analog input
int index = 0;                  // the index of the current reading
float total = 0;                // the running total
float average = 0;              // the average
float currentValue = 0;
float maxCurrentValue = 0;
unsigned long timeNow = 0;
unsigned long lastTime = 0;
unsigned long lastRotated = millis();
int currentDisplay = 1;

#define INTERVAL 40             // refresh rate in millis. 40 = 25hz update rate
#define ROTATION_INTERVAL 3000  // display rotation interval in millis

void setup() {
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readings[thisReading] = 0;
	uView.begin();		  // begin of MicroView
	uView.clear(ALL);	  // erase hardware memory inside the OLED controller
	uView.display();	  // display the content in the buffer memory, by default it is the MicroView logo
	delay(100);
	uView.clear(PAGE);  // erase the memory buffer, when next uView.display() is called, the OLED will be cleared.
}

// display string with float value
void formatPrintF( char *leftStr, float var1, char *rightStr) {
  String s = leftStr;
  s = s + var1;
  s = s + rightStr;
	uView.print(s);
}

// display string with int value
void formatPrintI( char *leftStr, int var1, char *rightStr) {
  String s = leftStr;
  s = s + var1;
  s = s + rightStr;
	uView.print(s);
}

double capacityCalc(float current) {
	// take the measurements and check the time since last called, then return the charge that has passed since last call in units of mAh
	return (current * (timeNow - lastTime) / 3600 );
}

float analogToVolts(int sensorValue) {
//  return 5.0 / 1024 * sensorValue;
  return 3.9 * 14.0; // let's assume a nominal voltage of 3.9v/cell * 14S - actual read would require 58 -> 5v conversion
}

float analogToAmps(int sensorValue) {
  // Data processing: 510-raw data from analogRead when the input is 0; 5-5v; the first 0.04-0.04V/A(sensitivity); the second 0.04-offset val;
//  return (sensorValue - 510) * 5.0 / 1024.0 / 0.04 - 1.92;
  float amps = (sensorValue - 526) * 5.0 / 1024.0 / 0.04;
  if (amps < 0.15 && amps > -0.15)
    return 0.0;
  else
    return amps;
}

void loop() {

	lastTime = timeNow;		// shift now back to lastTime
	timeNow = millis();		// keep track of when the measurements were made
	
  pinMode(voltageSensorPin, INPUT);
  pinMode(currentSensorPin, INPUT);
  voltageSensorValue = analogRead(voltageSensorPin);
  currentSensorValue = analogRead(currentSensorPin);

  total = total - readings[index];
  readings[index] = analogToAmps(currentSensorValue);
	currentConsumed += capacityCalc(readings[index]);
  total = total + readings[index];       
  index = index + 1;                    
  if (index >= numReadings)              
    index = 0;                           
  average = total/numReadings; // Smoothing algorithm (http://www.arduino.cc/en/Tutorial/Smoothing)    
  currentValue = average;
  if (currentValue > maxCurrentValue)
    maxCurrentValue = currentValue;

  if (currentDisplay == 1) {
    display1();
  }
  if (currentDisplay == 2) {
    display2();
  }
  if (currentDisplay == 3) {
    display3();
  }

  rotateDisplay();
	delay(INTERVAL);
}

void rotateDisplay() {
  unsigned long rightNow = millis();
  if ((rightNow - lastRotated) > ROTATION_INTERVAL) {
    currentDisplay++;
    lastRotated = rightNow;
  }
  if (currentDisplay > 3) currentDisplay = 1; // change to > 2 to disable raw sensor display screen
}

// current, max current, watts (estimated) and mAh
void display1() {
	uView.clear(PAGE);
	uView.setFontType(1);
	uView.setCursor(0,0);
	if (currentValue > 0 && currentValue < 1.0) {
  	formatPrintF("", currentValue, "A");
	} else {
  	formatPrintI("", currentValue, "A");
	}
	uView.setCursor(5*9,0);
//	formatPrintI("", analogToVolts(voltageSensorValue), "v");
	formatPrintI("", maxCurrentValue, "A");
	uView.setCursor(0,16);
//	formatPrintI("", currentSensorValue, "");
	formatPrintI("", analogToVolts(voltageSensorValue) * currentValue, "W");
	uView.setCursor(0,32);
	if (currentConsumed > 10000) {
  	formatPrintF("", currentConsumed / 1000, "Ah");
	} else {
  	formatPrintI("", currentConsumed, "mAh");
	}
	uView.display();
}

// current and mAh only
void display2() {
	uView.clear(PAGE);
	uView.setFontType(2);
	uView.setCursor(0,0);
	if (currentValue > 0 && currentValue < 1.0) {
  	formatPrintF("", currentValue, "A");
	} else {
  	formatPrintI("", currentValue, "A");
	}
	uView.setCursor(0,24);
	if (currentConsumed > 10000) {
  	formatPrintF("", currentConsumed / 1000, "Ah");
	} else {
  	formatPrintI("", currentConsumed, "mAh");
	}
	uView.display();

}

// raw sensor display
void display3() {
	uView.clear(PAGE);
	uView.setFontType(3);
	uView.setCursor(0,0);
  formatPrintI("", currentSensorValue, "A");
	uView.display();
}

/*
Name:       Labeller.ino
Created:	7/31/2018 11:33:41 AM
Updated:	12/31/2018 11:33:41 AM
Author:     Joe Hosemann
*/

#include <Arduino.h>

enum PinAssignments
{	
	//Analog
	pinAnalogLabelSensor = A1, // sensor that detects new label		
	//Digital
	pinPedal = 2,
	pinRelay = 7	
};

int sensorValueOnLabel() {
	return (int) analogRead(pinAnalogLabelSensor);
}
bool isOnLabel() {
	return sensorValueOnLabel() > 10;
}
bool isPedalDown() {
	return digitalRead(pinPedal) == LOW;
}
int pedalValue() {
	return (int)digitalRead(pinPedal);
}

void setup()
{
	Serial.begin(9600); // Serial Output
	pinMode(pinPedal, INPUT_PULLUP);
	pinMode(pinRelay, OUTPUT);

	analogWrite(pinAnalogLabelSensor, OUTPUT);
	pinMode(A1, OUTPUT);
	
	// force all analogs to output (keeps bugs out)
	pinMode(A0, OUTPUT);
	digitalWrite(A0, LOW);
	pinMode(A2, OUTPUT);
	digitalWrite(A2, LOW);
	pinMode(A3, OUTPUT);
	digitalWrite(A4, LOW);
	pinMode(A4, OUTPUT);
	digitalWrite(A4, LOW);
	pinMode(A5, OUTPUT);
	digitalWrite(A5, LOW);
	delay(5000);
}

void loop()
{
	//Serial.print("isPedalDown: ");
	//Serial.print(isPedalDown() == true ? "true" : "false");
	//Serial.print(", sensorValueOnLabel: ");
	//Serial.print(sensorValueOnLabel());
	//Serial.print(", isOnLabel: ");
	//Serial.print(isOnLabel()==true?"true":"false");
	//Serial.print(", pedalValue: ");
	//Serial.println(pedalValue());

	

	if (isPedalDown())
	{		
		moveToNextLabel();
	}
}

void moveToNextLabel()
{
	Serial.println("moveToNextLabel");
	digitalWrite(pinRelay, HIGH);
	delay(100);
	
	while (isOnLabel())
	{
		Serial.println("isOnLabel");
		delay(100);
	}
	Serial.print(", sensorValueOnLabel: ");
	Serial.print(sensorValueOnLabel());
	Serial.print("--isOnLabel: ");
	Serial.println(isOnLabel() == true ? "true" : "false");
	digitalWrite(pinRelay, LOW);
	delay(50);
}
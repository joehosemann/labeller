/*
    Name:       Labeller.ino
    Created:	7/31/2018 11:33:41 AM
    Author:     Joe Hosemann
	Pinout: 
		7Seg: A5 -> SCL, A4 -> SDA
		RotaryEncoder: 2 -> DT, 3 -> CLK, 4 -> SW
		Switch: 5 -> Top Right, 10k Resistor/Ground -> Bottom Right, 5v -> Bottom Left
		Relay: 6 -> S
*/

#include <Arduino.h>
#include <Adafruit_LEDBackpack.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <AccelStepper.h>

/* Rotary Encoder */
#include <Rotary.h>

Rotary rotary = Rotary(2, 3); // outputs to pins 2 and 3.


#define DISPLAY_ADDRESS 0x70

Adafruit_7segment disp = Adafruit_7segment();

/* Rotary Encoder (CLK, DT, SW, +, GND, [HOLE]) */


enum PinAssignments {
	encoderPinB = 2,		// left, labeled CLK, blue wire
	encoderPinA = 3,		// right, labeled DT, green wire
	encoderClearPin = 4,    // switch, labeled SW, yellow wire

	motorStepPin = 6,       // motor Step Pin, black wire
	motorDirPin = 7,        // motor Direction Pin, red wire
	motorEnablePin = 8,		// motor Enable Pin, yellow wire
	
	buttonGoPin = 10,		// "Go" button, blue wire
	buttonUpPin = 11,		// "Up" button, yellow wire
	buttonDownPin = 12		// "Down" button, white wire
};

int buttonState = 0;         // variable for reading the pushbutton status

int defaultEncoderPos = 152;
volatile unsigned int encoderPos = 152;  // a counter for the dial
unsigned int lastReportedPos = 1;   // change management
static boolean rotating = false;    // debounce management

int motorSpeed = 800;
int motorAccel = 800;

// interrupt service routine vars
boolean A_set = false;
boolean B_set = false;
boolean buttonReleased = true;

AccelStepper stepper(1, motorStepPin, motorDirPin);

void setup()
{

	setupRotaryEncoder();	

	pinMode(buttonGoPin, INPUT);
	pinMode(encoderClearPin, INPUT);

	//for (int i = 0; i <= A5; i++) {
	//	if (i == encoderPinA || i == encoderPinB || i == encoderClearPin || i == buttonGoPin)
	//		continue;
	//	pinMode(i, OUTPUT);
	//}

	stepper.setMaxSpeed(motorSpeed);
	stepper.setAcceleration(motorAccel);

	digitalWrite(motorEnablePin, LOW);
	
	disp.begin(DISPLAY_ADDRESS);

	Serial.begin(115200);  // Serial Output
}

void loop()
{
	rotating = true;  // reset the debouncer

	disp.print(encoderPos, DEC);
	disp.writeDisplay();

	digitalWrite(buttonGoPin, HIGH);
	digitalWrite(encoderClearPin, HIGH);

	if (lastReportedPos != encoderPos) {
		Serial.print("Index:");
		Serial.println(encoderPos, DEC);
		lastReportedPos = encoderPos;
	}
	if (digitalRead(encoderClearPin) == LOW) {
		Serial.println("encoderClearPin");
		//encoderPos = defaultEncoderPos;
	}	
	
	buttonState = digitalRead(buttonGoPin);
	if (buttonState == LOW) {

		if (!buttonReleased)
		{
			Serial.println("Button not released");
			return;
		}

		//Serial.println("Button pressed");
		buttonReleased = false;
		doMotor(encoderPos, "forward");

	}
	else
	{
		buttonReleased = true;
	}

}

void setupRotaryEncoder()
{
	attachInterrupt(0, rotaryEncoder_Rotate, CHANGE);
	attachInterrupt(1, rotaryEncoder_Rotate, CHANGE);
}

void rotaryEncoder_Rotate() {
	unsigned char result = rotary.process();
	if (result == DIR_CW) {
		encoderPos++;
	}
	else if (result == DIR_CCW) {
		encoderPos--;
	}
}

void doMotor(int steps, String direction)
{

	int _steps = steps*2;
	int _check = _steps - 100;

	//Serial.print("Steps: " + _steps);

	if (direction == "forward")
	{
		_steps = _steps * -1;
		_check = _steps + 100;
	}
	

	/*Serial.print("Motor start.  Steps: ");
	Serial.print(_steps, DEC);
	Serial.print(" , Direction: ");
	Serial.println(direction);*/
	stepper.disableOutputs();
	//delay(300);
	stepper.setCurrentPosition(0);

	/*Serial.print("Motor current position: ");
	Serial.println(stepper.currentPosition(), DEC);
*/
	stepper.moveTo(_steps);
	while (stepper.currentPosition() != _check) // Full speed up to 300
		stepper.run();
	stepper.stop(); // Stop as fast as possible: sets new target
	stepper.runToPosition();
	stepper.enableOutputs();
	/*
	Serial.print("Motor end.  Check: ");
	Serial.print(_check, DEC);
	Serial.print(" , CurrentPosition: ");
	Serial.println(stepper.currentPosition(), DEC);

	*/
}

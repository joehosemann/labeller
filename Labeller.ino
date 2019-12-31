/*
Name:       Labeller.ino
Created:	7/31/2018 11:33:41 AM
Updated:	12/31/2018 11:33:41 AM
Author:     Joe Hosemann
Pinout:
7Seg: A5 -> SCL, A4 -> SDA
RotaryEncoder: 2 -> DT, 3 -> CLK, 4 -> SW
Switch: 5 -> Top Right, 10k Resistor/Ground -> Bottom Right, 5v -> Bottom Left
Relay: 6 -> S
*/

#include <EEPROM.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Arduino.h>
#include <UTFT.h>
#include <UTouch.h>
// Declare which fonts we will be using
extern uint8_t BigFont[];
UTFT myGLCD(ITDB32S, 38, 39, 40, 41);
UTouch myTouch(6, 5, 4, 3, 2);

const String textOnLabel = "LABEL   ";
const String textNotOnLabel = "LABEL BACK";

enum PinAssignments
{
	
	pinAnalogLabelSensor = A1, // sensor that detects new label
	pinMotorStep = 12, // motor Step Pin
	pinMotorDir = 13, // motor Direction Pin
	
	pinMotorEnable = 9, // motor Enable Pin
	pinPedal = 8 // pin for pedal switch
};

int motorSpeed = 550;
int motorAccel = 500;
// Storage address of stepper/sensor offset
int sensorStorageAddress = 0;
int stepsToNextLabel = 0;
int newLabel = 0;
int x, y;
char stCurrent[20] = "";
int stCurrentLen = 0;
char stLast[20] = "";
int displayNumPad = 0;
AccelStepper stepper(1, pinMotorStep, pinMotorDir);

int sensorValueOnLabel() {
	return (int) analogRead(pinAnalogLabelSensor);
}
bool isOnLabel() {
	return sensorValueOnLabel() > 10;
}

void setup()
{
	Serial.begin(9600); // Serial Output
	pinMode(pinPedal, INPUT_PULLUP);
	digitalWrite(pinPedal, HIGH);

	pinMode(A0, OUTPUT);
	digitalWrite(A0, LOW);

	analogWrite(pinAnalogLabelSensor, OUTPUT);
	pinMode(A1, OUTPUT);
	//digitalWrite(A1, LOW);
	pinMode(A2, OUTPUT);
	digitalWrite(A2, LOW);
	pinMode(A3, OUTPUT);
	digitalWrite(A4, LOW);
	pinMode(A4, OUTPUT);
	digitalWrite(A4, LOW);
	pinMode(A5, OUTPUT);
	digitalWrite(A5, LOW);
	// Initial setup
	myGLCD.InitLCD();
	myTouch.InitTouch();
	myTouch.setPrecision(PREC_MEDIUM);
	resetScreen();
	//stepper.setEnablePin(motorEnablePin);
	stepper.setMaxSpeed(motorSpeed);
	stepper.setAcceleration(motorAccel);
	stepper.disableOutputs();

	
}

void resetScreen()
{
	myGLCD.clrScr();
	myGLCD.setFont(BigFont);
	myGLCD.setBackColor(0, 0, 255);
}

void loop()
{
	
	if (displayNumPad == 0)
	{
		drawHomeScreen();
	}
	else
		if (displayNumPad == 1)
		{
			drawNumButtons();
		}
	if (displayNumPad == 0)
	{
		loopHomeScreen();
	}
	else
		if (displayNumPad == 1)
		{
			loopDisplayNumPad();
		}
}

// / Returns true when complete
void moveToNextLabel()
{
	Serial.println("moveToNextLabel");
	//int val = analogRead(A1);
	stepsToNextLabel = recallValue(0);

	stepper.setCurrentPosition(0);
	//stepper.enableOutputs();
	//stepper.moveTo(-30);
	//stepper.runToPosition();

	stepper.moveTo(-50000);
	//Serial.print("start sensor read: ");
	//Serial.println(val);
	while (isOnLabel())
	{
		stepper.run();
		
		//Serial.println(val);
	}
	//Serial.println("end sensor read");
	stepper.stop(); // Stop as fast as possible: sets new target  
	stepper.setCurrentPosition(0);
	stepper.moveTo(stepsToNextLabel*-1); // Recalls the offset between the sensor and the start of the next label.
	//Serial.print("start stored delay at: ");
	//Serial.println(stepsToNextLabel);
	while (stepper.currentPosition() != stepsToNextLabel * -1)
		stepper.run();

	//Serial.println("end stored delay");
	stepper.stop();
	stepper.runToPosition();

	//stepper.disableOutputs();
}

void storeValue(int pin, int value)
{
	EEPROM.write(pin, value);
}

int recallValue(int pin)
{
	return EEPROM.read(pin);
}

void doMotor(int steps, String direction)
{
	int _steps = steps;
	int _check = _steps - 100;

	if (direction == "forward")
	{
		_steps = _steps * -1;
		_check = _steps + 100;
	}

	stepper.enableOutputs();
	stepper.setCurrentPosition(0);
	stepper.moveTo(_steps);
	while (stepper.currentPosition() != _check) // Full speed up to 300
		stepper.run();
	stepper.stop(); // Stop as fast as possible: sets new target
	stepper.runToPosition();
	stepper.disableOutputs();
}

void drawHomeScreen()
{

	resetScreen();

	//Serial.println("drawHomeScreen");
	// Run
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(30, 34, 124, 212);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(30, 34, 124, 212);
	myGLCD.print("Run", 53, 114);
	// Forward
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(154, 34, 298, 81);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(154, 34, 298, 81);
	myGLCD.print("Fwd", 160, 49);
	// Reverse
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(154, 100, 298, 146);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(154, 100, 298, 146);
	myGLCD.print("Rev", 160, 115);
	// Delay
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(154, 166, 298, 212);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(154, 166, 298, 212);
	myGLCD.print("Delay", 160, 181);
}
void loopHomeScreen()
{
	boolean OK = false;
	int sensorValue;
	float voltage;
	String str;
	myGLCD.setColor(0, 255, 0);
	int loopCount = 0;
	while (true)
	{
		Serial.print("labelSensor: ");
		Serial.println(sensorValueOnLabel());

		int buttonState = 0;
		buttonState = digitalRead(pinPedal);

		if (buttonState == LOW)
		{
			moveToNextLabel();
		}

		 if (isOnLabel() && (str != textOnLabel))
		{
			str = textOnLabel;
			myGLCD.print(str, LEFT, 224);
		}
		else if (!isOnLabel() && (str != textNotOnLabel))
		{
			str = textNotOnLabel;
			myGLCD.print(str, LEFT, 224);
		}

		if (myTouch.dataAvailable())
		{
			myTouch.read();
			x = myTouch.getX();
			y = myTouch.getY();
			// Run
			if ((y >= 34) && (y <= 212) && (x >= 30) && (x < 124))
			{
				waitForIt(30, 34, 124, 212);
				moveToNextLabel();
			}
			// Fwd
			if ((y >= 34) && (y <= 81) && (x >= 154) && (x < 298))
			{
				motorWaitForIt(50000, 154, 34, 298, 81);
			}
			// Rev
			if ((y >= 100) && (y <= 146) && (x >= 154) && (x < 298))
			{
				motorWaitForIt(-50000, 154, 100, 298, 146);
			}
			// Delay
			if ((y >= 166) && (y <= 212) && (x >= 154) && (x < 298))
			{
				resetScreen();
				displayNumPad = 1;
				return;  // exit loop
			}
		}

	}
}

void drawNumButtons()
{

	resetScreen();

	// Draw the upper row of buttons
	for (x = 0; x < 5; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(10 + (x * 60), 10, 60 + (x * 60), 60);
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(10 + (x * 60), 10, 60 + (x * 60), 60);
		myGLCD.printNumI(x + 1, 27 + (x * 60), 27);
	}
	// Draw the center row of buttons
	for (x = 0; x < 5; x++)
	{
		myGLCD.setColor(0, 0, 255);
		myGLCD.fillRoundRect(10 + (x * 60), 70, 60 + (x * 60), 120);
		myGLCD.setColor(255, 255, 255);
		myGLCD.drawRoundRect(10 + (x * 60), 70, 60 + (x * 60), 120);
		if (x < 4)
			myGLCD.printNumI(x + 6, 27 + (x * 60), 87);
	}
	myGLCD.print("0", 267, 87);
	// Draw the lower row of buttons
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(10, 130, 150, 180);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(10, 130, 150, 180);
	myGLCD.print("Exit", 40, 147);
	myGLCD.setColor(0, 0, 255);
	myGLCD.fillRoundRect(160, 130, 300, 180);
	myGLCD.setColor(255, 255, 255);
	myGLCD.drawRoundRect(160, 130, 300, 180);
	myGLCD.print("Enter", 190, 147);
	myGLCD.setBackColor(0, 0, 0);
}

void updateStr(int val)
{
	if (stCurrentLen < 20)
	{
		stCurrent[stCurrentLen] = val;
		stCurrent[stCurrentLen + 1] = '\0';
		stCurrentLen++;
		myGLCD.setColor(0, 255, 0);
		myGLCD.print(stCurrent, LEFT, 224);
		delay(100);
	}
	else
	{
		myGLCD.setColor(255, 0, 0);
		myGLCD.print("BUFFER FULL!", CENTER, 192);
		delay(500);
		myGLCD.print("            ", CENTER, 192);
		delay(500);
		myGLCD.print("BUFFER FULL!", CENTER, 192);
		delay(500);
		myGLCD.print("            ", CENTER, 192);
		myGLCD.setColor(0, 255, 0);
	}
	drawNumButtons();
}

// Draw a red frame while a button is touched
void waitForIt(int x1, int y1, int x2, int y2)
{
	//myGLCD.setColor(255, 0, 0);
	//myGLCD.drawRoundRect(x1, y1, x2, y2);
	while (myTouch.dataAvailable())
		myTouch.read();
	//myGLCD.setColor(255, 255, 255);
	//myGLCD.drawRoundRect(x1, y1, x2, y2);
}

// Draw a red frame while a button is touched
void motorWaitForIt(int steps, int x1, int y1, int x2, int y2)
{
	//stepper.enableOutputs();
	//myGLCD.setColor(255, 0, 0);
	//myGLCD.drawRoundRect(x1, y1, x2, y2);
	stepper.setCurrentPosition(0);
	stepper.moveTo(steps);
	while (myTouch.dataAvailable())
	{
		myTouch.read();
		stepper.run();
	}
	stepper.stop();
	stepper.setCurrentPosition(0);
	//myGLCD.setColor(255, 255, 255);
	//myGLCD.drawRoundRect(x1, y1, x2, y2);
	//stepper.disableOutputs();
}

void loopDisplayNumPad()
{
	stepsToNextLabel = recallValue(0);
	String tempVal = String(stepsToNextLabel);
	myGLCD.setColor(0, 255, 0);
	myGLCD.print(tempVal, LEFT, 224);
	while (true)
	{

		if (myTouch.dataAvailable())
		{
			myTouch.read();
			x = myTouch.getX();
			y = myTouch.getY();
			if ((y >= 10) && (y <= 60))
			{
				if ((x >= 10) && (x <= 60))
				{
					// Button: 1
					waitForIt(10, 10, 60, 60);
					updateStr('1');
				}
				if ((x >= 70) && (x <= 120))
				{
					// Button: 2
					waitForIt(70, 10, 120, 60);
					updateStr('2');
				}
				if ((x >= 130) && (x <= 180))
				{
					// Button: 3
					waitForIt(130, 10, 180, 60);
					updateStr('3');
				}
				if ((x >= 190) && (x <= 240))
				{
					// Button: 4
					waitForIt(190, 10, 240, 60);
					updateStr('4');
				}
				if ((x >= 250) && (x <= 300))
				{
					// Button: 5
					waitForIt(250, 10, 300, 60);
					updateStr('5');
				}
			}
			// Center row
			if ((y >= 70) && (y <= 120))
			{
				if ((x >= 10) && (x <= 60))
				{
					// Button: 6
					waitForIt(10, 70, 60, 120);
					updateStr('6');
				}
				if ((x >= 70) && (x <= 120))
				{
					// Button: 7
					waitForIt(70, 70, 120, 120);
					updateStr('7');
				}
				if ((x >= 130) && (x <= 180))
					// Button: 8
				{
					waitForIt(130, 70, 180, 120);
					updateStr('8');
				}
				if ((x >= 190) && (x <= 240))
				{
					// Button: 9
					waitForIt(190, 70, 240, 120);
					updateStr('9');
				}
				if ((x >= 250) && (x <= 300))
				{
					// Button: 0
					waitForIt(250, 70, 300, 120);
					updateStr('0');
				}
			}
			// Upper row
			if ((y >= 130) && (y <= 180))
			{
				if ((x >= 10) && (x <= 150))
				{
					// Button: Exit
					waitForIt(10, 130, 150, 180);
					stCurrent[0] = '\0';
					stCurrentLen = 0;
					myGLCD.setColor(0, 0, 0);
					myGLCD.fillRect(0, 224, 319, 239);
				}
				if ((x >= 160) && (x <= 300))
				{
					waitForIt(160, 130, 300, 180);
					if (stCurrentLen > 0)
					{
						for (x = 0; x < stCurrentLen + 1; x++)
						{
							stLast[x] = stCurrent[x];
						}
						stCurrent[0] = '\0';
						stCurrentLen = 0;
						myGLCD.setColor(0, 0, 0);
						myGLCD.fillRect(0, 208, 319, 239);
						myGLCD.setColor(0, 255, 0);
						myGLCD.print(stLast, LEFT, 208);
						int val = atoi(stLast);
						storeValue(0, val);
					}
					else
					{
						/*myGLCD.setColor(255, 0, 0);
						myGLCD.print("BUFFER EMPTY", CENTER, 192);
						delay(500);
						myGLCD.print("            ", CENTER, 192);
						delay(500);
						myGLCD.print("BUFFER EMPTY", CENTER, 192);
						delay(500);
						myGLCD.print("            ", CENTER, 192);
						myGLCD.setColor(0, 255, 0);*/
					}
				}
				// No matter the result, Exit and Enter both go back to home screen.
				displayNumPad = 0;
				resetScreen();
				return; // Restart Loop
			}
		}
	}
}

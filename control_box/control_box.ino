#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <gfxfont.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int pumpPin = 12;
int val = 0;

int sensorPin1 = A0; //left
int sensorPin2 = A1; //center
int sensorPin3 = A2; //right

int doDig = 3; //digital 2
int vexReturn = 4; //digital 3

int stopFlag = 0; // If water error is detected set flag to stop code
int firstRun = 0; // Delay movement if first run

// Anything over ~450-500 can be considered 'dry'
// Anything under ~300 can be considered 'watered'
const int waterThreshold = 410;

SoftwareSerial moveSerial(9, 8);

void setupScreen() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);  // Don't proceed, loop forever
  }

  delay(2000);

  display.clearDisplay();

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println("    Pump Control");
  display.setCursor(0, 14);
  display.println("--------------------");
  display.display();
  delay(2000);

  display.clearDisplay();
}

void setup() {
  // Start serial channels
  Serial.begin(9600);
  moveSerial.begin(9600);

  pinMode(doDig, OUTPUT);
  pinMode(vexReturn, INPUT_PULLUP);

  digitalWrite(doDig, false);


  // Setup the oled screen
  setupScreen();

  // Set pump pins
  pinMode(pumpPin, OUTPUT);

  Serial.println("Plant controller starting");
}

void updateDisplay(int sensor1, int sensor2, int sensor3) {
  display.clearDisplay();

  // Display title bar on the display
  display.setCursor(0, 0);
  display.println("    Pump Control");
  display.setCursor(0, 6);
  display.println("--------------------");

  // Display water sensor readings on the display
  display.setCursor(0, 10);
  display.println("Water Sensors AVG: ");
  display.setCursor(0, 18);
  display.println((sensor1 + sensor2 + sensor3)/3);
  // display.println(sensor1);
  // display.println(sensor2);
  // display.println(sensor3);

  // Display the pump state on the display
  display.setCursor(0, 24);
  display.print("Pump State: ");
  display.println(digitalRead(pumpPin));
  display.display();
}

void loop() {

  while(stopFlag){}

  // Check for any commands to start the planting process from the pc
  if (Serial.available() > 0) {
    char pcCommand = Serial.read();

    if (pcCommand == '1' || pcCommand == '2' || pcCommand == '3') {
      Serial.print("PC command received: ");
      Serial.println(pcCommand);
      moveSerial.write(pcCommand);

      while (moveSerial.available() == 0) {
      }
      char done = moveSerial.read();
      if (done == 'D') {
        Serial.println("Move controller finished moving.");

        Serial.println("Starting vex code");
        // ----------------------------
        // INSERT VEX CODE CONTROL HERE
        digitalWrite(doDig, true);
        //delay(5000);
        // while(digitalRead(vexReturn) != 1)
        // {
        //   delay(1000);
        // }
        delay(22000);
        digitalWrite(doDig, false);
        // ----------------------------

        Serial.println("Planting process complete.");
        moveSerial.write('H');
        while (moveSerial.available() == 0) {
        }
        char homeDone = moveSerial.read();
        if (homeDone == 'D') {
          Serial.println("Tool returned to home, waiting for commands...");
        }
      }

    } else {
      Serial.println("Invalid PC command. Use 1, 2, or 3.");
    }
  }

  if (firstRun == 0) {
    firstRun++;
    delay(7000);
  }

  // Check for any plants that need watering
  // Take the current reading from the water sensors
  int sensor1 = analogRead(sensorPin1);
  int sensor2 = analogRead(sensorPin2);
  int sensor3 = analogRead(sensorPin3);
  // Serial.print("Sensor reading1: " ); Serial.println(sensor1);
  // Serial.print("Sensor reading2: " ); Serial.println(sensor2);
  // Serial.print("Sensor reading3: " ); Serial.println(sensor3);
  // Serial.print(sensorPin3);

  updateDisplay(sensor1, sensor2, sensor3);

  // Check if any plants need watering
  if (sensor1 > waterThreshold || sensor2 > waterThreshold ||
      sensor3 > waterThreshold) {
    int currentSensorPin = 0;

    char command;
    if (sensor1 > waterThreshold) {
      command = '1';
      currentSensorPin = sensorPin1;
    } else if (sensor2 > waterThreshold) {
      command = '2';
      currentSensorPin = sensorPin2;
    } else {
      command = '3';
      currentSensorPin = sensorPin3;
    }

    Serial.print("Water sensor triggered sensor ");
    Serial.println(command);
    moveSerial.write(command);

    // Wait for the move controller to indicate movement has been completed
    while (moveSerial.available() == 0) {
    }
    char done = moveSerial.read();
    if (done == 'D') {
      // Start the watering process, turn the pump on for a brief period
      Serial.println("Move controller finished, activiating pump...");

      digitalWrite(pumpPin, HIGH);
      updateDisplay(sensor1, sensor2, sensor3);

      delay(1000);

      digitalWrite(pumpPin, LOW);
      updateDisplay(sensor1, sensor2, sensor3);

      // Allow time for the water to soak into the soil and reach the sensor
      Serial.println("Waiting for water to saturate soil...");
      delay(5000);

      // Re-check the water value of the sensor to see if enough water has been
      // dispensed
      int sensorValue = analogRead("A" + currentSensorPin);
      // Serial.println(currentSensorPin);

      // Continue watering until the threshold is crossed, allowing time for the
      // water to soak in each time
      int waterCounter = 0;
      while ((sensorValue > waterThreshold) && (waterCounter < 4)) {
        Serial.println(
            "Sensor still below threshold. Running pump again for 5 "
            "seconds...");

        Serial.print("Sensor reading: " ); Serial.println(sensorValue);

        digitalWrite(pumpPin, HIGH);
        updateDisplay(sensor1, sensor2, sensor3);

        delay(1000);

        digitalWrite(pumpPin, LOW);
        updateDisplay(sensor1, sensor2, sensor3);

        Serial.println("Waiting for water to saturate soil...");
        delay(5000);

        int sensorValue = analogRead(currentSensorPin);
        waterCounter++;
      }

      if (waterCounter >= 4) {
        Serial.println("Error please check the pump has water, stopping program...");
        stopFlag = 1;
      }

      // Return the tool to its home position once watering is fully complete
      Serial.println("Watering complete, returning tool to home...");
      moveSerial.write('H');
      while (moveSerial.available() == 0) {
      }
      char homeDone = moveSerial.read();
      if (homeDone == 'D') {
        Serial.println("Tool returned to home, waiting for commands...");
      }
    }
  }

  delay(1000);
}

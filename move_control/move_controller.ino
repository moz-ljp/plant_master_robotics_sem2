#include <SoftwareSerial.h>

// Pin definitions
const int stepPin = 3;
const int dirPin = 4;
const int leftSwitchPin = 5;   // Left End Stop (Normally Open)
const int rightSwitchPin = 6;  // Right End Stop (Normally Open)

// Track positions and targets (in steps)
long currentPosition = 0;
long trackLength = 0;
long pot1 = 0, pot2 = 0, pot3 = 0;

// Step delay in microseconds
const int stepDelayMicroseconds = 2500;

// Setup software serial for the soil controller (TX, RX)
SoftwareSerial soilSerial(9, 8);

// Executes one step pulse for the motor
void stepMotor() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(stepDelayMicroseconds);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(stepDelayMicroseconds);
}

// Moves a given number of steps; positive means moving right, negative left.
void moveSteps(long steps) {
  digitalWrite(dirPin, (steps >= 0) ? HIGH : LOW);
  delay(1000);
  long stepsToMove = abs(steps);
  for (long i = 0; i < stepsToMove; i++) {
    stepMotor();
    // Update currentPosition according to direction
    currentPosition += (steps >= 0 ? 1 : -1);
  }
}

// Performs track calibration by moving to the left end, then to the right end,
// calculates the track length, and then calculates plant pot positions.
void calibrateTrack() {
  Serial.println("Starting calibration...");

  // Move to left end (home)
  digitalWrite(dirPin, LOW);
  delay(1000);
  while (digitalRead(leftSwitchPin) == HIGH) {
    stepMotor();
  }
  currentPosition = 0;
  Serial.println("Left end stop reached.");

  // Move to right end to determine track length
  digitalWrite(dirPin, HIGH);
  delay(1000);
  while (digitalRead(rightSwitchPin) == HIGH) {
    stepMotor();
    currentPosition++;
  }
  trackLength = currentPosition;
  Serial.print("Right end stop reached. Track length in steps: ");
  Serial.println(trackLength);

  // Calculate plant pot positions
  pot1 = trackLength / 4;
  pot2 = trackLength / 2;
  pot3 = (3.5 * trackLength) / 4;
  Serial.print("Target Positions: 1: ");
  Serial.print(pot1);
  Serial.print(" 2: ");
  Serial.print(pot2);
  Serial.print(" 3: ");
  Serial.println(pot3);

  // Return to home (left end)
  digitalWrite(dirPin, LOW);
  delay(1000);
  while (digitalRead(leftSwitchPin) == HIGH) {
    stepMotor();
    if (currentPosition > 0) currentPosition--;
  }
  currentPosition = 0;
  Serial.println("Returned home, awaiting further instructions.");
}

// Moves the motor to the given target position (in steps).
void moveToPosition(long targetPosition) {
  long stepsToMove = targetPosition - currentPosition;
  if (stepsToMove != 0) {
    moveSteps(stepsToMove);
  }
}

// Moves the motor back to home (left end).
void moveHome() {
  digitalWrite(dirPin, LOW);
  delay(1000);
  while (digitalRead(leftSwitchPin) == HIGH) {
    stepMotor();
    if (currentPosition > 0) currentPosition--;
  }
  currentPosition = 0;
  Serial.println("Moved home.");
}

void setup() {
  // Set motor control pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Set end stop switches pins
  pinMode(leftSwitchPin, INPUT_PULLUP);
  pinMode(rightSwitchPin, INPUT_PULLUP);

  // Start serial channels
  Serial.begin(9600);
  soilSerial.begin(9600);

  // Setup track length and plant pot positions
  calibrateTrack();
}

void loop() {
  if (soilSerial.available() > 0) {
    char cmd = soilSerial.read();
    Serial.print("Received command: ");
    Serial.println(cmd);

    switch (cmd) {
      case '1':
        moveToPosition(pot1);
        Serial.println("Moved to position 1");
        break;
      case '2':
        moveToPosition(pot2);
        Serial.println("Moved to position 2");
        break;
      case '3':
        moveToPosition(pot3);
        Serial.println("Moved to position 3");
        break;
      case 'H':
        moveHome();
        break;
      case 'D':
        break;
      default:
        Serial.println("Invalid command received");
        break;
    }
    // Send movement done back to the soil controller
    soilSerial.write('D');
  }
}

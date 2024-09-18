#include <CurieBLE.h>

// Define pin connections for both motors
const int dirPin1 = 2;
const int stepPin1 = 3;
const int dirPin2 = 8;
const int stepPin2 = 9;

// Define speeds (microseconds per step) for both motors
int stepDelay = 2000; // Default speed for both motors

// Define UUIDs for BLE service and characteristics
const char* serviceUUID = "12345678-1234-5678-1234-56789abcdef0"; // Replace with your own UUID
const char* motorControlUUID = "abcdef01-1234-5678-1234-56789abcdef0"; // UUID for Motor control (both motors)
const char* speedControlUUID = "abcdef02-1234-5678-1234-56789abcdef0"; // UUID for Speed control

BLEPeripheral blePeripheral; // Create peripheral instance
BLEService motorService(serviceUUID); // Create a service

// Create characteristics for control and speed
BLECharacteristic motorControlChar(motorControlUUID, BLERead | BLEWrite, 1); // 1 byte for command
BLECharacteristic speedControlChar(speedControlUUID, BLERead | BLEWrite, 1); // 1 byte for speed

// Variables to store the current command and speed
byte currentCommand = 'S'; // Default command: Stop
byte currentSpeed = 200; // Default speed value (0-255)
unsigned long commandStartTime = 0; // To track when the command was received
const unsigned long delayBeforeStart = 1000; // 1 second delay before starting the motors

void setup() {
  Serial.begin(9600); // Initialize serial communication
  
  // Declare pins as Outputs for both motors
  pinMode(stepPin1, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(stepPin2, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  
  // Initialize BLE
  blePeripheral.setLocalName("MotorControl");
  blePeripheral.setAdvertisedServiceUuid(motorService.uuid());

  // Add service and characteristics
  blePeripheral.addAttribute(motorService);
  blePeripheral.addAttribute(motorControlChar);
  blePeripheral.addAttribute(speedControlChar);

  // Set initial values for the characteristics
  byte initialCommand = 'S'; // Default command: Stop
  motorControlChar.setValue(&initialCommand, 1); // Set initial command value
  byte initialSpeed = 200; // Default speed (medium speed)
  speedControlChar.setValue(&initialSpeed, 1); // Set initial speed value

  // Set up event handlers
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  motorControlChar.setEventHandler(BLEWritten, motorControlCharacteristicWritten);
  speedControlChar.setEventHandler(BLEWritten, speedControlCharacteristicWritten);

  // Begin advertising
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // Poll BLE events
  blePeripheral.poll();
  
  // Check if we need to delay before starting the motors
  unsigned long currentTime = millis();
  
  if (currentCommand != 'S' && (currentTime - commandStartTime) >= delayBeforeStart) {
    controlMotors();
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  Serial.print("Connected to central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}

void motorControlCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  currentCommand = motorControlChar.value()[0]; // Update the current command
  commandStartTime = millis(); // Record the time when the command was received
  Serial.print("Command received: ");
  Serial.println((char)currentCommand); // Print command as char for readability
}

void speedControlCharacteristicWritten(BLECentral& central, BLECharacteristic& characteristic) {
  byte newSpeed = speedControlChar.value()[0]; // Update the current speed
  if (newSpeed >= 0 && newSpeed <= 255) {
    currentSpeed = newSpeed; // Set the new speed if it's within the valid range
    stepDelay = map(currentSpeed, 0, 255, 2000, 100); // Map speed to delay
    Serial.print("Speed received: ");
    Serial.println(currentSpeed); // Print speed for readability
  } else {
    Serial.println("Invalid speed value");
  }
}

void controlMotors() {
  // Handle motor control based on current command
  Serial.println("Controlling motors...");
  switch (currentCommand) {
    case 'L': // Forward (clockwise)
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, HIGH);
      Serial.println("Motors should move forward (clockwise).");
      spinMotors();
      break;
    case 'R': // Backward (counterclockwise)
      digitalWrite(dirPin1, LOW);
      digitalWrite(dirPin2, LOW);
      Serial.println("Motors should move backward (counterclockwise).");
      spinMotors();
      break;
    case 'F': // Turn Left
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW); // Motor 2 stops or spins backward
      Serial.println("Turning left.");
      spinMotors();
      break;
    case 'B': // Turn Right
      digitalWrite(dirPin1, LOW); // Motor 1 stops or spins backward
      digitalWrite(dirPin2, HIGH);
      Serial.println("Turning right.");
      spinMotors();
      break;
    case 'S': // Stop
      Serial.println("Motors should stop.");
      break;
    default:
      Serial.println("Unknown command");
      break;
  }
}

void spinMotors() {
  // Spin both motors with the current speed
  if (currentCommand == 'S') {
    // Stop motors
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
  } else {
    // Spin motors
    digitalWrite(stepPin1, HIGH);
    digitalWrite(stepPin2, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(stepPin1, LOW);
    digitalWrite(stepPin2, LOW);
    delayMicroseconds(stepDelay);
  }
}


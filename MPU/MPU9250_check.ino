#include <Wire.h>

// MPU9250 I2C address
const int MPU9250_ADDR = 0x68;

// MPU9250 Registers
const int PWR_MGMT_1 = 0x6B;
const int ACCEL_XOUT_H = 0x3B;
const int WHO_AM_I = 0x75;

const int ledPin = 9;
const float sensitivity = 15.0;  // Threshold in degrees
const int ledOnTime = 500;       // LED stays on for 500ms

float prevAngleX = 0;
unsigned long lastChangeTime = 0;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   MPU9250 Direction Detector      ║");
  Serial.println("╚════════════════════════════════════╝\n");
  
  // Initialize MPU9250
  delay(100);
  
  // Wake up MPU9250
  writeRegister(PWR_MGMT_1, 0x00);
  delay(100);
  
  // Verify connection
  byte whoami = readRegister(WHO_AM_I);
  Serial.print("WHO_AM_I: 0x");
  Serial.println(whoami, HEX);
  
  if (whoami == 0x71 || whoami == 0x73) {
    Serial.println("✓ MPU9250 detected and connected!");
  } else if (whoami == 0x70) {
    Serial.println("✓ MPU6500 detected and connected!");
  } else if (whoami == 0x68) {
    Serial.println("✓ MPU6050 detected and connected!");
  } else {
    Serial.println("✗ Unknown sensor!");
    Serial.print("Expected 0x68/0x70/0x71/0x73, got 0x");
    Serial.println(whoami, HEX);
    Serial.println("Check your wiring.");
    // Blink LED rapidly to indicate error
    while(1) {
      digitalWrite(ledPin, HIGH);
      delay(100);
      digitalWrite(ledPin, LOW);
      delay(100);
    }
  }
  
  // Configure accelerometer (±2g range)
  writeRegister(0x1C, 0x00);
  delay(10);
  
  // Configure gyroscope (±250°/s range)
  writeRegister(0x1B, 0x00);
  delay(10);
  
  // Set sample rate
  writeRegister(0x19, 0x07);
  delay(10);
  
  // Get initial angle
  int16_t ax, ay, az;
  readAccelData(&ax, &ay, &az);
  prevAngleX = atan2(ay, az) * 180.0 / PI;
  
  Serial.print("Initial angle: ");
  Serial.print(prevAngleX);
  Serial.println("°");
  Serial.println("\n✓ Ready! Tilt the sensor to detect direction changes.\n");
  
  // Test LED - blink 3 times
  Serial.println("Testing LED...");
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    Serial.println("LED should be ON");
    delay(500);
    digitalWrite(ledPin, LOW);
    Serial.println("LED should be OFF");
    delay(500);
  }
  Serial.println("LED test complete\n");
}

void loop() {
  int16_t ax, ay, az;
  readAccelData(&ax, &ay, &az);
  
  // Calculate angle from accelerometer data
  float angleX = atan2(ay, az) * 180.0 / PI;
  
  // Calculate the change in angle
  float angleDiff = abs(angleX - prevAngleX);
  
  // Detect significant direction change
  if (angleDiff > sensitivity) {
    digitalWrite(ledPin, HIGH);
    lastChangeTime = millis();
    
    // Debug output
    Serial.print("Direction change detected! Angle: ");
    Serial.print(angleX, 1);
    Serial.print("° | Change: ");
    Serial.print(angleDiff, 1);
    Serial.println("°");
    
    prevAngleX = angleX;
  }
  
  // Turn off LED after specified time
  if (millis() - lastChangeTime > ledOnTime) {
    digitalWrite(ledPin, LOW);
  }
  
  delay(50);
}

// Function to read accelerometer data
void readAccelData(int16_t* ax, int16_t* ay, int16_t* az) {
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 6, true);
  
  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
}

// Function to write to a register
void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Function to read from a register
byte readRegister(byte reg) {
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU9250_ADDR, 1, true);
  return Wire.read();
}

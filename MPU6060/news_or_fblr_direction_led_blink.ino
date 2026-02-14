#include <Wire.h>

// MPU6500/9250 I2C address
const int MPU_ADDR = 0x68;

// MPU Registers
const int PWR_MGMT_1 = 0x6B;
const int ACCEL_XOUT_H = 0x3B;
const int WHO_AM_I = 0x75;

// LED pins for 4 directions
const int ledNorth = 9;   // Forward tilt
const int ledSouth = 10;  // Backward tilt
const int ledEast = 11;   // Right tilt
const int ledWest = 12;   // Left tilt

// Angle thresholds (in degrees)
const float tiltThreshold = 20.0;  // Minimum tilt to trigger LED

void setup() {
  Wire.begin();
  Serial.begin(9600);
  
  // Setup LED pins
  pinMode(ledNorth, OUTPUT);
  pinMode(ledSouth, OUTPUT);
  pinMode(ledEast, OUTPUT);
  pinMode(ledWest, OUTPUT);
  
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║   4-Direction LED Indicator        ║");
  Serial.println("╚════════════════════════════════════╝\n");
  
  // Initialize sensor
  delay(100);
  writeRegister(PWR_MGMT_1, 0x00);
  delay(100);
  
  // Verify connection
  byte whoami = readRegister(WHO_AM_I);
  Serial.print("WHO_AM_I: 0x");
  Serial.println(whoami, HEX);
  
  if (whoami == 0x71 || whoami == 0x73) {
    Serial.println("✓ MPU9250 detected!");
  } else if (whoami == 0x70) {
    Serial.println("✓ MPU6500 detected!");
  } else if (whoami == 0x68) {
    Serial.println("✓ MPU6050 detected!");
  } else {
    Serial.println("✗ Unknown sensor!");
    while(1) {
      // Blink all LEDs to show error
      digitalWrite(ledNorth, HIGH);
      digitalWrite(ledSouth, HIGH);
      digitalWrite(ledEast, HIGH);
      digitalWrite(ledWest, HIGH);
      delay(200);
      digitalWrite(ledNorth, LOW);
      digitalWrite(ledSouth, LOW);
      digitalWrite(ledEast, LOW);
      digitalWrite(ledWest, LOW);
      delay(200);
    }
  }
  
  // Configure sensor
  writeRegister(0x1C, 0x00);  // Accelerometer ±2g
  writeRegister(0x1B, 0x00);  // Gyroscope ±250°/s
  writeRegister(0x19, 0x07);  // Sample rate
  delay(10);
  
  Serial.println("\n✓ Sensor initialized!");
  Serial.println("\nLED Configuration:");
  Serial.println("  North (Pin 9)  → Tilt Forward");
  Serial.println("  South (Pin 10) → Tilt Backward");
  Serial.println("  East  (Pin 11) → Tilt Right");
  Serial.println("  West  (Pin 12) → Tilt Left");
  Serial.println("\nTesting LEDs...");
  
  // Test each LED
  Serial.print("North... ");
  digitalWrite(ledNorth, HIGH);
  delay(500);
  digitalWrite(ledNorth, LOW);
  Serial.println("✓");
  
  Serial.print("South... ");
  digitalWrite(ledSouth, HIGH);
  delay(500);
  digitalWrite(ledSouth, LOW);
  Serial.println("✓");
  
  Serial.print("East... ");
  digitalWrite(ledEast, HIGH);
  delay(500);
  digitalWrite(ledEast, LOW);
  Serial.println("✓");
  
  Serial.print("West... ");
  digitalWrite(ledWest, HIGH);
  delay(500);
  digitalWrite(ledWest, LOW);
  Serial.println("✓");
  
  Serial.println("\n✓ Ready! Tilt the sensor in any direction.\n");
}

void loop() {
  int16_t ax, ay, az;
  readAccelData(&ax, &ay, &az);
  
  // Calculate tilt angles
  // angleX: positive = tilt backward (South), negative = tilt forward (North)
  // angleY: positive = tilt right (East), negative = tilt left (West)
  float angleX = atan2(ay, az) * 180.0 / PI;
  float angleY = atan2(ax, az) * 180.0 / PI;
  
  // Turn off all LEDs first
  digitalWrite(ledNorth, LOW);
  digitalWrite(ledSouth, LOW);
  digitalWrite(ledEast, LOW);
  digitalWrite(ledWest, LOW);
  
  String direction = "Level";
  
  // Check X-axis tilt (North/South)
  if (angleX < -tiltThreshold) {
    digitalWrite(ledNorth, HIGH);
    direction = "NORTH";
  } else if (angleX > tiltThreshold) {
    digitalWrite(ledSouth, HIGH);
    direction = "SOUTH";
  }
  
  // Check Y-axis tilt (East/West)
  if (angleY > tiltThreshold) {
    digitalWrite(ledEast, HIGH);
    if (direction != "Level") {
      direction += "-EAST";
    } else {
      direction = "EAST";
    }
  } else if (angleY < -tiltThreshold) {
    digitalWrite(ledWest, HIGH);
    if (direction != "Level") {
      direction += "-WEST";
    } else {
      direction = "WEST";
    }
  }
  
  // Print status every 500ms
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    Serial.print("Direction: ");
    Serial.print(direction);
    Serial.print(" | AngleX: ");
    Serial.print(angleX, 1);
    Serial.print("° | AngleY: ");
    Serial.print(angleY, 1);
    Serial.println("°");
    lastPrint = millis();
  }
  
  delay(50);
}

// Function to read accelerometer data
void readAccelData(int16_t* ax, int16_t* ay, int16_t* az) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(ACCEL_XOUT_H);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 6, true);
  
  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
}

// Function to write to a register
void writeRegister(byte reg, byte value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

// Function to read from a register
byte readRegister(byte reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 1, true);
  return Wire.read();
}

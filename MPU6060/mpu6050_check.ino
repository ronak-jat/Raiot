#include <Wire.h>

void setup() {
  Serial.begin(9600);
  while (!Serial) delay(10);
  
  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║  MPU6050 Final Communication Test    ║");
  Serial.println("╚═══════════════════════════════════════╝\n");
  
  Wire.begin();
  delay(100);
  
  // Test 1: Can we find the device?
  Serial.println("TEST 1: I2C Device Detection");
  Serial.println("-----------------------------");
  
  bool found68 = false, found69 = false;
  
  Wire.beginTransmission(0x68);
  if (Wire.endTransmission() == 0) {
    Serial.println("✓ Device responds at 0x68");
    found68 = true;
  }
  
  Wire.beginTransmission(0x69);
  if (Wire.endTransmission() == 0) {
    Serial.println("✓ Device responds at 0x69");
    found69 = true;
  }
  
  if (!found68 && !found69) {
    Serial.println("✗ NO DEVICE FOUND!");
    Serial.println("\n=== WIRING CHECK ===");
    Serial.println("Current connections should be:");
    Serial.println("  MPU6050 VCC → Arduino 3.3V or 5V");
    Serial.println("  MPU6050 GND → Arduino GND");
    Serial.println("  MPU6050 SDA → Arduino A4");
    Serial.println("  MPU6050 SCL → Arduino A5");
    Serial.println("  MPU6050 AD0 → Arduino GND");
    while(1);
  }
  
  int addr = found68 ? 0x68 : 0x69;
  Serial.print("Using address: 0x");
  Serial.println(addr, HEX);
  
  // Test 2: Read WHO_AM_I register
  Serial.println("\nTEST 2: Read WHO_AM_I Register");
  Serial.println("-------------------------------");
  
  Wire.beginTransmission(addr);
  Wire.write(0x75); // WHO_AM_I register
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 1, true);
  
  if (Wire.available()) {
    byte whoami = Wire.read();
    Serial.print("WHO_AM_I value: 0x");
    Serial.println(whoami, HEX);
    
    if (whoami == 0x68 || whoami == 0x34) {
      Serial.println("✓ Correct value - chip is responding!");
    } else if (whoami == 0x00 || whoami == 0xFF) {
      Serial.println("✗ Got 0x00 or 0xFF - communication problem!");
    } else {
      Serial.println("⚠ Unexpected value - might be wrong chip");
    }
  } else {
    Serial.println("✗ No data received!");
  }
  
  // Test 3: Try to write and read back a register
  Serial.println("\nTEST 3: Write/Read Test");
  Serial.println("------------------------");
  
  // Write to SMPLRT_DIV register (0x19)
  Serial.println("Writing 0x07 to register 0x19...");
  Wire.beginTransmission(addr);
  Wire.write(0x19);
  Wire.write(0x07);
  byte writeError = Wire.endTransmission();
  
  Serial.print("Write result: ");
  Serial.println(writeError == 0 ? "SUCCESS" : "FAILED");
  
  delay(10);
  
  // Read it back
  Serial.println("Reading back from register 0x19...");
  Wire.beginTransmission(addr);
  Wire.write(0x19);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 1, true);
  
  if (Wire.available()) {
    byte readVal = Wire.read();
    Serial.print("Read back: 0x");
    Serial.println(readVal, HEX);
    
    if (readVal == 0x07) {
      Serial.println("✓ Write/Read successful - I2C working!");
    } else {
      Serial.println("✗ Read value doesn't match - chip problem!");
    }
  }
  
  // Test 4: Full sensor register dump
  Serial.println("\nTEST 4: Sensor Register Dump");
  Serial.println("-----------------------------");
  
  // Wake up sensor
  Wire.beginTransmission(addr);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  delay(100);
  
  // Read important registers
  byte registers[] = {0x6B, 0x1B, 0x1C, 0x75, 0x19, 0x1A};
  String names[] = {"PWR_MGMT_1", "GYRO_CONFIG", "ACCEL_CONFIG", "WHO_AM_I", "SMPLRT_DIV", "CONFIG"};
  
  for (int i = 0; i < 6; i++) {
    Wire.beginTransmission(addr);
    Wire.write(registers[i]);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, 1, true);
    
    if (Wire.available()) {
      byte val = Wire.read();
      Serial.print(names[i]);
      Serial.print(" (0x");
      Serial.print(registers[i], HEX);
      Serial.print("): 0x");
      Serial.println(val, HEX);
    }
  }
  
  // Test 5: Read actual sensor data bytes
  Serial.println("\nTEST 5: Raw Byte Reading from Sensor");
  Serial.println("-------------------------------------");
  Serial.println("Reading 14 bytes starting from 0x3B...\n");
  
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(addr, 14, true);
  
  Serial.print("Bytes received: ");
  Serial.println(Wire.available());
  
  Serial.print("Raw bytes: ");
  while (Wire.available()) {
    byte b = Wire.read();
    if (b < 16) Serial.print("0");
    Serial.print(b, HEX);
    Serial.print(" ");
  }
  Serial.println("\n");
  
  if (Wire.available() == 0) {
    Serial.println("⚠ If all bytes are 00, the sensor chip is DEAD");
  }
  
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║           FINAL VERDICT            ║");
  Serial.println("╚════════════════════════════════════╝");
  Serial.println("If WHO_AM_I is correct BUT sensor data");
  Serial.println("is all zeros, your MPU6050 module is");
  Serial.println("DEFECTIVE and needs to be replaced.");
  Serial.println("\nThe chip responds to I2C but the");
  Serial.println("internal sensors are not working.\n");
}

void loop() {
  // Empty - all tests run once in setup
  delay(10000);
}

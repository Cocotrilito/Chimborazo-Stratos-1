#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>

Adafruit_BMP280 bmp;
Adafruit_MPU6050 mpu;
float currentHeight;
float initialPresure;
float maxAltitude = 0.0;
// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  // si hay errores
  if (!bmp.begin(0x76)) {
    Serial.println("!ERROR! NO BMP280");
    while (1);
  }
  initialPresure = bmp.readPressure() / 100.0;
  if (!mpu.begin()) {
    Serial.println("!ERROR! NO MPU");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);

}

void loop() {
  currentHeight = bmp.readAltitude(initialPresure);

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  //apogeo y logica
  if (currentHeight > maxAltitude) {
    maxAltitude = currentHeight;
  }

  if (currentHeight < (maxAltitude - 2.0)) {
    Serial.println("!!!!!MAX ALTITUDE DETECTED!!!!");
    Serial.println("STARTING DESCEND ");
  }
  
  
  Serial.print("Altitude: "); Serial.print(currentHeight);
  Serial.print(" | Accel Y:"); Serial.println(a.acceleration.y);

  delay(100);
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
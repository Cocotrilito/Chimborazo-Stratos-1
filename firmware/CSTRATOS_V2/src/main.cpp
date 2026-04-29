#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <ESP32Servo.h>
#include <MPU6050.h>


// pins
const int PIN_BAT_SENSE = 0;
const int PIN_SERVO1 = 1;
const int PIN_SERVO2 = 2;
const int PIN_PYRO = 21;
const int PIN_SDA = 22;
const int PIN_SCL = 23;
const int PIN_GPS_TX = 16;
const int PIN_GPS_RX = 17;
const int PIN_LED_ARMED = 20;
const int PIN_LED_ERROR = 18;

// altitude and ignition
float max_altitude = 0;
float ground_altitude = 0;
bool recovery_fired = false;


Adafruit_BMP085 bmp;
Servo servo1;
Servo servo2;
MPU6050 mpu;
int16_t ax, ay, az;
int16_t gx, gy, gz;

void setup() {
    Serial.begin(115200);
    pinMode(PIN_LED_ARMED, OUTPUT);
    pinMode(PIN_LED_ERROR, OUTPUT);
    pinMode(PIN_PYRO, OUTPUT);
    digitalWrite(PIN_PYRO, LOW);

    Wire.begin(PIN_SDA, PIN_SCL);
    delay(100);
    // BMP Altitude barometer
    if (!bmp.begin()) {
        digitalWrite(PIN_LED_ERROR, HIGH);
        Serial.println("Error No BMP");
        while (1);
    }

    float sum = 0;
    for(int i=0; i<10; i++) {
        sum += bmp.readAltitude();
        delay(50);
    }
    ground_altitude = sum / 10.0;

    // Accelerometer MPU6050
    Serial.println("Testing MPU");
    mpu.initialize();
    if (!mpu.testConnection()) {
        Serial.println("Error: MPU6050 no MPU");
        digitalWrite(PIN_LED_ERROR, HIGH);
    }
    // servos
    servo1.attach(PIN_SERVO1);
    servo2.attach(PIN_SERVO2);
    servo1.write(90); // 90 degrees
    servo2.write(90);

    // gps
    Serial1.begin(9600, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);

    digitalWrite(PIN_LED_ARMED, HIGH);
    Serial.println("System Ready");


}

void loop() {
    // bat sense
    int raw_bat = analogRead(PIN_BAT_SENSE);
    float voltage = (raw_bat / 4095.0) * 3.3 * 3.0;

    // altitude
    float current_alt = bmp.readAltitude();
    float relative_alt = current_alt - ground_altitude;

    if (relative_alt > max_altitude) {
        max_altitude = relative_alt;
    }

    // MPU
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);


    if (!recovery_fired) {
        // El Giroscopio nos da valores entre -32768 y 32767
        // Mapeamos el eje X (gx) para el Servo 1 y el eje Y (gy) para el Servo 2
        int angle1 = map(gx, -15000, 15000, 70, 110);
        int angle2 = map(gy, -15000, 15000, 70, 110);
        
        servo1.write(constrain(angle1, 70, 110));
        servo2.write(constrain(angle2, 70, 110));
    } else {
        // Si el paracaídas ya salió, ponemos los canards en posición neutral (90°)
        servo1.write(90);
        servo2.write(90);
    }

    if (max_altitude > 2.0 && (max_altitude - relative_alt) > 0.5 && !recovery_fired) {
        digitalWrite(PIN_PYRO, HIGH);
        Serial.println("!!DEPLOY!!");

        digitalWrite(PIN_LED_ARMED, LOW);
        digitalWrite(PIN_LED_ERROR, HIGH);

        delay(2000);
        digitalWrite(PIN_PYRO, LOW);
        recovery_fired = true;

    }

    Serial.printf("Alt: %.2f | Max: %.2f | Bat: %.2fV | Gx: %d\n", relative_alt, max_altitude, voltage, gx);
    
    if (voltage < 3.5) {
        digitalWrite(PIN_LED_ERROR, HIGH); 
    } else if (!recovery_fired) {
        digitalWrite(PIN_LED_ERROR, LOW);
    }

    delay(50);
}
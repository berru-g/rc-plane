#include <U8g2lib.h>
#include <ESP32Servo.h>

// --- OLED ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// --- ESC et Servos ---
Servo esc;
Servo servo1;
Servo servo2;

// --- Potentiomètres ---
const int potEscPin = 34;   // GPIO34 (A6) - ESC
const int potServo1Pin = 35; // GPIO35 (A7) - Servo 1
const int potServo2Pin = 32; // GPIO32 (A4) - Servo 2

int escValue = 1000;    // Valeur ESC (1000-2000µs)
int servo1Value = 0;   // Valeur Servo 1 (0-180°)
int servo2Value = 0;   // Valeur Servo 2 (0-180°)

void setup() {
  Serial.begin(115200);

  // Allouer les timers pour les servos/ESC
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Initialiser l'OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 20, "Test Moteur");
  u8g2.drawStr(10, 40, "+ 2 Servos");
  u8g2.sendBuffer();
  delay(1000);

  // Initialiser ESC et servos
  esc.attach(12);      // GPIO12
  servo1.attach(13);  // GPIO13
  servo2.attach(14);  // GPIO14

  // Arrêter tout au démarrage
  esc.writeMicroseconds(1000);    // ESC à 0%
  servo1.write(0);               // Servo 1 à 0°
  servo2.write(0);               // Servo 2 à 0°

  // Animation de démarrage
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10, 20, "Pret !");
  u8g2.drawStr(10, 40, "Tourne les potars");
  u8g2.sendBuffer();
  delay(1000);
}

void loop() {
  // Lire les potentiomètres (0-4095)
  int potEscValue = analogRead(potEscPin);
  int potServo1Value = analogRead(potServo1Pin);
  int potServo2Value = analogRead(potServo2Pin);

  // Mapper les valeurs
  escValue = map(potEscValue, 0, 4095, 1000, 2000);       // ESC: 1000-2000µs
  servo1Value = map(potServo1Value, 0, 4095, 0, 180);    // Servo 1: 0-180°
  servo2Value = map(potServo2Value, 0, 4095, 0, 180);    // Servo 2: 0-180°

  // Contrôler ESC et servos
  esc.writeMicroseconds(escValue);
  servo1.write(servo1Value);
  servo2.write(servo2Value);

  // Affichage OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(5, 10, "Test Moteur+Servos");
  u8g2.drawLine(0, 20, 128, 20);

  u8g2.setFont(u8g2_font_6x10_tr);

  // ESC
  u8g2.drawStr(0, 30, "ESC:");
  u8g2.drawStr(40, 30, String(map(potEscValue, 0, 4095, 0, 100)).c_str());
  u8g2.drawStr(70, 30, "%");
  u8g2.drawStr(90, 30, String(escValue).c_str());
  u8g2.drawStr(110, 30, "us");

  // Servo 1
  u8g2.drawStr(0, 45, "Servo1:");
  u8g2.drawStr(50, 45, String(servo1Value).c_str());
  u8g2.drawStr(70, 45, "deg");

  // Servo 2
  u8g2.drawStr(0, 60, "Servo2:");
  u8g2.drawStr(50, 60, String(servo2Value).c_str());
  u8g2.drawStr(70, 60, "deg");

  // Barres de progression
  u8g2.drawFrame(0, 70, 128, 5);
  u8g2.drawBox(0, 70, map(potEscValue, 0, 4095, 0, 128), 5);  // ESC

  u8g2.drawFrame(0, 78, 128, 5);
  u8g2.drawBox(0, 78, map(potServo1Value, 0, 4095, 0, 128), 5);  // Servo 1

  u8g2.drawFrame(0, 86, 128, 5);
  u8g2.drawBox(0, 86, map(potServo2Value, 0, 4095, 0, 128), 5);  // Servo 2

  u8g2.sendBuffer();
  delay(50);
}
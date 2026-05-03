/*
  Récepteur pour avion RC utilisant nRF24L01 et servos/ESC
  - Gouvernail (Rudder) sur D2
  - Profondeur (Elevator) sur D3
  - Moteur (ESC) sur D4

  Calibré ESC :
    esc.writeMicroseconds(1000); // Min throttle
    delay(1000);
    esc.writeMicroseconds(2000); // Max throttle
    delay(1000);
    esc.writeMicroseconds(1000); // Retour à min throttle
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// Servos
Servo servoRudder;   // Gouvernail (ch1)
Servo servoElevator; // Profondeur (ch2)
Servo esc;           // Moteur (ch3)

// Structure pour les données reçues (doit correspondre à l'émetteur)
struct Signal {
  byte throttle;  // Gaz (Joystick GAUCHE, haut/bas)
  byte pitch;     // Profondeur (Joystick DROIT, haut/bas)
  byte roll;      // Gouvernail (Joystick DROIT, gauche/droite)
  byte yaw;       // Non utilisé
  byte aux1;      // Non utilisé
  byte aux2;      // Non utilisé
};
Signal data;

const uint64_t pipeIn = 0xABCDABCD71LL; // Même adresse que l'émetteur

RF24 radio(9, 10); // CE, CSN

unsigned long lastRecvTime = 0;

void ResetData() {
  data.throttle = 0;    // Gaz à 0 (moteur arrêté)
  data.pitch = 127;     // Profondeur centrée
  data.roll = 127;      // Gouvernail centré
  data.yaw = 127;
  data.aux1 = 0;
  data.aux2 = 0;
}

void setup() {
  Serial.begin(115200);

  // Attache les servos/ESC aux broches
  servoRudder.attach(2);   // Gouvernail sur D2
  servoElevator.attach(3); // Profondeur sur D3
  esc.attach(4);            // ESC sur D4 (ou D5 si tu préfères)

  // Initialisation de l'ESC (calibration)
  esc.writeMicroseconds(1000); // Min throttle
  delay(1000);
  esc.writeMicroseconds(2000); // Max throttle
  delay(1000);
  esc.writeMicroseconds(1000); // Retour à min throttle

  // Initialisation du NRF24L01
  radio.begin();
  radio.openReadingPipe(1, pipeIn);
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();

  ResetData();
}

void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));
    lastRecvTime = millis();
  }
}

void loop() {
  recvData();

  // Si aucun signal reçu depuis 1 seconde, réinitialiser
  if (millis() - lastRecvTime > 1000) {
    ResetData();
  }

  // Mapper les valeurs 0-255 vers 1000-2000µs pour les servos/ESC
  int rudderValue = map(data.roll, 0, 255, 1000, 2000);    // Gouvernail
  int elevatorValue = map(data.pitch, 0, 255, 1000, 2000); // Profondeur
  int throttleValue = map(data.throttle, 0, 255, 1000, 2000); // Gaz

  // Écrire les valeurs sur les servos/ESC
  servoRudder.writeMicroseconds(rudderValue);
  servoElevator.writeMicroseconds(elevatorValue);
  esc.writeMicroseconds(throttleValue);

  // Affichage pour débogage (optionnel)
  Serial.print("Rudder: ");
  Serial.print(rudderValue);
  Serial.print(" | Elevator: ");
  Serial.print(elevatorValue);
  Serial.print(" | Throttle: ");
  Serial.println(throttleValue);
}
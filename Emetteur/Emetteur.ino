/*
  Emetteur pour ROV avec Arduino Nano et NRF24L01
  - Joystick GAUCHE (Y) → Gaz (throttle)
  - Joystick DROIT (Y) → Profondeur (pitch)
  - Joystick DROIT (X) → Gouvernail (roll)
  
  PCB connectés aux broches analogiques :
  - Joystick GAUCHE : A0 (Y), A1 (X)
  - Joystick DROIT : A2 (Y), A3 (X)
  
  Le code mappe les valeurs des joysticks de 0-1023 à 0-255 pour les envoyer via le module NRF24L01.
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xABCDABCD71LL; // Adresse radio (doit être la même que le récepteur)
RF24 radio(9, 10); // CE, CSN

struct Signal {
  byte throttle;  // Gaz (Joystick GAUCHE, haut/bas)
  byte pitch;     // Profondeur (Joystick DROIT, haut/bas)
  byte roll;      // Gouvernail (Joystick DROIT, gauche/droite)
  byte yaw;       // Non utilisé (peut être utilisé pour un 4ème canal)
  byte aux1;      // Non utilisé
  byte aux2;      // Non utilisé
};
Signal data;

void ResetData() {
  data.throttle = 0;    // Gaz à 0 (moteur arrêté)
  data.pitch = 127;     // Profondeur centrée
  data.roll = 127;      // Gouvernail centré
  data.yaw = 127;       // Non utilisé
  data.aux1 = 0;
  data.aux2 = 0;
}

void setup() {
  Serial.begin(115200); //115200
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS); // Pour une communication plus stable
  radio.setPALevel(RF24_PA_MAX);    // Puissance maximale pour une meilleure portée
  radio.stopListening();
  ResetData();
}

// Fonction pour mapper les valeurs des joysticks (0-1023 → 0-255)
int mapJoystick(int val, int lower, int middle, int upper) {
  val = constrain(val, lower, upper);
  if (val < middle) {
    return map(val, lower, middle, 0, 127); // De 0 à 127 (centre)
  } else {
    return map(val, middle, upper, 127, 255); // De 127 à 255
  }
}

void loop() {
  // Joystick GAUCHE (haut/bas) → Gaz (throttle)
  // Supposons que le joystick GAUCHE est branché sur A2 (Y) et A3 (X)
  // Ici, on utilise A2 pour le gaz (haut/bas)
  data.throttle = mapJoystick(analogRead(A1), 0, 512, 1023); // A1 depuis amélioration pcb

  // Joystick DROIT (haut/bas) → Profondeur (pitch)
  // Supposons que le joystick DROIT est branché sur A0 (Y) et A1 (X)
  data.pitch = mapJoystick(analogRead(A2), 0, 512, 1023);

  // Joystick DROIT (gauche/droite) → Gouvernail (roll)
  data.roll = mapJoystick(analogRead(A3), 0, 512, 1023);

  // Envoi des données
  if (radio.write(&data, sizeof(Signal))) {
    Serial.print("Throttle: ");
    Serial.print(data.throttle);
    Serial.print(" | Pitch: ");
    Serial.print(data.pitch);
    Serial.print(" | Roll: ");
    Serial.println(data.roll);
  }
  delay(50); // Envoi toutes les 50ms
}
// RC PLANE inspired by https://www.rcpano.net/2025/09/30/make-a-flying-wing-with-simple-materials-diy-rc-plane-remote-control/
// Transmetteur pour avion RC, 3 channel, 1 moteur et 2 servo. ESP32, NRF24L01 et OLED integré.
// revue de code : 
//   Adapté à l'ideaspark, optimisation du mappage des joysticks pour les plages de l'ESP32, ajout de messages d'erreur pour le NRF24L01, amélioration de l'affichage des commandes, et préparation pour les futures améliorations du circuit.
// Revue du circuit : 
//   Sécurité des GPIO, alimentation 7.4V réduit à 3.3V pouir le NRF24L01, ajout de condensateurs de découplage, protection contre les interférences électromagnétiques à venir...

#include <SPI.h>
#include <RF24.h>
#include <U8g2lib.h>

// OLED
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// NRF24L01
RF24 radio(9, 10); // CE, CSN (adapte si nécessaire)
const uint64_t pipeOut = 0xABCDABCD71LL;

// Structure des données
struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte aux1;
  byte aux2;
};
Signal data;

// Initialisation des données
void ResetData() {
  data.throttle = 0;
  data.pitch = 127;
  data.roll = 127;
  data.yaw = 127;
  data.aux1 = 0;
  data.aux2 = 0;
}

// Animation de chargement (inchangée)
void loadingAnimation() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20, 20, "Demarrage...");
  u8g2.sendBuffer();
  for (int i = 0; i <= 100; i += 5) {
    u8g2.clearBuffer();
    u8g2.drawStr(20, 20, "Demarrage...");
    u8g2.drawStr(20, 35, "Chargement:");
    u8g2.drawFrame(10, 45, 108, 10);
    u8g2.drawBox(10, 45, map(i, 0, 100, 0, 108), 10);
    char progressStr[10];
    snprintf(progressStr, 10, "%d%%", i);
    u8g2.drawStr(50, 60, progressStr);
    u8g2.sendBuffer();
    delay(50);
  }
}

// Message de bienvenue (inchangé)
void welcomeMessage() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x14B_tr);
  u8g2.drawStr(10, 20, "RC Plane");
  u8g2.drawStr(10, 40, "Transmetteur");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20, 60, "by berru-g 2026");
  u8g2.sendBuffer();
  delay(2000);
}

// Mappage des joysticks (adapté pour ESP32)
int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

// Affichage des commandes (inchangé)
void displayCommands() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(0, 10, "Commandes RC:");
  char line1[20], line2[20], line3[20], line4[20], line5[20], line6[20];
  snprintf(line1, 20, "Throttle: %d", data.throttle);
  snprintf(line2, 20, "Pitch: %d", data.pitch);
  snprintf(line3, 20, "Roll: %d", data.roll);
  snprintf(line4, 20, "Yaw: %d", data.yaw);
  snprintf(line5, 20, "Aux1: %d", data.aux1);
  snprintf(line6, 20, "Aux2: %d", data.aux2);
  u8g2.drawStr(0, 25, line1);
  u8g2.drawStr(0, 35, line2);
  u8g2.drawStr(0, 45, line3);
  u8g2.drawStr(0, 55, line4);
  u8g2.drawStr(70, 25, line5);
  u8g2.drawStr(70, 35, line6);
  u8g2.drawLine(0, 63, 128, 63);
  u8g2.drawLine(0, 64, map(data.throttle, 0, 255, 0, 128), 64);
  u8g2.drawLine(0, 64 - map(data.pitch, 0, 255, 0, 63), 0, 64);
  u8g2.drawLine(127, 64 - map(data.roll, 0, 255, 0, 63), 127, 64);
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();

  // Animation de chargement
  loadingAnimation();

  // Message de bienvenue
  welcomeMessage();

  // Initialisation NRF24
  if (!radio.begin()) {
    Serial.println("Erreur: NRF24 non détecté !");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "ERREUR: NRF24");
    u8g2.drawStr(0, 40, "non detecte!");
    u8g2.sendBuffer();
    while (1); // Bloque si le NRF24 n'est pas détecté
  }

  radio.openWritingPipe(pipeOut);
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();

  ResetData();
  Serial.println("NRF24 initialise avec succes !");
}

void loop() {
  // Lecture des commandes avec les bonnes broches ESP32
  // Remplace les GPIO ci-dessous par celles que tu utilises réellement !
  data.roll   = Border_Map(analogRead(34), 0, 2048, 4095, true);   // CH1 (ex: GPIO34)
  data.pitch  = Border_Map(analogRead(35), 0, 2048, 4095, true);   // CH2 (ex: GPIO35)
  data.throttle = Border_Map(analogRead(32), 0, 1360, 2200, true);  // CH3 (ex: GPIO32, adaptée pour ESC single side)
  data.yaw    = Border_Map(analogRead(33), 0, 2048, 4095, false);  // CH4 (ex: GPIO33)
  data.aux1   = digitalRead(16);                                    // CH5 (ex: GPIO16, évite GPIO0)
  data.aux2   = digitalRead(17);                                    // CH6 (ex: GPIO17, évite GPIO3)

  // Envoi des données
  if (!radio.write(&data, sizeof(Signal))) {
    Serial.println("Erreur: Echec de l'envoi des donnees");
    u8g2.clearBuffer();
    u8g2.drawStr(0, 20, "Erreur: Echec");
    u8g2.drawStr(0, 40, "envoi NRF24!");
    u8g2.sendBuffer();
    delay(1000);
  }

  // Affichage des commandes sur l'OLED
  displayCommands();
  delay(50);
}
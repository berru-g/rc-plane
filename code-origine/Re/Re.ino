// 6-Channel Delta Mix & V-Tail mix Receiver | 6 kanal Delta Miks & V-tail mix Alıcı (FOR ARDUINO MICRO PRO)

#include <SPI.h>          // Include SPI library for communication with the nRF24L01 module | nRF24L01 modülü ile iletişim için SPI kütüphanesini ekle
#include <nRF24L01.h>     // Include nRF24L01 library for nRF24L01 module | nRF24L01 modülü için kütüphaneyi ekle
#include <RF24.h>         // Include RF24 library for radio communication | Radyo iletişimi için RF24 kütüphanesini ekle
#include <Servo.h>        // Include Servo library for controlling servos | Servo motorları kontrol etmek için Servo kütüphanesini ekle

// Variables to store the pulse width for each channel | Her kanal için darbe genişliğini saklayacak değişkenler
int ch_width_1 = 0;
int ch_width_2 = 0;
int ch_width_3 = 0;
int ch_width_4 = 0;
int ch_width_5 = 0;
int ch_width_6 = 0;

// Servo objects for each channel | Her kanal için Servo nesneleri
Servo ch1;
Servo ch2;
Servo ch3;
Servo ch4;
Servo ch5;
Servo ch6;

// Structure to store the received signal data | Alınan sinyal verilerini saklamak için yapı
struct Signal {
  byte throttle;  // Throttle channel | Gaz kanalı
  byte pitch;     // Pitch channel   | Pitch kanalı
  byte roll;      // Roll channel    | Roll kanalı
  byte yaw;       // Yaw channel     | Yaw kanalı
  byte aux1;      // Auxiliary channel 1 | Yardımcı kanal 1
  byte aux2;      // Auxiliary channel 2 | Yardımcı kanal 2
};

Signal data;  // Create an instance of the Signal structure | Signal yapısının bir örneğini oluştur

// Define the radio communication pipe address | Radyo iletişim boru adresini tanımla
const uint64_t pipeIn = 0xABCDABCD71LL;

// Create an RF24 radio object with pins 9 (CE) and 10 (CSN) | 9 (CE) ve 10 (CSN) pinleri ile bir RF24 radyo nesnesi oluştur
RF24 radio(9, 10);

// Function to reset the signal data to default values | Sinyal verilerini varsayılan değerlere sıfırlayan fonksiyon
void ResetData() {
  data.throttle = 0;   // Set throttle to 0 | Gazı 0'a ayarla
  data.roll = 127;     // Set roll to neutral (127) | Roll'u nötr (127) yap
  data.pitch = 127;    // Set pitch to neutral (127) | Pitch'i nötr (127) yap
  data.yaw = 127;      // Set yaw to neutral (127) | Yaw'ı nötr (127) yap
  data.aux1 = 0;       // Set auxiliary channel 1 to 0 | Yardımcı kanal 1'i 0'a ayarla
  data.aux2 = 0;       // Set auxiliary channel 2 to 0 | Yardımcı kanal 2'yi 0'a ayarla
}

void setup() {
  // Attach servos to respective pins | Servo motorları ilgili pinlere bağla
  ch1.attach(2);
  ch2.attach(3);
  ch3.attach(4);
  ch4.attach(5);
  ch5.attach(6);
  ch6.attach(7);

  ResetData();  // Reset signal data to default | Sinyal verilerini varsayılana sıfırla

  // Initialize the radio module | Radyo modülünü başlat
  radio.begin();
  radio.openReadingPipe(1, pipeIn);  // Open the reading pipe | Okuma borusunu aç
  radio.setChannel(100);             // Set the radio channel to 100 | Radyo kanalını 100'e ayarla
  radio.setAutoAck(false);           // Disable auto-acknowledgment | Otomatik onaylamayı devre dışı bırak
  radio.setDataRate(RF24_250KBPS);   // Set data rate to 250 kbps | Veri hızını 250 kbps'ye ayarla
  radio.setPALevel(RF24_PA_MAX);     // Set power amplifier level to maximum | Güç amplifikatör seviyesini maksimuma ayarla
  radio.startListening();            // Start listening for incoming data | Gelen verileri dinlemeye başla
}

unsigned long lastRecvTime = 0;  // Variable to store the last received data time | Son alınan veri zamanını saklayan değişken

// Function to receive data from the radio module | Radyo modülünden veri alan fonksiyon
void recvData() {
  while (radio.available()) {
    radio.read(&data, sizeof(Signal));  // Read the data into the Signal structure | Veriyi Signal yapısına oku
    lastRecvTime = millis();            // Update the last received time | Son alınan zamanı güncelle
  }
}

void loop() {
  recvData();  // Receive data from the radio | Radyodan veri al

  unsigned long now = millis();  // Get the current time | Şu anki zamanı al
  if (now - lastRecvTime > 1000) {
    ResetData();  // If no data received for 1 second, reset the data | 1 saniye boyunca veri alınmazsa verileri sıfırla
  }

  // V-tail mixing process | V-tail miksleme işlemi
  int pitchValue = map(data.pitch, 0, 255, -150, 150);  // Scale pitch value between -150 and 150 | Pitch değerini -150 ile 150 arasına ölçekle
  int yawValue = map(data.yaw, 0, 255, -150, 150);      // Scale yaw value between -150 and 150 | Yaw değerini -150 ile 150 arasına ölçekle

  // Mixed values for channels 2 and 4 | 2. ve 4. kanallar için mikslenmiş değerler
  int ch2Value = pitchValue - yawValue;  // Channel 2: Pitch - Yaw | 2. Kanal: Pitch - Yaw
  int ch4Value = pitchValue + yawValue;  // Channel 4: Pitch + Yaw | 4. Kanal: Pitch + Yaw

  // Map the mixed values to servo pulse width (1000-2000 microseconds) | Karıştırılmış değerleri servo darbe genişliğine (1000-2000 mikrosaniye) dönüştür
  ch_width_2 = map(ch2Value, -300, 300, 1000, 2000);  // Map ch2Value to pulse width | ch2Value'yi darbe genişliğine dönüştür
  ch_width_4 = map(ch4Value, -300, 300, 2000, 1000);  // Map ch4Value to pulse width (reversed direction) | ch4Value'yi darbe genişliğine dönüştür (ters yön)

  // Map other channels to servo pulse width | Diğer kanalları servo darbe genişliğine dönüştür
  ch_width_1 = map(data.roll, 0, 255, 1000, 2000);      // Roll channel | Roll kanalı
  ch_width_3 = map(data.throttle, 0, 255, 1000, 2000);  // Throttle channel | Gaz kanalı
  ch_width_5 = map(data.aux1, 0, 1, 1000, 2000);        // Auxiliary channel 1 | Yardımcı kanal 1
  ch_width_6 = map(data.aux2, 0, 1, 1000, 2000);        // Auxiliary channel 2 | Yardımcı kanal 2

  // Send PWM signals to the servos | Servolara PWM sinyallerini gönder
  ch1.writeMicroseconds(ch_width_1);
  ch2.writeMicroseconds(ch_width_2);
  ch3.writeMicroseconds(ch_width_3);
  ch4.writeMicroseconds(ch_width_4);
  ch5.writeMicroseconds(ch_width_5);
  ch6.writeMicroseconds(ch_width_6);
}
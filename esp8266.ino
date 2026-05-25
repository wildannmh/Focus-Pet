// Library untuk menghubungkan ESP8266 ke jaringan WiFi
#include <ESP8266WiFi.h>

// Library untuk komunikasi WebSocket client
#include <WebSocketsClient.h>

// Library untuk parsing dan membuat data JSON
#include <ArduinoJson.h>

// Library untuk membuat komunikasi serial tambahan (software serial)
#include <SoftwareSerial.h>


// ======================================================
// BAGIAN KONFIGURASI WIFI DAN SERVER
// ======================================================

// Nama WiFi yang akan digunakan ESP8266 untuk terhubung ke internet/jaringan lokal
const char* ssid = "1390";

// Password WiFi
const char* password = "torabika";

// IP Address laptop/server Python yang menjalankan WebSocket server
// Ganti sesuai IP laptop masing-masing
const char* websocket_server = "192.168.1.12";

// Port WebSocket server Python
const int websocket_port = 8765;


// ======================================================
// PEMBUATAN OBJECT
// ======================================================

// Membuat object WebSocket client
WebSocketsClient webSocket;

// Membuat komunikasi serial tambahan menggunakan SoftwareSerial
// Parameter pertama = RX ESP8266
// Parameter kedua = TX ESP8266
//
// GPIO5 = D1 -> sebagai RX
// GPIO4 = D2 -> sebagai TX
//
// Jalur ini digunakan untuk komunikasi dengan Arduino Uno
SoftwareSerial arduinoSerial(5, 4); 


// ======================================================
// FUNCTION CALLBACK WEBSOCKET
// Function ini otomatis dipanggil ketika ada event
// dari WebSocket (misalnya menerima pesan)
// ======================================================

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  // Mengecek apakah data yang diterima berupa TEXT
  // (bukan binary atau koneksi)
  if (type == WStype_TEXT) {

    // Membuat object JSON sementara dengan kapasitas 200 byte
    StaticJsonDocument<200> doc;

    // Parsing data JSON dari payload WebSocket
    DeserializationError error = deserializeJson(doc, payload);
    
    // Jika parsing JSON berhasil
    if (!error) {

      // Mengambil value dari key "status"
      // Contoh JSON:
      // {"status":"focused"}
      const char* status = doc["status"];

      // Jika status = distracted
      if (strcmp(status, "distracted") == 0) {

        // Kirim karakter 'D' ke Arduino Uno
        // D = Distracted
        arduinoSerial.print("D");

      // Jika status = focused
      } else if (strcmp(status, "focused") == 0) {

        // Kirim karakter 'F' ke Arduino Uno
        // F = Focused
        arduinoSerial.print("F");
      }
    }
  }
}


// ======================================================
// SETUP
// Function ini dijalankan sekali saat ESP pertama hidup
// ======================================================

void setup() {

  // Membuka komunikasi serial utama ke PC
  // Baudrate 115200 digunakan untuk debugging di Serial Monitor
  Serial.begin(115200);

  // Membuka komunikasi serial ke Arduino Uno
  // Baudrate harus sama dengan yang ada di Arduino Uno
  arduinoSerial.begin(9600);


  // ==================================================
  // MENGHUBUNGKAN ESP KE WIFI
  // ==================================================

  // Memulai koneksi WiFi menggunakan SSID dan password
  WiFi.begin(ssid, password);

  // Selama ESP belum terhubung ke WiFi
  while (WiFi.status() != WL_CONNECTED) {

    // Tunggu 500 ms
    delay(500);

    // Cetak titik di Serial Monitor sebagai indikator loading
    Serial.print(".");
  }

  // Jika berhasil terhubung
  Serial.println("\nWiFi Terhubung!");


  // ==================================================
  // KONFIGURASI WEBSOCKET CLIENT
  // ==================================================

  // Memulai koneksi WebSocket ke server
  //
  // Parameter:
  // websocket_server = IP laptop/server
  // websocket_port   = port server
  // "/"               = endpoint/path WebSocket
  webSocket.begin(websocket_server, websocket_port, "/");

  // Menentukan function callback yang akan dijalankan
  // saat ada event WebSocket
  webSocket.onEvent(webSocketEvent);

  // Jika koneksi terputus, coba reconnect setiap 5 detik
  webSocket.setReconnectInterval(5000);
}


// ======================================================
// LOOP
// Function ini berjalan terus menerus
// ======================================================

void loop() {

  // Menjalankan proses WebSocket
  // Wajib dipanggil terus agar koneksi tetap aktif
  // dan bisa menerima data dari server
  webSocket.loop();
}
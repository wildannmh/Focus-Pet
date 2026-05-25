// Library untuk komunikasi I2C
#include <Wire.h>

// Library untuk mengontrol LCD I2C
#include <LiquidCrystal_I2C.h>

// Library untuk membuat komunikasi serial tambahan
#include <SoftwareSerial.h>


// ======================================================
// KONFIGURASI PIN
// ======================================================

// Pin potensiometer terhubung ke A0
#define POT_PIN A0

// Pin tombol push button
#define BUTTON_PIN 2

// Pin buzzer
#define BUZZER_PIN 8

// Membuat komunikasi serial tambahan
//
// RX Arduino Uno = pin 3
// TX Arduino Uno = pin 4
//
// Digunakan untuk komunikasi dengan ESP8266
SoftwareSerial espSerial(3, 4); 


// ======================================================
// INISIALISASI LCD
// ======================================================

// Membuat object LCD I2C
//
// 0x27 = alamat I2C LCD
// 16 = jumlah kolom
// 2  = jumlah baris
LiquidCrystal_I2C lcd(0x27, 16, 2); 


// ======================================================
// VARIABEL STATE MACHINE
// ======================================================

// Membuat enum (daftar kondisi/state sistem)
//
// SETUP      = mode pengaturan waktu
// POMODORO   = mode fokus/timer berjalan
// DISTRACTED = mode terganggu/tidak fokus
// DONE       = timer selesai
enum State { SETUP, POMODORO, DISTRACTED, DONE };


// Variabel untuk menyimpan state aktif sistem
// Awal sistem dimulai dari SETUP
State systemState = SETUP;


// Variabel untuk menyimpan waktu countdown
// Satuan dalam detik
long countdownTime = 0;


// Variabel penyimpan waktu sebelumnya
// Digunakan untuk timer non-blocking dengan millis()
unsigned long previousMillis = 0;


// Penanda apakah user sedang terdistraksi
//
// false = fokus
// true  = terdistraksi
bool isDistracted = false;


// ======================================================
// SETUP
// Fungsi ini dijalankan sekali saat Arduino menyala
// ======================================================

void setup() {

  // Membuka komunikasi serial ke PC
  // Untuk debugging melalui Serial Monitor
  Serial.begin(9600);

  // Membuka komunikasi serial ke ESP8266
  espSerial.begin(9600); 


  // ==================================================
  // KONFIGURASI MODE PIN
  // ==================================================

  // Tombol menggunakan INPUT_PULLUP
  //
  // Artinya:
  // - Kondisi normal = HIGH
  // - Saat ditekan = LOW
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Pin buzzer sebagai output
  pinMode(BUZZER_PIN, OUTPUT);


  // ==================================================
  // INISIALISASI LCD
  // ==================================================

  // Memulai LCD
  lcd.init();

  // Menyalakan backlight LCD
  lcd.backlight();
}


// ======================================================
// LOOP
// Fungsi utama yang berjalan terus menerus
// ======================================================

void loop() {

  // ==================================================
  // 1. CEK PESAN DARI ESP8266
  // ==================================================

  // Jika ada data masuk dari ESP8266
  if (espSerial.available() > 0) {

    // Membaca 1 karakter data
    char cmd = espSerial.read();

    // Jika menerima karakter 'D'
    // berarti user terdistraksi
    if (cmd == 'D') isDistracted = true;

    // Jika menerima karakter 'F'
    // berarti user kembali fokus
    if (cmd == 'F') isDistracted = false;
  }


  // ==================================================
  // 2. LOGIKA STATE MACHINE
  // ==================================================

  // Mengecek state sistem saat ini
  switch (systemState) {
    

    // ==================================================
    // STATE SETUP
    // MODE PENGATURAN WAKTU
    // ==================================================
    
    case SETUP:

      // Membaca nilai potensiometer
      // Nilai ADC 0 - 1023
      //
      // Diubah menjadi 1 - 60 menit
      //
      // Kemudian dikali 60 agar menjadi detik
      countdownTime = map(analogRead(POT_PIN), 0, 1023, 1, 60) * 60; 
      

      // Menampilkan teks di baris pertama LCD
      lcd.setCursor(0, 0);
      lcd.print("(^ _ ^) SET TIME");


      // Menampilkan durasi di baris kedua
      lcd.setCursor(0, 1);
      lcd.print("Durasi: ");


      // Jika menit kurang dari 10
      // tambahkan angka 0 di depan
      if (countdownTime / 60 < 10) lcd.print("0");

      // Menampilkan jumlah menit
      lcd.print(countdownTime / 60);

      // Menampilkan satuan menit
      lcd.print(" Mnt  ");


      // Jika tombol ditekan
      if (digitalRead(BUTTON_PIN) == LOW) {

        // Delay kecil untuk debounce tombol
        delay(300);

        // Pindah ke state POMODORO
        systemState = POMODORO;

        // Membersihkan LCD
        lcd.clear();
      }
      break;


    // ==================================================
    // STATE POMODORO
    // MODE FOKUS / TIMER BERJALAN
    // ==================================================

    case POMODORO:

      // Jika user terdistraksi
      if (isDistracted) {

        // Pindah ke state DISTRACTED
        systemState = DISTRACTED;

        // Bersihkan LCD
        lcd.clear();

        // Keluar dari state ini
        break;
      }


      // Memastikan buzzer mati saat fokus
      noTone(BUZZER_PIN);


      // ==================================================
      // TIMER MUNDUR TANPA DELAY
      // ==================================================

      // Jika sudah lewat 1 detik
      if (millis() - previousMillis >= 1000) {

        // Simpan waktu sekarang
        previousMillis = millis();

        // Jika waktu masih ada
        if (countdownTime > 0)

          // Kurangi 1 detik
          countdownTime--;

        // Jika waktu habis
        else {

          // Pindah ke state DONE
          systemState = DONE;

          // Bersihkan LCD
          lcd.clear();
        }
      }


      // ==================================================
      // TAMPILAN LCD MODE FOKUS
      // ==================================================

      // Menampilkan wajah fokus
      lcd.setCursor(0, 0);
      lcd.print("(O _ O) FOKUS...");


      // Menampilkan sisa waktu
      lcd.setCursor(0, 1);
      lcd.print("Sisa:   ");


      // Jika menit kurang dari 10
      if (countdownTime / 60 < 10) lcd.print("0");

      // Menampilkan menit
      lcd.print(countdownTime / 60);

      // Pemisah menit dan detik
      lcd.print(":");


      // Jika detik kurang dari 10
      if (countdownTime % 60 < 10) lcd.print("0");

      // Menampilkan detik
      lcd.print(countdownTime % 60);

      // Spasi tambahan untuk membersihkan sisa karakter
      lcd.print("    ");

      break;


    // ==================================================
    // STATE DISTRACTED
    // MODE SAAT USER TIDAK FOKUS
    // ==================================================

    case DISTRACTED:

      // ==================================================
      // BUZZER BERBUNYI BERKEDIP
      // ==================================================

      // Menggunakan millis() agar buzzer berbunyi
      // secara ritmis setiap 250 ms
      if ((millis() / 250) % 2 == 0)

        // Nyalakan buzzer dengan frekuensi 1000 Hz
        tone(BUZZER_PIN, 1000); 

      else

        // Matikan buzzer
        noTone(BUZZER_PIN);


      // ==================================================
      // TAMPILAN LCD MODE DISTRACTED
      // ==================================================

      lcd.setCursor(0, 0);
      lcd.print("(> _ <) MARAH!!!");

      lcd.setCursor(0, 1);
      lcd.print("KEMBALI KERJA!  ");


      // Jika user kembali fokus
      if (!isDistracted) {

        // Kembali ke mode POMODORO
        systemState = POMODORO;

        // Bersihkan LCD
        lcd.clear();
      }

      break;


    // ==================================================
    // STATE DONE
    // MODE TIMER SELESAI
    // ==================================================

    case DONE:

      // Menampilkan pesan selesai
      lcd.setCursor(0, 0);
      lcd.print("(^ o ^) SELESAI!");

      lcd.setCursor(0, 1);
      lcd.print("Tekan Tombol... ");
      

      // Membunyikan buzzer pendek
      //
      // 1500 = frekuensi bunyi
      // 200  = durasi bunyi dalam ms
      tone(BUZZER_PIN, 1500, 200); 


      // Jika tombol ditekan
      if (digitalRead(BUTTON_PIN) == LOW) {

        // Delay debounce tombol
        delay(300);

        // Kembali ke state SETUP
        systemState = SETUP;

        // Bersihkan LCD
        lcd.clear();
      }

      break;
  }
}
# Focus-Pet (Hybrid AI Desk Companion)

## Deskripsi Proyek

**Focus-Pet** adalah sistem monitoring fokus berbasis AI yang menggabungkan:

* **Python + MediaPipe** → mendeteksi keberadaan wajah user menggunakan webcam
* **ESP8266** → menerima status fokus melalui WebSocket
* **Arduino Uno** → menjalankan Pomodoro Timer, LCD, dan buzzer

Sistem ini dirancang untuk membantu user tetap fokus saat belajar atau bekerja menggunakan konsep **Pomodoro Timer** dengan pendamping virtual berbentuk “pet”.

---

# Youtube Video

[![Focus-Pet](https://img.youtube.com/vi/g2_aVf9HheE/maxresdefault.jpg)](https://youtu.be/g2_aVf9HheE)

---

# Cara Kerja Sistem

## Alur Sistem

1. Webcam laptop menangkap wajah user
2. Python menggunakan **MediaPipe Face Detection**
3. Jika wajah hilang lebih dari 5 detik:

   * status berubah menjadi `"distracted"`
4. Python mengirim status melalui **WebSocket**
5. ESP8266 menerima status tersebut
6. ESP8266 mengirim karakter:

   * `F` → focused
   * `D` → distracted
7. Arduino Uno:

   * Menampilkan status di LCD
   * Membunyikan buzzer jika user tidak fokus
   * Menjalankan timer Pomodoro

---

# Teknologi yang Digunakan

## Python Side

* Python 3.x
* OpenCV
* MediaPipe
* AsyncIO
* WebSockets

## Hardware Side

* Arduino Uno
* ESP8266 NodeMCU
* LCD I2C 16x2
* Potensiometer
* Push Button
* Buzzer
* Webcam Laptop

---

# Struktur Sistem

```text
+-------------------+
|   Laptop Python   |
|-------------------|
| OpenCV Webcam     |
| MediaPipe AI      |
| WebSocket Server  |
+---------+---------+
          |
          | WiFi
          |
+---------v---------+
|      ESP8266      |
|-------------------|
| WebSocket Client  |
| Serial ke Arduino |
+---------+---------+
          |
          | Serial
          |
+---------v---------+
|    Arduino Uno    |
|-------------------|
| LCD I2C           |
| Pomodoro Timer    |
| Buzzer            |
+-------------------+
```

---

# Wiring Hardware

## Arduino Uno

| Komponen      | Pin Arduino |
| ------------- | ----------- |
| Potensiometer | A0          |
| Push Button   | D2          |
| Buzzer        | D8          |
| LCD SDA       | A4          |
| LCD SCL       | A5          |
| ESP TX        | D3          |
| ESP RX        | D4          |

---

## ESP8266 NodeMCU

| ESP8266 | Arduino    |
| ------- | ---------- |
| D1 (RX) | TX Arduino |
| D2 (TX) | RX Arduino |
| GND     | GND        |

---

# Instalasi Python

## 1. Install Python

Gunakan Python versi 3.10 atau lebih baru.

---

## 2. Install Dependency

Buka terminal lalu jalankan:

```bash
pip install opencv-python mediapipe websockets
```

---

# Menjalankan Sistem

## 1. Jalankan Python Server

```bash
python main.py
```

Saat pertama dijalankan:

* model AI akan otomatis di-download
* webcam akan terbuka
* server websocket aktif di port `8765`

---

## 2. Upload Arduino Code

Upload file:

```text
arduino.ino
```

menggunakan Arduino IDE.

---

## 3. Upload ESP8266 Code

Upload file ESP8266 menggunakan board:

```text
NodeMCU 1.0 (ESP-12E Module)
```

---

# Konfigurasi WiFi ESP8266

Pada file ESP8266:

```cpp
const char* ssid = "NAMA_WIFI";
const char* password = "PASSWORD_WIFI";
```

---

# Konfigurasi IP Laptop

Ubah bagian berikut:

```cpp
const char* websocket_server = "192.168.1.12";
```

menjadi IP laptop/server Python.

## Cara Melihat IP Laptop

### Windows

```bash
ipconfig
```

Cari:

```text
IPv4 Address
```

Contoh:

```text
192.168.1.12
```

---

# Penjelasan AI Vision

## MediaPipe Face Detection

Program menggunakan model:

```text
blaze_face_short_range.tflite
```

Model ini digunakan untuk:

* mendeteksi wajah secara real-time
* ringan untuk CPU
* cocok untuk webcam laptop

---

# Penjelasan State Machine Arduino

Sistem Arduino menggunakan:

```cpp
enum State { SETUP, POMODORO, DISTRACTED, DONE };
```

---

## 1. SETUP

Mode pengaturan waktu Pomodoro menggunakan potensiometer.

LCD:

```text
(^ _ ^) SET TIME
Durasi: XX Mnt
```

---

## 2. POMODORO

Mode fokus.

* timer berjalan
* buzzer mati
* AI memonitor user

LCD:

```text
(O _ O) FOKUS...
Sisa: MM:SS
```

---

## 3. DISTRACTED

Aktif saat wajah hilang lebih dari 5 detik.

* buzzer berbunyi
* LCD memberi peringatan

LCD:

```text
(> _ <) MARAH!!!
KEMBALI KERJA!
```

---

## 4. DONE

Aktif saat timer selesai.

LCD:

```text
(^ o ^) SELESAI!
Tekan Tombol...
```

---

# Komunikasi Antar Device

## Python → ESP8266

Menggunakan:

```text
WebSocket + JSON
```

Contoh payload:

```json
{
  "status": "focused"
}
```

atau

```json
{
  "status": "distracted"
}
```

---

## ESP8266 → Arduino

Menggunakan serial komunikasi.

| Karakter | Arti       |
| -------- | ---------- |
| F        | Focused    |
| D        | Distracted |

---

# Sistem Toleransi Distraksi

User tidak langsung dianggap terdistraksi.

Program memberikan toleransi:

```python
TOLERANCE_SECONDS = 5
```

Artinya:

* jika wajah hilang kurang dari 5 detik → masih dianggap aman
* jika lebih dari 5 detik → mode DISTRACTED aktif

---

# Tampilan Webcam

## Fokus

```text
STATUS: FOKUS
```

warna hijau.

---

## Warning

```text
Peringatan... 3s
```

warna kuning.

---

## Distracted

```text
STATUS: TERDISTRAKSI!
```

warna merah.

---

# Library Arduino yang Dibutuhkan

Install melalui Arduino IDE:

## Arduino Uno

* LiquidCrystal_I2C
* SoftwareSerial

---

## ESP8266

Install board ESP8266 terlebih dahulu.

Library:

* ESP8266WiFi
* WebSockets
* ArduinoJson
* SoftwareSerial

---

# Pengujian Sistem

## Skenario Fokus

1. User berada di depan webcam
2. Timer berjalan normal
3. Buzzer mati

---

## Skenario Distraksi

1. User meninggalkan webcam
2. Setelah 5 detik:

   * buzzer menyala
   * LCD marah
   * state berubah menjadi DISTRACTED

---

## Skenario Kembali Fokus

1. User kembali ke depan webcam
2. Status kembali:

   * focused
   * buzzer mati
   * timer lanjut

---

# Dokumentasi



---

# Troubleshooting

## Webcam Tidak Terdeteksi

Ubah:

```python
cv2.VideoCapture(0)
```

menjadi:

```python
cv2.VideoCapture(1)
```

---

## ESP8266 Tidak Connect

Periksa:

* SSID
* Password
* IP laptop
* Firewall Windows

---

## LCD Tidak Menyala

Periksa alamat I2C:

```cpp
LiquidCrystal_I2C lcd(0x27, 16, 2);
```

Kadang menggunakan:

```cpp
0x3F
```

---

# Pengembangan Selanjutnya

Beberapa pengembangan yang bisa dilakukan:

* Eye tracking
* Deteksi kantuk
* Statistik fokus harian
* Integrasi mobile app
* Animasi pet OLED/TFT
* Voice reminder
* Cloud dashboard

---

# Tim Pengembang

## Focus-Pet

* Wildan Munawwar Habib
* Fawwaz Aufa Al Ghautsa Rafi

Teknik Informatika - Universitas Jenderal Soedirman

---

# 📄 Lisensi

Project ini dibuat untuk kebutuhan pembelajaran dan pengembangan akademik.

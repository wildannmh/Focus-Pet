# ======================================================
# IMPORT LIBRARY
# ======================================================

# Library OpenCV untuk pengolahan gambar dan webcam
import cv2

# Library utama MediaPipe
import mediapipe as mp

# Import modul task MediaPipe untuk Python
from mediapipe.tasks import python

# Import modul vision MediaPipe
from mediapipe.tasks.python import vision

# Library asyncio untuk menjalankan proses asynchronous
import asyncio

# Library WebSocket server
import websockets

# Library untuk membuat dan membaca JSON
import json

# Library untuk pengolahan waktu
import time

# Library untuk download file dari internet
import urllib.request

# Library untuk operasi file/folder
import os


# ======================================================
# DOWNLOAD MODEL FACE DETECTION
# ======================================================

# Nama file model AI MediaPipe
model_path = 'blaze_face_short_range.tflite'


# Jika file model belum ada
if not os.path.exists(model_path):

    # Tampilkan pesan download
    print(f"Downloading model to {model_path}...")

    # URL model MediaPipe
    url = f"https://storage.googleapis.com/mediapipe-models/face_detector/blaze_face_short_range/float16/1/{model_path}"

    # Download file model
    urllib.request.urlretrieve(url, model_path)

    # Tampilkan pesan selesai download
    print("Download complete.")


# ======================================================
# INISIALISASI MEDIAPIPE FACE DETECTION
# ======================================================

# Mengambil class BaseOptions dari MediaPipe
BaseOptions = mp.tasks.BaseOptions

# Mengambil class FaceDetector
FaceDetector = mp.tasks.vision.FaceDetector

# Mengambil class konfigurasi FaceDetector
FaceDetectorOptions = mp.tasks.vision.FaceDetectorOptions

# Mengambil mode running vision
VisionRunningMode = mp.tasks.vision.RunningMode


# Membuat konfigurasi detector
options = FaceDetectorOptions(

    # Menentukan lokasi file model AI
    base_options=BaseOptions(model_asset_path=model_path),

    # Confidence minimum deteksi wajah
    # Semakin tinggi semakin ketat
    min_detection_confidence=0.7,

    # Menggunakan mode IMAGE
    # karena setiap frame diproses sebagai gambar
    running_mode=VisionRunningMode.IMAGE
)


# Membuat object face detector
face_detection = FaceDetector.create_from_options(options)


# ======================================================
# VARIABEL GLOBAL STATUS
# ======================================================

# Status awal user dianggap fokus
current_status = "focused"

# Waktu awal user mulai terdistraksi
distracted_start_time = None

# Waktu toleransi sebelum dianggap terdistraksi penuh
# dalam satuan detik
TOLERANCE_SECONDS = 5


# ======================================================
# FUNCTION COMPUTER VISION
# ======================================================

async def ai_vision():

    # Menggunakan variabel global
    global current_status, distracted_start_time

    # Membuka webcam default laptop
    # 0 = webcam utama/default
    cap = cv2.VideoCapture(0)


    # Selama webcam masih aktif
    while cap.isOpened():

        # Membaca frame dari webcam
        success, image = cap.read()

        # Jika gagal membaca frame
        if not success:
            continue


        # ==================================================
        # KONVERSI GAMBAR KE RGB
        # ==================================================

        # OpenCV menggunakan format BGR
        # MediaPipe membutuhkan RGB
        image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)


        # Mengubah frame menjadi format MediaPipe Image
        mp_image = mp.Image(
            image_format=mp.ImageFormat.SRGB,
            data=image_rgb
        )


        # ==================================================
        # DETEKSI WAJAH
        # ==================================================

        # Melakukan deteksi wajah
        results = face_detection.detect(mp_image)


        # ==================================================
        # JIKA WAJAH TERDETEKSI
        # ==================================================

        if results.detections:

            # User dianggap fokus
            current_status = "focused"

            # Reset waktu distraksi
            distracted_start_time = None


            # Menampilkan teks status di layar webcam
            cv2.putText(
                image,                         # gambar
                "STATUS: FOKUS",              # teks
                (20, 50),                     # posisi
                cv2.FONT_HERSHEY_SIMPLEX,     # font
                1,                            # ukuran font
                (0, 255, 0),                  # warna hijau (BGR)
                2                             # ketebalan teks
            )


        # ==================================================
        # JIKA WAJAH TIDAK TERDETEKSI
        # ==================================================

        else:

            # Jika baru pertama kali tidak terdeteksi
            if distracted_start_time is None:

                # Simpan waktu mulai distraksi
                distracted_start_time = time.time()


            # Menghitung lama user tidak terdeteksi
            elapsed_time = time.time() - distracted_start_time


            # Jika melewati batas toleransi
            if elapsed_time > TOLERANCE_SECONDS:

                # User dianggap terdistraksi
                current_status = "distracted"


                # Tampilkan peringatan merah
                cv2.putText(
                    image,
                    "STATUS: TERDISTRAKSI!",
                    (20, 50),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1,
                    (0, 0, 255),  # merah
                    2
                )


            # Jika masih dalam masa toleransi
            else:

                # Tampilkan hitung mundur peringatan
                cv2.putText(
                    image,
                    f"Peringatan... {int(TOLERANCE_SECONDS - elapsed_time)}s",
                    (20, 50),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    1,
                    (0, 255, 255),  # kuning
                    2
                )


        # ==================================================
        # MENAMPILKAN WINDOW WEBCAM
        # ==================================================

        # Menampilkan hasil webcam
        cv2.imshow('Focus-Pet AI Vision', image)


        # Jika tombol ESC ditekan
        # ASCII ESC = 27
        if cv2.waitKey(5) & 0xFF == 27:

            # Keluar dari loop
            break
        

        # ==================================================
        # MEMBERI JEDA ASYNC
        # ==================================================

        # Jeda kecil agar websocket tetap bisa berjalan
        # secara bersamaan
        await asyncio.sleep(0.01)
            

    # ==================================================
    # MEMBERSIHKAN RESOURCE
    # ==================================================

    # Menutup webcam
    cap.release()

    # Menutup semua window OpenCV
    cv2.destroyAllWindows()


# ======================================================
# FUNCTION WEBSOCKET HANDLER
# ======================================================

async def websocket_handler(websocket):

    # Menggunakan variabel global
    global current_status

    # Menampilkan pesan jika ESP8266 terhubung
    print("ESP8266 Terhubung!")

    try:

        # Loop terus menerus
        while True:

            # Membuat payload JSON
            #
            # Contoh:
            # {"status":"focused"}
            payload = json.dumps({
                "status": current_status
            })


            # Mengirim JSON ke ESP8266
            await websocket.send(payload)


            # Tunggu 1 detik sebelum kirim lagi
            await asyncio.sleep(1)


    # Jika koneksi websocket terputus
    except websockets.exceptions.ConnectionClosed:

        # Tampilkan pesan disconnect
        print("ESP8266 Terputus.")


# ======================================================
# FUNCTION UTAMA
# ======================================================

async def main():

    # Membuat server websocket
    #
    # "0.0.0.0" = menerima koneksi dari semua device
    # 8765      = port websocket
    server = websockets.serve(
        websocket_handler,
        "0.0.0.0",
        8765
    )
    

    # ==================================================
    # MENJALANKAN SERVER DAN AI SECARA BERSAMAAN
    # ==================================================

    # asyncio.gather digunakan untuk menjalankan
    # beberapa proses asynchronous secara paralel
    await asyncio.gather(

        # Menjalankan websocket server
        server,

        # Menjalankan AI Vision
        ai_vision()
    )


# ======================================================
# ENTRY POINT PROGRAM
# ======================================================

# Jika file Python dijalankan langsung
if __name__ == "__main__":

    # Menjalankan event loop asyncio
    asyncio.run(main())
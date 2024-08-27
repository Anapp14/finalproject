import serial
import time

# Mengatur port serial sesuai dengan port Arduino Anda
port = 'COM6'  # Sesuaikan dengan port Anda
baud_rate = 9600

try:
    ser = serial.Serial(port, baud_rate)
    print(f"Terhubung ke {port} pada baud rate {baud_rate}")
except serial.SerialException as e:
    print(f"Gagal terhubung ke port: {e}")
    exit()

try:
    while True:
        if ser.in_waiting > 0:
            # Membaca baris data dari port serial
            line = ser.readline()
            try:
                # Mencoba mendekode menggunakan utf-8
                decoded_line = line.decode('utf-8').rstrip()
                print(f"Data diterima: {decoded_line}")
            except UnicodeDecodeError:
                # Jika gagal, gunakan codec latin-1
                decoded_line = line.decode('latin-1').rstrip()
                print(f"Data diterima dengan latin-1: {decoded_line}")
        else:
            # Tidak mencetak pesan jika tidak ada data yang diterima
            time.sleep(0.1)  # Penundaan yang lebih lama
except KeyboardInterrupt:
    print("Program dihentikan.")
finally:
    ser.close()
    print("Port serial ditutup.")

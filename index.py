import requests
import pandas as pd
import time
from datetime import datetime

# ThingSpeak API URL dan Channel ID
api_key = '2WEDLR29SB1UFFFI'  # Ganti dengan API Key dari channel ThingSpeak Anda
channel_id = '2610736'  # Ganti dengan Channel ID dari ThingSpeak Anda
url = f'https://api.thingspeak.com/channels/{channel_id}/feeds.json?api_key={api_key}&results=1'

# Nama file CSV
csv_file = 'rata-rata.csv'

def get_latest_thingspeak_entry():
    response = requests.get(url)
    if response.status_code == 200:
        data = response.json()
        feeds = data['feeds']
        if feeds:
            return feeds[0]
    return None

def update_csv_with_average(entries):
    # Mengonversi data ke dalam DataFrame pandas
    df = pd.DataFrame(entries)
    df['created_at'] = pd.to_datetime(df['created_at'])
    df['field1'] = pd.to_numeric(df['field1'], errors='coerce')
    df['field2'] = pd.to_numeric(df['field2'], errors='coerce')
    
    # Menghitung rata-rata suhu dan konsentrasi CO2
    average_suhu = df['field1'].mean()
    average_co2 = df['field2'].mean()
    
    # Menyiapkan data untuk disimpan ke CSV
    average_data = {
        'Tanggal': datetime.now().strftime('%Y-%m-%d %H:%M:%S'),
        'Rata-Rata suhu(celcius)': average_suhu,
        'Rata-rata Konsentrasi co2 (ppm)': average_co2
    }
    
    # Menyimpan rata-rata ke dalam file rata-rata.csv
    try:
        df_csv = pd.read_csv(csv_file)
        df_csv = pd.concat([df_csv, pd.DataFrame([average_data])], ignore_index=True)
    except FileNotFoundError:
        df_csv = pd.DataFrame([average_data])
    
    df_csv.to_csv(csv_file, index=False)
    print("Data rata-rata telah disimpan ke dalam file", csv_file)

# Inisialisasi
entries = []

# Jalankan pembaruan awal
latest_entry = get_latest_thingspeak_entry()
if latest_entry:
    entries.append(latest_entry)

# Periksa dan perbarui CSV setiap 10 detik
while True:
    latest_entry = get_latest_thingspeak_entry()
    if latest_entry and latest_entry not in entries:
        entries.append(latest_entry)
    
    if len(entries) >= 5:
        update_csv_with_average(entries)
        entries = []  # Reset entries setelah pembaruan
    
    time.sleep(10)

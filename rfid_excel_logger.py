import serial
import openpyxl
from datetime import datetime

# Excel setup
excel_file = "rfid_data_log.xlsx"
try:
    wb = openpyxl.load_workbook(excel_file)
    ws = wb.active
except FileNotFoundError:
    wb = openpyxl.Workbook()
    ws = wb.active
    ws.append(["Timestamp", "Product ID", "Product Name", "Quantity", "Value", "Tag Timestamp"])

# Serial setup
ser = serial.Serial('COM10', 9600, timeout=3)  # Change COM7 to your port

print("Listening on serial...")
while True:
    try:
        product_id = ser.readline().decode().strip().split(":", 1)[-1]
        name = ser.readline().decode().strip().split(":", 1)[-1]
        qty = ser.readline().decode().strip().split(":", 1)[-1]
        value = ser.readline().decode().strip().split(":", 1)[-1]
        tag_time = ser.readline().decode().strip().split(":", 1)[-1]
        
        now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        ws.append([now, product_id, name, qty, value, tag_time])
        wb.save(excel_file)
        print("Logged:", product_id, name)
    except Exception as e:
        print("Error:", e)

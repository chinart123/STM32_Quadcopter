import serial
import time
import os

# =======================================================
# SẾP NHỚ SỬA LẠI TÊN CỔNG COM DƯỚI ĐÂY CHO ĐÚNG NHÉ!
# =======================================================
PORT = 'COM3'       # Đổi thành cổng ST-Link của sếp (VD: COM4, COM5...)
BAUDRATE = 115200

# Tự động tạo tên file theo ngày giờ (không bao giờ bị ghi đè file cũ)
filename = f"flight_log_{time.strftime('%Y%m%d_%H%M%S')}.csv"
filepath = os.path.join(os.getcwd(), filename)

try:
    # Mở kết nối
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    print(f"[*] Đã kết nối thành công với cổng {PORT}!")
    
    # Mở file để ghi
    with open(filepath, "w", encoding="utf-8") as f:
        f.write("Time(ms),Roll,Pitch,Yaw,M1,M2,M3,M4,Speed(Bps),Volume(Bytes)\n") # Ghi tiêu đề cột cho Excel
        
        print(f"[*] Đang ghi dữ liệu vào file: {filename}")
        print("[*] Bấm Ctrl+C trên bàn phím để dừng quá trình ghi.\n")
        
        while True:
            # Đọc 1 dòng từ STM32 gửi lên
            line = ser.readline().decode('utf-8').strip()
            if line:
                f.write(line + "\n")
                f.flush() # Ép lưu vào ổ cứng ngay lập tức, chống mất data!
                print(f"Đã ghi: {line}")

except serial.SerialException:
    print(f"[!] LỖI TỨC THÌ: Không thể mở cổng {PORT}.")
    print("    -> Sếp hãy kiểm tra lại xem có đang mở Hercules không? (Phải TẮT Hercules thì Python mới giành được cổng COM nhé!)")
except KeyboardInterrupt:
    print("\n[*] Sếp đã ra lệnh dừng (Ctrl+C).")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print(f"\n[OK] Đã đóng cổng COM. File báo cáo của sếp nằm chễm chệ tại:\n -> {filepath}")
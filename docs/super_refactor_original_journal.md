# super_refactor.sh — Nhật ký & Tài liệu

> **Trạng thái:** ✅ Đã chạy thành công — KHÔNG chạy lại
> **Ngày chạy:** Trước khi bắt đầu phát triển STM32 milestone
> **Repo áp dụng:** `STM32_Quadcopter` (Keil µVision 5)

---

## Script này làm gì?

Đây là công cụ **one-time migration** — chạy 1 lần duy nhất để chuyển repo từ cấu trúc phẳng (tất cả file ở root) sang cấu trúc thư mục có tổ chức.

### Trước khi chạy
```
STM32_Quadcopter/          ← tất cả file nằm lộn xộn ở root
├── button.h
├── button.c
├── main.c
├── drn_button.c
├── drn_button.h
├── i2c_mpu_debug.c
├── Stage10.md
├── AO3400.pdf
└── STM32_Quadcopter.uvprojx
```

### Sau khi chạy
```
STM32_Quadcopter/
├── src/                   ← tất cả .c active
│   ├── main.c
│   └── drn_button.c
├── include/               ← tất cả .h active
│   └── drn_button.h
├── Old_Core_Files/        ← code legacy thế hệ 1
│   ├── button.c / .h
│   ├── main_board_choose.c / .h
│   └── i2c_mpu_debug.c / .h
├── Logs_and_Debug/        ← Stage*.md và log files
├── assets/                ← PDF, HTML, SVG
└── STM32_Quadcopter.uvprojx  ← đã được vá tự động
```

---

## 7 bước script thực hiện

### Bước 1 — Git backup an toàn
```bash
git init  # nếu chưa có
git add .
git commit -m "Backup trước khi chạy Script 100 điểm"
cp STM32_Quadcopter.uvprojx STM32_Quadcopter.uvprojx.bak
```
Tạo commit và file `.bak` để rollback nếu cần.

### Bước 2 — Tạo cấu trúc thư mục
```bash
mkdir -p src include 1_Docs_and_Reports Logs_and_Debug Old_Core_Files assets docs
```

### Bước 3 — Di chuyển header files
```bash
find . -maxdepth 1 -name "*.h" ! -name "stm32f10x.h" -exec mv {} include/ \;
```
Di chuyển tất cả `.h` vào `include/`, **ngoại trừ** `stm32f10x.h` vì file này do Keil RTE quản lý — di chuyển sẽ làm Keil mất link.

### Bước 4 — Phân loại Legacy code
```bash
mv include/button.h Old_Core_Files/
mv button.c Old_Core_Files/
mv include/main_board_choose.h Old_Core_Files/
# ...
```
Code thế hệ 1 (blocking, không có DRN_ prefix) được cách ly vào `Old_Core_Files/` — giữ để tham khảo, không build vào project.

### Bước 5 — Di chuyển active source code
```bash
mv *.c src/
```
Tất cả `.c` còn lại (bao gồm `hal_timer_pwm.c`) vào `src/`.

### Bước 6 — Dọn tài liệu
```bash
mv Stage*.md Logs_and_Debug/
mv *.pdf assets/
mv *.html assets/
mv *.svg assets/
```

### Bước 7 — ⭐ Vá file .uvprojx (phần quan trọng nhất)
```bash
# Cập nhật Include Path để Keil tìm được .h
sed -i 's|<IncludePath></IncludePath>|<IncludePath>.\include;.\Old_Core_Files</IncludePath>|g' STM32_Quadcopter.uvprojx

# Cập nhật đường dẫn từng file .c
for file in src/*.c; do
    filename=$(basename "$file")
    sed -i "s|<FilePath>.\$filename</FilePath>|<FilePath>.\src\$filename</FilePath>|g" STM32_Quadcopter.uvprojx
done
```

Đây là bước "ma thuật" — nếu không có bước này, mở Keil sau khi di chuyển file sẽ báo lỗi "File not found" cho tất cả. Script tự động cập nhật XML trong `.uvprojx` để Keil biết file đã chuyển đi đâu.

---

## Tại sao KHÔNG chạy lại?

Script giả định tất cả file đang nằm ở root. Nếu chạy lần 2:

```
Lần 2 xảy ra:
src/main.c  →  mv *.c src/  →  lỗi "already in src/"
hoặc tệ hơn: file bị move nhầm chỗ, .uvprojx bị patch lần 2 với path sai
```

Nếu cần rollback về trạng thái trước khi chạy:
```bash
git log --oneline  # tìm commit "Backup trước khi chạy Script 100 điểm"
git checkout <commit-hash>
# hoặc:
cp STM32_Quadcopter.uvprojx.bak STM32_Quadcopter.uvprojx
```

---

## Khác biệt với super_refactor_fixed.sh mới

| | Script cũ này | Script mới (super_refactor_fixed.sh) |
|---|---|---|
| Mục đích | Di chuyển file vào đúng thư mục | Đổi tên hàm/biến theo convention |
| Chạy bao nhiêu lần | 1 lần duy nhất ✅ Đã xong | Nhiều lần được, có --dry-run |
| Tác động | Cấu trúc thư mục + .uvprojx | Nội dung bên trong .c/.h/.cpp |
| Trạng thái | ✅ Đã hoàn thành | ❌ Chưa chạy |

---

## Lesson learned từ script này

Kỹ thuật vá `.uvprojx` bằng `sed` có thể tái sử dụng khi:
- Thêm nhiều file mới vào project cùng lúc
- Đổi tên thư mục trong Keil
- Sync Keil project sau khi thay đổi cấu trúc

Template vá .uvprojx cho file mới:
```bash
# Thêm file mới vào Keil project bằng script thay vì GUI:
NEW_FILE="drn_imu.c"
sed -i "/<Files>/a\\        <File>\\n          <FileName>$NEW_FILE</FileName>\\n          <FileType>1</FileType>\\n          <FilePath>.\\\\src\\\\$NEW_FILE</FilePath>\\n        </File>" STM32_Quadcopter.uvprojx
```
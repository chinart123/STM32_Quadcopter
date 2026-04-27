#!/bin/bash

echo "=== BẮT ĐẦU REFACTOR CẤU TRÚC BARE-METAL (ZERO-CONFIG) ==="

# 1. Backup dự phòng an toàn tuyệt đối
if [ ! -d ".git" ]; then
    echo "Khởi tạo Git và Backup state hiện tại..."
    git init > /dev/null
    git add . > /dev/null
    git commit -m "Backup trước khi chạy Script 100 điểm" > /dev/null
else
    git add . > /dev/null
    git commit -m "Backup trước khi chạy Script 100 điểm" > /dev/null
fi
cp STM32_Quadcopter.uvprojx STM32_Quadcopter.uvprojx.bak

# 2. Tạo các ngăn kéo
mkdir -p src include 1_Docs_and_Reports Logs_and_Debug Old_Core_Files assets docs

# 3. Phân loại Header (.h) -> BỎ QUA stm32f10x.h (RTE quản lý)
find . -maxdepth 1 -name "*.h" ! -name "stm32f10x.h" -exec mv {} include/ \;

# 4. Phân loại Legacy Code (Thế hệ 1) vào Old_Core_Files
mv include/button.h Old_Core_Files/ 2>/dev/null
mv button.c Old_Core_Files/ 2>/dev/null
mv include/main_board_choose.h Old_Core_Files/ 2>/dev/null
mv main_board_choose.c Old_Core_Files/ 2>/dev/null
mv include/i2c_mpu_debug.h Old_Core_Files/ 2>/dev/null
mv i2c_mpu_debug.c Old_Core_Files/ 2>/dev/null

# 5. Phân loại Active Code (.c) vào src/ (Bao gồm cả hal_timer_pwm.c)
mv *.c src/ 2>/dev/null

# 6. Dọn dẹp tài liệu
mv Stage*.md Logs_and_Debug/ 2>/dev/null
mv *.pdf assets/ 2>/dev/null
mv *.txt Logs_and_Debug/ 2>/dev/null
mv *.html assets/ 2>/dev/null
mv *.svg assets/ 2>/dev/null

# =================================================================
# 7. MA THUẬT: VÁ FILE .uvprojx CỦA KEILC BẰNG SED
# =================================================================
echo "Đang cập nhật đường dẫn vào KeilC Project..."

# Update <IncludePath> để KeilC biết đường tìm file .h
sed -i 's|<IncludePath></IncludePath>|<IncludePath>.\\include;.\\Old_Core_Files</IncludePath>|g' STM32_Quadcopter.uvprojx

# Cập nhật <FilePath> cho các file trong thư mục src/
for file in src/*.c; do
    filename=$(basename "$file")
    sed -i "s|<FilePath>\\.\\\\$filename</FilePath>|<FilePath>.\\\\src\\\\$filename</FilePath>|g" STM32_Quadcopter.uvprojx
done

# Cập nhật <FilePath> cho các file trong thư mục include/
for file in include/*.h; do
    filename=$(basename "$file")
    sed -i "s|<FilePath>\\.\\\\$filename</FilePath>|<FilePath>.\\\\include\\\\$filename</FilePath>|g" STM32_Quadcopter.uvprojx
done

# Cập nhật <FilePath> cho các file trong thư mục Old_Core_Files/
for file in Old_Core_Files/*; do
    filename=$(basename "$file")
    sed -i "s|<FilePath>\\.\\\\$filename</FilePath>|<FilePath>.\\\\Old_Core_Files\\\\$filename</FilePath>|g" STM32_Quadcopter.uvprojx
done

echo "=== HOÀN TẤT! HÃY MỞ KEILC VÀ NHẤN F7 ĐỂ BUILD ==="
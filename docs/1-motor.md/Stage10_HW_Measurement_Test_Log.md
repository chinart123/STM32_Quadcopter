# BÁO CÁO KIỂM THỬ VÀ PHÂN TÍCH LỖI PHẦN CỨNG (Stage 10 - Motor Debug)
**Ngày thực hiện:** [Điền ngày]
**Hệ thống:** Mạch điều khiển động cơ 8520 sử dụng MOSFET AO3400 và Diode SS54.
**Mục tiêu:** Xác định nguyên nhân gây lỗi mạch động lực thông qua phương pháp đo đạc độc lập (Out-of-circuit) và trên mạch (In-circuit).

---

## VÒNG 1: KIỂM TRA LINH KIỆN ĐỘC LẬP (OUT-OF-CIRCUIT)

**1.1. Điện trở Pull-down (10kΩ)**
* **Mục tiêu:** Kiểm tra sai số điện trở so với danh định.
* **Phương pháp:** Đo điện trở (Ohm) tại hai đầu linh kiện.
* **Kết quả:** `9.87 kΩ` (Kỳ vọng: `9.9 kΩ - 10.1 kΩ`).
* **Đánh giá:** **[PASS]** Giá trị nằm trong dải sai số cho phép. Linh kiện hoạt động bình thường.

**1.2. Diode Schottky (SS54)**
* **Mục tiêu:** Kiểm tra trạng thái phân cực thuận và khả năng cách điện phân cực nghịch.
* **Đo phân cực thuận (Anode -> Cathode):** `0.187 V` (Kỳ vọng: `0.15 V - 0.3 V`). $\rightarrow$ Đạt yêu cầu.
* **Đo phân cực nghịch (Cathode -> Anode):** `0 V` (Kỳ vọng: `OL` / Vô cực). $\rightarrow$ Lỗi ngắn mạch.
* **Đánh giá:** **[FAIL]** Diode đã bị đánh thủng hoàn toàn (Short-circuit), mất khả năng chặn dòng điện ngược. Nguyên nhân có thể do dòng xả quá giới hạn hoặc quá áp. Cần loại bỏ linh kiện.

**1.3. MOSFET AO3400 - Kiểm tra Diode Nội (Body Diode)**
* **Mục tiêu:** Kiểm tra cấu trúc Diode nội bộ từ chân Source sang chân Drain. Dựa theo Datasheet AO3400 (Trang 2, mục $V_{SD}$), điện áp thuận Typical là `0.7 V`, Max là `1 V`.
* **Phương pháp:** Đo Diode, que Đỏ tại Source, que Đen tại Drain.
* **Kết quả:** `0.602 V` (Kỳ vọng: `0.4 V - 1 V`).
* **Đánh giá:** **[PASS]** Cấu trúc bán dẫn P-N của Body Diode chưa bị phá hủy.

**1.4. MOSFET AO3400 - Kiểm tra Lớp Cách điện Gate (Gate Oxide Integrity)**
* **Mục tiêu:** Kiểm tra rò rỉ dòng điện giữa cực điều khiển (Gate) và kênh dẫn (Drain/Source). Theo Datasheet AO3400 (Trang 1, Absolute Maximum Ratings), giới hạn điện áp $V_{GS}$ tuyệt đối là `±12 V`. Theo tài liệu vật lý bán dẫn NMOS, lớp điện môi (Gate Oxide) cách ly hoàn toàn dòng điện ở trạng thái bình thường.
* **Phương pháp:** Đo Diode từ Gate sang Drain và Gate sang Source.
* **Kết quả:** Đo với Drain hiện `0.222 V`; đo với Source hiện `0.677 V` (Kỳ vọng: `OL` / Cách điện hoàn toàn).
* **Đánh giá:** **[FAIL]** Lớp điện môi Oxide (Silicon Dioxide) đã bị đánh thủng (Gate Oxide Breakdown). Sự cố này tạo ra hiện tượng ngắn mạch (Short-circuit) giữa Gate và kênh dẫn, khiến linh kiện hỏng vĩnh viễn. Nguyên nhân chính là do điện áp dội ngược (Flyback voltage) vượt quá ngưỡng `±12 V`.

**1.5. MOSFET AO3400 - Khả năng Chuyển mạch (Switching Test)**
* **Mục tiêu:** Kiểm tra khả năng mở kênh dẫn khi có áp $V_{GS} > V_{GS(th)}$. Theo Datasheet, ngưỡng mở $V_{GS(th)}$ là từ `0.65 V` đến `1.45 V`. Khi mở, nội trở $R_{DS(ON)}$ rất thấp (`~25 mΩ` ở 3.3V).
* **Phương pháp:** Nạp điện áp vào Gate, sau đó đo điện áp rớt qua Drain-Source.
* **Kết quả:** `0.682 V` (Kỳ vọng: Gần `0.00 V` do nội trở $R_{DS(ON)}$ nhỏ).
* **Đánh giá:** **[FAIL]** Việc lớp cách điện Gate bị thủng đã làm mất hoàn toàn khả năng điều khiển kênh dẫn của MOSFET.

---

## VÒNG 2: KIỂM TRA TRÊN MẠCH ĐÃ HÀN (IN-CIRCUIT, TẮT NGUỒN)

**2.1. Kiểm tra Mạch Xả Tĩnh Điện (Pull-down Resistor)**
* **Mục tiêu:** Đảm bảo trở 10kΩ được kết nối đúng giữa Gate và Source.
* **Phương pháp:** Đo trở kháng song song trên mạch.
* **Kết quả:** Giá trị không ổn định, dao động `3 kΩ -> 1.43 kΩ -> 10 kΩ -> 0 Ω -> 1.63 V`.
* **Đánh giá:** **[KHÔNG HỢP LỆ]** Do MOSFET bị ngắn mạch nội bộ (phát hiện ở mục 1.4), kết quả đo trở kháng bị nhiễu do đồng hồ đo song song điện trở 10kΩ với kênh dẫn bị hỏng của MOSFET.

**2.2. Kiểm tra Đồng bộ Mức Điện Áp Tham Chiếu (Common Ground)**
* **Mục tiêu:** Kiểm tra tính liên tục của đường Ground (GND) giữa hệ thống điều khiển và động lực.
* **Phương pháp:** Đo thông mạch (Continuity) giữa cực Âm nguồn cấp, Source của MOSFET và GND của Vi điều khiển.
* **Kết quả:** Điện trở hiển thị `~0 Ω`.
* **Đánh giá:** **[PASS]** Mốc tham chiếu 0V đã được thiết lập đồng bộ, đáp ứng tiêu chuẩn an toàn cho tín hiệu.

---

## KẾT LUẬN VÀ BIỆN PHÁP KHẮC PHỤC (ACTION ITEMS)

**1. Kết luận nguyên nhân lỗi:** Sự vắng mặt của mạch bảo vệ Flyback (hoặc việc đấu Diode sai nguyên lý nối tiếp) đã khiến suất điện động cảm ứng từ động cơ sinh ra điện áp quá mức (Vượt `30 V` $BV_{DSS}$ và `±12 V` $V_{GS}$). Điện áp này đã đánh thủng lớp cách điện Oxide của MOSFET và gây chập hoàn toàn Diode SS54. 

**2. Biện pháp xử lý phần cứng:**
* Loại bỏ vĩnh viễn MOSFET AO3400 và Diode SS54 đã hỏng.
* Cập nhật sơ đồ Schematic: Bắt buộc mắc Diode SS54 **song song ngược chiều** với động cơ để tạo vòng lặp xả (Freewheeling loop) nhằm ghim điện áp dư. 

**3. Biện pháp xử lý phần mềm:**
* Ngõ ra PWM tại chân **PA6** của vi điều khiển có rủi ro bị hư hỏng (Hỏng Output Buffer) do dòng điện rò rỉ từ Gate bị thủng dội ngược về. 
* Tiến hành cách ly chân PA6. Đã cấu hình lại Timer để xuất tín hiệu PWM chuyển sang chân **PA7** dự phòng.
# Stage 10.5 Final Release: Motor Hardware - Theory, Practice & Failure Analysis (4kHz PWM Version)
**Tài liệu Master: Thiết kế Tiêu chuẩn, Lý thuyết Động lực học, Nhật ký Debug và Phân tích Lỗi Động cơ 8520**

Tài liệu này ghi chép lại toàn bộ quá trình thiết kế, kiểm thử, phân tích lỗi và hoàn thiện mạch điều khiển động cơ DC Coreless 8520 bằng vi điều khiển (STM32/ESP32) sử dụng N-Channel Enhancement MOSFET.

---

## PHẦN 1: TỔNG QUAN LINH KIỆN & TÀI LIỆU (BOM & Datasheets)

**1. MOSFET: AO3400 (Mã in SMD: A09T) - Dòng N-Channel Enhancement**
* **Chức năng:** Làm "Van điện tử" đóng cắt dòng điện công suất lớn. Mặc định tắt (Normally OFF), mở khi có xung dương từ MCU.
* **Thông số tử huyệt:**
  * Điện áp Drain-Source Max ($BV_{DSS}$): `30V`. (Giới hạn điện áp nguồn cấp).
  * Điện áp Gate-Source Max ($V_{GS}$ Max): `±12V`. (Giới hạn điện áp điều khiển đâm thủng lớp cách điện).
  * Dòng điện liên tục Max ($I_D$): `5.8A`.
  * Nội trở khi mở ($R_{DS(ON)}$): `~25mΩ` (Lý thuyết) $\rightarrow$ Thực tế vận hành ở $V_{GS}=3.3V$ là khoảng **`50mΩ`**.
  * Tụ ký sinh Miller ($C_{rss}$): `50 pF`. (Cầu nối truyền xung dội ngược từ Drain về Gate).
  * Điện dung buồng lái Gate ($C_{iss}$): `630 pF`.
  * Thời gian ngắt ($t_{D(off)} + t_f$): `~29ns` (Tính tròn `30ns` cho lý thuyết. Tốc độ ngắt cực nhanh đẻ ra xung dội cực mạnh).
* **Datasheet:** [AO3400 Official Datasheet - Alpha & Omega](https://www.aosmd.com/sites/default/files/res/datasheets/AO3400.pdf)

**2. Flyback Diode: SS54 (Schottky Rectifier)**
* **Chức năng:** Làm "Van xả áp suất", được mắc song song ngược chiều với tải cảm. Tạo vòng lặp khép kín xả dòng quán tính (Flyback current) để triệt tiêu suất điện động cảm ứng sinh ra khi ngắt MOSFET.
* **Thông số tử huyệt:**
  * Dòng điện chịu đựng liên tục ($I_{F(AV)}$): `5A` (Dư sức gánh dòng xả 2A của Motor 8520).
  * Điện áp ngược cực đại ($V_{RRM}$): `40V` (Chặn an toàn nguồn Tổ ong/Pin LiPo).
  * Dòng đỉnh quá tải ($I_{FSM}$): `150A` (Sức chịu đựng cú va đập búa nước điện từ).
  * Áp rớt thuận ($V_F$): Cực thấp `~0.55V`. Mở cửa ngay lập tức khi áp dội vừa chớm vượt qua điện áp nguồn cấp.
* **Datasheet:** [SS54 Vishay Datasheet](https://www.vishay.com/docs/88746/ss52.pdf)

**3. Điện trở Pull-down: 10kΩ (Dán SMD 0805)**
* **Chức năng:** Làm "Đường cống xả", xả tĩnh điện và các dòng điện rò rỉ từ chân Gate về mốc GND 0V. Ngăn hiện tượng Floating Gate biến MOSFET thành chiếc ăng-ten hút tĩnh điện tự mở mạch ngoài ý muốn.
* **Thông số:** Giá trị `10kΩ`, Công suất `1/8 W`.

---

## PHẦN 2: LÝ THUYẾT & CÔNG THỨC ĐỘNG LỰC HỌC (BÚA NƯỚC ĐIỆN TỪ)

Để hiểu tận cùng bản chất vật lý của hệ thống, chúng ta tính toán dựa trên các hằng số: 
* Độ tự cảm cuộn dây động cơ: $L = 10 \mu H$.
* Điện trở cuộn dây khi tải nặng: $R_m = 2.3 \Omega$.
* Tụ ký sinh Miller: $C_{rss} = 50 pF$.
* Tụ buồng lái Gate: $C_{iss} = 630 pF$.
* Thời gian sập cửa MOSFET: $dt = 30 ns$.

### 2.1. Trạng thái ON (Định luật Ohm & Nhiệt lượng MOSFET)
Khi MOSFET mở toang cửa bằng áp kích 3.3V từ STM32, nội trở thực tế $R_{DS(ON)} \approx 0.05\Omega$. Dòng điện chạy qua động cơ:
$$I = \frac{V_{RMS}}{R_m} = \frac{4.6V}{2.3\Omega} = 2A$$

Điện áp rớt trên MOSFET (quyết định độ mát mẻ):
$$V_{Drain} = I \times R_{DS(ON)} = 2A \times 0.05\Omega = \mathbf{0.1V}$$

### 2.2. Trạng thái OFF (Sự hình thành Búa nước điện từ ở chân Drain)
Khi MOSFET ngắt cái "Rầm" trong $30ns$, dòng điện $I$ đang chảy qua cuộn dây bị chặn đứng. Cuộn dây lập tức sinh ra Suất điện động cảm ứng (áp dội):
$$V_L = L \cdot \frac{di}{dt}$$
* **NẾU KHÔNG CÓ DIODE BẢO VỆ:** Điện áp tại chân Drain vọt lên kinh hoàng: 
  $$V_{Drain\_Fail} = V_{CC} + V_L = V_{CC} + \left( 10\mu H \cdot \frac{I}{30ns} \right)$$
* **NẾU CÓ DIODE SS54 MẮC SONG SONG:** Diode mở cửa xả áp ngay lập tức, ghim chặt điện áp Drain ở mức an toàn:
  $$V_{Drain\_Safe} = V_{CC} + V_{F\_Diode} \approx V_{CC} + 0.5V$$

### 2.3. Kẻ nội gián truyền xung ($I_{gate\_spike}$)
Sự thay đổi điện áp đột ngột ($dV/dt$) tại chân Drain sẽ ép một dòng điện chạy xuyên qua tụ ký sinh tàng hình $C_{rss}$ đâm thẳng vào buồng lái Gate:
$$I_{gate} = C_{rss} \cdot \frac{dV_{Drain}}{dt}$$

### 2.4. Sự hủy diệt buồng lái ($\Delta V_{GS}$)
Dòng điện nội gián nạp vào chân Gate, tạo ra áp suất phá vỡ lớp cách điện (Giới hạn là $\pm12V$):
$$\Delta V_{GS} = (V_{Drain} - 0.1V) \cdot \frac{C_{rss}}{C_{iss} + C_{rss}}$$
*(Chỉ cần $\Delta V_{GS} > 12V$, MOSFET chết chập vĩnh viễn).*

---

## PHẦN 3: SƠ ĐỒ MẠCH ĐỘNG LỰC & PROMPT RENDER (Mermaid.js)

### 3.1. MẠCH LỖI: Mắc Diode nối tiếp (Fatal Error)
![Mạch Lỗi](Fail-Diagram.png)

> **Prompt cho AI vẽ Mạch Lỗi:**
> ```text
> Act as a Senior Hardware Engineer. Generate a Mermaid.js flowchart (Graph TD) illustrating an INCORRECT/FAILED N-Channel MOSFET motor control circuit.
> CRITICAL STYLE REQUIREMENT: You MUST use the database/cylinder shape syntax `NodeID[(Label)]` for ALL nodes.
> Nodes and Routing (Showing the Fatal Error):
> 1. `MCU[(MCU PWM Pin STM32 PA7)]` connects to `GateNet[(MOSFET Gate Net)]`.
> 2. `GateNet` connects to `MOSFET[(AO3400 N-Channel MOSFET)]` with the label `|Gate|`.
> 3. `GateNet` also connects to `PullDown[(10kΩ Pull-Down)]`.
> 4. `VCC[(+4.6V Power Supply)]` connects to `MotorPos[(8520 Motor Positive)]`.
> 5. THE ERROR: `MotorPos` connects down to `Diode[(SS54 Schottky Diode)]` with the label `|Motor Negative connects to Anode|`. 
> 6. `Diode` connects down to `DrainNet[(MOSFET Drain Net)]` with the label `|Cathode connects to Drain|`.
> 7. `DrainNet` connects to `MOSFET` with the label `|Drain|`.
> 8. `MOSFET` connects to `SourceNet[(MOSFET Source)]` with the label `|Source|`.
> 9. `PullDown`, `SourceNet`, and `VEE[(Power Supply Negative)]` all connect down to `GND[(System GND)]`.
> 10. Create an extra node `Warning[(FATAL ERROR: Series Diode provides NO Flyback Loop!)]` using a standard box shape `[]` and link it to the `Diode` node.
> Please apply a red or warning color to the `Diode` and `Warning` nodes.
> ```

### 3.2. MẠCH CHUẨN: Mắc Diode song song ngược (Proper Flyback)
![Mạch Chuẩn](PASS-DIAGRAM.png)

> **Prompt cho AI vẽ Mạch Chuẩn:**
> ```text
> Act as a Senior Hardware Engineer. Generate a Mermaid.js flowchart (Graph TD) illustrating a CORRECT N-Channel MOSFET motor control circuit with Flyback protection.
> CRITICAL STYLE REQUIREMENT: You MUST use the database/cylinder shape syntax `NodeID[(Label)]` for ALL nodes to give them a 3D cylindrical look. 
> Nodes and Routing:
> 1. `MCU[(MCU PWM Pin STM32 PA7)]` connects to `GateNet[(MOSFET Gate Net)]`.
> 2. `GateNet` connects to `MOSFET[(AO3400 N-Channel MOSFET)]` with the label `|Gate|`.
> 3. `GateNet` also connects to `PullDown[(10kΩ Pull-Down)]`.
> 4. `VCC[(+4.6V Power Supply)]` connects to `MotorPos[(8520 Motor Positive)]`.
> 5. `VCC` ALSO connects to `Diode[(SS54 Schottky Diode)]` with the label `|Cathode / Vạch trắng|`.
> 6. `MotorPos` connects down to `DrainNet[(MOSFET Drain Net)]` with the label `|Motor Negative|`.
> 7. `Diode` connects down to `DrainNet` with the label `|Anode / Flyback Loop|`.
> 8. `DrainNet` connects to `MOSFET` with the label `|Drain|`.
> 9. `MOSFET` connects to `SourceNet[(MOSFET Source)]` with the label `|Source|`.
> 10. `PullDown`, `SourceNet`, and `VEE[(Power Supply Negative)]` all connect down to `GND[(System GND)]`.
> Please apply a distinct fill color to the `Diode` node (e.g., light yellow) and the `MOSFET` node (e.g., light blue).
> ```

---

## PHẦN 4: NHẬT KÝ ĐO ĐẠC VÀ CHẨN ĐOÁN PHẦN CỨNG (Test Log)
*(Trích xuất từ quá trình debug thực tế mạch bị hỏng do thiếu Diode bảo vệ)*

### VÒNG 1: ĐO LINH KIỆN ĐỂ RỜI TRÊN BÀN (CHƯA HÀN)
1. **Kiểm tra điện trở 10k:** Đo được `9.87k Ω` $\rightarrow$ **[PASS]** Điện trở bình thường.
2. **Kiểm tra Diode SS54 (Chiều thuận):** Đo được `0.187V` $\rightarrow$ **[PASS]** Dẫn điện tốt.
3. **Kiểm tra Diode SS54 (Chiều nghịch):** Đo được `0V` thay vì `OL` $\rightarrow$ **[FAIL - CHẾT CHẬP]** Diode đã bị dòng điện đánh thủng, biến thành dây dẫn.
4. **Kiểm tra Diode nội MOSFET:** Đo được `0.602V` $\rightarrow$ **[PASS]** Lớp Diode nội bộ từ S lên D vẫn hoạt động.
5. **Kiểm tra lớp cách điện chân Gate:** Que Đỏ vào Gate, Que Đen vào Drain/Source đo ra `0.222V` / `0.677V` (Kỳ vọng: `OL`) $\rightarrow$ **[FAIL - THỦNG GATE]** Lớp cách điện đã bị đâm thủng.
6. **Test mồi điện (Đóng/mở MOSFET):** Kích Gate đo D-S ra `0.682V` thay vì `0.0x V` $\rightarrow$ **[FAIL - CHẾT HOÀN TOÀN]** MOSFET mất khả năng đóng mở mạch.

---

## PHẦN 5: CẤU HÌNH TẦN SỐ PWM VÀ BẢO VỆ QUÁ ÁP (BẢN 4kHz)

**1. Giới hạn Duty Cycle (Bảo vệ động cơ 8520):**
Động cơ 8520 chạy định mức ở **3.7V**. Vì hệ thống dùng nguồn Tổ ong **4.6V**, phần mềm cần khóa Duty Cycle tối đa:
$$D_{max} = \frac{V_{motor\_max}}{V_{supply}} \times 100\% = \frac{3.7V}{4.6V} \times 100\% \approx \mathbf{80.4\%}$$
👉 **Hành động:** Khóa thanh ghi PWM ở mức tối đa **80%**.

**2. Động lực học Tần số PWM 4kHz:**
* **Tần số (f):** `4000 Hz` | **Chu kỳ (T):** $250 \mu s$
* **Nội trở vận hành $R_{DS(on)}$:** $\sim 0.050 \Omega$
* **Sụt áp $V_{Drain}$:** $0.100V$
* **Phân tích Nhiệt độ MOSFET:**
  * Tổn hao dẫn (Static Loss): $P_{cond} = I^2 \times R \times D = 2^2 \times 0.05 \times 0.8 = \mathbf{0.160W}$
  * Tổn hao chuyển mạch (Switching Loss): $P_{sw} \approx \mathbf{0.009W}$
  * Nhiệt độ linh kiện tăng: Khoảng **$+17^\circ C$** so với môi trường. MOSFET chỉ ấm nhẹ, rất an toàn.
* **Độ gợn dòng (Ripple):** Dòng điện mượt hơn so với 500Hz, động cơ chạy êm và giảm sinh nhiệt rõ rệt.
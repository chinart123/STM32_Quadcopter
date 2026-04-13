# 1. Giải Thích Chuyên Sâu: Hiện Tượng Đánh Thủng Lớp Cổng MOSFET (Gate Oxide Breakdown)

Bên trong vi mạch MOSFET, chân Gate (Cổng điều khiển) không hề chạm vật lý vào kênh dẫn điện (nằm giữa chân Drain và Source). Nó được cách ly hoàn toàn bởi một lớp thủy tinh siêu mỏng gọi là Silicon Dioxide (Gate Oxide).

Khi bạn đấu sai Diode (mắc nối tiếp), động cơ lúc bị ngắt điện đột ngột sẽ sinh ra một xung điện áp khổng lồ (suất điện động cảm ứng) lên tới hàng trăm vôn dội ngược vào chân Drain. Xung điện này len lỏi qua các tụ điện ký sinh bên trong con chip, truyền một áp suất lớn hơn mức chịu đựng giới hạn (±12V) vào chân Gate. 

Áp lực này phóng một tia lửa điện xuyên thủng lớp kính cách điện mỏng manh đó, tạo ra một vết cháy đen kết nối vĩnh viễn (ngắn mạch/chập) chân Gate và chân Source lại với nhau. 

Do đó, khi hàn điện trở 10k song song giữa hai chân Gate và Source, dòng điện từ đồng hồ VOM sẽ chọn con đường dễ đi nhất (là đường bị chập 0 Ohm bên trong bụng con MOSFET hỏng) thay vì đi qua điện trở 10k. Đó là lý do chỉ số đo trên mạch nhảy loạn xạ và tụt về 0 Ohm.
# PHỤ LỤC TỔNG HỢP: BẢNG THEO DÕI KIỂM THỬ PHẦN CỨNG

Dưới đây là biểu mẫu đã được chia tách rõ ràng thành hai giai đoạn để bạn dễ dàng điền số liệu đo đạc thực tế.

## BẢNG 1: GIAI ĐOẠN CHƯA HÀN (ĐO LINH KIỆN RỜI)
*Mục đích: Đảm bảo 100% linh kiện mới mua đều sống trước khi đưa vào mạch.*

| Bài Đo (Thang đo VOM) | 1. Kết quả Fail đợt trước | 2. Kết quả đo đợt này (Ghi vào đây) | 3. Kết quả mong đợi (Lý thuyết) | 4. Giải thích nguyên lý |
| :--- | :--- | :--- | :--- | :--- |
| **Diode SS54: Chiều nghịch** <br>*(Thang Diode: Đỏ vào vạch trắng)* | `0 V` | `[         OL         ]` | **`OL` (Vô cực)** | Ở đợt trước, Diode đã bị đánh thủng nên đo ra 0V (chập). Diode sống phải cách điện hoàn toàn ở chiều nghịch. |
| **MOSFET: Cách điện Gate-Source** <br>*(Thang Diode: Đỏ vào G, Đen vào S)* | Đo ra `0.677V` | `[        OL          ]` | **`OL` (Vô cực)** | Ở đợt trước, lớp kính Oxide cách điện đã bị đâm thủng do quá áp ±12V. MOSFET sống thì buồng lái (Gate) phải cách ly tuyệt đối với kênh dẫn. |
| **MOSFET: Cách điện Gate-Drain** <br>*(Thang Diode: Đỏ vào G, Đen vào D)* | Đo ra `0.222V` | `[        OL          ]` | **`OL` (Vô cực)** | Tương tự như trên, chân Gate không được phép thông mạch với bất kỳ chân nào khác. |

---

## BẢNG 2: GIAI ĐOẠN ĐÃ HÀN (ĐO TRÊN MẠCH - KHÔNG CẤP NGUỒN)
*Mục đích: Kiểm tra lỗi do mỏ hàn (chập chân, hỏng do nhiệt/tĩnh điện) và xác nhận đã đi dây đúng sơ đồ chuẩn.*

| Bài Đo (Thang đo VOM) | 1. Kết quả Fail đợt trước | 2. Kết quả đo đợt này (Ghi vào đây) | 3. Kết quả mong đợi (Lý thuyết) | 4. Giải thích nguyên lý |
| :--- | :--- | :--- | :--- | :--- |
| **Đo Trở kéo 10k (Mắc G-S)** <br>*(Thang Ohm: 2 que vào G và S)* | Nhiễu loạn `3kΩ` đến `0Ω` | `[        9.88 kΩ          ]` | **`~10 kΩ`** | Ở đợt trước, do MOSFET chập ruột nên đồng hồ đo sai. Nếu MOSFET khỏe (cách điện tốt), đồng hồ sẽ đo đúng giá trị thực của điện trở dán trên board. |
| **Đo Vòng xả áp Flyback** <br>*(Thang Diode: Đỏ vào Âm Motor, Đen vào Dương Motor)* | *(Sơ đồ cũ mắc nối tiếp nên không có vòng xả này)* | `[           0.165V       ]` | **`0.15V - 0.3V`** | Bắt buộc phải mắc Diode song song ngược. Chỉ số này xác nhận Diode SS54 đã nằm đúng vị trí để mở cửa xả áp khi có xung dội từ động cơ. |
| **Đo Đồng bộ Mass (GND)** <br>*(Thang Thông mạch: 1 que vào GND STM32, 1 que vào -V Tổ ong)* | `~0 Ω` (Mạch cũ đã làm đúng bước này) | `[          Bíp        ]` | **Kêu bíp bíp (`~0 Ω`)** | Mốc 0V phải được thống nhất. Nếu không thông Mass, điện áp 3.3V từ chân tín hiệu STM32 sẽ vô nghĩa đối với MOSFET. |
---

# 2. Tóm Gọn Mục Tiêu Và Nguyên Lý Đấu Nối Chuẩn

Mục tiêu của mạch: Tạo ra một "van điện tử" (MOSFET) đóng mở siêu tốc để băm xung PWM, nhưng phải có đường xả áp suất an toàn để bảo vệ van.

* **Nguyên tắc 1 - Đồng Mass:** Bắt buộc có một đường dây nối thông chân Source của MOSFET, cực Âm của nguồn Tổ ong (-V), và chân GND của vi điều khiển STM32. Điều này tạo mốc tham chiếu 0V thống nhất.
* **Nguyên tắc 2 - Vòng Lặp Xả (Flyback Loop):** Bắt buộc mắc Diode SS54 song song ngược chiều với động cơ (Vạch trắng/Cathode nối với dây Dương; Đầu không vạch/Anode nối với dây Âm động cơ/chân Drain). Khi MOSFET ngắt, dòng quán tính từ động cơ sẽ chạy vòng qua Diode này tự triệt tiêu, ghim áp dội ở mức an toàn (~5.1V), bảo vệ tuyệt đối cho MOSFET.

---

# 3. Hướng Dẫn Đo Linh Kiện Rời (Khi chưa hàn - Out-of-circuit)

Thực hiện các bước này với linh kiện mới mua để đảm bảo chúng sống 100% trước khi hàn.

1.  **Điện Trở 10k SMD:**
    * *Cách đo:* Thang đo Ohm ($\Omega$). Chạm 2 que vào 2 đầu điện trở.
    * *Kết luận:* Hiện `~10 kΩ` là Đạt.
2.  **Diode SS54:**
    * *Cách đo:* Thang đo Diode.
    * *Chiều thuận:* Que Đỏ vào đầu không vạch (Anode), Que Đen vào vạch trắng (Cathode). Kỳ vọng: `0.15V - 0.3V`.
    * *Chiều nghịch:* Đổi ngược que đo. Kỳ vọng: Hiển thị `OL` (Vô cực/Cách điện). 
    * *Kết luận:* Đạt cả 2 chiều là sống.
3.  **MOSFET AO3400 (Quan trọng nhất):**
    * *Đo Diode Nội (Body Diode):* Thang đo Diode. Que Đỏ chạm Source, Que Đen chạm Drain. Kỳ vọng: `0.4V - 1V`.
    * *Đo Cách Điện Gate:* Que Đỏ chạm Gate, Que Đen chạm Source, sau đó Que Đen đổi sang Drain. Kỳ vọng: Cả 2 lần đều phải `OL`.
    * *Kết luận:* Đạt các thông số trên nghĩa là MOSFET chưa bị thủng.

---

# 4. Hướng Dẫn Đo Trên Mạch (Sau khi đã hàn - In-circuit)

Đảm bảo mạch đang **TẮT NGUỒN** hoàn toàn. Các phép đo này để check lỗi hàn chập hoặc chết linh kiện do mỏ hàn.

1.  **Đo Mạch Xả Tĩnh Điện:**
    * *Cách đo:* Thang đo Ohm. Chạm 2 que vào vị trí hàn chân Gate và Source.
    * *Kết luận:* Phải hiện đúng `~10 kΩ`. Nếu hiện 0 Ohm hoặc vài chục Ohm, bạn đã hàn chập chân hoặc MOSFET đã chết tĩnh điện.
2.  **Đo Vòng Lặp Flyback (Diode song song):**
    * *Cách đo:* Thang đo Diode. Que Đỏ chạm vào cực Âm động cơ (Drain), Que Đen chạm vào cực Dương động cơ (+V).
    * *Kết luận:* Phải đo được `0.15V - 0.3V`. Nếu ra OL là hàn sai chiều Diode hoặc hở mạch.
3.  **Đo Đồng Bộ Mass:**
    * *Cách đo:* Thang đo Thông mạch (Bíp). Chạm 1 que vào GND STM32, que kia vào -V Tổ ong (hoặc Source MOSFET).
    * *Kết luận:* Kêu bíp bíp liên tục, điện trở `~0 Ω` là mạch đã thông Mass chuẩn.

---




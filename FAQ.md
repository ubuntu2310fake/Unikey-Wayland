# Toàn tập Câu hỏi thường gặp (FAQ) & Chuyện phím

Dưới đây là tập hợp toàn bộ những thắc mắc từ nghiêm túc đến "tấu hài" về Unikey-Wayland trên tất cả các nền tảng.

## Nguồn gốc & Tên gọi

**Vì Unikey-Wayland không được tạo ra ngay từ đầu như một dự án bộ gõ đa nền tảng.**
Dự án sinh ra trên Linux Wayland, sau đó kiến trúc được mở rộng dần để hỗ trợ các môi trường khác. Một cái tên chung chung có thể làm mất dấu nguồn gốc của dự án khi một nền tảng có số lượng người dùng lớn hơn chiếm phần lớn sự chú ý. Vì vậy, tên Unikey-Wayland được giữ nguyên trên mọi nền tảng.

**Windows Edition có phải là phiên bản chính của Unikey-Wayland không?**
Không. Unikey-Wayland là tên dự án. Unikey-Wayland (Windows Edition) là phiên bản của dự án dành cho Windows. Việc một Edition có nhiều người dùng hơn không làm thay đổi nguồn gốc của dự án.

**Unikey-Wayland có phải là bộ gõ được phát triển ban đầu cho Windows không?**
Không. Unikey-Wayland bắt nguồn từ Linux Wayland và ban đầu tập trung vào KDE Plasma. Hỗ trợ Windows được bổ sung sau.

**Unikey-Wayland có phải là UniKey chính thức không?**
Không. Đây là một dự án mã nguồn mở độc lập. Dự án sử dụng Engine C++ có nguồn gốc từ mã nguồn mở Bamboo. Tên dự án không có nghĩa phần mềm được phát triển hoặc chứng thực bởi tác giả UniKey chính thức.

**Tại sao bản KDE chỉ có tên Unikey-Wayland mà các bản khác lại có chữ “Edition”?**
KDE Plasma Wayland là môi trường ban đầu. Các môi trường sau sử dụng chữ Edition để xác định backend (ví dụ: Windows Edition, GNOME Edition).

**Tại sao không gọi là Unikey-Native hay Unikey Cross-Platform?**
Vì dự án không sinh ra với tên đó, và "cross-platform" là khả năng chứ không phải nguồn gốc.

**Nếu 99% người dùng là Windows thì có đổi tên không?**
Không. Windows Edition vẫn là Windows Edition của Unikey-Wayland.

**Nếu sau này không còn dòng code Wayland nào thì sao?**
Tên dự án vẫn là Unikey-Wayland. Tên dự án phản ánh nguồn gốc lịch sử, không phải kết quả của việc đếm số dòng code. 🤣

## Kiến trúc & Kỹ thuật

**Unikey-Wayland có thực sự đa nền tảng không?**
Dự án sử dụng kiến trúc modular với các backend riêng cho từng môi trường (Wayland Protocol, IBus, Windows LL Hook), chứ không dùng giải pháp giả lập input duy nhất nào.

**Tại sao không dùng một backend duy nhất?**
Vì hệ thống input của các hệ điều hành hoàn toàn khác nhau. Cố ép chung một cơ chế sẽ dẫn đến lỗi lặp chữ, nháy chữ.

**Tại sao tên là “Unikey” nhưng lại dùng Bamboo?**
Tên dự án và tên lõi xử lý (Engine) là hai khái niệm khác nhau. Lõi Bamboo C++ xử lý việc trộn chữ tiếng Việt cực tốt (đặc biệt là phục vụ cơ chế Native) nên được chúng tôi tin dùng. 

**Tại sao không tự viết engine tiếng Việt từ đầu?**
Mục tiêu ban đầu là giải quyết bài toán tích hợp Input Method trên Wayland (rất khó nhằn), chứ không phải cố phát minh lại bánh xe thuật toán tiếng Việt.

**Native Mode và Preedit Mode là gì?**
- **Native Mode:** Xóa ký tự cũ và chèn ký tự mới trực tiếp (bạn gõ tự nhiên, không thấy gạch chân).
- **Preedit Mode:** Chữ đang gõ sẽ nằm trong một trạng thái chờ (thường có gạch chân) trước khi được xác nhận.

**Tại sao Unikey-Wayland không thích Preedit Mode?**
Đối với tiếng Việt (Telex/VNI), việc gạch chân liên tục khi bỏ dấu gây rối mắt. Dự án ưu tiên thay thế trực tiếp. Tuy nhiên, trên Linux, Preedit Mode vẫn được giữ lại làm phao cứu sinh cho các Terminal. Trên Windows, nó đã bị xóa sổ hoàn toàn bằng thuật toán lách lỗi Omnibox.

**Tại sao Terminal luôn là chỗ bộ gõ dễ gặp vấn đề?**
Terminal có cơ chế buffer và input cực kỳ khác biệt so với các trình soạn thảo văn bản (Word, Chrome). Việc áp dụng Native Mode cho Terminal không bao giờ ổn định 100%.

**Tại sao không cho người dùng tự chịu trách nhiệm và xóa blacklist Terminal?**
Vì một lỗi input method không chỉ ảnh hưởng đến giao diện bộ gõ, mà còn phá hỏng văn bản. Đối với các lỗi đã biết, chúng tôi tước quyền vô hiệu hóa Preedit để đảm bảo an toàn.

**Unikey-Wayland có dùng TSF trên Windows không?**
Tuyệt đối KHÔNG. TSF từng được nghiên cứu nhưng nó gây lỗi nháy chữ trên Chromium. Bản Windows hiện tại sử dụng Global Keyboard Hook kết hợp SendInput.

**Tại sao đôi khi bị lặp chữ?**
Do mất đồng bộ giữa bộ gõ và ứng dụng. Đây là lỗi nghiêm trọng, hãy báo cáo chi tiết cho chúng tôi (nêu rõ môi trường, tên ứng dụng, chuỗi phím đã bấm).

**Tại sao gõ chậm thì đúng nhưng gõ nhanh lại lỗi?**
Đây là dấu hiệu của hiện tượng Race Condition. Bộ gõ không được coi là ổn định nếu chỉ đúng khi gõ chậm. Unikey-Wayland luôn tự động test với tốc độ mô phỏng siêu cao.

**Tại sao Facebook, TikTok, Discord hoặc Terminal dễ làm lộ lỗi bộ gõ?**
Vì các web app (Electron) và Terminal sử dụng framework render chữ hoàn toàn khác biệt với các ứng dụng Native thông thường.

**Tại sao không thêm sleep(20ms) để hết lặp chữ?**
Delay chỉ che giấu lỗi chứ không giải quyết tận gốc, đồng thời sinh ra lỗi trên các máy có tốc độ khác nhau. 🤣 (Cái này là code smell đấy!)

**Tại sao Gõ tắt hoạt động ở Normal Mode nhưng lại bị tắt trong Preedit Mode (Terminal Mode)?**
Việc chèn một chuỗi văn bản dài vào giữa Preedit String dễ gây lỗi thay thế văn bản. Một tính năng tạo chữ sai nguy hiểm hơn một tính năng bị khóa. Do đó, trên phiên bản IBus (GNOME/X11), tính năng Gõ tắt bị vô hiệu hóa khi ở chế độ Preedit để bảo đảm an toàn. Tuy nhiên, trên bản Native gốc (KDE Plasma Wayland), tính năng Gõ tắt trong Preedit Mode vẫn được bật bình thường do cơ chế xử lý sự kiện của KWin hỗ trợ thay thế chuỗi an toàn hơn.

**Có thể dùng Unikey-Wayland cùng EVKey/OpenKey không?**
KHÔNG. Không nên bật 2 bộ gõ cùng lúc để tranh nhau bắt phím của bạn.

## Linux & Tương thích

**Unikey-Wayland có hỗ trợ GNOME / X11 không?**
Có, thông qua bản IBus Engine tương thích.

**Tôi đang dùng KDE X11, nên dùng bản nào?**
Hãy dùng bản qua IBus. Bản gốc (Wayland Client) chỉ dành riêng cho KDE Plasma Wayland.

**Unikey-Wayland có hỗ trợ Hyprland / Sway không?**
Khả năng tương thích phụ thuộc vào việc Compositor đó có hỗ trợ đầy đủ Input Method Protocol hay không. Nếu không, hãy dùng IBus Engine.

**Chỉ cần dùng Wayland là Unikey-Wayland sẽ chạy?**
Không. Mỗi Compositor triển khai Wayland Protocol một kiểu khác nhau.

**Tại sao GNOME Edition không hoạt động giống KDE?**
Vì GNOME và KDE xài kiến trúc nhúng Input Method hoàn toàn khác nhau. IBus (GNOME) không có cùng Semantics với KWin (KDE).

**Tại sao Windows Edition vẫn có giao diện Qt?**
Để duy trì codebase chung của màn hình cấu hình. Qt không đồng nghĩa với Linux-only. Giao diện Qt không làm bộ gõ chậm đi, vì lõi xử lý phím đã tách biệt hoàn toàn.

**Tôi có thể chạy X11 Edition trong WSL rồi gõ vào Microsoft Word không?**
Không. 🤣

## Hệ điều hành & Đồ họa (Tấu hài 🤣)

**Unikey-Wayland có chạy trên Windows không?**
Có. Tải file UnikeyWayland.exe. Không cần cài WSL hay Wayland.

**Wayland có phải tên tác giả không?**
Không.

**Wayland có phải công nghệ gõ tiếng Việt mới không?**
Không, Wayland là một Display Server trên Linux.

**Unikey-Wayland có gửi nội dung tôi gõ lên Internet không?**
Tuyệt đối không. Bộ gõ hoạt động 100% cục bộ offline. Nếu Windows Defender báo có Keylogger, đó là do hành vi bắt phím (Global Hook) bị nhận diện nhầm, mã nguồn chúng tôi mở hoàn toàn.

**Unikey-Wayland có dùng GPU không? RTX 5090 có giúp tôi gõ tiếng Việt nhanh hơn không?**
Không. Bạn không cần GPU acceleration để gõ được chữ "đ".

**Có bật DLSS hay Frame Generation để gõ nhanh gấp đôi được không?**
Không. Đây là bộ gõ tiếng Việt, không phải Black Myth: Wukong. 🤣

**Tôi cài Unikey-Wayland nhưng màn hình vẫn dùng X11. Có lỗi không?**
Không. Bộ gõ không có quyền năng đổi Display Server của bạn. 🤣

**Tôi dùng Windows nhưng gõ lệnh `echo $WAYLAND_DISPLAY` không có kết quả. Lỗi à?**
Không. Vì bạn đang dùng Windows. 🤣🤣🤣

**Tại sao mọi thứ lại phức tạp như vậy?**
Vì Input Method là thứ trông rất đơn giản cho tới khi bạn thử tự tay viết một cái.

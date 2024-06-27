Đã làm xong logic chuyển mode, chỉ cần viết các hàm thực thi chức năng của từng mode riêng biệt, sau đó sẽ gọi trong hàm loop của file main
Sử dụng cơ chế ngắt để chuyển đổi mode, thiết lập thêm một vài biến cờ để đảm bảo các mode được gọi tuần tự, tránh trường hợp bấm nút chuyển mode liên tục thì nó sẽ nhảy cóc mode.
Tạo riêng 1 luồng chỉ để xử lý việc đọc dữ liệu từ cảm biến, nếu mode nào cần sử dụng thì khởi tạo luồng, không dùng nữa thì xóa luồng đi để tiết kiệm tài nguyên.
Cảm biến đọc tín hiệu sẽ sử dụng đèn LED mặc định của ESP 32 để nhận biết, chỉ khi nào cảm biến đọc được giá trị sau khi người dùng đã đè ngón tay vào cảm biến, thì đèn LED mới nhấp nháy. Cảm biến mà không đọc nữa thì sẽ tắt đèn LED đi.


// File lo nhiệm vụ kiểm tra và duyệt mã PIN

// Biến toàn cục lưu IP của Client đang đợi duyệt
let currentPendingIp = "";
// Hàm quét Client mỗi 2 giây
function checkPendingClient() {
    fetch('/api/check_pending')
        .then(response => response.json())
        .then(data => {
            // Nếu phát hiện có Client đang xin cấp quyền
            if (data.pending_ip !== "") {
                // Chỉ báo động lần đầu để tránh Spam
                if (currentPendingIp !== data.pending_ip) {
                    currentPendingIp = data.pending_ip;
                    
                    // Thông báo trên Web
                    alert(" Phát hiện Client (" + currentPendingIp + ") đang xin cấp quyền Remote!\n\nVui lòng vào Tab 'PIN' và nhập mã được hiển thị trên màn hình Laptop Client nếu muốn cấp quyền.");
                }
            } else {
                // Không có Client nào đợi thì Reset
                currentPendingIp = "";
            }
        })
}

// Vòng lặp 2 giây/lần
setInterval(checkPendingClient, 2000);

// Hàm liên kết với sự kiện "onclick=SubmitPin()" bên index.html
function submitPin() {
    let pinInput = document.getElementById("pin-input");
    let errorMsg = document.getElementById("pin-error");
    let pinValue = pinInput.value.trim();

    // Validate trước khi gửi 
    if (pinValue === "") {
        errorMsg.innerText = "⚠️ Chưa nhập mã PIN!";
        errorMsg.style.fontSize = "15px";
        errorMsg.style.display = "block";
        return;
    }
    // Thay đổi trạng thái nút
    let btn = document.querySelector("button[onclick='submitPin()']");
    let originalText = btn.innerText;
    btn.innerText = "Đang xác thực...";
    btn.disabled = true;

    // Đưa mã PIN xuống C++ qua queryString (?pin=XXXX)
    fetch('/api/authorize?pin=' + pinValue)
        .then(response => response.json())
        .then(data => {
            if (data.status === "success") {
                // PIN đã khớp
                errorMsg.style.display = "none";
                pinInput.value = ""; // Dọn sạch ô nhập dữ liệu

                alert("✅ Cấp quyền thành công! GStreamer đã được kích hoạt cho IP" + currentPendingIp);
                currentPendingIp = ""; // Dọn dẹp hàng chờ
            } else {
                // Sai mã PIN hoặc không có hàng chờ
                errorMsg.innerText = "⚠️ " + (data.msg || "Mã PIN không chính xác!");
                errorMsg.style.fontSize = "15px";
                errorMsg.style.display = "block"; 
            }
        })
        .catch(err => {
            console.error("Lỗi khi gửi PIN:", err);
            errorMsg.innerText = "⚠️ Mất kết nối tới máy chủ Jetson!";
            errorMsg.style.fontSize = "15px";
            errorMsg.style.display = "block";
        })
        .finally(() => {
            // Trả lại nút bấm về trạng thái ban đầu
            btn.innerText = originalText;
            btn.disabled = false;
        });
}
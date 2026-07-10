import QtQuick
import QtQuick.Window
import QtQuick.Controls

Window {
    id: mainWindow
    width: 1280
    height: 720
    visible: true
    title: qsTr("Jetson Remote - Client")
    color: "#000000"

    // Phím tắt để gọi lại hộp thoại đổi IP (Bấm Ctrl + I)
    Shortcut {
        sequence: "Ctrl+I"
        onActivated: ipPopup.open()
    }

    Popup {
        id: pinPopup
        width: 350
        height: 200
        modal: true
        focus: true
        anchors.centerIn: parent
        closePolicy: Popup.CloseOnEscape

        background: Rectangle {
            color: "#222";
            radius: 8;
            border.color: "#00ff00";
            border.width: 2
        }
        Column {
            anchors.centerIn: parent;
            spacing: 15;
            Text {
                text: "Phát hiện thiết bị mới!"
                color: "#fff";
                font.bold: true;
                font.pixelSize: 16;
                anchors.horizontalCenter: parent.horizontalCenter
            }
            Text {
                text: "Vui lòng cung cấp mã PIN này cho Admin:"
                color: "#aaa"
                font.pixelSize: 15
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter

                // Ô Text chỉ đọc để hiển thị PIN
                TextField {
                    id: pinDisplay
                    text: ""
                    readOnly: true
                    color: "#00ff00"
                    font.pixelSize: 24
                    font.bold: true
                    font.letterSpacing: 5
                    horizontalAlignment: TextInput.AlignHCenter
                    background: Rectangle{
                        color: "#111";
                        border.color: "#555";
                        border.width: 1;
                        radius: 4
                    }
                }

                // Nút Copy
                Button {
                    id: copyBtn
                    width: 70
                    height: 37
                    text: "Copy"
                    onClicked: {
                        pinDisplay.selectAll()
                        pinDisplay.copy()
                        pinDisplay.deselect()
                        copyBtn.text = "Copied!"
                        copyTimer.start() // Đổi chữ lại sau 2 giây
                    }
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: "#008800";
                        radius: 4
                    }

                    // Bộ đếm thời gian để đổi chữ lại
                    Timer {
                        id: copyTimer
                        interval: 2000
                        onTriggered: copyBtn.text = "Copy"
                    }
                }
            }

            // Nút Close & Nhập lại IP
            Row {
                spacing: 10
                anchors.horizontalCenter: parent.horizontalCenter
                Button {
                    id: closeButton
                    width: 70
                    height: 25
                    text: "Close"
                    onClicked: {
                        watchdogTimer.restart()
                        pinPopup.close()
                        mainWindow.requestActivate()
                    }
                }
                Button {
                    id: returnIpPopupButton
                    width: 150
                    height: 25
                    text: "Change IP Address"
                    onClicked: {
                        pinPopup.close()
                        ipPopup.open()
                    }
                }
            }
        }
    }

    // Cửa sổ Pop-up nhập IP
    Popup {
        id: ipPopup
        anchors.centerIn: parent // Căn giữa màn hình
        width: 350
        height: 180
        modal: true // Làm tối background đằng sau
        focus: true
        closePolicy: Popup.CloseOnEscape // Bấm Esc để tắt

        onClosed: {
            console.log("[*] Pop-up đã đóng. Trả lại quyền nhập liệu cho màn hình chính!")
            mainArea.forceActiveFocus() // ÉP màn hình chính cầm lại quyền!
        }

        onOpened: {
            watchdogTimer.stop()
        }

        // Thiết kế background cho cái hộp thoại
        background: Rectangle {
            color: "#2b2b2b"
            radius: 10
            border.color: "#444444"
            border.width: 2
        }

        Column {
            anchors.centerIn: parent
            spacing: 15

            Text {
                text: "Type Jetson IP for mouse remote"
                color: "#ffffff"
                font.pixelSize: 16
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Rectangle {
                width: 280
                height: 40
                color: "#1e1e1e"
                radius: 5
                border.color: "#555"

                TextInput {
                    id: ipInput
                    anchors.fill: parent
                    anchors.margins: 10
                    verticalAlignment: TextInput.AlignVCenter
                    color: "#00ff00" // Màu chữ xanh lá
                    font.pixelSize: 16
                    text: "127.0.0.1" // IP mặc định
                    clip: true
                    selectByMouse: true
                }
            }

            Button {
                text: "Connect"
                width: 150
                height: 40
                anchors.horizontalCenter: parent.horizontalCenter

                // Giao diện nút bấm
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: parent.down ? "#005500" : "#008800"
                    radius: 5
                }

                onClicked: {
                    // Gửi IP xuống C++
                    backend.setTargetIp(ipInput.text)

                    // Lấy kích thước màn hình hiện tại (Nếu chưa có thì ngầm định 1280x720)
                    let w = videoReceiver.hostWidth > 0 ? videoReceiver.hostWidth : 1280
                    let h = videoReceiver.hostHeight > 0 ? videoReceiver.hostHeight : 720

                    let dynamicPin = Math.floor(1000 + Math.random() * 9000);
                    // Gọi đánh thức
                    console.log("Đang gọi Jetson ở IP: " + ipInput.text + ". Mã PIN của bạn là: " + dynamicPin)
                    backend.sendSignal(999, w, h, dynamicPin)

                    // Màn hình chờ duyệt
                    pinDisplay.text = dynamicPin.toString()
                    copyBtn.text = "Copy"

                    // Giấu Pop-up nhập IP, hiện Pop-up mã PIN lên
                    ipPopup.close()
                    pinPopup.open()
                    // Trả lại quyền điều khiển cho màn hình chính
                    //mainWindow.requestActivate()
                }
            }
        }
    }

    // Tự động mở hộp thoại Pop-up này khi App vừa bật lên!
    Component.onCompleted: {
        ipPopup.open()
    }

    // Màn hình hứng Video
    Image {
        id: videoFrame
        anchors.fill: parent
        fillMode: Image.Stretch
        cache: false // Phải tắt cache để load 60fps

        property int counter: 0
        // Ép QML phải vẽ lại ảnh liên tục khi có biến đổi
        source: "image://live/frame" + counter

        Connections {
            target: videoReceiver
            function onFrameUpdated() {
                videoFrame.counter++
                watchdogTimer.restart()
            }

            function onResolutionChanged() {
                console.log("Phát hiện Jetson đổi phân giải: " + videoReceiver.hostWidth + "x" + videoReceiver.hostHeight)
                // Lập tức nã gói tin cấu hình sang Jetson
                backend.sendSignal(777, videoReceiver.hostWidth, videoReceiver.hostHeight, 0)
            }
        }
    }

    // Cơ chế Watchdog: 5 giây không có frame là báo động!
    Timer {
        id: watchdogTimer
        interval: 5000 // 5000ms = 5 giây
        running: true
        repeat: false // Chỉ chạy 1 lần nếu bị timeout
        onTriggered: {
            console.log("[!] Deadlock Detected! GStreamer đã bị lỗi. Gửi lệnh 888 để Reset...")
            // Dùng số 888 làm Magic Number ra lệnh Kill/Restart
            backend.sendSignal(888, 0, 0, 0)
        }
    }

    Timer {
        id: keepAliveTimer
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            // Gửi tin hiệu 111 sang Jetson
            // Jetson nhận được tín hiệu này thì cập nhật lại last_packet_time, không bao giờ bị timeout
            backend.sendSignal(111, 0, 0, 0)
        }
    }

    // Màng bắt chuột
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton

        // Lấy kích thước thật từ C++ (Nếu chưa có data thì ngầm định 1280x720)
        function mapX(mouseX) {
            let hw = videoReceiver.hostWidth > 0 ? videoReceiver.hostWidth : 1280
            return Math.round((mouseX / width) * hw)
        }
        function mapY(mouseY) {
            let hh = videoReceiver.hostHeight > 0 ? videoReceiver.hostHeight : 720
            return Math.round((mouseY / height) * hh)
        }

        onPositionChanged: (mouse) => {
            let currentClick = 0
            if (mouse.buttons & Qt.LeftButton) currentClick = 1
            else if (mouse.buttons & Qt.RightButton) currentClick = 2
            else if (mouse.buttons & Qt.MiddleButton) currentClick = 3
            backend.sendMouseData(mapX(mouse.x), mapY(mouse.y), currentClick)
        }

        onPressed: (mouse) => {
            let clickType = 0
            if (mouse.button === Qt.LeftButton) clickType = 1
            else if (mouse.button === Qt.RightButton) clickType = 2
            else if (mouse.button === Qt.MiddleButton) clickType = 3
            mainArea.forceActiveFocus()
            backend.sendMouseData(mapX(mouse.x), mapY(mouse.y), clickType)
        }

        onReleased: (mouse) => {
            backend.sendMouseData(mapX(mouse.x), mapY(mouse.y), 0)
        }
        onWheel: (wheel) => {
            // angleDelta.y thường mang giá trị 120 (lên) hoặc -120 (xuống) mỗi khấc lăn
            let scrollDir = (wheel.angleDelta.y > 0) ? 1 : -1

            // Bắn luồng scroll sang (x, y hiện tại, click=0, scroll=scrollDir)
            backend.sendMouseData(mapX(wheel.x), mapY(wheel.y), 0, scrollDir)
        }
    }

    Item {
        id: mainArea
        anchors.fill: parent
        focus: true

        // Khi phím được bấm xuống
        Keys.onPressed: (event) => {
            // Chặn spam liên tục khi nhấn giữ 1 phím
            if (!event.isAutoRepeat) {
                backend.sendKeyData(event.nativeScanCode - 8, 1)
            }
            event.accepted = true
        }

        // Khi phím được nhả ra
        Keys.onReleased: (event) => {
            if (!event.isAutoRepeat) {
                backend.sendKeyData(event.nativeScanCode - 8, 0)
            }
            event.accepted = true
        }
    }

    // BẪY SỰ KIỆN: Khi người dùng bấm nút X tắt cửa sổ
    onClosing: {
        console.log("[!] Đang đóng app, gửi lệnh khai tử 998 cho Jetson...")
        // Ném lệnh 998 đi trước khi hệ thống kịp giết app (x, y, pin lúc này truyền 0)
        backend.sendSignal(998, 0, 0, 0)
    }
}

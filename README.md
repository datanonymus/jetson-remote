# 🚀 Jetson Remote V2.1.0
**Ultra Low-Latency Hardware-Accelerated Remote Desktop & Monitoring for NVIDIA Jetson**

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-NVIDIA%20Jetson%20%7C%20Linux-green.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Qt](https://img.shields.io/badge/Qt-6.x-41cd52.svg)

Jetson Remote V2.1.0 is a custom-built, ultra-low-latency remote control, streaming, and monitoring solution designed specifically for Robotics Engineers working with ROS2 on NVIDIA Jetson edge devices (Jetson Nano, Xavier NX, Orin, etc.). 

By bypassing traditional laggy protocols (like VNC or AnyDesk) and utilizing Jetson's native hardware encoder (`nvv4l2h264enc`) via UDP, this tool delivers real-time X11 desktop streaming and kernel-level mouse/keyboard injection.

## ✨ What's New in V2.0:
* **Real-time Web Dashboard:** Built-in lightweight HTTP server (Port 8080) streaming real-time hardware telemetry (`tegrastats`) including CPU, GPU, RAM, and Power consumption to a responsive Web UI.
* **Smart Power Management (Watchdog):** GStreamer pipeline automatically goes to sleep when the Client disconnects (Signal 998) or times out, saving critical battery/power on the Jetson.
* **Dynamic Resolution Auto-Sync:** The virtual mouse automatically grabs the current X11 display resolution and perfectly bounds the pointer without manual configuration.
* **Full Mouse & ROS2 Support:** Complete integration of Left, Right, and **Middle-click** (essential for Rviz map panning) and Scroll Wheel.
* **Zero-Lag Video Streaming:** Uses NVIDIA's NVMM (Hardware Acceleration) to compress and stream H.264 video directly over UDP.
## ✨ What's New in V2.1.0:
* **PIN Access and Save Client Info Function**: With the PIN Code Function, you can control who can access Jetson. Once you've granted access, you won't need to enter PIN Code on Web Server anymore.
* **Fixed Some Problems**: Fixed some bugs that I found and added some Pop-ups to give you a better experiences.

## 🏗 Architecture
This repository contains two main components:
* `Server_Jetson/`: The C++ UDP Server & Web API running on the NVIDIA Jetson.
* `Remote_Jetson_UI/`: The Qt5/Qt6/QML Client App running on your Host PC (Linux).

---

## 🛠 1. Jetson Server Setup

### Prerequisites
Your Jetson must be running NVIDIA L4T (Linux for Tegra) with the X11 display server.

```bash
sudo apt update
sudo apt install gstreamer1.0-tools gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav libx11-dev
```

### Build & Run
```bash
cd Server_Jetson
g++ jetson_remote.cpp jetson_remote_main.cpp -o jetson_remote -lpthread
sudo -E ./jetson_remote :0
```

*(Note: The `sudo -E` flag is required because the server needs root permission to access `/dev/uinput` for hardware-level mouse/keyboard simulation, while preserving your user's X11 environment variables).*

### Web Dashboard
Once the server is running, open any web browser on your network and navigate to:
`http://<Jetson_IP>:8080`
---

## 💻 2. Client UI Setup

### Prerequisites
You need a standard Linux distribution with Qt5 or Qt6 development packages installed.

### 1. For Qt5 development packages (Ubuntu 20.04/Ubuntu 22.04):
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake ninja-build \
    qtcreator qtbase5-dev qtdeclarative5-dev qttools5-dev qttools5-dev-tools \
    qml-module-qtquick-controls2 qml-module-qtquick-window2 \
    qml-module-qtmultimedia libqt5multimedia5-plugins qtmultimedia5-dev \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-qt5 libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```

### 2. For Qt6 development packages (Ubuntu 24.04/Ubuntu 26.04):
```bash
sudo apt update
sudo apt install -y \
    build-essential cmake ninja-build \
    qtcreator qt6-base-dev qt6-declarative-dev \
    qt6-tools-dev qt6-tools-dev-tools qt6-l10n-tools \
    qt6-multimedia-dev qml6-module-qtmultimedia \
    qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-window \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav gstreamer1.0-qt6 libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```

### Build & Run
1. Open the Remote_Jetson_UI project in **Qt Creator** or use `cmake` manually.
2. Build the project using the **Release** configuration.
3. Run the application.
4. Enter your Jetson's IP Address (LAN or VPN like Tailscale) and click **Connect**.
6. If you enter the server for the first time, a PIN Pop-up will display to give you a PIN code. Copy it and send to Admin server to grant access.
5. If you want to change IP Address without quit and run the app again, press `Ctrl + I` (or click the UI) to open the connection popup.

### 3. For Windows:
1. Just install already compiled app in Release tab
2. Run the application.
3. Enter your Jetson's IP Address (LAN or VPN like Tailscale) and click **Connect**.
4. If you enter the server for the first time, a PIN Pop-up will display to give you a PIN code. Copy it and send to Admin server to grant access.
5. If you want to change IP Address without quit and run the app again, press `Ctrl + I` (or click the UI) to open the connection popup.

---

## ⚠️ Known Limitations & Troubleshooting
* **NVIDIA 40" Headless Bug:** If you boot the Jetson without an HDMI monitor attached, Nvidia's driver defaults to a locked 720p virtual display and display the NVIDIA logo, never enter the desktop. **Solution:** Use an HDMI Dummy Plug to emulate display.
* **Keyboard Mapping (The "-8" Rule):** The Qt Client currently subtracts 8 from `nativeScanCode` to perfectly match X11/evdev mappings on Linux. Running the Qt Client on Windows may result in incorrect keystrokes.

---
**Developed by Nguyễn Trọng Đạt (With Gemini support)**

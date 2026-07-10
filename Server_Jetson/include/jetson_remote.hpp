// --- jetson_remote.h ---
#ifndef JETSON_REMOTE_HPP
#define JETSON_REMOTE_HPP

#include <thread>
#include <chrono>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <linux/uinput.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdlib>
#include <mutex>
#include <csignal>
#include <arpa/inet.h>
#include <fstream>
#include "httplib.h"

// Định nghĩa gói tin
struct MouseAndKeyboardPacket {
    int x, y, click, scroll; // Dùng cho chức năng của chuột
    int signal; // Dùng để gửi lệnh đặc biệt như "bật stream" hoặc "tắt stream"
    int is_keyboard, keycode, keystate; // Dùng cho chức năng của bàn phím
};

namespace JetsonRemote {
    // Khai báo biến toàn cục
    extern int uinput_fd;
    extern std::mutex mouse_mtx;
    extern std::string target_display;
    extern std::string target_ip;
    extern std::string target_bitrate;
    extern std::string latest_tegrastats;
    extern std::mutex stats_mtx;
    extern bool is_streaming;
    extern std::string pending_client_ip; // Biến toàn cục chứa mã pin
    extern int pairing_pin;
    extern bool is_allowed; // Biến kiểm tra xem đã được cấp phép hay chưa
    extern std::chrono::time_point<std::chrono::steady_clock> last_packet_time; // Biến thời gian toàn cục

    // Khai báo các hàm sẽ dùng
    void init_virtual_mouse(int width, int height);
    void cleanup_and_exit(int signum);
    void restart_gstreamer();
    void emit_event(int fd, int type, int code, int val);
    void monitor_resolution();
    std::string get_tegrastats_string();
    void start_web_server();
    void tegrastats_worker();
    void stop_gstreamer();
}
#endif // JETSON_REMOTE_HPP
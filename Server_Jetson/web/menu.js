const menuToggle = document.getElementById('menu-toggle');
const sidebar = document.getElementById('sidebar');
const overlay = document.getElementById('overlay');
const closeBtn = document.getElementById('close-btn');

// Hàm Tắt Sidebar
function closeSidebar() {
    sidebar.classList.remove('open');
    overlay.classList.remove('show');
}

// Bấm nút 3 gạch -> Mở Sidebar + Hiện màng đen
menuToggle.addEventListener('click', () => {
    sidebar.classList.add('open');
    overlay.classList.add('show');
});

// Bấm nút X hoặc bấm ra ngoài màng đen -> Đóng Sidebar
closeBtn.addEventListener('click', closeSidebar);
overlay.addEventListener('click', closeSidebar);

// Hàm chuyển Tab
function openTab(tabId, element) {
    const tabs = document.getElementsByClassName('tab-content');
    for (let i = 0; i < tabs.length; i++) {
        tabs[i].classList.remove('active');
    }
            
    const navs = document.querySelectorAll('.nav-links li');
    navs.forEach(nav => nav.classList.remove('active'));

    document.getElementById(tabId).classList.add('active');
    element.classList.add('active');

    closeSidebar();

    // Khi mở tab Monitor, giả vờ như màn hình vừa bị đổi kích thước để  Chart.js tự động giãn nở ra!
    if (tabId === 'monitor') {
        setTimeout(() => {
            window.dispatchEvent(new Event('resize'));
        }, 50); // Chờ 50ms cho CSS nó phình ra xong mới gọi
    }
}
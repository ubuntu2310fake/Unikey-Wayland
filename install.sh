#!/usr/bin/env bash
set -e

REPO_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLIENT_DIR="$REPO_DIR/wayland-client"

echo "=========================================================="
echo "    Vietnamese IME (Unikey-Wayland) Installation Script   "
echo "=========================================================="

# 1. Check prerequisites
echo "🔍 [1/4] Kiểm tra môi trường và công cụ biên dịch..."
command -v go >/dev/null 2>&1 || { echo "❌ Lỗi: Chưa cài đặt 'go' (Golang)."; exit 1; }
command -v g++ >/dev/null 2>&1 || { echo "❌ Lỗi: Chưa cài đặt 'g++'."; exit 1; }
command -v gcc >/dev/null 2>&1 || { echo "❌ Lỗi: Chưa cài đặt 'gcc'."; exit 1; }
command -v pkg-config >/dev/null 2>&1 || { echo "❌ Lỗi: Chưa cài đặt 'pkg-config'."; exit 1; }
command -v wayland-scanner >/dev/null 2>&1 || { echo "❌ Lỗi: Chưa cài đặt 'wayland-scanner'."; exit 1; }

# Tìm kiếm Qt6 moc binary
MOC_BIN=""
for path in \
    "$(which moc-qt6 2>/dev/null)" \
    "$(which moc 2>/dev/null)" \
    "/usr/lib64/qt6/libexec/moc" \
    "/usr/lib/qt6/libexec/moc" \
    "/usr/lib64/qt6/bin/moc" \
    "/usr/lib/qt6/bin/moc" \
    "/usr/local/qt6/bin/moc"
do
    if [ -n "$path" ] && [ -x "$path" ]; then
        MOC_BIN="$path"
        break
    fi
done

if [ -z "$MOC_BIN" ]; then
    echo "❌ Lỗi: Không tìm thấy Qt6 moc. Vui lòng cài đặt gói phát triển Qt6 (qt6-qtbase-devel / qt6-base-dev)."
    exit 1
fi
echo "   -> Tìm thấy Qt6 moc tại: $MOC_BIN"

cd "$CLIENT_DIR"

# 2. Build Go Bamboo Core C-Archive
echo "🔨 [2/4] Đang biên dịch lõi Go Bamboo Core (libbamboo.a)..."
cd src
CGO_ENABLED=1 go build -buildmode=c-archive -o libbamboo.a bamboo_wrapper.go
cd ..

# 3. Generate Wayland Protocols & Qt MOC
echo "⚙️ [3/4] Đang sinh mã giao thức Wayland và biên dịch C++..."
wayland-scanner client-header protocols/input-method-unstable-v1.xml include/input-method-unstable-v1-client-protocol.h
wayland-scanner private-code protocols/input-method-unstable-v1.xml src/input-method-unstable-v1-protocol.c

"$MOC_BIN" src/mainwindow.h -o src/moc_mainwindow.cpp
"$MOC_BIN" src/macrodialog.h -o src/moc_macrodialog.cpp
"$MOC_BIN" src/windowtracker.h -o src/moc_windowtracker.cpp
"$MOC_BIN" src/trayicon.h -o src/moc_trayicon.cpp

mkdir -p "$HOME/.local/bin"
gcc -c src/input-method-unstable-v1-protocol.c -Iinclude -o src/input-method-unstable-v1-protocol.o
g++ -std=c++17 -O2 -D_LINUX -Isrc -Iinclude $(pkg-config --cflags Qt6Widgets Qt6Gui Qt6Core Qt6DBus wayland-client) \
  src/main.cpp src/mainwindow.cpp src/macrodialog.cpp src/windowtracker.cpp src/trayicon.cpp \
  src/moc_mainwindow.cpp src/moc_macrodialog.cpp src/moc_trayicon.cpp src/moc_windowtracker.cpp \
  src/input-method-unstable-v1-protocol.o src/libbamboo.a \
  $(pkg-config --libs Qt6Widgets Qt6Gui Qt6Core Qt6DBus wayland-client) -lpthread -lresolv \
  -o "$HOME/.local/bin/unikey-wayland"

# 4. Install Desktop Entry, Icon & Configure KWin
echo "✨ [4/4] Cài đặt Desktop Entry, Icon và kích hoạt KWin Virtual Keyboard..."
mkdir -p "$HOME/.local/share/applications" "$HOME/.local/share/icons/hicolor/scalable/apps"
cp "$REPO_DIR/io.github.ubuntu2310fake.UnikeyWayland.desktop" "$HOME/.local/share/applications/"
cp "$REPO_DIR/io.github.ubuntu2310fake.UnikeyWayland.svg" "$HOME/.local/share/icons/hicolor/scalable/apps/"
update-desktop-database "$HOME/.local/share/applications/" 2>/dev/null || true

# Tạo default preedit_apps.txt nếu chưa có
mkdir -p "$HOME/UnikeyWayland"
if [ ! -f "$HOME/UnikeyWayland/preedit_apps.txt" ]; then
    cat << 'APPS' > "$HOME/UnikeyWayland/preedit_apps.txt"
kitty
alacritty
konsole
gnome-terminal
xfce4-terminal
lxterminal
android-studio
java
APPS
fi

# Cấu hình KWin Wayland
if command -v kwriteconfig6 >/dev/null 2>&1; then
    kwriteconfig6 --file kwinrc --group Wayland --key InputMethod "$HOME/.local/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop"
    kwriteconfig6 --file kwinrc --group Wayland --key VirtualKeyboard "$HOME/.local/share/applications/io.github.ubuntu2310fake.UnikeyWayland.desktop"
    kwriteconfig6 --file kwinrc --group Wayland --key VirtualKeyboardEnabled true
fi

# Kích hoạt / Tải lại bộ gõ
pkill -9 unikey-wayland 2>/dev/null || true
dbus-send --session --dest=org.kde.KWin --type=method_call /KWin org.kde.KWin.reconfigure 2>/dev/null || true
dbus-send --session --dest=org.kde.KWin --type=method_call /VirtualKeyboard org.freedesktop.DBus.Properties.Set string:"org.kde.kwin.VirtualKeyboard" string:"enabled" variant:boolean:true 2>/dev/null || true

echo "=========================================================="
echo "🎉 Cài đặt hoàn tất! Unikey-Wayland đã được kích hoạt."
echo "⌨️ Phím tắt mặc định: Ctrl + Shift để chuyển đổi V/E."
echo "=========================================================="

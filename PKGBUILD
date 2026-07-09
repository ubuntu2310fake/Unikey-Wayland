# Maintainer: Trương Hiếu <truonghieu@example.com>
pkgname=unikey-wayland-git
_pkgname=unikey-wayland
pkgver=1.0.2.r0.g5835b34
pkgrel=1
pkgdesc="Unikey Wayland input method for Vietnamese (Qt6/Wayland)"
arch=('x86_64')
url="https://github.com/ubuntu2310fake/Unikey-Wayland"
license=('GPL')
depends=('qt6-base' 'qt6-wayland' 'wayland')
makedepends=('git' 'cmake' 'wayland-protocols')
optdepends=(
  'gnome-shell: For active window class tracking under GNOME Wayland'
  'kwin: For active window class tracking under KDE Plasma Wayland'
)
provides=("$_pkgname")
conflicts=("$_pkgname")
source=("$_pkgname::git+${url}.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/$_pkgname"
  git describe --long --tags 2>/dev/null | sed 's/\([^-]*-g\)/r\1/;s/-/./g' || \
  printf "1.0.2.r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
  cmake -B build -S "$srcdir/$_pkgname/wayland-client" \
    -DCMAKE_BUILD_TYPE=None \
    -DCMAKE_INSTALL_PREFIX=/usr
  cmake --build build
}

package() {
  DESTDIR="$pkgdir" cmake --install build
}

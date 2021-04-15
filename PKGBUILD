# Maintainer: Inexor <https://discord.gg/Sz4SKXpxRN>

buildarch=

pkgname="inexor-git"
pkgver=0.1.3
pkgrel=1
groups=()
arch=(x86_64)
pkgdesc=
url="https://github.com/inexorgame/vulkan-renderer.git"
license=('MIT')
depends=('conan' 'cmake' 'ninja' 'make')
makedepends=('git' 'conan')
source=("https://github.com/inexorgame/vulkan-renderer.git")
sha512sums=('SKIP')

prepare() {
	git clone https://github.com/inexorgame/vulkan-renderer.git
}

pkgver() {
	cd vulkan-renderer
	cmake . \
	-Bbuild \
	-DCMAKE_BUILD_TYPE=Debug \
	-GNinja
	cd build
	ninja
	cd ..
	./build/bin/inexor-vulkan-renderer-example
}



build() {
	:
}

package(){
	:
}




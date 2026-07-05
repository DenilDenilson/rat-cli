pkgname=rat-llm-git
pkgver=0.8.1.r0.g8ca2a55
pkgrel=1
pkgdesc="Recursive cat para convertir un directorio en contexto listo para pegar en un LLM"
arch=('x86_64')
url="https://github.com/DenilDenilson/rat-llm"
license=('MIT')
depends=('glibc')
makedepends=('git' 'gcc')
provides=('rat-llm')
conflicts=('rat-llm')
options=('!debug')

source=("git+https://github.com/DenilDenilson/rat-llm.git")
sha256sums=('SKIP')

pkgver() {
  cd "$srcdir/rat-llm"
  git describe --long --tags --abbrev=7 2>/dev/null \
    | sed 's/^v//; s/\([^-]*\)-\([0-9]*\)-g\(.*\)/\1.r\2.g\3/' \
    || printf "0.0.0.r%s.g%s" \
         "$(git rev-list --count HEAD)" \
         "$(git rev-parse --short HEAD)"
}

build() {
  cd "$srcdir/rat-llm"
  gcc -O2 -Wall -Wextra -pedantic src/main.c -o rat-llm
}

package() {
  cd "$srcdir/rat-llm"
  install -Dm755 rat-llm "$pkgdir/usr/bin/rat-llm"
  install -Dm644 LICENSE "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
  install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
}
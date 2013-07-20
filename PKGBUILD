# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
_gitname="alopex"
pkgname="${_gitname}-git"
pkgver=2.229.e61ed7a
pkgrel=1
pkgdesc="A Tiny, Tabbed, Tiling Window Manager with Fur"
url="http://trilbywhite.github.io/alopex/"
arch=('any')
license=('GPLv3')
depends=('libx11' 'libxrandr')
makedepends=('git')
source=("${_gitname}::git://github.com/TrilbyWhite/alopex.git")
sha256sums=('SKIP')

pkgver() {
	cd "${_gitname}";
	echo "2.$(git rev-list --count HEAD).$(git describe --always )"
}

prepare() {
	_dir="$HOME/.config/alopex"
	[[ -n "$XDG_CONFIG_HOME" ]] && _dir="$XDG_CONFIG_HOME/alopex"
	if [[ -d "$_dir" ]]; then
		for _file in config.h icons.h theme.h; do
			if [[ -a "${_dir}/${_file}" ]]; then
				cp "${_dir}/${_file}" "${srcdir}/${_gitname}/${_file}"
				msg2 "Using ${_file%.h} from $_dir/$_file"
				msg2 "Check the default $_file for changes"
			fi
		done
	fi
}

build() {
	cd "${_gitname}"
	make
}

package() {
	cd "${_gitname}"
	make PREFIX=/usr DESTDIR="${pkgdir}" install
}

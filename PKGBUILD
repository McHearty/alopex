# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
pkgname=ttwm-git
_gitname="ttwm"
pkgver=0.181.1a63f31
pkgrel=1
pkgdesc="Tiny Tiler/Tabbed Tiler Window Manager"
url="http://github.com/TrilbyWhite/ttwm.git"
arch=('any')
license=('GPLv3')
depends=('libx11' 'libxrandr')
makedepends=('git')
source=("$_gitname::git://github.com/TrilbyWhite/ttwm.git")
sha256sums=('SKIP')

pkgver() {
	cd "${srcdir}/$_gitname";
	echo 1.$(git rev-list --count HEAD).$(git describe --always )
}

prepare() {
	for config in {"$HOME"/.ttwm_,"$XDG_CONFIG_HOME"/ttwm/}{conf,config,icons}.h; do
		if [[ -f "$config" ]]; then
			case "$config" in
				*conf.h | *config.h)
					cp "$config" "${srcdir}"/ttwm/config.h
					echo "Using configuration from $config"
					echo "Check the default config.h for changes"
					;;
				*icons.h)
					cp "$config" "${srcdir}"/ttwm/icons.h
					echo "Using icons from $config"
					;;
			esac
		fi
	done
}

build() {
	cd "$_gitname"
	make
	## try different initial themes:
	##   WinterCoat is assumed if none are specified.
	#MOLT=WinterCoat make
	#MOLT=SummerCoat make
}

package() {
	cd "$_gitname"
	make PREFIX=/usr DESTDIR="${pkgdir}" install
}

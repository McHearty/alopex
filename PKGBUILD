# Maintainer: Jesse McClure AKA "Trilby" <jmcclure [at] cns [dot] umass [dot] edu>
pkgname=ttwm-git
pkgver=20120811
pkgrel=4
pkgdesc="Tiny Tiler/Tabbed Tiler Window Manager"
url="http://github.com/TrilbyWhite/ttwm.git"
arch=('any')
license=('GPLv3')
depends=('libx11')
makedepends=('git')
_gitroot="git://github.com/TrilbyWhite/ttwm.git"
_gitname="ttwm"

build() {
    cd "$srcdir"
    msg "Connecting to GIT server...."
    if [ -d $_gitname ] ; then
        cd $_gitname && git pull origin
        msg "The local files are updated."
    else
        git clone $_gitroot $_gitname
    fi
    msg "GIT checkout done or server timeout"
    msg "Starting make..."
    rm -rf "$srcdir/$_gitname-build"
    git clone "$srcdir/$_gitname" "$srcdir/$_gitname-build"
    cd "$srcdir/$_gitname-build"
	if [[ -f ~/.ttwm_conf.h ]]; then
		msg "Using user config from ~/.ttwm_conf.h"
		msg "  Be sure to check for changes to default config.h"
		cp ~/.ttwm_conf.h ./config.h
	else
		msg "Using default config"
	fi
	make
}

package() {
	cd "$srcdir/$_gitname-build"
	make PREFIX=/usr DESTDIR=$pkgdir install
}


#!/usr/bin/make -f
## boostrap.mk (for aconnectgui)
## Mac Radigan

.PHONY: bootstrap update packages-apt install
.DEFAULT_GOAL := bootstrap

bootstrap:
	autoreconf --force --install

install: bootstrap
	./configure --prefix=/opt/local && make && sudo make install

update:
	git pull

packages-apt:
	sudo apt-get install -y libfltk1.3-dev
	sudo apt-get install -y autoconf
	sudo apt-get install -y automake
	sudo apt-get install -y libtool
	sudo apt-get install -y build-essential

## *EOF*

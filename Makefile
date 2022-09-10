# Main makefile for SCORE
PROG = SCOREPLUS
TRGT = POWER_SIMULATION POWER_SLAVE COORDINATOR
$(PROG): $(TRGT)
POWER_SIMULATION:
	cd ./powersimulation; make
POWER_SLAVE:
	cd ./powerslave; make
COORDINATOR:
	cd ./coordinator; make
install:
#	This is for installed demander energy services, not necessary. And the install process assumes .core, not go well with current.
#	cd ./coreconfig; make install
	sudo ln -s $(CURDIR)/powersimulation/dist/Debug/GNU-Linux-x86/powersimulation /usr/local/sbin/powersimulation
	sudo ln -s $(CURDIR)/powerslave/POWER_SLAVE /usr/local/sbin/powerslave
	sudo ln -s $(CURDIR)/coordinator/dist/Debug/GNU-Linux-x86/coordinator /usr/local/sbin/coordinator
	chmod 755 ./scoreplusinit.bash
	sudo ln -s $(CURDIR)/scoreplusinit.bash /etc/init.d/scoreplus
#	sudo cp ./scoreplusinit.bash /etc/init.d/scoreplus
	chmod 755 ./scoreplus 

uninstall:
	sudo rm /usr/local/sbin/powersimulation
	sudo rm /usr/local/sbin/powerslave
	sudo rm /usr/local/sbin/coordinator
	sudo rm /etc/init.d/scoreplus
release:
	mkdir $(HOME)/Desktop/scoreplus
	cp -R * $(HOME)/Desktop/scoreplus
	cd $(HOME)/Desktop/scoreplus;find . -name .svn -print0 | xargs -0 rm -rf
	cd $(HOME)/Desktop;tar -zcvf scoreplus.tar.gz scoreplus
	cd $(HOME)/Desktop;rm -rf scoreplus

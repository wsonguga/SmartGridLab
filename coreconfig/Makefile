install:
	chmod 755 configure
	./configure
	cp *.py ~/.core/myservices/
	cp nodes.conf ~/.core/
	cd ..
	sudo sed -i 's/#controlnet = 172.16.0.0\/24/controlnet = 172.16.0.0\/24/' /etc/core/core.conf
	sudo sed -i "23icustom_services_dir = $(HOME)/.core/myservices" /etc/core/core.conf
	

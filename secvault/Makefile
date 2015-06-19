SOURCE = secvault.c secvault.h svctl.c secvault_load secvault_unload Makefile

obj-m += secvault.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=um V=1 modules
	gcc -o svctl svctl.c 

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) ARCH=um V=1 clean
	rm -f svctl

svctl: 
	gcc -o svctl svctl.c 

upload: $(SOURCE) 
	echo "Uploading $(SOURCE) ..."
	scp $(SOURCE) os:~/test_module/ 

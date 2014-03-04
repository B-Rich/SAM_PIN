all: inscount.cpp XML/xmlwriter.cpp
	g++ -DBIGARRAY_MULTIPLIER=1 -Wall -Werror -Wno-unknown-pragmas \
	 -fno-stack-protector -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX  -I/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//source/include/pin -I/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//source/include/pin/gen -I/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//extras/components/include -I/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//extras/xed2-intel64/include -I/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//source/tools/InstLib -O3 -fomit-frame-pointer -fno-strict-aliasing -c inscount.cpp -o obj-intel64/inscount.o

	g++ -c -Wall -fPIC XML/xmlwriter.cpp

	g++ -shared -Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//source/include/pin/pintool.ver    -o obj-intel64/inscount.so obj-intel64/inscount.o xmlwriter.o -L/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//intel64/lib -L/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//intel64/lib-ext -L/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//intel64/runtime/glibc -L/home/asus/Downloads/pin-2.13-62732-gcc.4.4.7-linux//extras/xed2-intel64/lib -lpin -lxed -ldwarf -lelf -ldl

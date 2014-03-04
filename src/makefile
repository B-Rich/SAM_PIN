PIN_USER = $(PIN_ROOT)


all: inscount.cpp XML/xmlwriter.cpp
	g++ -DBIGARRAY_MULTIPLIER=1 -Wall -Werror -Wno-unknown-pragmas \
	 -fno-stack-protector -DTARGET_IA32E -DHOST_IA32E -fPIC -DTARGET_LINUX \
	-I$(PIN_USER)//source/include/pin \
	-I$(PIN_USER)//source/include/pin/gen \
	-I$(PIN_USER)//extras/components/include \
	-I$(PIN_USER)//extras/xed2-intel64/include \
	-I$(PIN_USER)//source/tools/InstLib \
	-O3 -fomit-frame-pointer -fno-strict-aliasing -c inscount.cpp -o obj-intel64/inscount.o

	g++ -c -Wall -fPIC XML/xmlwriter.cpp

	g++ -shared -Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=$(PIN_USER)//source/include/pin/pintool.ver \
	-o obj-intel64/inscount.so obj-intel64/inscount.o xmlwriter.o \
	-L$(PIN_USER)//intel64/lib \
	-L$(PIN_USER)//intel64/lib-ext \
	-L$(PIN_USER)//intel64/runtime/glibc \
	-L$(PIN_USER)//extras/xed2-intel64/lib \
	-lpin -lxed -ldwarf -lelf -ldl

clean:
	rm *.o XML/*.o *.out obj-intel64/* *.log
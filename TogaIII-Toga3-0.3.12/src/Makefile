GCC=g++
CFLAGS=-static -Os -fomit-frame-pointer -DIS_64BIT -std=c++11 -s -flto -ffunction-sections -fdata-sections -Wl,--gc-sections
WIN=x86_64-w64-mingw32-g++
WINFLAGS=-static -Wall -fstrict-aliasing -fno-exceptions -fno-rtti  -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fPIC -fomit-frame-pointer -std=c++11
WIN2FLAGS=-static -Wall -DIS_64BIT -fstrict-aliasing -fno-exceptions -fno-rtti  -Wno-unused-variable -Wno-unused-result -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fPIC -fomit-frame-pointer -std=c++11

all:
	ld -r -b binary -o toganet.o toganet.bin
	$(GCC) $(CFLAGS) *.cpp ./nnue/*.cpp toganet.o -o toga3_basic -lm -lpthread

win:
	$(WIN) $(WINFLAGS) -Ofast *.cpp ./nnue/*.cpp toganet.o -o toga3_avx2.exe -lm -lpthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -DUSE_AVX2 -mavx2
	$(WIN) $(WINFLAGS) -Ofast *.cpp ./nnue/*.cpp toganet.o -o toga3_bmi.exe -lm -lpthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse -DUSE_AVX2 -mavx2 -mbmi2
	$(WIN) $(WINFLAGS) -Ofast *.cpp ./nnue/*.cpp toganet.o -o toga3_sse.exe -lm -lpthread -DUSE_SSE41 -msse4.1 -DUSE_SSSE3 -mssse3 -DUSE_SSE2 -msse2 -DUSE_SSE -msse
	$(WIN) $(WINFLAGS) -Ofast *.cpp ./nnue/*.cpp toganet.o -o toga3_basic.exe -lm -lpthread

release: all win
	rm -f *.zip
	zip -9r linux.zip toga3_bmi toga3_avx2 toga3_sse toga3_basic
	zip -9r win.zip toga3_avx2.exe toga3_sse.exe toga3_basic.exe toga3_bmi.exe


CC = gcc
CXX = g++

OUTPUT := dual_tool.exe
OBJS=main.o debug.o ../eternity_common/Utils.o ../eternity_common/crypto/sha1.o ../eternity_common/crypto/md5.o ../eternity_common/crypto/rijndael.o 
OBJS += ../eternity_common/BaseFile.o ../eternity_common/DBXV2/BcmFile.o ../eternity_common/DBXV2/BacFile.o
OBJS += ../eternity_common/tinyxml/tinyxml.o ../eternity_common/tinyxml/tinystr.o ../eternity_common/tinyxml/tinyxmlerror.o ../eternity_common/tinyxml/tinyxmlparser.o
CFLAGS=-Wall -I. -I../eternity_common -std=gnu99 -mno-ms-bitfields -O2 -masm=intel 
CFLAGS += -static
CXXFLAGS=-Wall -I../eternity_common -O2 -std=c++11 -DTIXML_USE_STL -masm=intel 
LDFLAGS=-L. 
LIBS =  -lz -lstdc++ -lversion

all: $(OUTPUT)

clean:
	rm -f $(OUTPUT) *.o
	rm -f ../eternity_common/*.o
	rm -f ../eternity_common/DBXV2/*.o
	rm -f ../eternity_common/tinyxml/*.o
	rm -f ../eternity_common/crypto/*.o

$(OUTPUT): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $^ $(LIBS)

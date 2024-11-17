TARGETNAME=test
TARGETTYPE=exec
INCFLAGS=-I../../../include -I../../../build/include
LDFLAGS=-Wl,-rpath=$(shell pwd -P)/../../.. \
		-Wl,-rpath=$(shell pwd -P)/../../../build \
		-L../../.. -L../../../build -lzoro

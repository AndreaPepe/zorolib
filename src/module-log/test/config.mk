TARGETNAME=test
TARGETTYPE=exec
INCFLAGS=-I../../../include -I../../../build/include
LDFLAGS=-Wl,-rpath=$(shell pwd -P)/../../.. -L../../.. -L../../../build -lzoro

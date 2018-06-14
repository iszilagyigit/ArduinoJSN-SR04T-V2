
#
# Makefile to compile  for 
# AndroidRT on Raspberry PI3
#
#

# prefix for cross-compiling from a linux computer.
TOOLPREFIX = ~/work/arm7-andoid-toolchain/bin/arm-linux-androideabi-
#TOOLPREFIX = ~/work/arm7-andoid-toolchain/bin/

# compiler flags -march=armv7-a
CFLAGS  = -Wall -march=armv7-a

# Linker flags 
# -pie Position Independent Executable (needed for android)
# see also: 
# https://stackoverflow.com/questions/39957435/text-relocations-despite-fpic
# https://stackoverflow.com/questions/33510601/avoiding-text-relocations-when-mixing-c-c-and-assembly-in-a-so

# -s strip all symbols (for small size)
LDFLAGS = -pie -s

# note: clang produce smaller size (-dn options accepted only if gcc used)

all: clean compile
 
compile: spi

spi:
	$(TOOLPREFIX)clang $(CFLAGS) -o spitest $(LDFLAGS) spitest.c
	adb push spitest /data/test/ 


.PHONY : clean 	
clean:
	rm -vf spitest

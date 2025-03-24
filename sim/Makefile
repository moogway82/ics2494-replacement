# SOURCES = ics2494-replacement-board.c 
# OBJS = ics2494-replacement-board.o
# TARGET = ics2494-replacement-board.elf
SIMAVR = ../../simavr

all: ics2494-replacement-board.elf ics2494-replacement-board.c i2c_eeprom.c

ics2494-replacement-board.elf: ics2494-replacement-board.o i2c_eeprom.o
	cc -MMD -O2 \
		-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-missing-field-initializers -Wno-sign-compare \
		-g -fPIC  -DHAVE_LIBELF=1   \
		-Wl,-rpath,${SIMAVR}/obj-x86_64-linux-gnu \
		-o ics2494-replacement-board.elf ics2494-replacement-board.o i2c_eeprom.o \
		-L/${SIMAVR}/obj-x86_64-linux-gnu -lsimavr -lm -lelf 

ics2494-replacement-board.o: ics2494-replacement-board.c
	cc --std=gnu99 -Wall \
		-I. -I/include -I${SIMAVR}/examples/parts -I${SIMAVR}/include -I${SIMAVR}/simavr/sim \
		-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-missing-field-initializers -Wno-sign-compare \
		-g -fPIC  -DHAVE_LIBELF=1  -MMD ics2494-replacement-board.c -c \
		-o ics2494-replacement-board.o

i2c_eeprom.o: i2c_eeprom.c
	cc --std=gnu99 -Wall \
		-I. -I/include -I${SIMAVR}/examples/parts -I${SIMAVR}/include -I${SIMAVR}/simavr/sim \
		-O2 -Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-missing-field-initializers -Wno-sign-compare \
		-g -fPIC  -DHAVE_LIBELF=1  -MMD i2c_eeprom.c -c -o i2c_eeprom.o


clean:
	rm ics2494-replacement-board.o i2c_eeprom.o ics2494-replacement-board.elf ics2494-replacement-board.d i2c_eeprom.d


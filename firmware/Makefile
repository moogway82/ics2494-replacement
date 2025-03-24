SOURCES = main.c si5351a_registers.h USI_TWI_Master.c USI_TWI_Master.h
TARGET = ics2494-replacement.elf
DEBUG_TARGET = ics2494-replacement.axf
SIMAVR = ../../simavr

all: ${SOURCES} ${TARGET}

${TARGET}: ${SOURCES}
	avr-gcc -Wall -Os -std=gnu99 \
        -mmcu=attiny85 \
        -DF_CPU=8000000 \
        -fno-inline-small-functions \
        -ffunction-sections -fdata-sections \
        -Wl,--relax,--gc-sections \
        -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 \
        -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr \
        ${SOURCES} \
        -o ${TARGET}


debug:${DEBUG_TARGET}

${DEBUG_TARGET}: ${SOURCES}
	avr-gcc -Wall -gdwarf-2 -Os -std=gnu99 \
        -mmcu=attiny85 \
        -DF_CPU=8000000 \
        -fno-inline-small-functions \
        -ffunction-sections -fdata-sections \
        -Wl,--relax,--gc-sections \
        -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 \
        -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr \
        ${SOURCES} \
        -o ${DEBUG_TARGET}

sim: debug
	simavr -g 1234 -m attiny85 ${DEBUG_TARGET}

clean:
	rm ics2494-replacement.axf ics2494-replacement.elf



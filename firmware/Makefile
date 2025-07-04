# OLD GCC Section
# 	avr-gcc -Wall -Os -std=gnu99 \
#         -mmcu=attiny85 \
#         -DF_CPU=8000000 \
#         -fno-inline-small-functions \
#         -ffunction-sections -fdata-sections \
#         -Wl,--relax,--gc-sections \
#         -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000 \
#         -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr -I${SIMAVR}/simavr/sim/avr \
#         ${SOURCES} \
#         -o ${FILENAME}.elf


SOURCES = main.c si5351a_registers.h USI_TWI_Master.c USI_TWI_Master.h
FILENAME = ics2494-replacement
TARGET = ${FILENAME}.hex
DEBUG_TARGET = ${FILENAME}.axf
SIMAVR = ../../simavr

all: ${SOURCES} ${TARGET}

${FILENAME}.elf: ${SOURCES}
	avr-gcc -Wall -Os -std=gnu99 \
        -mmcu=attiny85 \
        -DF_CPU=8000000 \
		${SOURCES} \
        -o ${FILENAME}.elf

	avr-size ${FILENAME}.elf
	avr-objdump -d ${FILENAME}.elf > ${FILENAME}.lst
	avr-objdump -j .data -s ${FILENAME}.elf >> ${FILENAME}.lst

${TARGET}: ${FILENAME}.elf
	avr-objcopy -O ihex ${FILENAME}.elf ${TARGET}


debug:${DEBUG_TARGET}

${DEBUG_TARGET}: ${SOURCES}
	avr-gcc -Wall -gdwarf-2 -Os -std=gnu99 \
        -mmcu=attiny85 \
        -DF_CPU=8000000 \
        ${SOURCES} \
        -o ${DEBUG_TARGET}

sim: debug
	simavr -g 1234 -m attiny85 ${DEBUG_TARGET}

clean:
	rm ${FILENAME}.axf ${TARGET} ${FILENAME}.elf ${FILENAME}.hex ${FILENAME}.lst

run: debug all
	avr-gdb -ex 'target remote :1234' ./${FILENAME}.axf

prog:
	avrdude -c ft232h -p t85 -U flash:w:${TARGET}:i -v

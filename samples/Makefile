SAMPLEROM = czech_v0.1.rom
INDEXTABLE= table_v0.1.rom

all: flash.rom

flash.rom: $(SAMPLEROM) $(INDEXTABLE)
	cat $(INDEXTABLE) $(SAMPLEROM) > flash.rom


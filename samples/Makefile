SAMPLEROM = czech_v0.3_mulaw_16kHz.rom
INDEXTABLE= table_v0.2_16kHz.rom

all: flash.rom

flash.rom: $(SAMPLEROM) $(INDEXTABLE)
	cat $(INDEXTABLE) $(SAMPLEROM) > flash.rom


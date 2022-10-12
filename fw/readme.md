## Notes

#### ADC Driver
The ADC reads each pot 256 times and averages the readings out in order to capture a higher resolution sample. 

Since the ADC doesn't have a hardware accumulator, a DMA channel is used in a weird way. 

The DMA pulls samples from the ADC and writes them to a static "dummy" address. The accumulation is achieved by using the DMA's data sniffer which is intended for generating checksums. 

- ADC samples 256 times in free running mode
- DMA is essentially grabbing these samples and throwing them away
- The DMA is accumulating these samples into the data sniffer register
- The DMA finishes adding 256 12 bit samples (20 bits now)
- An ISR fires, the 20 bit data is brought down to 16 bits and stored in RAM, and the sniffer is cleared.
- The next ADC channel is setup, repeat.

This takes up very little ram, and very little CPU time.  
The ISR fires every 256 ADC samples and only a couple of bytes of RAM is used for the dummy buffer.
# SuddenVFOv1.2
Amendments to the GQRP Club Sudden VFO to allow multiple band switching and relay activation for band filters.

This amended code takes the GQRP Club Sudden VFO kit and the ocde written by Kevin Wheatley, and modifies it to enable a true multi-band HF VFO for 160-10m
including all WARC bands and 60m. Other cosmetic improvements (IMHO!) are also made.

A full list of changes is below:

    - Multiple band selection is now available. The VFO tuning steps now have two functions. A short press 
      continues to change tuning step, whilst a longer press (>0.5s) allows increment / decrement of band from 160-10m.
    - When a band is selected, or VFO's swapped, the Teensy now outputs a LOGIC HIGH to pins 24-33 inclusive, 
      ensuring only one (the correct one for the band) is selected at any time. These pins are accessed by solder
      pads on the UNDERSIDE of the Teensy, so fitting the Teensy direct to the board must be avoided and header
      sockets used instead. The intent is to switch a transistor to drive a relay on a QRPLabs LPF / BPF board.
      The output could be used in parallel to switch both LPF and BPF boards at the same time. They might also be used to
      activate an antenna changeover relay for different bands.
      Each band is allocated a band code and a pin as follows:
          Band  Code Pin
          160   0    24
          80    1    25
          60    2    26
          40    3    27
          30    4    28
          20    5    29
          17    6    30
          15  ` 7    31
          12    8    32
          10    9    33
    - Fixed the issue where the x 10Mhz digit is not removed when moving to a frequency below 10Mhz
    - Cursor for VFO step can now be placed under the x 1 Mhz position - useful for gen. cov. rx or 28Mhz band
    - Unit display changed from Hz to Mhz as the proper unit to match the decimal display
    - Allow a different default frequency and band for each VFO
    - RIT display simplified. I found the full frequency display cluttered. Now RIT is displayed merely as a 
      +- offset from main frequency and sisplayed in Hz.
    
    - All sections of code added or amended by G4USI are commented with a series of +++++++++++++++++++++++++++      

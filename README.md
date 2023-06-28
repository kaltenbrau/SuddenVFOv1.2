PJRC.com announced the Teensy 3.2 used in this project is now obsolete, this is my modified version of the code to allow use of the Teensy 4 instead.
The Teensy 4 is a drop in replacement for the GQRP Sudden VFO project, it will not work without extra hardware for other of Kevin Wheatley's
projects that use the DAC out for raised cosine TX envelope shaping as the chip does not have a DAC output.

All code changes made by M0UAW, Clint, are demarcated by comments with the original code and my callsign/date of changes
*DO NOT REGARD THIS CODE AS FINAL, COMPLETE OR TO HAVE ANY GUARANTEES, IT IS EXPERIMENTAL AND IT IS UP TO YOU TO TEST*


Original project text:
# SuddenVFO - now at v1.3
Amendments to the GQRP Club Sudden VFO to allow multiple band switching and relay activation for band filters.

This amended code takes the GQRP Club Sudden VFO kit and the ocde written by Kevin Wheatley, and modifies it to enable a true multi-band HF VFO for 160-10m
including all WARC bands and 60m. Other cosmetic improvements (IMHO!) are also made.

Two CAD file drawings (pdf and odg format) are attached which give the locations and dimensions of the PCB front components, to aid in cutting a fascia for the PCB to sit behind, for a more professional look. If you are careful you can use the ruler in Word or similar and size the drawing to be a 1:1 match to the outer dimensions, then print it and stick it to your fascia as your cutting guide.

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
    - Vers. 1.3 July 2022 fixed a bug, whereby the simplified RIT display affected the RX output clock. Now fixed in this version.
    
    - All sections of code added or amended by G4USI are commented with a series of +++++++++++++++++++++++++++      

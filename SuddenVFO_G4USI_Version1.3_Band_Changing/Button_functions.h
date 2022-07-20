/*
* ***********************************************************************************
  Copyright (c) 2020  M0KHZ - Kevin Wheatley

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

* ***********************************************************************************
*/


void switch_bouncer() {

  while (1) {

    bouncer_VFO_SW.update ( );
    bouncer_RIT_SW.update ( );
    bouncer_VFO_STEP_UP.update ( );
    bouncer_VFO_STEP_DN.update ( );

    if (bouncer_VFO_SW.fallingEdge()) {

      flag.vfo_A_active = ! flag.vfo_A_active;
      flag.vfo_B_active = ! flag.vfo_B_active;
      flag.freq_Changed = true;
      flag.vfo_A_B_changed = true;
      flag.freq_multi_bar = true;
      flag.bandChanged = true; //this forces us to amend pins for LPF/BPF selection in case the VFOA and VFOB bands were different! +++++++++++++++++++++++++++++
    }

    if (bouncer_RIT_SW.fallingEdge()) {

      flag.keyer_help = !flag.keyer_help;
      if (flag.keyer_help == true) {
        flag.keyer_page_reDraw = true;
      }

      if (flag.keyer_help == false) {
        flag.ReDraw_Screen = true;
        flag.freq_Changed = true;

        flag.Smeter = false;
        flag.vfo_A_B_changed = true;
        flag.freq_multi_bar = true;
        flag.keyer_mode_changed = true;
      }
    }

    if (bouncer_VFO_STEP_UP.fallingEdge()) {

      unsigned long pressStart = millis();//Start timer++++++++++++++++++++

      while (digitalRead(VFO_STEP_UP) == LOW) { //check to see if LOW and change band if button held one second+++++++++++++++
        if (millis() > pressStart + 500) {// to here
          if (flag.vfo_A_active == true) {
            if (bandA == 9) {
              bandA = 0;
            }
            else {
              bandA++;
            }
            u.tune_multi = 2; // we have changed band rather than step, put step marker back to origin ++++++++++++++++++++
            flag.bandChanged = true;
            flag.freq_multi_bar = true;
            break;
          }
          if (flag.vfo_B_active == true) {
            if (bandB == 9) {
              bandB = 0;
            }
            else {
              bandB++;
            }
            u.tune_multi = 2; // we have changed band rather than step, put step marker back to origin+++++++++++++++++++++++++
            flag.bandChanged = true;
            flag.freq_multi_bar = true;
            break;
          }
        }
      }
      
      //only change the frq_step if band not changed+++++++++++++++++++++++
      if (flag.bandChanged == false) {
        u.tune_multi++;
        if (u.tune_multi == 8) u.tune_multi = 1; //was 7 and 6 - seeking to change Mhz +++++++++++++++++++++++++++++++++++
        flag.freq_multi_bar = true;
      }
    }

    if (bouncer_VFO_STEP_DN.fallingEdge()) {

      unsigned long pressStart = millis();//Start timer++++++++++++++++++++

      while (digitalRead(VFO_STEP_DN) == LOW) { //check to see if LOW and change band if button held one second+++++++++++++++
        if (millis() > pressStart + 500) {// to here
          if (flag.vfo_A_active == true) {
            if (bandA == 0) {
              bandA = 9;
            }
            else {
              bandA--;
            }
            u.tune_multi = 2; // we have changed band rather than step, put step marker back to origin
            flag.bandChanged = true;
            flag.freq_multi_bar = true;
            break;
          }
          if (flag.vfo_B_active == true) {
            if (bandB == 0) {
              bandB = 9;
            }
            else {
              bandB--;
            }
            u.tune_multi = 2; // we have changed band rather than step, put step marker back to origin
            flag.bandChanged = true;
            flag.freq_multi_bar = true;
            break;
          }
        }
      }
      
      //only change the frq_step if band not changed+++++++++++++++++++++++
      if (flag.bandChanged == false) {
        u.tune_multi--;
        if (u.tune_multi == 0) u.tune_multi = 7; //was 7 and 6 - seeking to change Mhz +++++++++++++++++++++++++++++++++++
        flag.freq_multi_bar = true;
      }
    }
  }
}




// ****** End of File

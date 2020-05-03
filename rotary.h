/*
 * Pines a usar para CLK y DT del encoder rotativo
 */
Rotary r = Rotary(3, 2);

/*
 * Rutina del encoder
 */
ISR(PCINT2_vect) {
  unsigned char result = r.process();
  if (result) {
    if (result == DIR_CW) {
      rx = rx + increment;
    }
    else {
      rx = rx - increment;
    }
    if (rx >= 30000000) { // Limite superior del VFO: 30MHz
      rx = rx2;
    }
    if (rx <= 1000000) { // Limite inferior del VFO: 1MHz
      rx = rx2;
    }
  }
}

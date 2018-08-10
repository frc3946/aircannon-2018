/******************************************************************************/
/* Golf Cart Air Cannon trigger                                               */
/* Gus Michel II, 2 Aug 2018                                                  */
/*                                                                            */
/* Hardware:                                                                  */
/* Controller: Arduino Pro Micro                                              */
/* Display: SparkFun 9482, a 4 digit 7 segment display, straight LEDs         */
/* Pressure sensor: Amazon B0748BHLQL, 100 psi version                        */
/* Duration: generic 10k poteniometer                                         */
/* Fire button: generic momentary switch, normally open, to ground            */
/* Relay for solenoid: 3-pin from Amazon, normally negative                   */
/*                                                                            */
/* Operation:                                                                 */
/* Pressure is set with the regulator and displayed on the display (see       */
/* below).  When the fire button is pressed, opens the solenoid for a         */
/* duration determined by the duration knob, roughly 0.0 ms to 1 sec.         */
/* The button is then locked out (ignored) for one second.                    */
/*                                                                            */
/* Display:                                                                   */
/* If the pressure is changing, the display shows P###, indicating the        */
/* tank pressure in PSI.  Otherwise, the display shows t###, indicating the   */
/* fire duration in milliseconds.  Number displayed is "##.#" if < 100,       */
/* "###." if >= 100. Note: The display is not driven during firing.           */
/* We can only drive two LEDs at a time due to 40mA limits per pin.  Each LED */
/* is driven for 1 ms; the display loop is throttled to 20 ms for uniformity. */
/******************************************************************************/

/* pin definitions, subject to change */
const int DIGIT1=9, DIGIT2=8, DIGIT3=4, DIGIT4=2;
const int SEGA=14, SEGB=10, SEGC=15, SEGD=7, SEGE=5, SEGF=A1, SEGG=16, SEGH=3;
const int ON=1, OFF=0;

/* LED and lockout durations and lock times, all longs, in microseconds */
const long LED_INT=1000, LOOP_INT=20000;
long loopLock;

void setup() {  // put your setup code here, to run once:
  loopLock=micros(); /* clear lockouts */
  /* initialize the display pins */
  pinMode(SEGA, OUTPUT);
  digitalWrite(SEGA, HIGH);
  pinMode(SEGB, OUTPUT);
  digitalWrite(SEGB, HIGH);
  pinMode(SEGC, OUTPUT);
  digitalWrite(SEGC, HIGH);
  pinMode(SEGD, OUTPUT);
  digitalWrite(SEGD, HIGH);
  pinMode(SEGE, OUTPUT);
  digitalWrite(SEGE, HIGH);
  pinMode(SEGF, OUTPUT);
  digitalWrite(SEGF, HIGH);
  pinMode(SEGG, OUTPUT);
  digitalWrite(SEGG, HIGH);
  pinMode(SEGH, OUTPUT);
  digitalWrite(SEGH, HIGH);
  pinMode(DIGIT1, OUTPUT);
  digitalWrite(DIGIT1, LOW);
  pinMode(DIGIT2, OUTPUT);
  digitalWrite(DIGIT2, LOW);
  pinMode(DIGIT3, OUTPUT);
  digitalWrite(DIGIT3, LOW);
  pinMode(DIGIT4, OUTPUT);
  digitalWrite(DIGIT4, LOW);
}

void loop() {  // put your main code here, to run repeatedly:
  long tim = micros();
  int erval = (tim/10000) % 10000;
  if (tim<loopLock) return;
  loopLock=tim + LOOP_INT;
 /* Serial.print(erval);*/
  show_interval(erval);
}

void show_interval(int interval) { /* shows interval given in tenths of ms */
  show_digit(DIGIT1, OFF, OFF, OFF, ON, ON, ON, ON, OFF);
  show_tenths(DIGIT2, DIGIT3, DIGIT4, interval);
}

void show_tenths(int d0, int d1, int d2, int val) {
  /* Shows an integer specified in tenths in three digits */
  if (val >= 0 && val < 1000) {               /* show ##.# */
    show_numeral(d0, val/100, OFF);
    val %=100;
    show_numeral(d1, val/10, ON);
    show_numeral(d2, val%10, OFF);
  } else if (val >= 1000 && val < 10000) {    /* show ###. */
    show_numeral(d0, val/1000, OFF);
    val %=1000;
    show_numeral(d1, val/100, OFF);
    show_numeral(d2, (val%100)/10, ON);
  } else {                                    /* show Err */
    show_digit(d0, ON,  OFF, OFF, ON,  ON,  ON,  ON,  OFF);
    show_digit(d1, OFF, OFF, OFF, OFF, ON,  OFF, ON,  OFF);
    show_digit(d2, OFF, OFF, OFF, OFF, ON,  OFF, ON,  OFF);
  }
}

void show_numeral(int digit, int numeral, int decimal) { 
  switch (numeral) { /* not used, but works for hexadecimal */
    case 0:  show_digit(digit, ON,  ON,  ON,  ON,  ON,  ON,  OFF, decimal); break;
    case 1:  show_digit(digit, OFF, ON,  ON,  OFF, OFF, OFF, OFF, decimal); break;
    case 2:  show_digit(digit, ON,  ON,  OFF, ON,  ON,  OFF, ON,  decimal); break;
    case 3:  show_digit(digit, ON,  ON,  ON,  ON,  OFF, OFF, ON,  decimal); break;
    case 4:  show_digit(digit, OFF, ON,  ON,  OFF, OFF, ON,  ON,  decimal); break;
    case 5:  show_digit(digit, ON,  OFF, ON,  ON,  OFF, ON,  ON,  decimal); break;
    case 6:  show_digit(digit, ON,  OFF, ON,  ON,  ON,  ON,  ON,  decimal); break;
    case 7:  show_digit(digit, ON,  ON,  ON,  OFF, OFF, OFF, OFF, decimal); break;
    case 8:  show_digit(digit, ON,  ON,  ON,  ON,  ON,  ON,  ON,  decimal); break;
    case 9:  show_digit(digit, ON,  ON,  ON,  ON,  OFF, ON,  ON,  decimal); break;
    case 10: show_digit(digit, ON,  ON,  ON,  OFF, ON,  ON,  ON,  decimal); break;
    case 11: show_digit(digit, OFF, OFF, ON,  ON,  ON,  ON,  ON,  decimal); break;
    case 12: show_digit(digit, ON,  OFF, OFF, ON,  ON,  ON,  OFF, decimal); break;
    case 13: show_digit(digit, OFF, ON,  ON,  ON,  ON,  OFF, ON,  decimal); break;
    case 14: show_digit(digit, ON,  OFF, OFF, ON,  ON,  ON,  ON,  decimal); break;
    case 15: show_digit(digit, ON,  OFF, OFF, OFF, ON,  ON,  ON,  decimal); break;
    default: /* display three horizontal bars and error message */
             show_digit(digit, ON,  OFF, OFF, ON, OFF,  OFF, ON,  decimal);
             Serial.print("Bad numeral value in show_numeral: ");
             Serial.println(numeral);;
  }
}

void show_digit(int digit, int seg_a, int seg_b, int seg_c, int seg_d, 
                int seg_e, int seg_f, int seg_g, int seg_h) {
  /* segments are clockwise from top, then middle, then decimal. */
  if (seg_a) digitalWrite(SEGA, LOW);
  if (seg_b) digitalWrite(SEGB, LOW);
  digitalWrite(digit, HIGH);
  delayMicroseconds(LED_INT);
  digitalWrite(digit, LOW);
  digitalWrite(SEGA, HIGH);
  digitalWrite(SEGB, HIGH);

  digitalWrite(SEGC, seg_c ? LOW: HIGH);
  digitalWrite(SEGD, seg_d ? LOW: HIGH);
  digitalWrite(digit, HIGH);
  delayMicroseconds(LED_INT);
  digitalWrite(digit, LOW);
  digitalWrite(SEGC, HIGH);
  digitalWrite(SEGD, HIGH);

  digitalWrite(SEGE, seg_e ? LOW: HIGH);
  digitalWrite(SEGF, seg_f ? LOW: HIGH);
  digitalWrite(digit, HIGH);
  delayMicroseconds(LED_INT);
  digitalWrite(digit, LOW);
  digitalWrite(SEGE, HIGH);
  digitalWrite(SEGF, HIGH);

  digitalWrite(SEGG, seg_g ? LOW: HIGH);
  digitalWrite(SEGH, seg_h ? LOW: HIGH);
  digitalWrite(digit, HIGH);
  delayMicroseconds(LED_INT);
  digitalWrite(digit, LOW);
  digitalWrite(SEGG, HIGH);
  digitalWrite(SEGH, HIGH);
}

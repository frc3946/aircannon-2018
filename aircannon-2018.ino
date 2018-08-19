/******************************************************************************/
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
const int DURAT_PIN=A2, PRESS_PIN = A3, FIRE_PIN=A0, SOLENOID_PIN=6;
const int DIGIT1=9, DIGIT2=8, DIGIT3=4, DIGIT4=2;
const int SEGA=14, SEGB=10, SEGC=15, SEGD=7, SEGE=5, SEGF=A1, SEGG=16, SEGH=3;
const int ON=1, OFF=0;

/* LED and lockout durations and lock times, all longs, in microseconds */
const long LED_INT=1000, LOOP_INT=20000, FIRE_INT=1000000, PRESS_INT=5000000, DURAT_INT=5000000;
long loopLock, fireLock, pressLock, duratLock;

/* pressure memory is stored in AtoD counts, not dPSI */
const int PRESS_THRESH = 3, DURAT_THRESH = 1;
int prevPres, prevDurat;

void setup() {  // put your setup code here, to run once:
  loopLock=fireLock=pressLock=micros(); /* clear lockouts */
  /* set up non-display pins and get initial pressure */
  pinMode(DURAT_PIN, INPUT);
  pinMode(PRESS_PIN, INPUT);
  pinMode(FIRE_PIN, INPUT_PULLUP);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW);
  prevPres = analogRead(PRESS_PIN);
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
  if (tim<loopLock) return;
  loopLock=tim + LOOP_INT;
  int curPres = analogRead(PRESS_PIN);
  int curDurat = get_interval();
  if (!digitalRead(FIRE_PIN) && tim >= fireLock) {    /* Fire button pressed */
    fire_cannon();
    fireLock = tim + FIRE_INT;   /* lock out the fire function */
  } else { /* display pressure if changing, duration otherwise */
    if (curDurat-prevDurat>DURAT_THRESH || prevDurat-curDurat>DURAT_THRESH) {
      prevDurat=curDurat;
      duratLock = tim + DURAT_INT;
    }
    if (curPres-prevPres>PRESS_THRESH || prevPres-curPres>PRESS_THRESH) {    /* lock in showing pressure */
      prevPres=curPres;
      pressLock = tim + PRESS_INT;
    }
    if (tim < duratLock) {
      show_interval(curDurat);
    } else if (tim < pressLock) { /* display either pressure or interval */
      show_pressure(todPSI(curPres));
    } else {
      show_interval(curDurat);
    }
  }
  Serial.println(get_interval());
}

void fire_cannon() { /* fires the cannon, right? */
  int interval = get_interval();
  digitalWrite(SOLENOID_PIN, HIGH);
  if (interval < 100) {
    delayMicroseconds(interval * 100);
  } else {
    delay(interval/10);
  }
  digitalWrite(SOLENOID_PIN, LOW);
}

int get_interval() { /* returns solenoid interval in tenths of milliseconds */
  int interval = (1023-analogRead(DURAT_PIN))<<1;
  return(interval); /* TODO add offset and possibly scale */
}

void show_pressure(int pres) { /* shows pressure given in tenths of PSI */
  show_digit(DIGIT1, ON, ON, OFF, OFF, ON, ON, ON, OFF);
  show_tenths(DIGIT2, DIGIT3, DIGIT4, pres);
}

void show_interval(int interval) { /* shows interval given in tenths of ms */
  show_digit(DIGIT1, OFF, OFF, OFF, ON, ON, ON, ON, OFF);
  show_tenths(DIGIT2, DIGIT3, DIGIT4, interval);
}

unsigned int todPSI (unsigned int atodval) {
  /* converts pressor sensor atod to pressure in tenths of PSI */
  /* Hardcoded based on published spec 0.5V = 0psi, 4.5V = 100 psi */
  if (atodval <= 102) return(0);
  return (((atodval * 39) >> 5) - 122); /* force order of execution */
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

void show_numeral(int digit, int numeral, int dec) { 
  switch (numeral) { /* not used, but works for hexadecimal */
    case 0:  show_digit(digit, ON,  ON,  ON,  ON,  ON,  ON,  OFF, dec); break;
    case 1:  show_digit(digit, OFF, ON,  ON,  OFF, OFF, OFF, OFF, dec); break;
    case 2:  show_digit(digit, ON,  ON,  OFF, ON,  ON,  OFF, ON,  dec); break;
    case 3:  show_digit(digit, ON,  ON,  ON,  ON,  OFF, OFF, ON,  dec); break;
    case 4:  show_digit(digit, OFF, ON,  ON,  OFF, OFF, ON,  ON,  dec); break;
    case 5:  show_digit(digit, ON,  OFF, ON,  ON,  OFF, ON,  ON,  dec); break;
    case 6:  show_digit(digit, ON,  OFF, ON,  ON,  ON,  ON,  ON,  dec); break;
    case 7:  show_digit(digit, ON,  ON,  ON,  OFF, OFF, OFF, OFF, dec); break;
    case 8:  show_digit(digit, ON,  ON,  ON,  ON,  ON,  ON,  ON,  dec); break;
    case 9:  show_digit(digit, ON,  ON,  ON,  ON,  OFF, ON,  ON,  dec); break;
    case 10: show_digit(digit, ON,  ON,  ON,  OFF, ON,  ON,  ON,  dec); break;
    case 11: show_digit(digit, OFF, OFF, ON,  ON,  ON,  ON,  ON,  dec); break;
    case 12: show_digit(digit, ON,  OFF, OFF, ON,  ON,  ON,  OFF, dec); break;
    case 13: show_digit(digit, OFF, ON,  ON,  ON,  ON,  OFF, ON,  dec); break;
    case 14: show_digit(digit, ON,  OFF, OFF, ON,  ON,  ON,  ON,  dec); break;
    case 15: show_digit(digit, ON,  OFF, OFF, OFF, ON,  ON,  ON,  dec); break;
    default: /* display three horizontal bars and error message */
             show_digit(digit, ON,  OFF, OFF, ON, OFF,  OFF, ON,  dec);
             Serial.print("Bad numeral value in show_numeral: ");
             Serial.println(numeral);
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

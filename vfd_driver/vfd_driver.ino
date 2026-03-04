static byte vfd_buffer[20][2][35];
static bool fil_step = false;
static uint16_t load_pos = 0;

// shift register pins in PORTD
#define PIN_SI  (1 << PD2) // D2
#define PIN_LAT (1 << PD3) // D3
#define PIN_BLK (1 << PD4) // D4
#define PIN_CLK (1 << PD5) // D5
#define PIN_SO  (1 << PD6) // D6

// filament H-bridge pins in PORTB
#define PIN_FIL_A (1 << PB0) // D8
#define PIN_FIL_B (1 << PB1) // D9

static void vfd_init_pins(void) {
  PORTD &= ~(PIN_SI | PIN_LAT | PIN_CLK | PIN_SO);
  PORTD |= PIN_BLK;
  DDRD &= PIN_SO;
  DDRD |= PIN_SI | PIN_LAT | PIN_BLK | PIN_CLK;

  PORTB |= PIN_FIL_A | PIN_FIL_B;
  DDRB |= PIN_FIL_A | PIN_FIL_B;
}

static inline void __attribute__((always_inline)) vfd_clock_cycle(void) {
  __asm__("nop"); __asm__("nop");
  PORTD |= PIN_CLK;
  __asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop"); __asm__("nop");
  PORTD &= ~PIN_CLK;
}

static inline void __attribute__((always_inline)) vfd_data_cycle(bool data) {
  if(data) PORTD |= PIN_SI;
  else PORTD &= ~PIN_SI;
  vfd_clock_cycle();
}

static void vfd_cycle(byte column) {
  // latch and show previous cycle
  PORTD |= PIN_BLK;
  delayMicroseconds(3);
  PORTD |= PIN_LAT;
  delayMicroseconds(3);
  PORTD &= ~PIN_LAT;
  delayMicroseconds(3);
  PORTD &= ~PIN_BLK;
  delayMicroseconds(3);
  
  // select gate (column of two characters)
  for(byte i = 0; i < 20; i++) {
    vfd_data_cycle(i == column);
  }

  // draw both characters in column, both 5x7 anodes
  for(byte character = 0; character < 2; character++) {
    byte* char_start = &vfd_buffer[column][character][0];

    // 7x5
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));
    vfd_data_cycle(*(char_start++));

    // last 3 bits per character: underscore, dot and comma
    PORTD &= ~PIN_SI;
    vfd_clock_cycle();
    vfd_clock_cycle();
    vfd_clock_cycle();
  }
}

static void vfd_full_refresh(void) {
  // all 20 columns
  for(byte column = 0; column < 20; column++) {
    vfd_cycle(column);
  }

  // filament AC voltage generation via H-bridge
  if(fil_step) {
    fil_step = false;
    PORTB |= PIN_FIL_A;
    delayMicroseconds(10);
    PORTB &= ~PIN_FIL_B;
  } else {
    fil_step = true;
    PORTB |= PIN_FIL_B;
    delayMicroseconds(10);
    PORTB &= ~PIN_FIL_A;
  }
}

void setup() {
  vfd_init_pins();
  Serial.begin(115200);
}

void loop() {
  vfd_full_refresh();

  // fill buffer with incoming bytes
  byte* buf = (byte*)vfd_buffer;
  while(Serial.available()) {
    byte incoming = Serial.read();
    for(byte i = 0; i < 8; i++) {
      buf[load_pos++] = incoming & 1;
      if(load_pos == sizeof(vfd_buffer)) load_pos = 0;
      incoming >>= 1;
    }
  }
}

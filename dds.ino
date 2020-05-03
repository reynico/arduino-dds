// https://github.com/arduino-libraries/LiquidCrystal
#include "lib/LiquidCrystal.h"
#include "lib/LiquidCrystal.cpp"

// https://github.com/thijse/Arduino-EEPROMEx
#include "lib/EEPROMex.h"
#include "lib/EEPROMex.cpp"

// https://github.com/buxtronix/arduino/tree/master/libraries/Rotary
#include "lib/Rotary.h"
#include "lib/Rotary.cpp"

// https://github.com/marcobrianza/ClickButton
#include "lib/ClickButton.h"
#include "lib/ClickButton.cpp"

/*
   Si DEBUG está definido, entonces se muestran los mensajes
   por el monitor serie.
*/
//#define DEBUG

// Pantalla LCD
LiquidCrystal lcd(12, 13, 7, 6, 5, 4);

// Libreria ClickButton
int step_button_status = 0;
ClickButton step_button(A3, LOW, CLICKBTN_PULLUP);

#include "common.h"
#include "ad9850.h"
#include "smeter.h"
#include "rotary.h"

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  Sprintln("Modo de depuracion activado!");
#endif
  lcd.begin(16, 2);

  /*
     Creamos los caracteres especiales
     para mostrar en el S-meter.
  */
  byte fillbar[8] = {
    B00000,
    B01000,
    B01100,
    B01010,
    B01001,
    B01010,
    B01100,
    B01000
  };
  byte mark[8] = {
    B00000,
    B01010,
    B10001,
    B10101,
    B10001,
    B01010,
    B00000,
    B00000
  };
  lcd.createChar(0, fillbar);
  lcd.createChar(1, mark);

  /*
     Inicializamos un timer para el encoder rotativo
     de esta manera el encoder no bloquea el flujo
     normal de loop();
  */
  PCICR |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT18) | (1 << PCINT19);
  sei();

  /*
     Configuramos 4 salidas para controlar el AD9850.
  */
  pinMode(FQ_UD, OUTPUT);
  pinMode(W_CLK, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);
  pulse_high(RESET);
  pulse_high(W_CLK);
  pulse_high(FQ_UD);

  /*
     Configuramos tres entradas con pull-up para
     el cambio de banda. Como estamos utilizando
     una única llave para las 3 bandas, utilizamos
     3 pines diferentes de manera digital.
  */
  pinMode(A0, INPUT_PULLUP); // banda 20m
  pinMode(A2, INPUT_PULLUP); // banda 40m
  pinMode(A1, INPUT_PULLUP); // banda 60m

  /*
     El código contempla el uso de la memoria EEPROM del
     Arduino. De esta manera podemos almacenar la última
     frecuencia utilizada y recuperarla al encender el
     equipo.
  */
  if (EEPROM.read(0) == true) {
    Sprintln("Leyendo memoria EEPROM...");
    initial_band = EEPROM.read(1);
    initial_rx = EEPROM.readLong(2);
  } else {
    Sprintln("La memoria EEPROM está vacía. Inicializando con valores por defecto...");
  }
  for (int j = 0; j < 8; j++)
    lcd.createChar(j, block[j]);
  lcd.setCursor(1, 0);
  lcd.print("LU3DJ DDS VFO");
  delay (2000);
  mode = vfo_mode(initial_band);
  lcd.setCursor(0, 0);
  lcd.print(mode);
}

void loop() {
  /* El cambio de banda se realiza con una llave
      giratoria que pone a masa una de las tres
      entradas por vez.
  */
  if (digitalRead(A0) == LOW && band != 20) {
    band = 20;
    mode = vfo_mode(band);
    if (band == initial_band) {
      rx = initial_rx;
    } else {
      rx = 14150000;
    }
  }
  if (digitalRead(A2) == LOW && band != 40) {
    band = 40;
    mode = vfo_mode(band);
    if (band == initial_band) {
      rx = initial_rx;
    } else {
      rx = 7100000;
    }
  }
  if (digitalRead(A1) == LOW && band != 60) {
    band = 60;
    mode = vfo_mode(band);
    if (band == initial_band) {
      rx = initial_rx;
    } else {
      rx = 5360000;
    }
  }

  /*
     Si la frecuencia mostrada es diferente a la frecuencia que
     obtengo al tocar el encoder, la actualizo tanto en la
     pantalla como en el DDS AD9850.
  */
  if (rx != rx2) {
    show_freq();
    send_frequency(rx);
    rx2 = rx;
  }

  /*
     step_button es el botón del encoder rotativo. Con un click
     corto cicla entre los diferentes saltos de incremento. Con
     dis clicks cortos cicla entre los diferentes saltos de
     decremento. Con un click largo cicla los modos USB, LSB y 
     generador*. Generador es un modo en el que la IF no se suma 
     ni se resta y la frecuencia de salida es la mostrada en el display.
  */
  step_button.Update();
  if (step_button.clicks != 0) step_button_status = step_button.clicks;
  if (step_button.clicks == 1) {
    set_increment();
  }
  if (step_button.clicks == 2) {
    set_decrement();
  }
  if (step_button.clicks == -1) {
    if (mode == "U") { 
      /*
       * El modo generador está comentado para que no esté disponible
       * al usar el equipo. Pueden descomentar el bloque inferior
       * si lo desean
       */
      //      mode = "G";
      //      Sprintln("Modo configurado: Generador");
      //      if_freq = 0;
      //      add_if = false;
      //    }
      //    else if (mode == "G") {

      mode = "L";
      Sprintln("Modo configurado: LSB");
      if_freq = 2000000;
      add_if = false;
    }
    else if (mode == "L") {
      mode = "U";
      Sprintln("Modo configurado: USB");
      if_freq = 2000000;
      add_if = true;
    }
    lcd.setCursor(0, 0);
    lcd.print(mode);
  }

  /*
     Luego de sintonizar una frecuencia, si pasaron mas de dos
     segundos la misma se almacena en la memoria EEPROM.
  */
  if (!mem_saved) {
    if (time_passed + 2000 < millis()) {
      store_memory();
    }
  }

  /*
     send_Frequency envía la señal al AD9850.
  */
  send_frequency(rx);

  /*
     El S-meter funciona tanto para transmisión (modulación)
     como para recepción (RF). La forma de cambio es sensando
     el nivel de señal en A4, que es la entrada de señal de RF.
     Si la señal de RF es menor a 90 entonces estamos en
     transmisión y esa es la señal mostrada por el S-meter.
     Caso contrario se mostrará la señal de recepción.
  */
  if (millis() < lastT)
    return;
  lastT += T_REFRESH;
  if (analogRead(A4) < 90) {
    smeter_signal = map(sqrt(analogRead(A5) * 16), 0, 128, 0, 80);
    smeter(1, smeter_signal);
  } else {
    smeter_signal = map(sqrt(analogRead(A4) * 16), 40, 128, 0, 100);
    smeter(1, smeter_signal);
  }

}

/*
   El incremento de frecuencia se hace de manera
   cíclica por los valores definidos abajo. Cada
   vez que se llama a la función set_increment()
   ésta prepara automáticamente el próximo valor.
*/
void set_increment() {
  switch (increment) {
    case 10:
      increment = 100;
      hertz = "100 Hz";
      hertz_position = 10;
      break;
    case 100:
      increment = 1000;
      hertz = "1 Khz";
      hertz_position = 11;
      break;
    case 1000:
      increment = 10000;
      hertz = "10 Khz";
      hertz_position = 10;
      break;
    case 10000:
      increment = 100000;
      hertz = "100 Khz";
      hertz_position = 9;
      break;
    case 100000:
      increment = 1000000;
      hertz = "1 Mhz";
      hertz_position = 11;
      break;
    default:
      increment = 10;
      hertz = "10 Hz";
      hertz_position = 11;
      break;
  }
  Sprint("Salto: ");
  Sprintln(hertz);
  lcd.setCursor(1, 0);
  lcd.print("              ");
  lcd.setCursor(6, 0);
  lcd.print(hertz);
  delay(1000);
  show_freq();
}

void set_decrement() {
  switch (increment) {
    case 1000000:
      increment = 100000;
      hertz = "100 Khz";
      hertz_position = 10;
      break;
    case 100000:
      increment = 10000;
      hertz = "10 Khz";
      hertz_position = 11;
      break;
    case 10000:
      increment = 1000;
      hertz = "1 Khz";
      hertz_position = 10;
      break;
    case 1000:
      increment = 100;
      hertz = "100 Hz";
      hertz_position = 9;
      break;
    case 100:
      increment = 10;
      hertz = "10 Hz";
      hertz_position = 11;
      break;
    case 10:
      increment = 1000000;
      hertz = "1 Mhz";
      hertz_position = 11;
      break;
  }
  Sprint("Salto: ");
  Sprintln(hertz);
  lcd.setCursor(1, 0);
  lcd.print("              ");
  lcd.setCursor(6, 0);
  lcd.print(hertz);
  delay(1000);
  show_freq();
}

/*
   Ésta función se encarga de mostrar la frecuencia
   en pantalla.
*/
void show_freq() {
  lcd.setCursor(1, 0);
  lcd.print("              ");
  if (rx < 10000000) {
    lcd.setCursor(3, 0);
  } else {
    lcd.setCursor(2, 0);
  }
  lcd.print(rx / 1000);
  lcd.print(".");
  lcd.print((rx / 100) % 10);
  lcd.print((rx / 10) % 10);
  lcd.print(" KHz ");
  Sprint("Frecuencia: ");
  Sprintln(rx);
  time_passed = millis();
  mem_saved = false; // Triggerea el guardado en memoria
}

/*
   store_memory() guarda la frecuencia elegida en la memoria
   EEPROM cada vez que se llama.
*/
void store_memory() {
  EEPROM.write(0, true);
  EEPROM.write(1, band);
  EEPROM.writeLong(2, rx);
  mem_saved = true;
  Sprintln("Frecuencia almacenada en memoria EEPROM.");
}

/*
   vfo_mode devuelve el modo en el que se configuró
   el DDS, dependiendo de la banda elegida.
*/
String vfo_mode (int initial_band) {
  if_freq = 2000000;
  lcd.setCursor(0, 0);
  switch (initial_band) {
    case 20:
      add_if = true;
      lcd.print("U");
      return "U";
      break;
    case 40:
      add_if = false;
      lcd.print("L");
      return "L";
      break;
    case 60:
      add_if = true;
      lcd.print("U");
      return "U";
      break;
    case 80:
      add_if = false;
      lcd.print("L");
      return "L";
      break;
  }
}

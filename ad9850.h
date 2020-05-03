/*
 * Configuración de pines del AD9850
 */
#define W_CLK 8   // CLK
#define FQ_UD 9   // FQ
#define DATA 10   // DATA
#define RESET 11  // RESET
#define pulse_high(pin) {digitalWrite(pin, HIGH); digitalWrite(pin, LOW);}

/*
 * Envía la frecuencia al AD9850
 */
void tfr_byte(byte data)
{
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(DATA, data & 0x01);
    pulse_high(W_CLK);
  }
}

/*
 * Aquí se realiza el envío de frecuencia al AD9850.
 * El número mágico 4294799000 se puede variar para
 * ajustar la frecuencia de salida. La alimentación
 * del Arduino hace variar la frecuencia.
 */
void send_frequency(double frequency) {
  if (add_if) {
    frequency = frequency + if_freq;
  } else {
    frequency = frequency - if_freq;
  }
  int32_t freq = frequency * 4294799000 / 125000000; // Ajustar la frecuencia desde aquí
  for (int b = 0; b < 4; b++, freq >>= 8) {
    tfr_byte(freq & 0xFF);
  }
  tfr_byte(0x000);
  pulse_high(FQ_UD);
}

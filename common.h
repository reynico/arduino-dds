// Configuración para mostrar la información por monitor serie
#ifdef DEBUG
  #define Sprintln(a) (Serial.println(a))
  #define Sprint(a) (Serial.print(a))
#else
  #define Sprintln(a)
  #define Sprint(a)
#endif

// Valores iniciales para variables generales
int32_t initial_rx;
int32_t rx = 7100000;   // Frecuencia inicial del VFO
int32_t rx2 = 1;        // Variable secundaria para la frecuencia actual del VFO
int32_t increment = 10; // Salto inicial para el VFO, 10Hz
int initial_band = 40;  // Banda inicial del VFO si no se encontró almacenada en memoria
String mode;            // Modo inicial del VFO
int band;               // Banda configurada por la llave selectora

// Frecuencia intermedia
int32_t if_freq = 0000000;  // Frecuencia intermedia inicial para el VFO
bool add_if = true;         // Si esta variable es true, se suma la IF al VFO. Si esta variable es false, se resta

// VFO
String hertz = " 10 Hz";   // Valor de salto inicial para el VFO
int  hertz_position = 10;  // Posición del texto de salto en la pantalla

// Nemoria EEPROM
bool mem_saved = true;          // Valor para notificar si la frecuencia se guardó (true) o no (false) en memoria
int32_t time_passed = millis(); // Contador de tiempo para comparar cuanto tiempo pasó desde que se configuró una frecuencia

// VU Meter
int smeter_signal = 0;                // Nivel de señal inicial del S-Meter
#define T_REFRESH    100              // Tiempo de refresco en milisegundos para el S-Meter
#define T_PEAKHOLD   (3 * T_REFRESH)  // Timepo de refresco en milisegundos para el indicador de pico del S-Meter

Se muestran una serie de reformas y ajustes necesarios para instalar el DDS en
un equipo tipo Cahuane FR-300. Al final del documento encontrarán la forma de
operar el DDS.

- [Reformas](#reformas)
  - [Diagrama esquemático completo](#diagrama-esquem%c3%a1tico-completo)
  - [Pinout del Arduino Nano](#pinout-del-arduino-nano)
  - [Fuente de alimentación](#fuente-de-alimentaci%c3%b3n)
  - [Selector de canales](#selector-de-canales)
  - [Ajuste de frecuencia por encoder rotativo](#ajuste-de-frecuencia-por-encoder-rotativo)
  - [Pantalla LCD](#pantalla-lcd)
  - [DDS AD9850](#dds-ad9850)
  - [S-Meter](#s-meter)
  - [Radiofrecuencia](#radiofrecuencia)
- [Instalación del código en el Arduino Nano](#instalaci%c3%b3n-del-c%c3%b3digo-en-el-arduino-nano)
  - [Subir el código](#subir-el-c%c3%b3digo)
  - [Configuraciones posibles](#configuraciones-posibles)
- [Operación](#operaci%c3%b3n)
  - [VFO](#vfo)
  - [Saltos de incremento](#saltos-de-incremento)
  - [Bandas](#bandas)
  - [Modo de operación (LSB y USB)](#modo-de-operaci%c3%b3n-lsb-y-usb)
  - [Memoria VFO](#memoria-vfo)

# Reformas

## Diagrama esquemático completo
El diagrama eléctrico completo se muestra a continuación. Mas adelante se
detalla cada sección por separado para mejor interpretación

[![Diagrama eléctrico](assets/dds_esquema_electrico.png)](assets/dds_esquema_electrico.png)

## Pinout del Arduino Nano
Es importante familiarizarse con la notación de los pines del Arduino Nano. De
esta manera será mas sencillo seguir la guía de ensamble

![Arduino Nano](assets/arduino_nano_pinout.png "Arduino Nano")

## Fuente de alimentación
Se utiliza un regulador de tensión de la linea 78xx, en este caso de 5 volts
(7805) con un capacitor electrolítico de 10v y 100uF a la salida para filtrar
cualquier tipo de ruido que pueda traer problemas a la electrónica sensible.

## Selector de canales
El DDS utiliza la llave selectora de canales del equipo para cambiar entre las
diferentes bandas. La llave rotativa tiene una isla completa vacía que puede
utilizarse para este fin

Existe una isla vacía en la llave selectora original, que es la que utilizaremos
para el Arduino de la siguiente manera:

![Llave selectora](assets/llave_selectora_canales.png "Llave selectora")

Cada pin de la llave selectora es conectada al Arduino Nano. Si no sabe que
pines conectar, por favor refiérase al [pinout del Arduino
Nano](#pinout-del-arduino-nano) descripto anteriormente

## Ajuste de frecuencia por encoder rotativo
El VFO funciona a través de un encoder rotativo. Es un dispositivo que maneja
una señal digital del tipo tren de pulsos. El Arduino interpreta esa información
y el código permite ajustar la frecuencia. También posee un botón que tiene
diferentes usos dependiendo de la forma en que se toque. Para mas información al
respecto, leer la sección [operación](#operaci%c3%b3n).

![Encoder rotativo](assets/rotary_pinout.png "Encoder rotativo")

## Pantalla LCD
Se utiliza una pantalla LCD de 16x2 caracteres basada en el controlador HD44780.
El contraste se regula a través de un potenciometro que varía la tensión en el
pin `VEE`. Se puede utilizar un potenciometro de 10kohms para ajustar el
contraste hasta el nivel deseado, luego medir la resistencia del potenciometro
en esa posición y colocar una resistencia fija. La iluminación del display se
hace a través de los pines `LED+` y `LED-`. Se puede colocar una resistencia de
pequeño valor si el nivel de luz es demasiado fuerte. Exceptuando la
alimentación por los pines `VSS` y `VCC`, el resto de los pines se conectan
directamente al Arduino.

![Pinout LCD](assets/lcd_pinout.png "Pinout LCD")

## DDS AD9850
La conexión al DDS es bastante sencilla. Se necesita alimentación de +5v y masa,
y cuatro pines de control, `W_CLK`, `FQ_UD`, `DATA` y `RESET`. La salida de
antena debe ir conectada conforme se indica en la sección [Radiofrecuencia](#radiofrecuencia).

![Pinout DDS](assets/dds_pinout.png "Pinout DDS")

## S-Meter
El S-Meter funciona tanto en transmisión como en recepción. Para el modo de
transmisión se utiliza la tensión existente en el led rojo de TX del equipo

![Cahuane led TX](assets/cahuane_led_tx.png "Cahuane led TX")

La señal es acondicionada a través de un diodo `D1` y un divisor resistivo
conformado por `R2` y `R3` y un capacitor a masa. La tensión resultante es
ingresada a través del pin `Analog 5` (A5) del Arduino Nano.

![S-Meter TX](assets/smeter_pinout.png "S-Meter TX")

Para la recepción se utiliza la señal del AGC del equipo que es posteriormente
filtrada con una resistencia `R4` y un capacitor a masa `C4`.

![Cahuane AGC](assets/agc_pin.jpg "Cahuane AGC")


## Radiofrecuencia
Se utiliza la salida `SINB` del DDS a través de un capacitor de 100nF para
alimentar la placa PL154 a través de la llave selectora de bandas original.

# Instalación del código en el Arduino Nano
Una vez clonado este repositorio, abrir el archivo `dds.ino` en Arduino. Todas
las librerías necesarias están incluidas en el repositorio, de esta manera no
hay que tomarse el trabajo extra de instalar versiones especificas de cada
librería. El código se encuentra comentado para que sea mas sencilla su revisión
y modificación

![Código Arduino](assets/codigo_arduino.png "Código Arduino")

## Subir el código
Con el Arduino Nano conectado a un puerto USB, presionar el botón `Upload` en el
software Arduino

## Configuraciones posibles
* **Modo debug:** El modo debug se utiliza en caso de que algo del funcionamiento
  del DDS no esté ocurriendo como es deseado. Para poder encontrar el problema
  es de utilidad activar los mensajes de depuración. Para esto, descomentar la
  [línea 21](https://github.com/reynico/arduino-dds/blob/master/dds.ino#L21)
* **Pinout:** Se puede variar el pinout de cada parte. Para ello es necesario
  modificar la configuración de los pines en el archivo correspondiente al
  módulo que se desea modificar. En caso de querer cambiar los pines del DDS,
  entonces se debe editar en el archivo [ad9850.h](https://github.com/reynico/arduino-dds/blob/master/ad9850.h)

# Operación

La operación se realiza con dos controles: el selector de banda original (se
utiliza una isla vacía) y un encoder rotativo con botón. El encoder cumple la
función de VFO, sintonizando la frecuencia, y el botón incluído en él tiene
diferentes funciones dependiendo de los toques. Un toque corto cambia el step o
incremento entre 10 Hz, 100 Hz, 1 KHz, 10 KHz, 100 KHz y 1 MHz de manera
ascendente. Dos toques cortos hacen lo mismo de manera descendente. Un toque
largo (+2 segundos) intercambia entre los modos LSB y USB.

![Operacion](assets/frente_cahuane.png "Operacion")


## VFO
El VFO está programado para funcionar entre 1 a 30 MHz. Girando el encoder
rotativo se ajusta la frecuencia deseada y es mostrada en tiempo real en la
primer línea de la pantalla.

## Saltos de incremento
El botón del encoder cumple la función de posicionar diferentes niveles de
incremento. Con un toque corto el salto aumenta, con dos toques cortos disminuye
como en la siguiente figura:

![Saltos de incremento](assets/incremento_decremento.png "Saltos de incremento")

El salto de frecuencia es indicado por un segundo en la pantalla, reemplazando
a la frecuencia del VFO por ese período.

## Bandas
Las bandas se activan con la perilla selectora de bandas original. El código
está programado para tener tres bandas: 20, 40 y 60 metros. Esto es modificable

## Modo de operación (LSB y USB)
Si bien cada banda tiene preconfigurado su modo de operación, el modo se puede
invalidar con un pulso largo en el botón de [VFO](#vfo). Ésto cicla los modos
entre USB y LSB y es indicado en la esquina superior izquierda de la pantalla.

## Memoria VFO
El código contempla el uso de la memoria EEPROM del Arduino Nano para guardar la
última frecuencia elegida en el VFO. El guardado en memoria se realiza luego de
dos segundos de inactividad en el encoder rotativo.

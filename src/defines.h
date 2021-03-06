#define HARDWARE_VERSION fl_200a

#if HARDWARE_VERSION==fl_100a
  #define SPI_SCK 13
  #define SPI_MOSI 12
  #define SPI_MISO 14
  #define MEM_CS 4
  #define LED_STRIP 5
  #define BUTTON_LED 16
  #define BUTTON 15
#endif

#if HARDWARE_VERSION==fl_200
  #define SPI_SCK 13
  #define SPI_MOSI 12
  #define SPI_MISO 14
  #define MEM_CS 4
  #define LED_STRIP 5
  #define BUTTON_LED 15
  #define BUTTON 16
#endif

#if HARDWARE_VERSION==fl_200a
  #define SPI_SCK 12
  #define SPI_MOSI 13
  #define SPI_MISO 14
  #define MEM_CS 4
  #define LED_STRIP 5
  #define BUTTON_LED 15
  #define BUTTON 16
#endif

//These CRADLE defines are pins when we're in our programming jig/cradle
#define CRADLE_DETECT 12
#define CRADLE_DONE_LED 14

#define BUTTON_LED_ON 1
#define BUTTON_LED_OFF 0

#define BUTTON_UP 1
#define BUTTON_DOWN 0


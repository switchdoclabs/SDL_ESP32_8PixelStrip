#include "esp32_digital_led_lib.h"

// strands for our system

strand_t STRANDS[] = { // Avoid using any of the strapping pins on the ESP32
  { .rmtChannel = 1, .gpioNum = 21, .ledType = LED_SK6812W_V1, .brightLimit = 128, .numPixels =  16,
    .pixels = nullptr, ._stateVars = nullptr
  },
  //{ .rmtChannel = 2, .gpioNum = 18, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels =  93,
  //  .pixels = nullptr, ._stateVars = nullptr
  // },
  // {.rmtChannel = 3, .gpioNum = 19, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels =  64,
  //  .pixels = nullptr, ._stateVars = nullptr},
  //{.rmtChannel = 0, .gpioNum = 16, .ledType = LED_WS2812B_V3, .brightLimit = 32, .numPixels = 256,
  // .pixels = nullptr, ._stateVars = nullptr},
  //  {.rmtChannel = 0, .gpioNum = 16, .ledType = LED_SK6812W_V1, .brightLimit = 32, .numPixels = 300,
  //   .pixels = nullptr, ._stateVars = nullptr},
  //{ .rmtChannel = 0, .gpioNum = 16, .ledType = LED_SK6812W_V1, .brightLimit = 32, .numPixels = 24,
  // .pixels = nullptr, ._stateVars = nullptr
  //},
};

//strand_t * strands [] = { &STRANDS[0], &STRANDS[1], &STRANDS[2], &STRANDS[3] };
strand_t * strands [] = { &STRANDS[0] };
int STRANDCNT = sizeof(STRANDS) / sizeof(STRANDS[0]);

// Forward declarations
void rainbow(strand_t *, unsigned long, unsigned long);
void scanner(strand_t *, unsigned long, unsigned long);
void dumpDebugBuffer(int, char *);


#if DEBUG_ESP32_DIGITAL_LED_LIB
int digitalLeds_debugBufferSz = 1024;
char * digitalLeds_debugBuffer = static_cast<char*>(calloc(digitalLeds_debugBufferSz, sizeof(char)));
#endif

void gpioSetup(int gpioNum, int gpioMode, int gpioVal) {
#if defined(ARDUINO) && ARDUINO >= 100
  pinMode (gpioNum, gpioMode);
  digitalWrite (gpioNum, gpioVal);
#elif defined(ESP_PLATFORM)
  gpio_num_t gpioNumNative = static_cast<gpio_num_t>(gpioNum);
  gpio_mode_t gpioModeNative = static_cast<gpio_mode_t>(gpioMode);
  gpio_pad_select_gpio(gpioNumNative);
  gpio_set_direction(gpioNumNative, gpioModeNative);
  gpio_set_level(gpioNumNative, gpioVal);
#endif
}

// starndard colors

//pixelColor_t pixelFromRGBW(uint8_t r, uint8_t g, uint8_t b, uint8_t w)

const pixelColor_t BrightRed = pixelFromRGBW(255, 0, 0, 0);
const pixelColor_t BrightGreen = pixelFromRGBW(0, 255, 0, 0);
const pixelColor_t BrightBlue = pixelFromRGBW(0, 0, 255, 0);

const pixelColor_t Red = pixelFromRGBW(255, 0, 0, 0);
const pixelColor_t Green = pixelFromRGBW(0, 255, 0, 0);
const pixelColor_t Blue = pixelFromRGBW(0, 0, 255, 0);


const pixelColor_t BrightYellow = pixelFromRGBW(255, 255, 0, 0);
const pixelColor_t BrightCyan = pixelFromRGBW(0, 255, 255, 0);
const pixelColor_t BrightPurple = pixelFromRGBW(255, 0, 255, 0);

const pixelColor_t DarkRed = pixelFromRGBW(32, 0, 0, 0);
const pixelColor_t DarkGreen = pixelFromRGBW(0, 32, 0, 0);
const pixelColor_t DarkBlue = pixelFromRGBW(0, 0, 32, 0);

const pixelColor_t DarkYellow = pixelFromRGBW(32, 32, 0, 0);
const pixelColor_t DarkCyan = pixelFromRGBW(0, 32, 32, 0);
const pixelColor_t DarkPurple = pixelFromRGBW(32, 0, 32, 0);

const pixelColor_t White = pixelFromRGBW(0, 0, 0, 255);
const pixelColor_t Black = pixelFromRGBW(0, 0, 0, 0);


#define min(X, Y) (((X)<(Y))?(X):(Y))

//**************************************************************************//
int getMaxMalloc(int min_mem, int max_mem) {
  int prev_size = min_mem;
  int curr_size = min_mem;
  int max_free = 0;
  //  Serial.print("checkmem: testing alloc from ");
  //  Serial.print(min_mem);
  //  Serial.print(" : ");
  //  Serial.print(max_mem);
  //  Serial.println(" bytes");
  while (1) {
    void * foo1 = malloc(curr_size);
    //    Serial.print("checkmem: attempt alloc of ");
    //    Serial.print(curr_size);
    //    Serial.print(" bytes --> pointer 0x");
    //    Serial.println((uintptr_t)foo1, HEX);
    if (foo1 == nullptr) {  // Back off
      max_mem = min(curr_size, max_mem);
      //      Serial.print("checkmem: backoff 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.print(", max_mem = ");
      //      Serial.print(max_mem);
      //      Serial.println();
      curr_size = (int)(curr_size - (curr_size - prev_size) / 2.0);
      //      Serial.print("checkmem: backoff 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.println();
    }
    else {  // Advance
      free(foo1);
      max_free = curr_size;
      prev_size = curr_size;
      curr_size =  min(curr_size * 2, max_mem);
      //      Serial.print("checkmem: advance 2 prev = ");
      //      Serial.print(prev_size);
      //      Serial.print(", curr = ");
      //      Serial.print(curr_size);
      //      Serial.println();
    }
    if (abs(curr_size - prev_size) == 0) {
      break;
    }
  }
  Serial.print("checkmem: max free heap = ");
  Serial.print(esp_get_free_heap_size());
  Serial.print(" bytes, max allocable = ");
  Serial.print(max_free);
  Serial.println(" bytes");
  return max_free;
}

void dumpSysInfo() {
  esp_chip_info_t sysinfo;
  esp_chip_info(&sysinfo);
  Serial.print("Model: ");
  Serial.print((int)sysinfo.model);
  Serial.print("; Features: 0x");
  Serial.print((int)sysinfo.features, HEX);
  Serial.print("; Cores: ");
  Serial.print((int)sysinfo.cores);
  Serial.print("; Revision: r");
  Serial.println((int)sysinfo.revision);
}

void dumpDebugBuffer(int id, char * debugBuffer)
{
  Serial.print("DEBUG: (");
  Serial.print(id);
  Serial.print(") ");
  Serial.println(debugBuffer);
  debugBuffer[0] = 0;
}


//**************************************************************************//
class Scannerer {
  private:
    strand_t * pStrand;
    pixelColor_t minColor;
    pixelColor_t maxColor;
    int prevIdx;
    int currIdx;
  public:
    Scannerer(strand_t *, pixelColor_t);
    void drawNext();
};

Scannerer::Scannerer(strand_t * pStrandIn, pixelColor_t maxColorIn)
{
  pStrand = pStrandIn;
  minColor = pixelFromRGBW(0, 0, 0, 0);
  maxColor = maxColorIn;
  prevIdx = 0;
  currIdx = 0;
}

void Scannerer::drawNext()
{
  pStrand->pixels[prevIdx] = minColor;
  pStrand->pixels[currIdx] = maxColor;
  digitalLeds_updatePixels(pStrand);
  prevIdx = currIdx;
  currIdx++;
  if (currIdx >= pStrand->numPixels) {
    currIdx = 0;
  }
}

void scanners(strand_t * strands[], int numStrands, unsigned long delay_ms, unsigned long timeout_ms)
{
  //Scannerer scan(pStrand); Scannerer * pScanner = &scan;
  Scannerer * pScanner[numStrands];
  int i;
  uint8_t c = strands[0]->brightLimit; // TODO: improve
  pixelColor_t scanColors [] = {
    pixelFromRGBW(c, 0, 0, 0),
    pixelFromRGBW(0, c, 0, 0),
    pixelFromRGBW(c, c, 0, 0),
    pixelFromRGBW(0, 0, c, 0),
    pixelFromRGBW(c, 0, c, 0),
    pixelFromRGBW(0, c, c, 0),
    pixelFromRGBW(c, c, c, 0),
    pixelFromRGBW(0, 0, 0, c),
  };


  int colors = random(7);
  Serial.print("DEMO: scanners(");
  for (i = 0; i < numStrands; i++) {
    pScanner[i] = new Scannerer(strands[i], scanColors[colors]);
    if (i > 0) {
      Serial.print(", ");
    }
    Serial.print("ch");
    Serial.print(strands[i]->rmtChannel);
    Serial.print(" (0x");
    Serial.print((uint32_t)pScanner[i], HEX);
    Serial.print(")");
    Serial.print(" #");
    Serial.print((uint32_t)scanColors[i].num, HEX);
  }
  Serial.print(")");
  Serial.println();
  unsigned long start_ms = millis();
  while (timeout_ms == 0 || (millis() - start_ms < timeout_ms)) {
    for (i = 0; i < numStrands; i++) {
      pScanner[i]->drawNext();
    }
    delay(delay_ms);
  }
  for (i = 0; i < numStrands; i++) {
    delete pScanner[i];
    digitalLeds_resetPixels(strands[i]);
  }
}

void scanner(strand_t * pStrand, unsigned long delay_ms, unsigned long timeout_ms)
{
  strand_t * strands [] = { pStrand };
  scanners(strands, 1, delay_ms, timeout_ms);
}

//**************************************************************************//
class Rainbower {
  private:
    strand_t * pStrand;
    const uint8_t color_div = 4;
    const uint8_t anim_step = 1;
    uint8_t anim_max;
    uint8_t stepVal1;
    uint8_t stepVal2;
    pixelColor_t color1;
    pixelColor_t color2;
  public:
    Rainbower(strand_t *);
    void drawNext();
};

Rainbower::Rainbower(strand_t * pStrandIn)
{
  pStrand = pStrandIn;
  anim_max = pStrand->brightLimit - anim_step;
  stepVal1 = 0;
  stepVal2 = 0;
  color1 = pixelFromRGBW(anim_max, 0, 0, 0);
  color2 = pixelFromRGBW(anim_max, 0, 0, 0);
}

void Rainbower::drawNext()
{
  color1 = color2;
  stepVal1 = stepVal2;
  for (uint16_t i = 0; i < pStrand->numPixels; i++) {
    pStrand->pixels[i] = pixelFromRGBW(color1.r / color_div, color1.g / color_div, color1.b / color_div, 0);
    if (i == 1) {
      color2 = color1;
      stepVal2 = stepVal1;
    }
    switch (stepVal1) {
      case 0:
        color1.g += anim_step;
        if (color1.g >= anim_max)
          stepVal1++;
        break;
      case 1:
        color1.r -= anim_step;
        if (color1.r == 0)
          stepVal1++;
        break;
      case 2:
        color1.b += anim_step;
        if (color1.b >= anim_max)
          stepVal1++;
        break;
      case 3:
        color1.g -= anim_step;
        if (color1.g == 0)
          stepVal1++;
        break;
      case 4:
        color1.r += anim_step;
        if (color1.r >= anim_max)
          stepVal1++;
        break;
      case 5:
        color1.b -= anim_step;
        if (color1.b == 0)
          stepVal1 = 0;
        break;
    }
  }
  digitalLeds_updatePixels(pStrand);
}

void rainbows(strand_t * strands[], int numStrands, unsigned long delay_ms, unsigned long timeout_ms)
{
  //Rainbower rbow(pStrand); Rainbower * pRbow = &rbow;
  Rainbower * pRbow[numStrands];
  int i;
  Serial.print("DEMO: rainbows(");
  for (i = 0; i < numStrands; i++) {
    pRbow[i] = new Rainbower(strands[i]);
    if (i > 0) {
      Serial.print(", ");
    }
    Serial.print("ch");
    Serial.print(strands[i]->rmtChannel);
    Serial.print(" (0x");
    Serial.print((uint32_t)pRbow[i], HEX);
    Serial.print(")");
  }
  Serial.print(")");
  Serial.println();
  unsigned long start_ms = millis();
  while (timeout_ms == 0 || (millis() - start_ms < timeout_ms)) {
    for (i = 0; i < numStrands; i++) {
      pRbow[i]->drawNext();
    }
    delay(delay_ms);
  }
  for (i = 0; i < numStrands; i++) {
    delete pRbow[i];
    digitalLeds_resetPixels(strands[i]);
  }
}

void rainbow(strand_t * pStrand, unsigned long delay_ms, unsigned long timeout_ms)
{
  strand_t * strands [] = { pStrand };
  rainbows(strands, 1, delay_ms, timeout_ms);
}

//**************************************************************************//
//  // print tests
//  Serial.println(0xFFFFFFFF, DEC);
//  Serial.println(0xFFFFFFFF, HEX);
//  Serial.println(0xFFFFFFFF, BIN);
//  Serial.println(0x7FFFFFFF, DEC);
//  Serial.println(0x7FFFFFFF, HEX);
//  Serial.println(0x7FFFFFFF, BIN);
//  Serial.println(0x00000000, DEC);
//  Serial.println(0x00000000, HEX);
//  Serial.println(0x00000000, BIN);
//  Serial.println(        -1, DEC);
//  Serial.println(        -1, HEX);
//  Serial.println(        -1, BIN);

//**************************************************************************//



void BC24clearStrip(strand_t * pStrand)
{

  digitalLeds_resetPixels(pStrand);

}

void BC24setStrip(strand_t * pStrand, pixelColor_t myColor )
{

  for (int i = 0; i < pStrand->numPixels; i ++)
  {
    pStrand->pixels[i] = myColor;
    digitalLeds_updatePixels(pStrand);

  }

}

void BC24CircleRainbow()
{
  strand_t * pStrand = &STRANDS[0];

  int latestQueueEntry;
  while (1)
  {

    rainbow(pStrand, 0, 2000);


    scanner(pStrand, 0, 2000);

    scanner(pStrand, 1, 2000);

 
    scanner(pStrand, 0, 2000);
  
    rainbow(pStrand, 0, 2000);

    scanner(pStrand, 1, 2000); // A tiny delay can smooth things out

    rainbow(pStrand, 0, 2000);

    scanner(pStrand, 5, 2000);

    rainbow(pStrand, 0, 2000);

    rainbow(pStrand, 0, 2000);

    rainbow(pStrand, 0, 2000);
    digitalLeds_resetPixels(pStrand);

  }

}



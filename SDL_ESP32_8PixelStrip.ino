// SDL_ESP32_8PixelStrip
// SwitchDoc Labs Drivers for 8 Pixel Chainable RGBW Strips
// May 2018
//
//
// Note this uses the RMT device on the ESP32 to provide a seemless control stream to the Pixels
// You can go a really long way (you will have to add external power eventually!)
//


#include "PixelFunctions.h"


void setup() {
  // put your setup code here, to run once:



  Serial.begin(115200);

  
  dumpSysInfo();
  getMaxMalloc(1 * 1024, 16 * 1024 * 1024);

  if (digitalLeds_initStrands(STRANDS, STRANDCNT)) {
    Serial.println("Init FAILURE: halting");
    while (true) {};
  }
  for (int i = 0; i < STRANDCNT; i++) {
    strand_t * pStrand = &STRANDS[i];
    Serial.print("Strand ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print((uint32_t)(pStrand->pixels), HEX);
    Serial.println();
#if DEBUG_ESP32_DIGITAL_LED_LIB
    dumpDebugBuffer(-2, digitalLeds_debugBuffer);
#endif
    digitalLeds_resetPixels(pStrand);
#if DEBUG_ESP32_DIGITAL_LED_LIB
    dumpDebugBuffer(-1, digitalLeds_debugBuffer);
#endif
  }
  Serial.println("Init complete");


  Serial.println();
  Serial.println();
  Serial.println("--------------------");
  Serial.println("8 Pixel Strip Software Demo");
  Serial.println("--------------------");
  Serial.println("Version: 1.0");


  Serial.print("Compiled at:");
  Serial.print (__TIME__);
  Serial.print(" ");
  Serial.println(__DATE__);


  // Free heap on ESP32
  Serial.print("Free Heap Space on BC24:");
  Serial.println(ESP.getFreeHeap());




}

void loop() {
  // put your main code here, to run repeatedly:
      BC24CircleRainbow();
}

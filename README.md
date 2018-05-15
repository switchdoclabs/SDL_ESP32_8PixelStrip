SDL_RMT_8PixelStrip <BR>
SwitchDoc Labs <BR>
May 11, 2018<BR>

This library uses the RMT module on the ESP32 to generate the tight signal required to program Pixels (and NeoPixels).

It completely fixes the "Pixel 19" and "Pixel 26" problem caused by the WiFi and other interrupts on the ESP32.

The RMT (Remote Control) module driver can be used to send and receive infrared remote control signals. 
Due to flexibility of RMT module, the driver can also be used to generate many other types of signals.

This library uses the Martin F. Falatic Library and is based on public domain code by Chris Osborn 


##Software Installation <BR>


You will need to set your ESP32 board type in the Arduino IDE and make sure you have installed the ESP32 SDK.



![alt text](http://www.switchdoc.com/wp-content/uploads/2018/05/IMG_5718.jpg)


##Hardware Installation <BR>

To Hook 8 Pixel Strip up to an ESP32:

1) Take a Grove to Female Pin Header Cable (or a Male Pin Header Cable if you are using a Breadboard)

2) Connect Yellow (P1) to GPIO#21 on your ESP32 

3) Connect Red to 3.3V on your ESP32 (Note:  You can connect this to 5.0V too)

4) Connect Black to GND on your ESP32


##To Modify

Change the following at the top of the PixelFunctions to use other pins or if you are chaining 8 pixel strips.
The example uses Pin 21 and has two 8 Pixel Strips chained together.

<pre>
  { .rmtChannel = 1, .gpioNum = 21, .ledType = LED_SK6812W_V1, .brightLimit = 128, .numPixels =  16,
    .pixels = nullptr, ._stateVars = nullptr
  },
</pre>


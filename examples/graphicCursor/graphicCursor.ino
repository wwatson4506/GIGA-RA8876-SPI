// graphicsCursor.ino

/*
* This sketch uses a USB mouse and USBhost_t36 to control the RA8876 graphics
* cursor. It initializes the graphis cursor memory with four different
* cursor shapes. These are defined in Ra8876_Lite and can be changed. 
* tft.Select_Graphic_Cursor_1();  Selects Pen Cursor.
* tft.Select_Graphic_Cursor_2();  Selects Arrow Cursor.
* tft.Select_Graphic_Cursor_3();  Selects Hour Glass (Busy) Cursor.
* tft.Select_Graphic_Cursor_4();  Selects Error (Stop) Cursor.
* These graphic cursors have two color setting, Foreground color and
* Outline color.
* I have included a few experimental routines of my own for detecting
* single and double clicks of the left mouse button.
*
* For double and single click testing mouse.cpp in USBHost_t36 library
* has to have "//	buttons = 0;" commented out.
* 
void MouseController::mouseDataClear() {
	mouseEvent = false;
//	buttons = 0;
	mouseX  = 0;
	mouseY  = 0;
	wheel   = 0;
	wheelH  = 0;
}

The button presses automatically cleared on release.
* I don't think they need to be part of "mouseDataClear();". Also
* wheel and wheelH are cleared when there is a button press or there is
* movement of the mouse.
*/

#include "RA8876_Config_SPI.h"
#include <SPI.h>
#include <RA8876_GIGA.h>
#include <LibPrintf.h>
#include "USBMouse.h"


RA8876_GIGA tft = RA8876_GIGA(RA8876_CS, RA8876_RESET); //Using standard SPI pins

USBMouse mouse;
extern usbMouseMsg_struct mouse_msg;

#define LEFT_MOUSE_BUTTON 1
#define RIGHT_MOUSE_BUTTON 2
#define MIDDLE_MOUSE_BUTTON 4
uint8_t scCount = 0;
uint8_t dcCount = 0;

void setup() {
  //I'm guessing most copies of this display are using external PWM
  //backlight control instead of the internal RA8876 PWM.
  //Connect a Teensy pin to pin 14 on the display.
  //Can use analogWrite() but I suggest you increase the PWM frequency first so it doesn't sing.
#if defined(BACKLITE) // Defined in RA8876_Config.h.
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);
#endif
  Serial.begin(9600);
  while (!Serial && millis() < 1000) {} //wait for Serial Monitor

  while (!mouse.connect()) {
    delay(5000);
    printf("No mouse connected\n");
  }
  printf("Mouse is connected\n");
  
  mouse_msg.scaledX = 512;
  mouse_msg.scaledY = 300;

  Serial.println("USB Mouse and Graphic Cursor Testing");

  tft.setBusWidth(8);  // 16 bit traansfers not optimized yet.

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif

  tft.fillScreen(DARKBLUE);
  tft.setFontSize(1, false);
  tft.setCursor(0,0);
  tft.setTextColor(YELLOW, DARKBLUE);
  tft.print("USB Mouse and Graphic Cursor Testing");

  tft.Graphic_cursor_initial(); // Initialize all 4 cursors.
  tft.Select_Graphic_Cursor_2(); // Select Arrow Cursor.
  tft.Enable_Graphic_Cursor(); // Turn on selected graphic cursor.
  // You can play with the cursor colors in the following two lines.
  tft.Set_Graphic_Cursor_Color_1(0xff); // White forground Color. (0 - 255)
  tft.Set_Graphic_Cursor_Color_2(0x00); // Black outline Color. (0 - 255)
  tft.Graphic_Cursor_XY(mouse_msg.scaledX, mouse_msg.scaledY); // Center cursor on screen.
  tft.drawSquareFill(300,100,800,500,WHITE);
  tft.setTextColor(GREEN, DARKBLUE);

}

void loop() {
  char sbuf[256] = {};
  if(mouse.mouseEvent()) { // Wait for a mouse event.
    tft.Graphic_Cursor_XY(mouse_msg.scaledX, mouse_msg.scaledY); // Center cursor on screen
    tft.textxy(0,5);
    sprintf(sbuf,"Mouse X: %4.4d", mouse_msg.scaledX);
	tft.println(sbuf);
    sprintf(sbuf,"Mouse Y: %4.4d", mouse_msg.scaledY);
	tft.println(sbuf);
    sprintf(sbuf,"Buttons: %d", mouse.getMouseButtons());
	tft.println(sbuf);
    sprintf(sbuf,"Wheel: %2d", mouse_msg.wheel);
	tft.println(sbuf);
    sprintf(sbuf,"WheelH: %2d", mouse_msg.wheelH);
	tft.println(sbuf);

	mouse.countMouseClicks(LEFT_MOUSE_BUTTON); // ***** This method is blocking *****

    scCount += mouse.getSnglClick(); // Add to Single Click Count.
    dcCount += mouse.getDblClick(); // Add to Double Click Count.
    sprintf(sbuf,"Single Clicks: %d", scCount);
	tft.println(sbuf);
    sprintf(sbuf,"Double Clicks: %d", dcCount);
	tft.println(sbuf);

    mouse.mouseDataClear();
  }	
}

// Simple example to embed pictures in your sketch
// and draw on the ILI9341 display with writeRect()
//
// By Frank Bösing
//
// https://forum.pjrc.com/threads/32601-SPI-Library-Issue-w-ILI9341-TFT-amp-PN532-NFC-Module-on-Teensy-3-2?p=94534&viewfull=1#post94534
#include "RA8876_Config_SPI.h"
#include <SPI.h>
#include <RA8876_GIGA.h>
#include <LibPrintf.h>
#include "font_ComicSansMS.h"
#include "elapsedMillis.h"

#define USE_WRITERECT

RA8876_GIGA tft = RA8876_GIGA(RA8876_CS, RA8876_RESET); //Using standard SPI pins

// Converted to code with:
// http://www.rinkydinkelectronics.com/t_imageconverter565.php
//
#include "teensy40_pinout1.h" //the picture
#include "teensy40_pinout2.h" //the picture
#include "teensy40_front.h"  // Try to load from DMAMEM?
#include "Teensy41_Cardlike.h"
#include "TD_T4_TopCard.h"   // Top Dog card...

/* GIMP (https://www.gimp.org/) can also be used to export the image using the following steps:

    1. File -> Export As
    2. In the Export Image dialog, use 'C source code (*.c)' as filetype.
    3. Press export to get the export options dialog.
    4. Type the desired variable name into the 'prefixed name' box.
    5. Uncheck 'GLIB types (guint8*)'
    6. Check 'Save as RGB565 (16-bit)'
    7. Press export to save your image.

  Assuming 'image_name' was typed in the 'prefixed name' box of step 4, you can have to include the c file as above,
  using the image can be done with:

    tft.writeRect(0, 0, image_name.width, image_name.height, (uint16_t*)(image_name.pixel_data));

  See also https://forum.pjrc.com/threads/35575-Export-for-ILI9341_t3-with-GIMP
  */

uint8_t rotation = 0;

void setup() {
  //I'm guessing most copies of this display are using external PWM
  //backlight control instead of the internal RA8876 PWM.
  //Connect a Teensy pin to pin 14 on the display.
  //Can use analogWrite() but I suggest you increase the PWM frequency first so it doesn't sing.
#if defined(BACKLITE) // Defined in RA8876_Config.h.
  pinMode(BACKLITE, OUTPUT);
  digitalWrite(BACKLITE, HIGH);
#endif
  Serial.begin(115200);
  while (!Serial && millis() < 5000) ;

  tft.setBusWidth(8);  // 16 bit transfers not optimized yet.

#if defined(USE_SPI_47000000)
  tft.begin(47000000); // Max is 47000000 MHz (using short 3" wires)
#else
  tft.begin(); // default SPI clock speed is 30000000 MHz 
#endif

  printf("\n\nStart RA8876 picture embed test"); Serial.flush();
  printf("Screen Width:%d Height: %d\n", tft.width(), tft.height());
  tft.fillScreen(BLACK);
  delay(250);
  tft.fillScreen(RED);
  delay(250);
  tft.fillScreen(GREEN);
  delay(250);
  tft.fillScreen(BLUE);
  Serial.println("entering an 's' char will toggle on/off step mode");
}

// See if writerect works
void drawImage(uint16_t image_width, uint16_t image_height, uint16_t *image, uint16_t bgColor, bool preRotatedImage)  {
  tft.setFont(ComicSansMS_10);
  tft.setTextColor(WHITE);
  elapsedMillis em = 0;
  // first lets fill in part of screen that our image does not cover
  int16_t start_x = (tft.width() - image_width) / 2;
  int16_t start_y = (tft.height() - image_height) / 2;

  tft.fillRect(0, 0, tft.width(), start_y, bgColor);  // top
  tft.fillRect(0, start_y, start_x, image_height, bgColor); // left
  tft.fillRect(start_x + image_width, start_y, tft.width() - (start_x + image_width), image_height, bgColor); // right
  tft.fillRect(0, start_y + image_height, tft.width(), tft.height() - (start_y + image_height), bgColor); // top;

  if (preRotatedImage) {
    if (image) tft.writeRotatedRect(start_x, start_y, image_width, image_height, image);
    else {
      tft.setCursor(CENTER, CENTER, true);
      tft.print("Rotate image failed");
    }
  }
  else tft.writeRect(start_x, start_y, image_width, image_height, image);
//  else tft.putPicture_16bppData16(start_x, start_y, image_width, image_height, (const unsigned short *)image);
  uint32_t dt = em;
  tft.setCursor(10, 10);
  tft.print(dt, DEC);
  uint32_t dt2 = em;
  tft.print(" ");
  tft.print(dt2, DEC);
  printf("Image: %d Total: %d\n", dt, dt2);
}

void loop(void) {
  uint16_t *rotated_image = nullptr;
  printf("Set Rotation(%d)\n", rotation);
  tft.setRotation(rotation);
  tft.setFont(ComicSansMS_24);
  tft.fillScreen(RED);
  tft.setCursor(tft.width() / 2, tft.height() / 2, true);
  tft.setTextColor(GREEN);
  tft.print("Rotation: ");
  tft.println(rotation);
  if (DelayOrStep()) return;
  rotation = (rotation + 1) & 0x3;

  Serial.print("Display Front of card ");
  drawImage(240, 320, (uint16_t*)teensy40_pinout1, RED, false);
  if (DelayOrStep()) return;

  Serial.print("Display Back of card ");
  drawImage(240, 320, (uint16_t*)teensy40_pinout2, GREEN, false);
  if (DelayOrStep()) return;

  Serial.print("Display T4.1 Extended card ");
  drawImage(575, 424, (uint16_t*)teensy41_Cardlike, BLUE, false);
  if (DelayOrStep()) return;

  Serial.print("Display Talldog T4 card ");
  drawImage(400, 272, (uint16_t*)td_t4_top, BLUE, false);
  if (DelayOrStep()) return;

  Serial.print("Display front of chip ");
  drawImage(240, 320, (uint16_t*)teensy40_front, BLUE, false);
  if (DelayOrStep()) return;

  // lets try to pre rotate image and see if it speeds up
  Serial.print("Display rotated T4.1 Extended card ");
  rotated_image = tft.rotateImageRect(575, 424, (uint16_t*)teensy41_Cardlike);
  drawImage(575, 424, (uint16_t*)rotated_image, DARKGREEN, true);
  if (DelayOrStep()) return;

  Serial.print("Display TallDog T4  pre rotated Card ");
  rotated_image = tft.rotateImageRect(400, 272, (uint16_t*)td_t4_top);
  drawImage(400, 272, (uint16_t*)rotated_image, DARKGREEN, true);
  if (DelayOrStep()) return;

  Serial.print("Display TallDog T4  pre rotated Card ");
  rotated_image = tft.rotateImageRect(240, 320, (uint16_t*)teensy40_pinout1);
  drawImage(240, 320, (uint16_t*)rotated_image, DARKGREEN, true);

  if (rotated_image) SDRAM.free(rotated_image);
  if (DelayOrStep()) return;
}

bool step_mode = false;
bool DelayOrStep() {

  if (Serial.available()) {
    uint8_t ch = Serial.read();
    while (Serial.read() != -1) ; // get rid of the rest...
    if (ch == 's' || ch == 'S') {
      if (step_mode) {
        step_mode = false;
        Serial.println("*** Step mode turned off ***");
      } else {
        step_mode = true;
        Serial.println("*** step mode turned ON ***");
      }
    } else if ((ch >= '0') && (ch <= '3')) {
      rotation = ch - '0';
      printf("Switching to rotation: %d\n", rotation);
      return true;  // tell caller we changed to new rotation.

    }
    return false;
  }
  if (step_mode) {
    int ch;
    Serial.println("Press any key to continue");
    while ((ch = Serial.read()) == -1) ;
    while (Serial.read() != -1) ;
    if (ch == 's' || ch == 'S') {
      step_mode = false;
      Serial.println("*** Step mode turned off ***");
    } else if ((ch >= '0') && (ch <= '3')) {
      rotation = ch - '0';
      printf("Switching to rotation: %d\n", rotation);
      return true;  // tell caller we changed to new rotation.
    }
  } else {
    delay(5000);
  }
  return false;
}

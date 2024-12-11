#ifndef USBMOUSE_H__
#define USBMOUSE_H__

#include <Arduino_USBHostMbed5.h>
#include "USBHostMouseEx.h"
#include <LibPrintf.h>

#define LEFT_MOUSE_BUTTON 1
#define RIGHT_MOUSE_BUTTON 2
#define MIDDLE_MOUSE_BUTTON 4

typedef struct usbMouseMsg_struct usbMouseMsg_t;

struct usbMouseMsg_struct {
	uint8_t buttons;
	uint8_t snglClick;
	uint8_t dblClick;
	uint8_t clickCount;
	int8_t mousex;
	int8_t mousey;
	int16_t scaledX;
	int16_t scaledY;
	int8_t wheel;
	int8_t wheelH;
	int16_t mouseXpos;
	int16_t mouseYpos;
	bool mouseEvent;
};



class USBMouse {
public:

  USBMouse();	
  
  bool USBMouseInit(void);
  bool connect(void);
  bool connected(void);
  bool available();
  void mouseDataClear(void);
  uint16_t getMouseXpos(void);
  uint16_t getMouseYpos(void);
  uint8_t getMouseButtons(void);
  int8_t getMouseWheel(void);
  int8_t getMouseWheelH(void);
  bool mouseEvent(void);
  bool mouseFocused(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  bool mouseFocusedT(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
  void countMouseClicks(uint8_t button);
  uint8_t getSnglClick(void);
  uint8_t getDblClick(void);
  void scaleMouseXY(void);

protected:

private:

  int16_t	_width = 1024; // These will be replaced by actual
  int16_t	_height = 600; // screen x and y sizes. 

  int16_t fine_dx = 0;
  int16_t fine_dy = 0;
  int16_t event_dx = 0;
  int16_t event_dy = 0;
  int16_t nevent_dx = 0;
  int16_t nevent_dy = 0;
  int16_t delta = 127; // Mouse speed
  int16_t accel = 5;   // Acccel rate
  int16_t scaleX = 2;  // X step
  int16_t scaleY = 2;  // Y step

void init();

};

#endif

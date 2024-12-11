// USBMouse.cpp

#include "USBMouse.h"

usbMouseMsg_struct mouse_msg;

USBHostMouseEx rodent;

// Constructor
USBMouse::USBMouse() {
  init();
}

//***********************************************
// Set mouse coords  to the center of the screen.
//***********************************************
void USBMouse::init() {
  mouse_msg.scaledX = _width / 2;
  mouse_msg.scaledY = _height / 2;
}

//*******************************
// Connect USB mouse if attached.
//*******************************
bool USBMouse::connect(void) {
    return rodent.connect();
}
	
//**************************************
// Check if attached mouse is connected.
//**************************************
bool USBMouse::connected(void) {
	return rodent.connected();
}

//*****************************************
// Returns current mouse scaled X position.
//*****************************************
uint16_t USBMouse::getMouseXpos(void) {
	return mouse_msg.scaledX;
}

//*****************************************
// Returns current mouse scaled Y position.
//*****************************************
uint16_t USBMouse::getMouseYpos(void) {
	return mouse_msg.scaledY;
}

uint8_t USBMouse::getMouseButtons(void) {
	return mouse_msg.buttons;
}

int8_t USBMouse::getMouseWheel(void) {
	return mouse_msg.wheel;
}

int8_t USBMouse::getMouseWheelH(void) {
	return mouse_msg.wheelH;
}

bool USBMouse::available() {
  return mouseEvent();
}

void USBMouse::mouseDataClear(void) {
	rodent.mouseDataClear();
}

bool USBMouse::mouseEvent(void) {
	if(!rodent.available())
		return false;
	mouse_msg.wheel = (int8_t)rodent.getWheel();
	mouse_msg.wheelH = (int8_t)rodent.getWheelH();
	mouse_msg.buttons = (uint8_t)rodent.getButtons();
	scaleMouseXY();
	return true;
}

void USBMouse::countMouseClicks(uint8_t button) {
	uint8_t clickCount = 0;
	uint32_t timerStart = 0;
	uint32_t timeOut = 375;
	bool getOut = false;

    button &= 0x07; // Mask off three buttons
	while(!getOut) {
		// Check for Left mouse button double click.
		if((rodent.getButtons() & button)) {
			if(clickCount == 0) {
				mouse_msg.snglClick++;
				timerStart = millis(); 
			}
			clickCount++;
			if(clickCount >= 2) {
				clickCount = 0;
				mouse_msg.dblClick++;
				getOut = true;
			}
			while(rodent.getButtons() & button) delayMicroseconds(5); 
		}
		if(((millis()-timerStart) > timeOut) || (getOut == true))
			break;
	}
}
	
uint8_t USBMouse::getDblClick(void) {
	uint8_t dClick = mouse_msg.dblClick;
	if(dClick > 0)
		mouse_msg.dblClick = 0;
	return dClick;
}

//*******************************************
// 
//*******************************************
uint8_t USBMouse::getSnglClick(void) {
	uint8_t sClick = mouse_msg.snglClick;
	if(sClick > 0)
		mouse_msg.snglClick = 0;
	return sClick;
}

//*******************************************
// Check if mouse coords in a specific range.
//*******************************************
boolean USBMouse::mouseFocused(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	uint16_t mouseXpos = getMouseXpos();
	uint16_t mouseYpos = getMouseYpos();

	if((mouseXpos > x0) && (mouseXpos < x1) &&
       (mouseYpos > y0) && (mouseYpos < y1))
		return true;
	else
		return false;
}

//******************************************
// Scale mouse mouse x,y to screen x,y size.
//******************************************
void USBMouse::scaleMouseXY(void) {
	
	nevent_dx = (int16_t)rodent.getMouseX();
	nevent_dy = (int16_t)rodent.getMouseY();
	
	if(abs(nevent_dx) + abs(nevent_dy) > delta) {
		nevent_dx *= accel;
		nevent_dy *= accel;
	}
	
	event_dx += nevent_dx;
	event_dy += nevent_dy;
	
	fine_dx += event_dx; 
	fine_dy += event_dy; 
	
	event_dx = fine_dx / scaleX;
	event_dy = fine_dy / scaleY;
	
	fine_dx %= scaleX;
	fine_dy %= scaleY;
	
	mouse_msg.scaledX += event_dx;
	mouse_msg.scaledY += event_dy;

  if(mouse_msg.scaledX < 0) mouse_msg.scaledX = 0;
  if(mouse_msg.scaledX > (uint16_t)_width) mouse_msg.scaledX = (uint16_t)_width;
  if(mouse_msg.scaledY < 0)	mouse_msg.scaledY = 0;
  if(mouse_msg.scaledY > (uint16_t)_height)	mouse_msg.scaledY = (uint16_t)_height;
}

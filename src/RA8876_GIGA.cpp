//**************************************************************//
//**************************************************************//
/*
 * Ra8876LiteTeensy.cpp
 * Modified Version of: File Name : RA8876_t3.cpp
 *			Author    : RAiO Application Team
 *			Edit Date : 09/13/2017
 *			Version   : v2.0  1.modify bte_DestinationMemoryStartAddr bug
 *                 			  2.modify ra8876SdramInitial Auto_Refresh
 *                 			  3.modify ra8876PllInitial
 * 	  	      : For Teensy 3.x and T4
 *                    : By Warren Watson
 *                    : 06/07/2018 - 11/31/2019
 *                    : Copyright (c) 2017-2019 Warren Watson.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//**************************************************************//

#include "Arduino.h"
#include <SPI.h>
#include "RA8876_GIGA.h"
#include <api/itoa.h>
#include "pinDefinitions.h"
#include <LibPrintf.h>

//**************************************************************//
// RA8876_GIGA()
//**************************************************************//
RA8876_GIGA::SPI_Hardware_info_t RA8876_GIGA::s_spi_hardware_mapping[2] = {
#if defined(ARDUINO_GIGA)
//  {&SPI,  (SPI_TypeDef *) SPI1_BASE, &dmaInterrupt, 38, 37},
//  {&SPI1, (SPI_TypeDef *) SPI5_BASE, &dmaInterrupt1, 86, 85}
#elif defined(ARDUINO_PORTENTA_H7_M7)
//  {&SPI,  (SPI_TypeDef *) SPI2_BASE, &dmaInterrupt, 40, 39},
//  {nullptr, (SPI_TypeDef *) nullptr, &dmaInterrupt1, 86, 85}
#endif
};

RA8876_GIGA::RA8876_GIGA(SPIClass *pspi, uint8_t cs, uint8_t rst, DMA_Stream_TypeDef * dmaStream) {
  _pspi = pspi;
  _cs = cs;
  _rst = rst;
  _dmaStream = dmaStream;
  setBusWidth(8);
}

// Create RA8876 driver instance
RA8876_GIGA::RA8876_GIGA(uint8_t CSp, uint8_t RSTp, uint8_t mosi_pin, uint8_t sclk_pin, uint8_t miso_pin) {
  _mosi = mosi_pin;
  _miso = miso_pin;
  _sclk = sclk_pin;
  _cs = CSp;
  _rst = RSTp;
  setBusWidth(8);
}

//**************************************************************//
// RA8876_begin()
//**************************************************************//
boolean RA8876_GIGA::begin(uint32_t spi_clock) {
    // initialize the bus for GIGA
    //  Figure out which SPI Buss to use.
	_SPI_CLOCK = spi_clock;           // #define ILI9341_SPICLOCK 30000000

  SDRAM.begin();
  
  _pspi = &SPI1;
  _pspi->begin();

  // map to hardware index.
  for (_spi_num = 0; _spi_num < (sizeof(s_spi_hardware_mapping)/sizeof(s_spi_hardware_mapping[0])); _spi_num++)
  {
    if (s_spi_hardware_mapping[_spi_num].pspi == _pspi) {
      _pgigaSpi = s_spi_hardware_mapping[_spi_num].pgigaSpi;
      break;
    }   
  }

  static  GPIO_TypeDef * const port_table[] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOE, GPIOF, GPIOG, GPIOH, GPIOI, GPIOJ, GPIOK };

  pinMode(_cs, OUTPUT);
  digitalWrite(_cs, HIGH);

  PinName pin_name = g_APinDescription[_cs].name;
  GPIO_TypeDef  *  port = port_table[pin_name >> 4];
  _csBSRR = (__IO uint32_t *)(&port->BSRR);
  _cspinmask = 1 << (pin_name & 0xf);

    // toggle RST low to reset
    if (_rst < 255) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(5);
        digitalWrite(_rst, LOW);
        delay(20);
        digitalWrite(_rst, HIGH);
        delay(150);
    }

    if (!checkIcReady()) {
		printf("RA8876 Not connected!!!\n");
        return false;
    }    

    // read ID code must disable pll, 01h bit7 set 0

    lcdRegDataWrite(0x01, 0x08);
    delay(1);
    if ((lcdRegDataRead(0xff) != 0x76) && (lcdRegDataRead(0xff) != 0x77)) {
   		printf("Write reg 0x01,0x08 Failed!!!\n");
        return false;
    }
    // Initialize RA8876 to default settings
    if (!ra8876Initialize()) {
		printf("ra8876Initialize() Failed!!!\n");
        return false;
    }
    // return success
    return true;
}

//**************************************************************//
// Write to a RA8876 register
//**************************************************************//
void RA8876_GIGA::lcdRegWrite(ru8 reg, bool finalize) {
    ru16 _data = (RA8876_SPI_CMDWRITE16 | reg);

    startSend();
    _pspi->transfer16(_data);
    endSend(finalize);
}

void RA8876_GIGA::LCD_CmdWrite(unsigned char cmd) {

    startSend();
    _pspi->transfer16(0x00);
    _pspi->transfer(cmd);
    endSend(true);
}

//**************************************************************//
// Write RA8876 Data
//**************************************************************//
void RA8876_GIGA::lcdDataWrite(ru8 data, bool finalize) {
    ru16 _data = (RA8876_SPI_DATAWRITE16 | data);

    startSend();
    _pspi->transfer16(_data);
    endSend(finalize);
}

//**************************************************************//
// Read RA8876 Data
//**************************************************************//
ru8 RA8876_GIGA::lcdDataRead(bool finalize) {
    ru16 _data = (RA8876_SPI_DATAREAD16 | 0x00);

    startSend();
    ru8 data = _pspi->transfer16(_data);
    endSend(finalize);
    return data;
}

//**************************************************************//
// Read RA8876 Data 16-bit
//**************************************************************//
ru16 RA8876_GIGA::lcdDataRead16(bool finalize) {
    ru16 _data = (RA8876_SPI_DATAREAD16 | 0x00);

    startSend();
    ru16 data = _pspi->transfer16(_data);
    endSend(finalize);
    return data;
}

//**************************************************************//
// Read RA8876 status register
//**************************************************************//
ru8 RA8876_GIGA::lcdStatusRead(bool finalize) {
    startSend();
    ru8 data = _pspi->transfer16(RA8876_SPI_STATUSREAD16);
    endSend(finalize);
    return data;
}

//**************************************************************//
// Write Data to a RA8876 register
//**************************************************************//
void RA8876_GIGA::lcdRegDataWrite(ru8 reg, ru8 data, bool finalize) {
    // write the register we wish to write to, then send the data
    // don't need to release _CS between the two transfers
    // ru16 _reg = (RA8876_SPI_CMDWRITE16 | reg);
    // ru16 _data = (RA8876_SPI_DATAWRITE16 | data);
    uint8_t buf[4] = {RA8876_SPI_CMDWRITE, reg, RA8876_SPI_DATAWRITE, data};
    startSend();
    //_pspi->transfer16(_reg);
    //_pspi->transfer16(_data);
    _pspi->transfer(buf, 4);
    endSend(finalize);
}

//**************************************************************//
// Read a RA8876 register Data
//**************************************************************//
ru8 RA8876_GIGA::lcdRegDataRead(ru8 reg, bool finalize) {
    lcdRegWrite(reg, finalize);
    return lcdDataRead();
}

//**************************************************************//
// support SPI interface to write 16bpp data after Regwrite 04h
//**************************************************************//
void RA8876_GIGA::lcdDataWrite16bbp(ru16 data, bool finalize) {
    startSend();
    _pspi->transfer(RA8876_SPI_DATAWRITE);
    _pspi->transfer16(data);
    endSend(finalize);
}

//**************************************************************//
// Send data from the microcontroller to the RA8876
// Does a Raster OPeration to combine with an image already in memory
// For a simple overwrite operation, use ROP 12
//**************************************************************//
void RA8876_GIGA::bteMpuWriteWithROPData8(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width,
                                        ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code, const unsigned char *data) {
    bteMpuWriteWithROP(s1_addr, s1_image_width, s1_x, s1_y, des_addr, des_image_width, des_x, des_y, width, height, rop_code);

    startSend();
    _pspi->transfer(RA8876_SPI_DATAWRITE);

    // If you try _pspi->transfer(data, length) then this tries to write received data into the data buffer
    // but if we were given a PROGMEM  or a const (unwriteable) data pointer then _pspi->transfer will lock up totally.
    // So we explicitly tell it we don't care about any return data.
    _pspi->transfer((char *)data, NULL, width * height * 2);

    endSend(true);

}
//**************************************************************//
// For 16-bit byte-reversed data.
// Note this is 4-5 milliseconds slower than the 8-bit version above
// as the bulk byte-reversing SPI transfer operation is not available
// on all Teensys.
//**************************************************************//
void RA8876_GIGA::bteMpuWriteWithROPData16(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width,
                                         ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code, const unsigned short *data) {
    ru16 i, j;
    bteMpuWriteWithROP(s1_addr, s1_image_width, s1_x, s1_y, des_addr, des_image_width, des_x, des_y, width, height, rop_code);

    startSend();
    _pspi->transfer(RA8876_SPI_DATAWRITE);

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            _pspi->transfer16(*data);
            data++;
        }
    }

    endSend(true);
}

//==========================================================================================

//**************************************************************//
/* Write 16bpp(RGB565) picture data for user operation          */
/* Not recommended for future use - use BTE instead             */
//**************************************************************//
void RA8876_GIGA::putPicture_16bpp(ru16 x, ru16 y, ru16 width, ru16 height) {
    graphicMode(true);
    activeWindowXY(x, y);
    activeWindowWH(width, height);
    setPixelCursor(x, y);
    ramAccessPrepare();
    // Now your program has to send the image data pixels
}

//*******************************************************************//
/* write 16bpp(RGB565) picture data in byte format from data pointer */
/* Not recommended for future use - use BTE instead                  */
//*******************************************************************//
void RA8876_GIGA::putPicture_16bppData8(ru16 x, ru16 y, ru16 width, ru16 height, const unsigned char *data) {
    ru16 i, j;
    graphicMode(true);
    activeWindowXY(x, y);
    activeWindowWH(width, height);
    setPixelCursor(x, y);
    ramAccessPrepare();
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            // checkWriteFifoNotFull();  //if high speed mcu and without Xnwait check
            lcdDataWrite(*data);
            data++;
            // checkWriteFifoNotFull();  //if high speed mcu and without Xnwait check
            lcdDataWrite(*data);
            data++;
        }
    }
    checkWriteFifoEmpty(); // if high speed mcu and without Xnwait check
    activeWindowXY(0, 0);
    activeWindowWH(_width, _height);
}

//****************************************************************//
/* Write 16bpp(RGB565) picture data word format from data pointer */
/* Not recommended for future use - use BTE instead               */
//****************************************************************//
void RA8876_GIGA::putPicture_16bppData16(ru16 x, ru16 y, ru16 width, ru16 height, const unsigned short *data) {
    ru16 i, j;
	uint16_t temp = 0;
    putPicture_16bpp(x, y, width, height);

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            // checkWriteFifoNotFull();//if high speed mcu and without Xnwait check
			temp =  (uint16_t)(*data << 8) | (uint16_t)(*data >> 8);
            lcdDataWrite16bbp(temp);
//            lcdDataWrite16bbp(*data);
            data++;
            // checkWriteFifoEmpty();//if high speed mcu and without Xnwait check
        }
    }
    checkWriteFifoEmpty(); // if high speed mcu and without Xnwait check
    activeWindowXY(0, 0);
    activeWindowWH(_width, _height);
}

//**************************************************************//
// write data after setting, using lcdDataWrite() or lcdDataWrite16bbp()
//**************************************************************//
void RA8876_GIGA::bteMpuWriteWithROP(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width, ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code) {
    check2dBusy();
    graphicMode(true);
    bte_Source1_MemoryStartAddr(s1_addr);
    bte_Source1_ImageWidth(s1_image_width);
    bte_Source1_WindowStartXY(s1_x, s1_y);
    bte_DestinationMemoryStartAddr(des_addr);
    bte_DestinationImageWidth(des_image_width);
    bte_DestinationWindowStartXY(des_x, des_y);
    bte_WindowSize(width, height);
    lcdRegDataWrite(RA8876_BTE_CTRL1, rop_code << 4 | RA8876_BTE_MPU_WRITE_WITH_ROP);                                                             // 91h
    lcdRegDataWrite(RA8876_BTE_COLR, RA8876_S0_COLOR_DEPTH_16BPP << 5 | RA8876_S1_COLOR_DEPTH_16BPP << 2 | RA8876_DESTINATION_COLOR_DEPTH_16BPP); // 92h
    lcdRegDataWrite(RA8876_BTE_CTRL0, RA8876_BTE_ENABLE << 4);                                                                                    // 90h
    ramAccessPrepare();
}

//**************************************************************//
// Send data from the microcontroller to the RA8876
// Does a chromakey (transparent color) to combine with the image already in memory
//**************************************************************//
void RA8876_GIGA::bteMpuWriteWithChromaKeyData8(ru32 des_addr, ru16 des_image_width, ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru16 chromakey_color, const unsigned char *data) {
    bteMpuWriteWithChromaKey(des_addr, des_image_width, des_x, des_y, width, height, chromakey_color);

    startSend();
    _pspi->transfer(RA8876_SPI_DATAWRITE);

    _pspi->transfer((char *)data, NULL, width * height * 2);
    endSend(true);
}

//**************************************************************//
// Chromakey for 16-bit byte-reversed data. (Slower than 8-bit.)
//**************************************************************//
void RA8876_GIGA::bteMpuWriteWithChromaKeyData16(ru32 des_addr, ru16 des_image_width, ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru16 chromakey_color, const unsigned short *data) {
    ru16 i, j;
    bteMpuWriteWithChromaKey(des_addr, des_image_width, des_x, des_y, width, height, chromakey_color);

    startSend();
    _pspi->transfer(RA8876_SPI_DATAWRITE);
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            _pspi->transfer16(*data);
            data++;
        }
    }
    endSend(true);
}

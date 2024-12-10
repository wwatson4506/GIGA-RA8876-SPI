//**************************************************************//
// Teensy 4.1 8080 Parallel 8/16 bit with 8 bit ASYNC support.
//**************************************************************//
/*
 * Ra8876LiteTeensy.cpp
 * Modified Version of: File Name : RA8876_t41_p.cpp
 *          Author    : RAiO Application Team
 *          Edit Date : 09/13/2017
 *          Version   : v2.0  1.modify bte_DestinationMemoryStartAddr bug
 *                            2.modify ra8876SdramInitial Auto_Refresh
 *                            3.modify ra8876PllInitial
 ****************************************************************
 *                    : New 8080 Parallel version
 *                    : For MicroMod
 *                    : By Warren Watson
 *                    : 06/07/2018 - 05/03/2024
 *                    : Copyright (c) 2017-2024 Warren Watson.
 *****************************************************************
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
/*File Name : tft.h
 *          : For Teensy 3.x and T4
 *          : By Warren Watson
 *          : 06/07/2018 - 11/31/2019
 *          : Copyright (c) 2017-2019 Warren Watson.
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
/***************************************************************
 *  Modified by mjs513 and KurtE and MorganS
 *  Combined libraries and added functions to be compatible with
 *  other display libraries
 *  See PJRC Forum Thread: https://forum.pjrc.com/threads/58565-RA8876LiteTeensy-For-Teensy-T36-and-T40/page5
 *
 ***************************************************************/
#ifndef _RA8876_GIGA_P
#define _RA8876_GIGA_P

// Allow us to enable or disable capabilities, particully Frame Buffer and
// Clipping for speed and size
#ifndef DISABLE_RA8876_FRAMEBUFFER
// disable for first pass
#define ENABLE_RA8876_FRAMEBUFFER
#endif

#include "Arduino.h"
#include "RA8876Registers.h"
#include "GIGA_digitalWriteFast.h"
#include "RA8876_GIGA_common.h"
#include <SPI.h>
#include <stdint.h>

// Default to a relatively slow speed for breadboard testing.
// const ru32 SPIspeed = 47000000;
const ru32 SPIspeed = 30000000;

class RA8876_GIGA : public RA8876_common {
  public:
    RA8876_GIGA(SPIClass *pspi, uint8_t cs, uint8_t rst = 255, DMA_Stream_TypeDef * dmaStream = DMA1_Stream1);

    RA8876_GIGA(uint8_t CSp, uint8_t RSTp = 255, uint8_t mosi_pin = 11, uint8_t sclk_pin = 13, uint8_t miso_pin = 12);
  
    boolean begin(uint32_t spi_clock = SPIspeed);

    /*access*/
    //dummy
//    ru16 lcdDataRead16(bool finalize = true) { return 0; };
    void lcdRegWrite(ru8 reg, bool finalize = true);
    void lcdDataWrite(ru8 data, bool finalize = true);
    ru8 lcdDataRead(bool finalize = true);
    ru16 lcdDataRead16(bool finalize = true);
    ru16 lcdDataRead16bpp(bool finalize = true);
    ru8 lcdStatusRead(bool finalize = true);
    void lcdRegDataWrite(ru8 reg, ru8 data, bool finalize = true);
    ru8 lcdRegDataRead(ru8 reg, bool finalize = true);
    void lcdDataWrite16bbp(ru16 data, bool finalize = true);

    /*BTE function*/
    void bteMpuWriteWithROPData8(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width,
                                 ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code, const unsigned char *data);
    void bteMpuWriteWithROPData16(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width,
                                  ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code, const unsigned short *data);
    void bteMpuWriteWithROP(ru32 s1_addr, ru16 s1_image_width, ru16 s1_x, ru16 s1_y, ru32 des_addr, ru16 des_image_width,
                            ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru8 rop_code);

    void bteMpuWriteWithChromaKeyData8(ru32 des_addr, ru16 des_image_width, ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru16 chromakey_color,
                                       const unsigned char *data);

    void bteMpuWriteWithChromaKeyData16(ru32 des_addr, ru16 des_image_width, ru16 des_x, ru16 des_y, ru16 width, ru16 height, ru16 chromakey_color,
                                        const unsigned short *data);

    /*  Picture Functions */
    void putPicture_16bpp(ru16 x, ru16 y, ru16 width, ru16 height);                                   // not recommended: use BTE instead
    void putPicture_16bppData8(ru16 x, ru16 y, ru16 width, ru16 height, const unsigned char *data);   // not recommended: use BTE instead
    void putPicture_16bppData16(ru16 x, ru16 y, ru16 width, ru16 height, const unsigned short *data); // not recommended: use BTE instead


    void LCD_CmdWrite(unsigned char cmd);

//  private:
    // int _xnscs, _xnreset;
    int _mosi;
    int _miso;
    int _sclk;
    int _cs;
    int _rst;
    int _errorCode;

  __IO uint32_t *_csBSRR; 
  uint16_t _cspinmask;

//  volatile uint8_t _data_sent_not_completed = 0;  // how much data has been sent that we are waiting for info
  volatile bool  _data_sent_since_last_transmit_complete = 0;  // have we sent anything since

  SPI_TypeDef *_pgigaSpi = nullptr;
  DMA_TypeDef *_pdma = nullptr;
  DMA_Stream_TypeDef *_dmaStream = DMA1_Stream1;
  DMAMUX_Channel_TypeDef *_dmamux = nullptr;
  IRQn_Type _dmaTXIrq;
  uint8_t _dma_channel;

  // Mapping structure from which SPI, to other info on that SPI..
  typedef struct {
    SPIClass      *pspi;          // Which SPI is this (SPI, SPI1)
    SPI_TypeDef   *pgigaSpi;      // What is the underlying hardware SPI object;
    void (*txdmaisr)(void);        // which call back should we use for this one.
    uint8_t       tx_dmamux1_req_id; // What is the DMAMUX Request ID for this SPI object
    uint8_t       rx_dmamux1_req_id; // What is the DMAMUX Request ID for this SPI object
  } SPI_Hardware_info_t;

  static SPI_Hardware_info_t s_spi_hardware_mapping[2];

  SPIClass *_pspi = nullptr;

    uint8_t _spi_num;         // Which buss is this spi on?
    uint32_t _SPI_CLOCK;      // #define ILI9341_SPICLOCK 30000000
    uint32_t _SPI_CLOCK_READ; // #define ILI9341_SPICLOCK_READ 2000000 - Not used on RA8876...

#ifdef ENABLE_RA8876_FRAMEBUFFER
  // Add support for optional frame buffer
  uint16_t *_pfbtft;              // Optional Frame buffer
  uint8_t _use_fbtft;             // Are we in frame buffer mode?
  uint16_t *_we_allocated_buffer; // We allocated the buffer;
  int16_t _changed_min_x, _changed_max_x, _changed_min_y, _changed_max_y;
  bool _updateChangedAreasOnly = false; // current default off,
  void (*_frame_complete_callback)() = nullptr;
  bool _frame_callback_on_HalfDone = false;

  static RA8876_GIGA *_dmaActiveDisplay[2]; // Use pointer to this as a way to get
                                         // back to object...
  volatile uint8_t _dma_state = 0;            // DMA status
  volatile uint32_t _dma_frame_count = 0;     // Can return a frame count...
  volatile uint16_t _dma_sub_frame_count = 0; // Can return a frame count...

  // GIGA DMA stuff - WIP

  static void dmaInterrupt(void);
  static void dmaInterrupt1(void);

  void process_dma_interrupt(void);
#endif

void outputToSPI(uint8_t c) {
    //_pspi->transfer(c);
#if 1
    uint32_t sr;
    uint8_t unused __attribute__((unused));
    _pgigaSpi->CR1 |= SPI_CR1_CSTART;

    // first read any pending data
    uint32_t to = 1000;

    while (!((sr = _pgigaSpi->SR) & SPI_SR_TXP) && (--to)) { }

    if (to) {
      *((__IO uint8_t *)&_pgigaSpi->TXDR) = c;
//      _data_sent_not_completed++;
      _data_sent_since_last_transmit_complete = true;
    } else {
      Serial.println("**TO** Write:  ");
      Serial.print("\tCR1:  "); Serial.println(_pgigaSpi->CR1, HEX);         /*!< SPI/I2S Control register 1,                      Address offset: 0x00 */
      Serial.print("\tCR2:  "); Serial.println(_pgigaSpi->CR2, HEX);         /*!< SPI Control register 2,                          Address offset: 0x04 */
      Serial.print("\tCFG1:  "); Serial.println(_pgigaSpi->CFG1, HEX);       /*!< SPI Configuration register 1,                    Address offset: 0x08 */
      Serial.print("\tCFG2:  "); Serial.println(_pgigaSpi->CFG2, HEX);       /*!< SPI Configuration register 2,                    Address offset: 0x0C */
      Serial.print("\tIER:  "); Serial.println(_pgigaSpi->IER, HEX);         /*!< SPI/I2S Interrupt Enable register,               Address offset: 0x10 */
      Serial.print("\tSR:  "); Serial.println(_pgigaSpi->SR, HEX);           /*!< SPI/I2S Status register,                         Address offset: 0x14 */
      delay(1000);
    }

    if (sr & SPI_SR_RXP ) {
      unused = *((__IO uint8_t *)&_pgigaSpi->RXDR);
//      if (_data_sent_not_completed) _data_sent_not_completed--;
    }

#else    
    uint32_t sr;
    uint8_t unused __attribute__((unused));
    _pgigaSpi->CR1 |= SPI_CR1_CSTART;

    // first read any pending data
    uint32_t start_time_us = micros();
    static const uint32_t TIMEOUT_US = 125;
    uint32_t delta_time = 0;
    while((delta_time = (micros() - start_time_us)) < TIMEOUT_US) {
      sr = _pgigaSpi->SR;

      // Check to see if there is any data in the RX FIFO
      if (sr & SPI_SR_OVR) {
        // have overflow, clear it
        _pgigaSpi->IFCR = SPI_IFCR_OVRC;
        // printf("@@@@@@@@@@ OVERFLOW @@@@@@@@@@\n");
      }

      if (sr & SPI_SR_RXP ) {
        unused = *((__IO uint8_t *)&_pgigaSpi->RXDR);
        if (_data_sent_not_completed) _data_sent_not_completed--;
        continue;
      }

      // See if there is room in the FIFO
      if (sr & SPI_SR_TXP ) break;

    }

    if (delta_time >=  TIMEOUT_US) {
      Serial.println("**TO** Write:  ");
      Serial.print("\tCR1:  "); Serial.println(_pgigaSpi->CR1, HEX);         /*!< SPI/I2S Control register 1,                      Address offset: 0x00 */
      Serial.print("\tCR2:  "); Serial.println(_pgigaSpi->CR2, HEX);         /*!< SPI Control register 2,                          Address offset: 0x04 */
      Serial.print("\tCFG1:  "); Serial.println(_pgigaSpi->CFG1, HEX);       /*!< SPI Configuration register 1,                    Address offset: 0x08 */
      Serial.print("\tCFG2:  "); Serial.println(_pgigaSpi->CFG2, HEX);       /*!< SPI Configuration register 2,                    Address offset: 0x0C */
      Serial.print("\tIER:  "); Serial.println(_pgigaSpi->IER, HEX);         /*!< SPI/I2S Interrupt Enable register,               Address offset: 0x10 */
      Serial.print("\tSR:  "); Serial.println(_pgigaSpi->SR, HEX);           /*!< SPI/I2S Status register,                         Address offset: 0x14 */
      delay(1000);
    } else {
      *((__IO uint8_t *)&_pgigaSpi->TXDR) = c;
      _data_sent_not_completed++;
      _data_sent_since_last_transmit_complete = true;
    }
#endif    
  }

  void outputToSPI16(uint16_t data) {
    //_pspi->transfer16(data);
    outputToSPI(data >> 8); //MSB
    outputToSPI(data & 0xff);
  }

  void beginSPITransaction(uint32_t clock) __attribute__((always_inline)) {
    //digitalWrite(_cs, LOW);
    *_csBSRR = (uint32_t)(_cspinmask << 16);  // reset
    _pspi->beginTransaction(SPISettings(clock, MSBFIRST, SPI_MODE0));
    uint32_t cr1 = _pgigaSpi->CR1;
    _pgigaSpi->CR1 &= ~SPI_CR1_SPE_Msk;

    uint32_t cfg2 = _pgigaSpi->CFG2; 
    cfg2 &= ~SPI_CFG2_MIDI_Msk;

    if (clock == _SPI_CLOCK_READ) cfg2 |= (4 << SPI_CFG2_MIDI_Pos);  // give more gap
    else cfg2 |= (1 << SPI_CFG2_MIDI_Pos);

    _pgigaSpi->CFG2 = cfg2;
    _pgigaSpi->CR1 |= (cr1 &SPI_CR1_SPE_Msk);
//    _data_sent_not_completed = 0; // start at 0 each time

  }
  void endSPITransaction() __attribute__((always_inline)) {
    _pspi->endTransaction();
    *_csBSRR = (uint32_t)(_cspinmask);  // set
    //digitalWrite(_cs, HIGH);
  }

  // Start off stupid
  uint8_t _dcpinAsserted;

  void waitTransmitComplete(uint8_t called_from = 0) {
    // so far nothing
    uint32_t start_time_us;
    uint32_t sr = 0;
    static uint32_t wtcCallCount = 0;

#if 0
    while((_data_sent_not_completed != 0) && (--to)) {
      sr = _pgigaSpi->SR;
      if (sr & SPI_SR_OVR) {
        // have overflow, clear it
        _pgigaSpi->IFCR = SPI_IFCR_OVRC;
      }
      if (sr & SPI_SR_RXP ) {
        uint8_t unused __attribute__((unused));
        uint8_t rxplvlBefore = (sr >> SPI_SR_RXPLVL_Pos) & 0x3;

        unused = *((__IO uint8_t *)&_pgigaSpi->RXDR);
        uint8_t rxplvlAfter = ( _pgigaSpi->SR >> SPI_SR_RXPLVL_Pos) & 0x3;
        static uint8_t trace_count = 64;
        _data_sent_not_completed--;
        if (trace_count) {
          trace_count--;
          Serial.write(rxplvlBefore + '0');
          Serial.write(rxplvlAfter + '0');
          Serial.println();
        }
      }
    }
#else    
//    while((((sr = _pgigaSpi->SR) & SPI_SR_TXC) == 0) && (--to)) {
    wtcCallCount++; // increment the count of how many times we were called.
//    static uint8_t trace_count = 64;
    uint32_t sr_prev = _pgigaSpi->SR;
//    if (trace_count) {
//      Serial.print(wtcCallCount, DEC);
//      Serial.print(" ");
//      Serial.print(sr_prev, HEX);
//      Serial.print(":");
//      Serial.print(_data_sent_not_completed, DEC);
//    }

    // lets try with either TXC or EOT...
    start_time_us = micros();
    static const uint32_t TIMEOUT_US = 250;
    uint32_t delta_time = 0;
    while((delta_time = (micros() - start_time_us)) < TIMEOUT_US) {
      sr = _pgigaSpi->SR;
//      if (trace_count) {
//        if (sr != sr_prev) {
//          Serial.print("->");
//          sr_prev = sr;
//          Serial.print(sr_prev, HEX);
//          Serial.print(":");
//          Serial.print(_data_sent_not_completed, DEC);
//          start_time = micros();  // keep the printing from screwing up timing
//        }
//      }

      if (sr & SPI_SR_OVR) {
        // have overflow, clear it
        _pgigaSpi->IFCR = SPI_IFCR_OVRC;
        // printf("@@@@@@@@@@ OVERFLOW @@@@@@@@@@\n");
      }

      if (sr & SPI_SR_RXP ) {
        uint8_t unused __attribute__((unused));
        unused = *((__IO uint8_t *)&_pgigaSpi->RXDR);
        //if (_data_sent_not_completed) _data_sent_not_completed--;
        continue; // go check for more
      }

      // If nothing has been sent, we can get out of here now.
      if (!_data_sent_since_last_transmit_complete) {
        break;
      }

      // Now check for end of transmission.
      if ((sr & (SPI_SR_EOT | SPI_SR_TXC)) != 0) {
        break;
      }

    }
//    if (trace_count) {
//      Serial.print("$");
//      Serial.println(_data_sent_not_completed, DEC);
//      trace_count--;
//    }

#endif
    if (delta_time >= TIMEOUT_US) {
      Serial.print("**TO** WTC:  ");
      Serial.print(wtcCallCount, DEC);
      Serial.print(" ");
      Serial.print(called_from, DEC);
      Serial.print(" ");
      Serial.print(sr_prev, HEX);
      Serial.print(" ");
      Serial.println(sr, HEX);
//      Serial.print(" ");
//      Serial.println(_data_sent_not_completed, DEC);
    }
    _data_sent_since_last_transmit_complete = false;
  }

// SPI Functions - should these be private?
inline __attribute__((always_inline)) void startSend() {
	digitalWriteFast(_cs, LOW);
    if (!RA8876_BUSY) {
        RA8876_BUSY = true;
        _pspi->beginTransaction(SPISettings(_SPI_CLOCK, MSBFIRST, SPI_MODE0));
    }
}

inline __attribute__((always_inline)) void endSend(bool finalize) {
	digitalWriteFast(_cs, HIGH);
    if (finalize) {
       _pspi->endTransaction();
        RA8876_BUSY = false;
    }
}
  protected:
};

#endif

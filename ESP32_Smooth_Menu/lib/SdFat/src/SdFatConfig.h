/**
 * Copyright (c) 2011-2022 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/**
 * \file
 * \brief configuration definitions
 */
#ifndef SdFatConfig_h
#define SdFatConfig_h
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif  // __AVR__
//
// To try UTF-8 encoded filenames.
// #define USE_UTF8_LONG_NAMES 1
//
// For minimum flash size use these settings:
// #define USE_FAT_FILE_FLAG_CONTIGUOUS 0
// #define ENABLE_DEDICATED_SPI 0
// #define USE_LONG_FILE_NAMES 0
// #define SDFAT_FILE_TYPE 1
// #define CHECK_FLASH_PROGRAMMING 0  // May cause SD to sleep at high current.
//
// Options can be set in a makefile or an IDE like platformIO
// if they are in a #ifndef/#endif block below.
//------------------------------------------------------------------------------
/** For Debug - must be one */
#define ENABLE_ARDUINO_FEATURES 1
/** For Debug - must be one */
#define ENABLE_ARDUINO_SERIAL 1
/** For Debug - must be one */
#define ENABLE_ARDUINO_STRING 1
//------------------------------------------------------------------------------
#if ENABLE_ARDUINO_FEATURES
#include "Arduino.h"
#ifdef PLATFORM_ID
// Only defined if a Particle device.
#include "application.h"
#endif  // PLATFORM_ID
#endif  // ENABLE_ARDUINO_FEATURES
//------------------------------------------------------------------------------
/**
 * File types for SdFat, File, SdFile, SdBaseFile, fstream,
 * ifstream, and ofstream.
 *
 * Set SDFAT_FILE_TYPE to:
 *
 * 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
 */
#ifndef SDFAT_FILE_TYPE
#if defined(__AVR__) && FLASHEND < 0X8000
// 32K AVR boards.
#define SDFAT_FILE_TYPE 1
#else  // defined(__AVR__) && FLASHEND < 0X8000
// All other boards.
#define SDFAT_FILE_TYPE 3
#endif  // defined(__AVR__) && FLASHEND < 0X8000
#endif  // SDFAT_FILE_TYPE
//------------------------------------------------------------------------------
/**
*将USE_FAT_FILE_FLAG_CONTIGOUS设置为非零值以优化对
连续文件。使用少量闪光灯。
 */
#ifndef USE_FAT_FILE_FLAG_CONTIGUOUS
#define USE_FAT_FILE_FLAG_CONTIGUOUS 1
#endif  // USE_FAT_FILE_FLAG_CONTIGUOUS
//------------------------------------------------------------------------------
/**
 * Set ENABLE_DEDICATED_SPI non-zero to enable dedicated use of the SPI bus.
 * Selecting dedicated SPI in SdSpiConfig() will produce better
 * performance by using very large multi-block transfers to and
 * from the SD card.
 *
 * Enabling dedicated SPI will cost extra flash and RAM.
 */
#ifndef ENABLE_DEDICATED_SPI
#if defined(__AVR__) && FLASHEND < 0X8000
// 32K AVR boards.
#define ENABLE_DEDICATED_SPI 1
#else  // defined(__AVR__) && FLASHEND < 0X8000
// All other boards.
#define ENABLE_DEDICATED_SPI 1
#endif  // defined(__AVR__) && FLASHEND < 0X8000
#endif  // ENABLE_DEDICATED_SPI
//------------------------------------------------------------------------------
// Driver options
/**
 * If the symbol SPI_DRIVER_SELECT is:
 *
 * 0 - An optimized custom SPI driver is used if it exists
 *     else the standard library driver is used.
 *
 * 1 - The standard library driver is always used.
 *
 * 2 - An external SPI driver of SoftSpiDriver template class is always used.
 *
 * 3 - An external SPI driver derived from SdSpiBaseClass is always used.
 */
#ifndef SPI_DRIVER_SELECT
#define SPI_DRIVER_SELECT 0
#endif  // SPI_DRIVER_SELECT
/**
 * If USE_SPI_ARRAY_TRANSFER is one and the standard SPI library is
 * use, the array transfer function, transfer(buf, count), will be used.
 * This option will allocate a 512 byte temporary buffer for send.
 * This may be faster for some boards.  Do not use this with AVR boards.
 *
 * Warning: the next options are often fastest but only available for some
 * non-Arduino board packages.
 *
 * If USE_SPI_ARRAY_TRANSFER is two use transfer(nullptr, buf, count) for
 * receive and transfer(buf, nullptr, count) for send.
 *
 * If USE_SPI_ARRAY_TRANSFER is three use transfer(nullptr, buf, count) for
 * receive and transfer(buf, rxTmp, count) for send. Try this with Adafruit
 * SAMD51.
 *
 * If USE_SPI_ARRAY_TRANSFER is four use transfer(txTmp, buf, count) for
 * receive and transfer(buf, rxTmp, count) for send. Try this with STM32.
 */
#ifndef USE_SPI_ARRAY_TRANSFER
#define USE_SPI_ARRAY_TRANSFER 0
#endif  // USE_SPI_ARRAY_TRANSFER
//------------------------------------------------------------------------------
/**
 * SD maximum initialization clock rate.
 */
#ifndef SD_MAX_INIT_RATE_KHZ
#define SD_MAX_INIT_RATE_KHZ 300
#endif  // SD_MAX_INIT_RATE_KHZ
/**
 * Set USE_BLOCK_DEVICE_INTERFACE nonzero to use a generic block device.
 * This allow use of an external FsBlockDevice driver that is derived from
 * the FsBlockDeviceInterface like this:
 *
 * class UsbMscDriver : public FsBlockDeviceInterface {
 *   ... code for USB mass storage class driver.
 * };
 *
 * UsbMscDriver usbMsc;
 * FsVolume key;
 * ...
 *
 *   // Init USB MSC driver.
 *   if (!usbMsc.begin()) {
 *     ... handle driver init failure.
 *   }
 *   // Init FAT/exFAT volume.
 *   if (!key.begin(&usbMsc)) {
 *     ... handle FAT/exFAT failure.
 *   }
 */
#ifndef USE_BLOCK_DEVICE_INTERFACE
#define USE_BLOCK_DEVICE_INTERFACE 0
#endif  // USE_BLOCK_DEVICE_INTERFACE
//------------------------------------------------------------------------------
/**
 * SD_CHIP_SELECT_MODE defines how the functions
 * void sdCsInit(SdCsPin_t pin) {pinMode(pin, OUTPUT);}
 * and
 * void sdCsWrite(SdCsPin_t pin, bool level) {digitalWrite(pin, level);}
 * are defined.
 *
 * 0 - Internal definition is a strong symbol and can't be replaced.
 *
 * 1 - Internal definition is a weak symbol and can be replaced.
 *
 * 2 - No internal definition and must be defined in the application.
 */
#ifndef SD_CHIP_SELECT_MODE
#define SD_CHIP_SELECT_MODE 0
#endif  // SD_CHIP_SELECT_MODE
/** Type for card chip select pin. */
typedef uint8_t SdCsPin_t;
//------------------------------------------------------------------------------
/**
 * Set USE_LONG_FILE_NAMES nonzero to use long file names (LFN) in FAT16/FAT32.
 * exFAT always uses long file names.
 *
 * Long File Name are limited to a maximum length of 255 characters.
 *
 * This implementation allows 7-bit characters in the range
 * 0X20 to 0X7E except the following characters are not allowed:
 *
 *  < (less than)
 *  > (greater than)
 *  : (colon)
 *  " (double quote)
 *  / (forward slash)
 *  \ (backslash)
 *  | (vertical bar or pipe)
 *  ? (question mark)
 *  * (asterisk)
 *
 */
#ifndef USE_LONG_FILE_NAMES
#define USE_LONG_FILE_NAMES 1
#endif  // USE_LONG_FILE_NAMES
/**
 * Set USE_UTF8_LONG_NAMES nonzero to use UTF-8 file names. Use of UTF-8 names
 * will require significantly more flash memory and a small amount of extra
 * RAM.
 *
 * UTF-8 filenames allow encoding of 1,112,064 code points in Unicode using
 * one to four one-byte (8-bit) code units.
 *
 * As of Version 13.0, the Unicode Standard defines 143,859 characters.
 *
 * getName() will return UTF-8 strings and printName() will write UTF-8 strings.
 */
#ifndef USE_UTF8_LONG_NAMES
#define USE_UTF8_LONG_NAMES 1
#endif  // USE_UTF8_LONG_NAMES

#if USE_UTF8_LONG_NAMES && !USE_LONG_FILE_NAMES
#error "USE_UTF8_LONG_NAMES requires USE_LONG_FILE_NAMES to be non-zero."
#endif  // USE_UTF8_LONG_NAMES && !USE_LONG_FILE_NAMES
//------------------------------------------------------------------------------
/**
*将MAINTAN_FREE_CLUSTER_COUNT设置为非零，以保持可用簇的计数更新。
这将在第一次调用之后提高freeClusterCount（）调用的速度。需要额外的闪光灯。
 */
#ifndef MAINTAIN_FREE_CLUSTER_COUNT
#define MAINTAIN_FREE_CLUSTER_COUNT 1
#endif  // MAINTAIN_FREE_CLUSTER_COUNT
//------------------------------------------------------------------------------
/**
 * Set the default file time stamp when a RTC callback is not used.
 * A valid date and time is required by the FAT/exFAT standard.
 *
 * The default below is YYYY-01-01 00:00:00 midnight where YYYY is
 * the compile year from the __DATE__ macro.  This is easy to recognize
 * as a placeholder for a correct date/time.
 *
 * The full compile date is:
 * FS_DATE(compileYear(), compileMonth(), compileDay())
 *
 * The full compile time is:
 * FS_TIME(compileHour(), compileMinute(), compileSecond())
 */
#define FS_DEFAULT_DATE FS_DATE(compileYear(), 1, 1)
/** 00:00:00 midnight */
#define FS_DEFAULT_TIME FS_TIME(0, 0, 0)
//------------------------------------------------------------------------------
/**
 *如果CHECK_FLASH_PROGRAMMING为零，则允许单扇区闪存编程和其他操作重叠，以提高写入性能。
 *
 *除非CHECK_FLASH_PROGRAMMING为非零，否则某些板卡将不会在低功率模式下休眠。
 */
#ifndef CHECK_FLASH_PROGRAMMING
#define CHECK_FLASH_PROGRAMMING 0
#endif  // CHECK_FLASH_PROGRAMMING
//------------------------------------------------------------------------------
/**
 *要为SPI启用SD卡CRC检查，请将USE_SD_CRC设置为非零。
 *
 *将USE_SD_CRC设置为1以使用较小的CRC-CCITT函数。此功能
 *对于AVR来说较慢，但对于ARM和其他处理器来说可能较快。
 *
 *将USE_SD_CRC设置为2以使用较大的表驱动的CRC-CCITT函数。这
 *AVR的功能更快，但ARM和其他处理器的功能可能较慢。
 */
#ifndef USE_SD_CRC
#define USE_SD_CRC 1
#endif  // USE_SD_CRC
//------------------------------------------------------------------------------
/** If the symbol USE_FCNTL_H is nonzero, open flags for access modes O_RDONLY,
 * O_WRONLY, O_RDWR and the open modifiers O_APPEND, O_CREAT, O_EXCL, O_SYNC
 * will be defined by including the system file fcntl.h.
 */
#ifndef USE_FCNTL_H
#if defined(__AVR__)
// AVR fcntl.h does not define open flags.
#define USE_FCNTL_H 0
#elif defined(PLATFORM_ID)
// Particle boards - use fcntl.h.
#define USE_FCNTL_H 1
#elif defined(__arm__)
// ARM gcc defines open flags.
#define USE_FCNTL_H 1
#elif defined(ESP32)
#define USE_FCNTL_H 1
#else  // defined(__AVR__)
#define USE_FCNTL_H 0
#endif  // defined(__AVR__)
#endif  // USE_FCNTL_H
//------------------------------------------------------------------------------
/**
 *将INCLUDE_SDIOS设置为非零值以将SDIOS.h包含在SdFat.h中。
 *sdios.h提供了C++风格的IO流。
 */
#ifndef INCLUDE_SDIOS
#define INCLUDE_SDIOS 0
#endif  // INCLUDE_SDIOS
//------------------------------------------------------------------------------
/**
*将FAT12_SUPPORT设置为非零值，以便在FAT12卷的情况下启用。
*FAT12没有经过很好的测试，需要额外的闪光。
 */
#ifndef FAT12_SUPPORT
#define FAT12_SUPPORT 0
#endif  // FAT12_SUPPORT
//------------------------------------------------------------------------------
/**
*将DESTRUCTOR_CLOSES_FILE设置为非零值可关闭其析构函数中的文件。
*
*导致在ARM中使用大量堆。
 */
#ifndef DESTRUCTOR_CLOSES_FILE
#define DESTRUCTOR_CLOSES_FILE 0
#endif  // DESTRUCTOR_CLOSES_FILE
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is nonzero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for SD I/O so it must write the current
 * data sector to the SD, read the directory sector from the SD, update the
 * directory entry, write the directory sector to the SD and read the data
 * sector back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#ifndef ENDL_CALLS_FLUSH
#define ENDL_CALLS_FLUSH 0
#endif  // ENDL_CALLS_FLUSH
//------------------------------------------------------------------------------
/**
 * Set USE_SIMPLE_LITTLE_ENDIAN nonzero for little endian processors
 * with no memory alignment restrictions.
 */
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ && \
    (defined(__AVR__) || defined(__ARM_FEATURE_UNALIGNED))
#define USE_SIMPLE_LITTLE_ENDIAN 1
#else  // __BYTE_ORDER_
#define USE_SIMPLE_LITTLE_ENDIAN 0
#endif  // __BYTE_ORDER_
//------------------------------------------------------------------------------
/**
*将USE_SEPARATE_FAT_CACHE设置为非零值，
以便为FAT16/FAT32表项使用第二个512字节的缓存。
这提高了非512字节倍数的大型写入的性能
 */
#ifdef __arm__
#define USE_SEPARATE_FAT_CACHE 1
#else  // __arm__
#define USE_SEPARATE_FAT_CACHE 1
#endif  // __arm__
//------------------------------------------------------------------------------
/**
*将USE_EXFAT_BITMAP_CACHE设置为非零值，
以便为EXFAT位图项使用第二个512字节的缓存。
这提高了非512字节倍数的大型写入的性能。
 */
#ifdef __arm__
#define USE_EXFAT_BITMAP_CACHE 1
#else  // __arm__
#define USE_EXFAT_BITMAP_CACHE 0
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_SECTOR_IO nonzero to use multi-sector SD read/write.
 *
 * Don't use mult-sector read/write on small AVR boards.
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_SECTOR_IO 0
#else  // RAMEND
#define USE_MULTI_SECTOR_IO 1
#endif  // RAMEND
//------------------------------------------------------------------------------
/** Enable SDIO driver if available. */
#if defined(__MK64FX512__) || defined(__MK66FX1M0__)
// Pseudo pin select for SDIO.
#ifndef BUILTIN_SDCARD
#define BUILTIN_SDCARD 254
#endif  // BUILTIN_SDCARD
// SPI for built-in card.
#ifndef SDCARD_SPI
#define SDCARD_SPI SPI1
#define SDCARD_MISO_PIN 59
#define SDCARD_MOSI_PIN 61
#define SDCARD_SCK_PIN 60
#define SDCARD_SS_PIN 62
#endif  // SDCARD_SPI
#define HAS_SDIO_CLASS 1
#endif  // defined(__MK64FX512__) || defined(__MK66FX1M0__)
#if defined(__IMXRT1062__)
#define HAS_SDIO_CLASS 1
#endif  // defined(__IMXRT1062__)
//------------------------------------------------------------------------------
/**
 * Determine the default SPI configuration.
 */
#if defined(ARDUINO_ARCH_APOLLO3) ||                                         \
    (defined(__AVR__) && defined(SPDR) && defined(SPSR) && defined(SPIF)) || \
    (defined(__AVR__) && defined(SPI0) && defined(SPI_RXCIF_bm)) ||          \
    defined(ESP8266) || defined(ESP32) || defined(PLATFORM_ID) ||            \
    defined(ARDUINO_SAM_DUE) || defined(STM32_CORE_VERSION) ||               \
    defined(__STM32F1__) || defined(__STM32F4__) ||                          \
    (defined(CORE_TEENSY) && defined(__arm__))
#define SD_HAS_CUSTOM_SPI 1
#else  // SD_HAS_CUSTOM_SPI
// Use standard SPI library.
#define SD_HAS_CUSTOM_SPI 0
#endif  // SD_HAS_CUSTOM_SPI
//------------------------------------------------------------------------------
#ifndef HAS_SDIO_CLASS
/** Default is no SDIO. */
#define HAS_SDIO_CLASS 0
#endif  // HAS_SDIO_CLASS

#endif  // SdFatConfig_h

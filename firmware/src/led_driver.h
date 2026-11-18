// WS2812 / SK6812 driver for the ESP32-C3 using the SPI peripheral + DMA.
//
// Why not a normal LED library?  The C3's RMT hardware can only hold 48 bits, but 4 RGB LEDs
// need 96. The library has to refill it from an interrupt in the MIDDLE of each frame, and a
// Wi-Fi task or flash write that delays that interrupt corrupts the frame = visible flicker.
// Here the whole frame is copied to RAM once and the DMA engine clocks it out on its own, so
// nothing the CPU is doing can disturb the timing.
//
// Idle level: on the C3 the SPI data line rests HIGH between transfers. WS2812/SK6812 LEDs need it
// LOW, otherwise every frame starts with a phantom "1" bit (that showed up as a green glow even
// when the lamp was off). Fix: the pin is inverted in the GPIO matrix and the data is inverted to
// match, so the pin rests LOW and the LEDs still receive exactly the right bits.
//
// Encoding: SPI at 2.4 MHz, every LED bit becomes 3 SPI bits (1 -> 110, 0 -> 100).
//   1 = 833 ns high / 417 ns low,  0 = 417 ns high / 833 ns low   (WS2812 wants ~800/400)
#pragma once
#include <Arduino.h>
#include "driver/spi_master.h"
#include "esp_heap_caps.h"
#include "esp_rom_gpio.h"
#include "soc/gpio_sig_map.h"

class SpiLeds {
 public:
  // bytesPerLed: 3 = GRB (WS2812), 4 = GRBW (SK6812 RGBW)
  bool begin(int pin, uint16_t count, uint8_t bytesPerLed) {
    _count = count;
    _bpl = bytesPerLed;
    _dataLen = (size_t)count * bytesPerLed;
    _encLen = _dataLen * 3;                 // 8 LED bits -> 24 SPI bits -> 3 bytes
    _total = _encLen + RESET_BYTES;
    _data = (uint8_t*)calloc(_dataLen, 1);
    _last = (uint8_t*)calloc(_dataLen, 1);
    _buf = (uint8_t*)heap_caps_calloc(_total, 1, MALLOC_CAP_DMA);
    if (!_data || !_last || !_buf) return false;

    spi_bus_config_t bus = {};
    bus.mosi_io_num = pin;
    bus.miso_io_num = -1;
    bus.sclk_io_num = -1;
    bus.quadwp_io_num = -1;
    bus.quadhd_io_num = -1;
    bus.max_transfer_sz = (int)_total;
    if (spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) return false;

    spi_device_interface_config_t dev = {};
    dev.clock_speed_hz = 2400000;
    dev.mode = 0;
    dev.spics_io_num = -1;
    dev.queue_size = 1;
    if (spi_bus_add_device(SPI2_HOST, &dev, &_dev) != ESP_OK) return false;
    esp_rom_gpio_connect_out_signal(pin, FSPID_OUT_IDX, true /* invert */, false);
    _ok = true;
    return true;
  }

  bool ok() const { return _ok; }
  void setRgbFirst(bool v) { _rgbFirst = v; }

  // r,g,b,w are 0..255. On 3-byte LEDs the white value is ignored.
  void set(uint16_t i, uint8_t r, uint8_t g, uint8_t b, uint8_t w = 0) {
    if (i >= _count) return;
    uint8_t* p = _data + (size_t)i * _bpl;
    if (_rgbFirst) { p[0] = r; p[1] = g; } else { p[0] = g; p[1] = r; }   // GRB(W) is the usual order
    p[2] = b;
    if (_bpl == 4) p[3] = w;
  }

  void clear() { memset(_data, 0, _dataLen); }

  // Sends the frame if it differs from the last one (or force == true). Returns true if sent.
  bool show(bool force = false) {
    if (!_ok) return false;
    if (!force && memcmp(_data, _last, _dataLen) == 0) return false;
    for (size_t n = 0; n < _dataLen; n++) {
      uint8_t v = _data[n];
      uint32_t w = 0;
      for (int b = 7; b >= 0; b--) w = (w << 3) | ((v >> b) & 1 ? 0b110 : 0b100);
      w = ~w;                                  // pin is inverted, so invert the data too
      _buf[n * 3 + 0] = w >> 16;
      _buf[n * 3 + 1] = w >> 8;
      _buf[n * 3 + 2] = w;
    }
    memset(_buf + _encLen, 0xFF, RESET_BYTES);  // 0xFF here = LOW on the pin: >80 us latch
    spi_transaction_t t = {};
    t.length = _total * 8;
    t.tx_buffer = _buf;
    esp_err_t e = spi_device_polling_transmit(_dev, &t);
    if (e != ESP_OK) { _errors++; return false; }
    memcpy(_last, _data, _dataLen);
    _sent++;
    return true;
  }

  uint32_t framesSent() const { return _sent; }
  uint32_t errors() const { return _errors; }

 private:
  static constexpr size_t RESET_BYTES = 24;  // 192 bits at 2.4 MHz = 80 us
  uint16_t _count = 0;
  uint8_t _bpl = 3;
  size_t _dataLen = 0, _encLen = 0, _total = 0;
  uint8_t *_data = nullptr, *_last = nullptr, *_buf = nullptr;
  spi_device_handle_t _dev = nullptr;
  bool _ok = false;
  bool _rgbFirst = false;
  uint32_t _sent = 0, _errors = 0;
};

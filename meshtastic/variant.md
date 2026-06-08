# `variants/esp32c3/diy/meshboy/platformio.ini`

```ini
[env:meshboy]
extends = esp32c3_base
board = esp32-c3-devkitm-1
build_flags =
  ${esp32c3_base.build_flags}
  -D PRIVATE_HW
  -I variants/esp32c3/diy/meshboy
  -D ARDUINO_USB_MODE=1
  -D ARDUINO_USB_CDC_ON_BOOT=1
board_level = extra
```

# `variants/esp32c3/diy/meshboy/variant.h`

```cpp
// thanks to https://github.com/f4goh/CheapMesh/blob/main/firmware/variant.h !!

#pragma once

#define BUTTON_PIN 9

#define USE_SX1262
#define LORA_SCK 10
#define LORA_MISO 6
#define LORA_MOSI 7
#define LORA_CS 8
#define LORA_DIO0 RADIOLIB_NC
#define LORA_RESET 5
#define LORA_DIO1 3
#define LORA_DIO2 RADIOLIB_NC
#define LORA_BUSY 4
#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RESET LORA_RESET
#define SX126X_DIO2_AS_RF_SWITCH
#define SX126X_DIO3_TCXO_VOLTAGE 1.8
```
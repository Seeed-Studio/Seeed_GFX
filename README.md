# Seeed Arduino LCD

A professional graphics library for Seeed hardware platforms, forked from [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) with significant enhancements to support Seeed's development boards and display modules.

## Key Features

- **Enhanced Processor Support**: Expanded compatibility with Seeed's diverse microcontroller lineup
- **E-Paper Display Integration**: Full support for E-Paper displays with SSD1680, SSD1681, and JD79686B drivers
- **Platform-Specific Optimizations**: Tailored implementations for superior performance on Seeed hardware
- **Unified API**: Consistent programming interface across different display technologies
- **Power Management**: Advanced sleep/wake functionality for battery-powered applications

## Supported Platforms

### Core Hardware Support
- Wio Terminal
- XIAO SAMD21
- XIAO RP2040/RP2350
- XIAO ESP32C3/ESP32S3/ESP32C6
- XIAO NRF52840
- XIAO MG24
- XIAO RA4M1

### Display Technologies
- Standard TFT LCD displays (inherited from TFT_eSPI)
- E-Paper displays (new implementation)
  - SSD1680-based displays
  - SSD1681-based displays
  - JD79686B-based displays

## E-Paper Implementation

The library includes a comprehensive implementation for E-Paper displays, providing:

- **EPaper Class**: Specialized class extending TFT_eSprite for E-Paper functionality
- **Dual-Buffer Rendering**: Efficient update mechanism using sprite-based buffering
- **Power Management**: Automatic sleep/wake cycles to maximize battery life
- **Partial Updates**: Support for region-specific refresh to improve update speed
- **Display Orientation**: Flexible orientation settings via configurable mirroring

```cpp
// Example usage
EPaper epaper;
epaper.begin();
epaper.fillScreen(TFT_WHITE);
epaper.drawString("Hello E-Paper", 10, 10, 2);
epaper.update();  // Explicit update required for E-Paper displays
```

## Technical Enhancements

### Processor-Specific Optimizations
- Added specialized implementations for SAMD21, RA4M1, nRF52840, and MG24 processors
- Expanded ESP32 variant support including C3, C6, and S3 models
- Register-level optimizations for maximum performance on each platform

### File System Integration
- Seamless integration with Seeed_Arduino_FS for compatible platforms
- Fallback to standard FS library for other platforms

### Low-Level Display Interface
- Platform-specific implementations of SPI and parallel interfaces
- DMA acceleration where supported by hardware
- Custom pin configurations for Seeed development boards

## Example Applications

The library includes numerous examples demonstrating:
- Basic drawing and text rendering
- Advanced sprite manipulation
- E-Paper-specific applications
- Hardware-accelerated rendering techniques

## Installation

1. Download this repository
2. Install it into your Arduino libraries folder
3. Select the appropriate setup configuration for your hardware


## Online Configuration Tool

To simplify the hardware setup, we provide an online tool that helps you quickly generate the configuration code for your specific hardware combination.

➡️ **[Click here to open the Online Configuration Tool](https://qingwind6.github.io/Display_online_config_tool/)**

### How to Use
1.  **Open the Configuration Tool**: Click the link above to open the online tool in your browser.
2.  **Select Your Hardware**: Choose your board and display combination from the dropdown menus. If an ePaper screen is selected, choose your driver board as well.
3.  **Generate and Copy Code**: Copy the entire configuration code generated in the text box.
4.  **Create a Custom Config File**: In your Arduino **sketch folder** (the same folder where your `.ino` file is located), create a new file named `Custom_config.h`.
5.  **Paste the Code**: Paste the code you copied in step 3 into the `Custom_config.h` file and save it.
6.  **Compile and Upload**: You can now compile and upload your sketch. The library will automatically detect and apply your custom configuration.


## Documentation

For detailed implementation information:
- Refer to original TFT_eSPI documentation for core functionality
- See examples directory for platform-specific usage guidance
- Explore the Extensions directory for advanced features

## Licensing

This library is released under the same license as the original TFT_eSPI library.

## Acknowledgments

Based on the excellent work of Bodmer's TFT_eSPI library, with significant enhancements by the Seeed Studio development team to support Seeed's hardware ecosystem.

For more information, please refer [here](./OREADME.md)
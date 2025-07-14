# Soil Moisture Sensor Firmware for ESP32 Wrover

This repository provides firmware for a soil moisture sensor powered by the ESP32 Wrover microcontroller. The firmware enables automated soil moisture readings, user-configurable scan intervals, and visual feedback via an onboard display.

## Features

- **Configurable scan interval:** Users can select how often the sensor scans for soil moisture (in hours) using a physical button.
- **Display integration:** Scan intervals and sensor readings are shown on a connected display.
- **Button controls:** Short or long presses on the button allow interval adjustments and trigger scans.
- **Power management:** The firmware uses deep sleep to conserve power, waking up periodically or on button press for scans.
- **Sensor feedback:** Soil moisture values are read and evaluated, and the current status is displayed.

## How It Works

1. **Interval Selection:**  
   On startup, users are prompted to choose a scan interval (e.g., every 1–24 hours) using short button presses. A long press sets the interval.

2. **Scan Execution:**  
   - After interval selection, the device scans for soil moisture at the configured interval.
   - Scans can also be triggered by pressing the button.
   - Results are shown directly on the display.

3. **Sleep/Wake Logic:**  
   - The device enters deep sleep between scans to save energy.
   - Wakes up either by timer or button press.

## Getting Started

### Hardware Requirements

- ESP32 Wrover board
- Soil moisture sensor compatible with ESP32 ADC
- Compatible display (e.g., OLED, supported by U8g library)
- Push button for user input

### Setup Instructions

1. **Clone this repository:**
   ```bash
   git clone https://github.com/rup2014/Soil-Moisture-Sensor-ESP32Wrover-Firmware.git
   ```

2. **Open the project:**  
   Use [Arduino IDE](https://www.arduino.cc/en/software) or PlatformIO.

3. **Install dependencies:**  
   - U8g display library
   - ESP32 board support for Arduino

4. **Connect hardware:**  
   - Wire up the soil moisture sensor to a touch pin.
   - Connect the display to the ESP32 via I2C/SPI (as per your hardware).
   - Attach the button to a digital input.

5. **Configure pins:**  
   Adjust pin assignments in `SoilMoistureReaderESP32Wrover.ino` if needed.

6. **Upload the firmware:**  
   Flash the code onto your ESP32 Wrover board.

### Usage

- On boot, select the scan interval using the button (short press to increment, long press to set).
- After interval is set, press the button to start a scan or wait for the timer.
- View soil moisture readings on the display.
- Device sleeps automatically to conserve power.

## File Structure

- `SoilMoistureReaderESP32Wrover/SoilMoistureReaderESP32Wrover.ino`: Main firmware logic.
- `Consts.h`: Constants and display bitmaps.
- Other support files as needed.

## License

*No license file detected. Please add a LICENSE file to clarify usage permissions.*

## Author

- [rup2014](https://github.com/rup2014)

---

If you have any questions or need help, please open an issue in this repository.

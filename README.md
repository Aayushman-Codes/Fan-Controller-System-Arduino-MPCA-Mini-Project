# Automatic Fan Control System
## MPCA Mini Project — 4th Semester, Section A
### A temperature and light intensity based fan controllor and regulator built using Arduino Uno R3 for MPCA Course Mini Project

**Description:**An intelligent, climate-responsive fan controller built on an **Arduino Uno R3** that dynamically regulates a DC fan's speed based on real-time ambient temperature and environmental lighting — using PWM for smooth, proportional control instead of simple ON/OFF switching.

**Team Members:**
| Sl. No | Name | SRN |
|--------|------|-----|
| 1 | Aayushman Singh | PES1UG24AM006 |
| 2 | Abhay TS | PES1UG24AM008 |
| 3 | Abhishek PH | PES1UG24AM013 |

---

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Components](#components)
- [Circuit Diagram](#circuit-diagram)
- [Pin Configuration](#pin-configuration)
- [Circuit Description](#circuit-description)
- [How It Works](#how-it-works)
- [Control Logic](#control-logic)
- [Thresholds & Calibration](#thresholds--calibration)
- [Serial Monitor Output](#serial-monitor-output)
- [Getting Started](#getting-started)
- [Pseudocode](#pseudocode)
- [Use Cases](#use-cases)
- [Project Structure](#project-structure)

---

## Overview

The **Automatic Fan Control System** is an embedded prototype that integrates multiple sensors to intelligently manage a DC cooling fan. Rather than a binary ON/OFF approach, it uses **Pulse Width Modulation (PWM)** to scale fan speed proportionally with temperature, saving energy while maintaining effective cooling.

A key design highlight is **multi-sensor redundancy** — three independent temperature sensors (LM35, DHT11, DHT22) are all read simultaneously. The system uses the maximum valid temperature reading across all three to drive control decisions, making it robust against individual sensor failures.

Additionally, a **dual-trigger OR logic** system means the fan can be activated either by high temperature *or* by a manual LDR (light sensor) override, providing flexible control modes.

---

## Features

- **Proportional PWM Fan Control** — Fan speed scales smoothly with temperature (not just ON/OFF)
- **Triple Sensor Redundancy** — LM35 + DHT11 + DHT22 all polled; max valid reading is used
- **Fault Tolerance** — Invalid/NaN sensor readings automatically fall back to the LM35 analog reading
- **Dual-Trigger OR Logic** — Fan activates if temperature is high *OR* the LDR detects a "dark/cupped" state
- **LDR Manual Override** — Covering the LDR triggers the fan at a high baseline speed regardless of temperature
- **Hardware Protection** — TIP122 Darlington transistor for current amplification; 1N4007 flyback diode prevents back-EMF damage
- **Status LED** — Visual indicator lights up whenever the fan system is active
- **Serial Monitor Logging** — All sensor values and fan state are printed every second for real-time monitoring
- **Averaged LM35 Reading** — LM35 sampled 15 times with a rolling average to reduce noise

---

## Components

| Category | Component | Quantity | Notes |
|---|---|---|---|
| Microcontroller | Arduino Uno R3 | 1 | Main controller |
| Temperature Sensor | LM35 | 1 | Analog sensor, pin A0 |
| Temperature & Humidity | DHT11 | 1 | Digital, pin D2 |
| Temperature & Humidity | DHT22 | 1 | Digital, pin D3; higher precision |
| Light Sensor | LDR Sensor Module | 1 | Analog, pin A1 |
| Actuator | 5V DC Fan | 1 | PWM-controlled via transistor |
| Driver | TIP122 Darlington Transistor | 1 | Current amplifier for fan |
| Protection | 1N4007 Diode | 1 | Flyback diode across fan terminals |
| Status Indicator | LED | 1 | Pin D5, lights when system active |
| Passive | 10 kΩ Resistor | 1 | Base resistor for TIP122 |
| Passive | 220 Ω Resistor | 1 | Current-limiting resistor for LED |
| Prototyping | Breadboard | 1 | |
| Wiring | Male-to-Male Jumper Wires | Several | |
| Wiring | Male-to-Female Jumper Wires | Several | |
| Power | USB Power Supply (5V) | 1 | Via Arduino USB port |

---

## Circuit Diagram
A fully handmade diagram:
![circuit diagram1](./Circuit%20Diagram/image.png)

Another circuit diagram made using Tinkercad online application:
![circuit diagram2](./Circuit%20Diagram/image-1.png)

---

## Pin Configuration

| Arduino Pin | Connected To | Mode |
|---|---|---|
| `A0` | LM35 (analog output) | INPUT (Analog) |
| `A1` | LDR Sensor Module | INPUT (Analog) |
| `D2` | DHT11 data pin | INPUT (Digital) |
| `D3` | DHT22 data pin | INPUT (Digital) |
| `D5` | Status LED (via 220Ω resistor) | OUTPUT (Digital) |
| `D9` | Fan PWM (via TIP122 base & 10kΩ) | OUTPUT (PWM) |
| `5V` | DHT11 VCC, DHT22 VCC, LDR VCC | Power |
| `GND` | All component grounds | Ground |

> **Note:** Pin D9 is used for the fan PWM because it is one of Arduino Uno's hardware PWM-capable pins (Timer 1), ensuring stable and efficient PWM output.

---

## Circuit Description

### Power & Current Management
The Arduino's GPIO pins can only source ~40mA safely — far too little to drive a DC motor. A **TIP122 Darlington transistor** is used as an electronic switch:
- Arduino D9 → 10kΩ resistor → TIP122 Base (controls the switch)
- TIP122 Collector → Fan negative terminal
- Fan positive terminal → 5V supply
- TIP122 Emitter → GND

The `analogWrite()` on D9 generates a PWM duty cycle, which controls how fast the transistor switches, effectively varying the average current delivered to the fan.

### Back-EMF Protection
DC motors generate a reverse voltage spike (back-EMF) when they suddenly stop or slow down. The **1N4007 flyback diode** is placed in parallel with the fan (cathode toward 5V, anode toward collector) to safely absorb and dissipate these spikes, protecting the Arduino and transistor from damage.

### Sensors
- **LM35:** Analog voltage output (10mV/°C), read on A0. Averaged over 15 samples to reduce noise. Formula: `temp = (avg_reading × 5.0 × 100) / 1024` with a `-7°C` calibration offset.
- **DHT11 & DHT22:** One-wire digital protocol, read via the `DHT` library. DHT11 has a `-1°C` calibration offset applied.
- **LDR Module:** Returns a higher analog value when the sensor is covered/dark (due to onboard LEDs reflecting). This inverted behaviour is leveraged as a manual override trigger.

---

## How It Works

The main loop runs continuously with a 1-second delay between cycles:

1. **LM35 Temperature Acquisition** — Reads the analog pin 15 times, computes an average, converts to Celsius, applies a `-7°C` calibration offset.
2. **DHT11 & DHT22 Acquisition** — Reads both digital sensors. DHT11 gets a `-1°C` calibration offset.
3. **Sensor Validation** — Any reading that is NaN, below 0°C, or above 100°C is replaced by the LM35 reading as a fallback.
4. **Maximum Temperature** — `maxTemp = max(LM35, max(DHT11, DHT22))`
5. **LDR Check** — Reads the LDR. If the raw analog value exceeds 700, `isDark = true` (sensor is cupped/covered).
6. **Fan Control Decision (OR Logic)** — If `maxTemp >= 33.75°C` OR `isDark == true`, the fan activates. Otherwise it turns off.
7. **PWM Calculation** — If temperature-driven: fan speed is mapped proportionally from `[33.75°C → 45.0°C]` to `[PWM 150 → PWM 255]`. If LDR-override triggered only: fixed PWM of 200.
8. **Hardware Output** — PWM written to fan pin; LED updated; all readings printed to Serial Monitor.

---

## Control Logic

```
IF (maxTemp >= 33.75°C) OR (LDR is cupped/dark):
    IF maxTemp >= 33.75°C:
        fanPWM = map(maxTemp, 33.75, 45.0, 150, 255)
    ELSE (LDR override only):
        fanPWM = 200
    fanPWM = constrain(fanPWM, 150, 255)
    LED = ON
ELSE:
    fanPWM = 0
    LED = OFF
```

The fan PWM range of **150–255** (out of 0–255) corresponds to roughly **59%–100% duty cycle**, ensuring the fan always has enough voltage to start and spin properly when activated.

---

## Thresholds & Calibration

| Parameter | Value | Description |
|---|---|---|
| `TEMP_MIN` | `33.75°C` | Temperature at which fan starts (minimum fan speed: PWM 150) |
| `TEMP_MAX` | `45.0°C` | Temperature at which fan reaches maximum speed (PWM 255) |
| `LDR_BRIGHT_THRESHOLD` | `700` | LDR raw ADC value above which the "dark/cupped" trigger fires |
| LM35 offset | `-7°C` | Hardware calibration correction for LM35 |
| DHT11 offset | `-1°C` | Hardware calibration correction for DHT11 |
| LM35 samples | `15` | Number of averaged ADC readings per cycle |
| Loop delay | `1000 ms` | Interval between sensor read cycles |

---

## Serial Monitor Output

Open the Arduino IDE Serial Monitor at **9600 baud** to see live output like:

```
--- System Booted: Detecting Sensor Readings ---
LM35: 34.21 | DHT11: 33.95 | DHT22: 34.10 | MAX: 34.21C | LDR: 820 [LIGHT|FAN OFF] | Fan PWM: 0
LM35: 35.60 | DHT11: 35.20 | DHT22: 35.45 | MAX: 35.60C | LDR: 822 [LIGHT|FAN OFF] | Fan PWM: 163
LM35: 36.80 | DHT11: 36.50 | DHT22: 36.70 | MAX: 36.80C | LDR: 85  [DARK|FAN ON]  | Fan PWM: 200
```

> **Note:** The displayed `LDR` value is inverted (`1023 - raw_reading`) for logical readability — higher displayed values mean brighter/more light detected. The LDR had a design defect which maked it show higher light in a dark room than in a bright room due to its sensing LEDs being placed wrongly too close to the LDR sensor within the module. Identification and bypassing of this design defect to show correct result within out project showcases our Engineering as well as Intuitive skills.

---

## Getting Started

### Prerequisites
- [Arduino IDE](https://www.arduino.cc/en/software) (version 1.8.x or 2.x)
- **DHT sensor library** by Adafruit — install via Library Manager (`Sketch > Include Library > Manage Libraries > search "DHT sensor library"`)

### Steps

1. **Clone or download this repository:**
   ```bash
   git clone https://github.com/Aayushman-Codes/Fan-Controller-System-Arduino-MPCA-Mini-Project
   ```

2. **Wire the circuit** according to the [Pin Configuration](#pin-configuration) and [Circuit Description](#circuit-description) sections above.

3. **Open the sketch:**
   Navigate to `fan_controller_code/fan_controller.ino` and open it in the Arduino IDE.

4. **Install the DHT library** if not already installed (see Prerequisites).

5. **Select Board and Port:**
   - `Tools > Board > Arduino AVR Boards > Arduino Uno`
   - `Tools > Port > (your Arduino's COM/tty port)`

6. **Upload the sketch** by clicking the Upload button (→) or pressing `Ctrl+U`.

7. **Open Serial Monitor** (`Tools > Serial Monitor` or `Ctrl+Shift+M`), set baud rate to **9600**, and observe live sensor readings.

---

## Pseudocode

```
CONSTANTS:
    T_min ← 33.75
    T_max ← 45.0
    LDR_threshold ← 700

SETUP:
    Initialize Serial Monitor (9600 baud)
    Set analog reference to DEFAULT (5V)
    Initialize DHT11, DHT22
    Set pin modes (LM35, LDR as INPUT; LED, FAN as OUTPUT)
    Set Fan PWM ← 0, LED ← OFF

LOOP (every 1 second):
    // Step 1: LM35 Averaged Reading
    total ← 0
    FOR i = 1 TO 15:
        total ← total + analogRead(A0)
        wait 2ms
    avg ← total / 15.0
    temp_lm35 ← (avg × 5.0 × 100.0) / 1024.0 - 7

    // Step 2: DHT Readings
    temp_dht11 ← DHT11.readTemperature() - 1
    temp_dht22 ← DHT22.readTemperature()

    // Step 3: Fault Tolerance
    IF temp_dht11 is NaN OR < 0 OR > 100 → temp_dht11 ← temp_lm35
    IF temp_dht22 is NaN OR < 0 OR > 100 → temp_dht22 ← temp_lm35

    // Step 4: Max Temperature
    max_temp ← max(temp_lm35, temp_dht11, temp_dht22)

    // Step 5: LDR Override Check
    light_raw ← analogRead(A1)
    is_dark ← (light_raw > LDR_threshold)

    // Step 6: Control Decision
    IF (max_temp >= T_min) OR (is_dark):
        IF max_temp >= T_min:
            pwm ← map(max_temp, T_min, T_max, 150, 255)
        ELSE:
            pwm ← 200
        pwm ← constrain(pwm, 150, 255)
        LED ← ON
    ELSE:
        pwm ← 0
        LED ← OFF

    // Step 7: Output
    analogWrite(FAN_PIN, pwm)
    Print all sensor data to Serial Monitor
    wait 1000ms
```

---

## Use Cases

- **Laptop & PC Cooling:** Modern laptops run dark internally during thermal throttling. The LDR override simulates how a darkness-based sensor could trigger cooling fans when components overheat.
- **Smart Room Cooling:** Deploy in a room where you want automatic fan speed scaling with temperature, without manual intervention.
- **Rural Smart Climate Systems:** A low-cost embedded alternative for temperature-responsive ventilation in agricultural or residential settings.
- **Embedded Systems Learning:** Demonstrates multi-sensor fusion, PWM motor control, transistor-based switching, hardware protection design, and fault-tolerant sensor reading — all on a single Arduino board.

---

## Project Structure

```
Fan-Controller-System-Arduino-MPCA-Mini-Project/
│
├── fan_controller_code/
│   └── fan_controller.ino      # Main Arduino sketch
│
├── Fan_Controller_System.mp4   # Demo video
├── README.md                   # This file
└── .gitattributes
```

---

## Library Dependencies

| Library | Version | Install via |
|---|---|---|
| `DHT sensor library` | Latest | Arduino Library Manager (by Adafruit) |
| `Adafruit Unified Sensor` | Latest | Arduino Library Manager (dependency of DHT lib) |

---


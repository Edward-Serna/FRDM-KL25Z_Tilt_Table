# Tilt Table: PID Ball Balancing System

A closed-loop control system that balances a metal ball at the center of a X/Y axis tilt table using a resistive touch panel for position sensing and two servo motors for actuation. Built on the **FRDM-KL25Z** (NXP Kinetis KL25Z) development board.

---
## Demo
<div align="center">
    <video src="resource/demo1.mp4" width="130" height="300" controls></video>
    <video src="resource/demo2.mp4" width="130" height="300" controls></video>
</div>


## Table of Contents

- [Overview](#overview)
- [Hardware](#hardware)
- [Software Architecture](#software-architecture)
- [PID Controller](#pid-controller)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Configuration & Tuning](#configuration--tuning)
- [Serial Output](#serial-output)
- [Known Limitations](#known-limitations)

---

## Overview

The tilt table uses a resistive touch panel as a 2D position sensor. The X and Y coordinates of the ball are read via the ADC (two-channel, interrupt-driven), passed into independent PID controllers for each axis, and the resulting correction signals drive two servos via TPM (Timer/PWM Module) to tilt the platform back toward center.

---

## Hardware

| Component | Details |
|---|---|
| Microcontroller | NXP FRDM-KL25Z (Cortex-M0+, 48 MHz) |
| Position Sensor | Resistive touch panel (X/Y via ADC channels) |
| Actuators | 2× hobby servos (X-axis and Y-axis) |
| Shield | Tilt Table Shield (provides servo power and touch panel breakout) |
| Power | USB (MCU) + 6–6.5 V battery pack (servos via onboard regulator) |
| IDE | MCUXpresso / Kinetis Design Studio |

> **Servo power warning:** The servos require 6–6.5 V from the battery. Do **not** exceed this; the onboard regulator on the shield is adjustable. The touch panel can be read without battery power, but the servos will not move.

### Pin Assignments

| Signal | Port/Pin | Connector |
|---|---|---|
| UART TX (debug) | PORTA2 | J1[4] / D1 |
| UART RX (debug) | PORTA1 | J1[2] / D0 |
| Servo Left / X | PORTB8 | J9[1] |
| Servo Right / Y | PORTB9 | J9[3] |

> Note: Servo mounting orientation varies between physical tables. If the table tilts the wrong direction, negate the servo command for that axis in the control loop.

---

## Software Architecture

```
Touch Panel (ADC interrupt)
        │
        ▼
  adc16_interrupt.c       ← ADC16 two-channel interrupt-driven sampling
        │
        ▼
   X/Y position (float)
        │
        ├──────────────────────────────────────┐
        ▼                                      ▼
  PID (X-axis)                          PID (Y-axis)
  pid.c / pid.h                         pid.c / pid.h
        │                                      │
        ▼                                      ▼
  TFC_SetServo(0, cmd_x)           TFC_SetServo(1, cmd_y)
        │                                      │
        └──────────────┬───────────────────────┘
                       ▼
              Servo PWM via TPM
           (fsl_tpm.c / fsl_tpm.h)
```

The main control loop runs at approximately 50 Hz (every 20 ms via `TFC_Ticker`). The ADC fires an interrupt each time a new sample is ready, updating the latest X/Y readings. The main loop consumes those readings, runs them through both PID controllers, and writes the output to the two servo channels.

---

## PID Controller

The PID implementation lives in `source/pid.c` and `source/pid.h`. Each axis gets its own independent `PID` instance.

### API

```c
PID* InitPID();                                          // allocate and zero a PID struct
void SetPIDGain(PID*, float Kp, float Ki, float Kd);     // set gains
void SetPIDSetpoint(PID*, float setpoint);               // set target (center = 0.0)
void SetPIDLimits(PID*, float lower, float upper);       // clamp output to servo range [-1, 1]
void ResetPID(PID*);                                     // reset integrator and startup flag
float GetPIDOutput(PID*, float input, float dTime);      // run one PID iteration
```

### Implementation Notes

- **Integral anti-windup**: the accumulated integral term is clamped to `[lowerLimit, upperLimit]` each iteration. If `Ki == 0`, the integral is zeroed.
- **Derivative on measurement**: the derivative term is computed from the change in *input* (not error), which avoids derivative kick on setpoint changes.
- **Startup bumpless transfer**: on the first call after init or reset, the controller returns the setpoint directly and seeds `lastInput`, preventing a large initial derivative spike.

---

## Project Structure

```
.
├── source/
│   ├── adc16_interrupt.c   # Main program, ADC setup, control loop
│   ├── pid.c               # PID controller implementation
│   └── pid.h               # PID struct and API declarations
├── board/
│   ├── board.c / board.h   # FRDM-KL25Z board init (clock, UART, LEDs)
│   ├── clock_config.c/h    # PLL/clock tree configuration
│   └── pin_mux.c/h         # MCUXpresso-generated pin multiplexing
├── drivers/
│   ├── fsl_adc16.c/h       # ADC16 peripheral driver
│   ├── fsl_tpm.c/h         # TPM (servo PWM) driver
│   ├── fsl_gpio.c/h        # GPIO driver
│   └── ...                 # Other NXP SDK peripheral drivers
├── CMSIS/
│   ├── MKL25Z4.h           # Device header (register definitions)
│   ├── arm_math.h          # CMSIS-DSP math library
│   └── ...
├── utilities/
│   └── fsl_debug_console.c/h  # Printf over UART
├── startup/
│   └── startup_mkl25z4.c   # Interrupt vector table, reset handler
├── resource/
│   ├── Tilt_Table_Calibration.xlsx  # Touch panel calibration data
│   ├── Matlab_Output.png            # MATLAB simulation/plot output
│   └── ...                          # Lab docs and reference slides
└── doc/
    └── readme.txt          # Original project shell description
```

---

## Getting Started

### Prerequisites

- **MCUXpresso IDE** (recommended) or Kinetis Design Studio 3.2+
- **NXP FRDM-KL25Z** board with Tilt Table Shield
- A serial terminal (e.g., PuTTY, Tera Term, or the MCUXpresso console)

### Build & Flash

1. Import the project into MCUXpresso: **File → Import → Existing Projects into Workspace**, select this directory.
2. Build with the `PE Debug` configuration (or `Run1` for the tuned run configuration).
3. Connect the FRDM-KL25Z via USB to the OpenSDA port.
4. Connect the 6–6.5 V battery to the Tilt Shield.
5. Click **Debug** (or **Run**) to flash and start execution.

### Serial Monitor

Open a terminal with these settings to observe live position data:

| Setting | Value |
|---|---|
| Baud rate | 115200 |
| Data bits | 8 |
| Parity | None |
| Stop bits | 1 |
| Flow control | None |

---

## Configuration & Tuning

### Touch Panel Calibration

Raw ADC values from the touch panel must be mapped to physical coordinates before being fed to the PID. Calibration data is in `resource/Tilt_Table_Calibration.xlsx`. Update the correction functions in `adc16_interrupt.c` with your table's specific min/max ADC readings for each axis.

### PID Gains

Gains are set in `adc16_interrupt.c` when initializing each axis PID. Start with the values below and tune from there:

```c
SetPIDGain(pidX, 0.30f, 0.01f, 0.05f);   // Kp, Ki, Kd — X axis
SetPIDGain(pidY, 0.30f, 0.01f, 0.05f);   // Kp, Ki, Kd — Y axis
SetPIDSetpoint(pidX, 0.0f);              // center
SetPIDSetpoint(pidY, 0.0f);
SetPIDLimits(pidX, -1.0f, 1.0f);         // servo range
SetPIDLimits(pidY, -1.0f, 1.0f);
```

**Tuning tips:**
- Increase **Kp** until the ball oscillates around center, then back off slightly.
- Add a small **Kd** to dampen oscillation — this is usually the most impactful term for a physical pendulum-like system.
- **Ki** should be small; too much integral will cause slow, growing oscillations.

### Servo Direction

Each physical tilt table may have its servos mounted in the opposite orientation. If a given axis tilts the wrong way, negate the output before calling `TFC_SetServo`:

```c
TFC_SetServo(0, -cmd_x);   // flip X servo direction if needed
```

---

## Serial Output

When running, the program prints raw X/Y ADC readings to the UART console each control tick, e.g.:

```
X: 2048  Y: 1975
X: 2051  Y: 1980
```

This is useful for verifying touch panel readings and checking calibration before closing the control loop.

---

## Known Limitations

- **Per-table calibration required**: touch panel ADC ranges vary between units. The calibration worksheet must be re-run for each individual table.
- **Servo direction**: must be verified and potentially reversed for each physical table.
- **No integral windup protection on startup**: ensure `ResetPID()` is called before enabling the control loop if the ball starts far from center.
- **Fixed control rate**: the 20 ms ticker interval is hardcoded; adjusting it requires retuning the PID gains.
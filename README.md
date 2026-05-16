# Shortwave SDR Receiver

A direct conversion Software Defined Radio (SDR) receiver for the shortwave HF band (2.3 to 30 MHz), implemented on a Renesas RX231 microcontroller (R5F52318ADFP). The receiver digitises baseband IQ signals and performs all demodulation, filtering, and signal processing in firmware.

## Hardware Overview

The receiver uses a direct conversion architecture - an Si5351 clock generator produces a local oscillator (LO) signal which is fed to a quadrature mixer, producing baseband I and Q signals centred at 0 Hz. The baseband IQ signals are amplified and filtered at 25kHz before being digitised by the RX231's 12-bit ADC at 48kHz.

<img width="1398" height="609" alt="block-diagram" src="https://github.com/user-attachments/assets/b2753c9a-0f9f-4daa-b83e-3e3cc327ff82" />

Key hardware connections:

| Function | Signal | Port / Pin | Interrupt |
|---|---|---|---|
| I channel ADC input | AN000 | P40 / 95 | - |
| Q channel ADC input | AN001 | P41 / 93 | - |
| AGC control voltage output | DA0 | P03 / 2 | - |
| Audio output | DA1 | P05 / 100 | - |
| I2C SDA (display + VFO) | RIIC0 SDA | P13 / 33 | - |
| I2C SCL (display + VFO) | RIIC0 SCL | P12 / 34 | - |
| Rotary encoder CLK | IRQ1 | P31 / 19 | IRQ1 |
| Rotary encoder DATA | IRQ2 | P32 / 18 | IRQ2 |
| Rotary encoder switch | - | P33 / 17 | - |

The VFO and OLED display are connected via a shared I2C bus. A variable gain amplifier on the baseband input is controlled by DA0 to implement hardware AGC. Audio is output on DA1, indirectly at a sample rate of 48kHz.

## Firmware Architecture

All signal processing runs on the RX231 at 48MHz. The ADC is triggered at 48kHz by MTU0 and each sample is processed in the S12AD0 ISR.

### Signal Processing Chain (per sample, in ISR)

1. Dynamic DC bias removal (dual IIR trackers - slow for audio, fast for FFT)
2. Fixed digital gain
3. Demodulation - AM, USB, LSB, CW, FM
4. IIR audio filter - narrow, normal, wide (exact filter widths different for each mode)
5. AGC peak detection - DA0 updated at ~10 Hz
6. Audio output written directly to DA1
7. FFT sample collection into IQ buffers

### Demodulation Modes

| Mode | Method | VFO Offset |
|---|---|---|
| AM | Magnitude approximation + lowpass filter | None |
| USB | Phasing method (I+Q) + bandpass filter | -1500 Hz |
| LSB | Phasing method (I-Q) + bandpass filter | +1500 Hz |
| CW | USB method + narrow bandpass filter | -700 Hz |
| FM | Phase discriminator + lowpass filter | None |

### FFT Waterfall Display

A 128 point complex FFT (`fix_fft` library) is run in the main loop on each filled IQ buffer. The spectrum is displayed on the OLED as a bandscope covering +/-24kHz around the LO frequency. Bin 0 (DC) and bin 64 (Nyquist) are zeroed to prevent noise/artefacts dominating the waterfall spectrum.

### User Interface

A rotary encoder with push switch allows the user to change the receive frequency, mode, filter width. The OLED display (SSD1309, 128x64) also shows the FFT waterfall spectrum with the selected filter width indicated to scale across it.

## Repository Structure
```shortwave-sdr-receiver/
├── src/                        Source files
│   ├── display/                OLED display module (u8g2)
│   ├── dsp/                    DSP - demodulation, filtering, AGC
│   ├── fft/                    FFT waterfall processing
│   ├── globals/                Global object definitions
│   ├── rotary_encoder/         Rotary encoder FSM driver
│   ├── vfo/                    VFO module (Si5351)
│   ├── u8g2/                   RX231 HAL for u8g2 library
│   └── main.cpp                Main entry point and control loop
├── libs/                       Third party libraries
│   ├── si5351/                 Si5351 clock generator library
│   ├── u8g2/                   u8g2 display library
│   └── fix_fft/                Fixed-point FFT library
├── smc_gen/                    Renesas Smart Configurator generated code
│   ├── Config_DA/              DAC configuration (DA0 AGC, DA1 audio)
│   ├── Config_ICU/             Interrupt controller (rotary encoder)
│   ├── Config_MTU0/            Timer (48kHz ADC trigger)
│   ├── Config_PORT/            GPIO configuration
│   ├── Config_RIIC0/           I2C master (for display + VFO)
│   ├── Config_S12AD0/          ADC configuration + ISR (custom user code)
│   ├── r_bsp/                  Renesas board support package
│   └── r_config/               BSP configuration
├── startup/                    CC-RX compiler startup files
├── shortwave-sdr-receiver.mtpj CS+ project file
└── shortwave-sdr-receiver.scfg Smart Configurator configuration
```

## Libraries Used

| Library | Source | Modifications |
|---|---|---|
| `Si5351Arduino` | [etherkit/Si5351Arduino](https://github.com/etherkit/Si5351Arduino) | Arduino Wire I2C calls replaced with Renesas RIIC0 driver in `si5351_rx231.cpp` |
| `u8g2` | [olikraus/u8g2](https://github.com/olikraus/u8g2) | RX231 I2C HAL added in `u8g2_rx231.c`. Unused display drivers and font files excluded from build to reduce flash usage |
| `fix_fft` | [Tomwi/fix_fft](https://gist.github.com/Tomwi/3842231) | `FIX_MPY` changed from `inline` to `static` for CC-RX compiler compatibility |

## Build Environment

- **IDE:** Renesas CS+ with Smart Configurator
- **Compiler:** CC-RX
- **Target:** Renesas RX231 (R5F52318AxFP, 48MHz, 512KB flash, 64KB RAM)

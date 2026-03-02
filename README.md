# EZ Scope: Microcontroller-Based Digital Oscilloscope

A mixed-signal embedded data acquisition system built with an **MSP430G2553** microcontroller and a **MATLAB graphical user interface**. This project demonstrates hardware-level optimization, custom UART communication, and software-based signal processing to create a functional PC oscilloscope.

## 🛠️ System Architecture
* **Hardware:** TI MSP430G2553 (16 MHz System Clock)
* **Firmware:** C (Developed and profiled in Code Composer Studio)
* **PC Interface:** MATLAB App Designer GUI
* **Communication:** UART (115200 baud) via hardware interrupts

## 🚀 Performance & Cycle Profiling
A major focus of this project was maximizing the sampling rate of the 10-bit Successive Approximation Register (SAR) ADC. By writing a custom hardware timer script (`cycle_profiler.c`) to measure instruction execution, the firmware was optimized to minimize latency.

* **Theoretical Hardware Limit:** 17 clock cycles (4 for sample + 13 for conversion)
* **Software Overhead:** 32 clock cycles (Interrupt polling, bit-shifting, and RAM storage)
* **Total Measured Latency:** **49 clock cycles per sample**

Using the 16 MHz system clock, this 49-cycle latency yields the following real-world specifications:
* **Sampling Rate:** 326.5 kSPS (3.06 μs per sample)
* **Alias-Free Bandwidth (Nyquist Limit):** 163.3 kHz
* **Capture Window:** 1.225 ms (400-point data buffer)
* **Minimum Visible Frequency:** ~816 Hz

## 📂 Repository Structure
* **`main.c`** The final, optimized firmware. It configures the MCU to wait in a low-power state for a serial 'U' trigger, captures 400 data points via the ADC, and rapidly transmits the buffer to the PC.
* **`cycle_profiler.c`** A diagnostic test script using Timer A0 (`TA0R`) to scientifically measure the exact CPU clock cycles required for the ADC capture loop. 
* **`EZ_Scope_App.mlapp`** The MATLAB App Designer file containing the GUI. It features dynamic X/Y scaling, serial port management, and custom software-level trigger algorithms (Rising and Falling edge detection with adjustable voltage thresholds).
* **`EZ_Scope_Lab_Report.pdf`** A comprehensive technical report detailing the system theory, design methodology, and complete uncertainty/error propagation analysis.

## 💻 How to Use
1. **Flash the MCU:** Load `main.c` onto the MSP430 Launchpad using Code Composer Studio. Ensure jumpers are configured for hardware UART.
2. **Connect:** Connect the Launchpad to the PC via USB. Note the COM port number in Device Manager.
3. **Run the App:** Open `EZ_Scope_App.mlapp` in MATLAB. Update the COM port variable in the `startupFcn` if necessary, and run the app.
4. **Capture:** Use the "Single Trigger" or "Run" (continuous) buttons to capture and visualize 0-3.3V analog signals applied to Pin P1.4.

## 📄 Full Documentation
For a deep dive into the underlying physics, system constraints, and mathematical models used in this project, please refer to the attached [Lab Report PDF](./EZ_Scope_Lab_Report.pdf).

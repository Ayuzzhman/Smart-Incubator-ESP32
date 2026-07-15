# Simulated ESP32 Smart Incubator with Interrupt-Driven Override

A production-concept embedded systems project designed to monitor ambient temperatures and automatically actuate a safety ventilation hatch when thermal thresholds are crossed. 

This upgraded system features a **hardware-interrupt manual override** and an **asynchronous, non-blocking acoustic alarm system**, simulated entirely in-browser using Wokwi.

---

## 🛠️ System Architecture & Components
* **Controller:** ESP32 Microcontroller (C++ / Arduino Framework)
* **Sensor:** DHT22 (High-accuracy relative humidity and temperature sensor)
* **Display:** 16x2 LCD screen utilizing the I2C communication protocol
* **Actuator:** PWM-controlled SG90 Servo Motor (representing a physical ventilation hatch)
* **Input:** Tactile Pushbutton (for physical override)
* **Output:** Active Piezo Buzzer (for auditory warning status)

---

## 🚀 Interactive Live Simulation
Test this entire project directly in your web browser with zero hardware required:

👉 **[Click Here to Run the Live Wokwi Simulation](https://wokwi.com/projects/469602501112262657)**

---

## 🧠 Advanced Engineering Implementation

### 1. Hardware Interrupts & Software Debouncing (The Pushbutton)
Instead of inefficiently polling the state of the physical button in the main program execution loop, this project implements a **hardware interrupt** bound to **GPIO 14**. 
* The button pin is configured with an internal pull-up resistor (`INPUT_PULLUP`).
* An **Interrupt Service Routine (ISR)** triggers on the `FALLING` edge when the button pulls the signal to Ground.
* **Software Debouncing:** To prevent physical switch bouncing (elastic contact vibration making one click register as multiple triggers), the ISR uses a non-blocking timing check (`millis()`) to ignore consecutive signal changes within a $250\text{ ms}$ window.

### 2. Non-Blocking Multitasking (The Buzzer)
Standard microcontroller programs rely on `delay()`, which blocks the CPU thread and freezes the system. This system is written completely **delay-free**:
* The sensor reading interval ($2000\text{ ms}$) and the buzzer alarm pulse rate ($500\text{ ms}$) are managed asynchronously using `millis()` comparisons.
* This architecture ensures that even when the acoustic alarm is actively pulsing, the controller remains highly responsive to the manual override button instantly.

---

## ⚙️ Logic States

| State | Condition | Display | Vent Status | Buzzer |
| :--- | :--- | :--- | :--- | :--- |
| **Normal** | Temp $\le 35^\circ\text{C}$ | `Temp: XX.X C` | `CLOSED` ($0^\circ$) | Silent |
| **Alarm** | Temp $> 35^\circ\text{C}$ | `Temp: XX.X C (HOT!)` | `OPEN (ALRM)` ($90^\circ$) | Pulsing Beep ($500\text{ ms}$) |
| **Manual Override** | Button Pressed | `OVERRIDE ACTIVE` | `OPEN (MAN)` ($90^\circ$) | Silent |

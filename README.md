# RISC-V RV32IM Toolchain: Assembler & Pipeline Simulator 🚀

A comprehensive C++ simulation suite for the **RISC-V RV32IM** architecture, featuring a **Two-Pass Assembler** and a **5-Stage Pipeline Simulator**.

## 📋 Overview

This project provides a complete educational toolchain for understanding Computer Architecture:
1.  **Assembler:** Converts human-readable RISC-V assembly (with labels and pseudo-instructions) into binary machine code.
2.  **Simulator:** Executes the generated machine code on a classic 5-stage pipeline, visualizing hazards and register states cycle-by-cycle.

---

## 🛠️ Components

### 1. RISC-V Assembler
A two-pass assembler that resolves symbols and encodes instructions into 32-bit binary.
* **Architecture:** RV32IM (Integer + Multiplication/Division).
* **Features:** Handles symbolic labels, expands pseudo-instructions (e.g., `mv`, `li`, `beqz`), and strips inline comments.

### 2. Pipeline Simulator
A cycle-accurate simulator implementing the IF, ID, EX, MEM, and WB stages.
* **Hazard Handling:**
    * **Data Hazards:** Resolved via a Forwarding Unit.
    * **Load-Use Hazards:** Handled by injecting bubbles (stalls).
    * **Control Hazards:** Handled by flushing the pipeline on branches/jumps.
* **Visualization:** Color-coded terminal output tracking the pipeline status per clock cycle.

---

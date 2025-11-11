# RISC-V 5-Stage Pipeline Simulator 🚀

A C++ implementation of a 32-bit RISC-V processor (RV32IM) utilizing a classic 5-stage pipeline. This simulator executes machine code file-by-file, providing a detailed, cycle-by-cycle visualization of the pipeline stages, hazard handling, and register state.

## 📋 Overview

This project simulates the execution flow of RISC-V machine code through the following five pipeline stages:
1.  **IF (Instruction Fetch):** Fetches instruction from Instruction Memory.
2.  **ID (Instruction Decode):** Decodes opcode, reads registers, and generates control signals.
3.  **EX (Execute):** Performs ALU operations and computes branch targets.
4.  **MEM (Memory Access):** Reads/Writes to Data Memory.
5.  **WB (Write Back):** Writes results back to the Register File.

It is designed to handle various hazards inherent to pipelining, making it a robust educational tool for understanding Computer Architecture.

## ✨ Key Features

* **Architecture:** RV32IM (Integer + Multiplication/Division Extension).
* **Hazard Handling:**
    * **Data Hazards:** Implements a **Forwarding Unit** (Operand Forwarding) to resolve dependencies without stalling when possible.
    * **Load-Use Hazards:** Detects load-use dependencies and injects bubbles (stalls) into the pipeline.
    * **Control Hazards:** Handles Branch and Jump instructions by flushing the pipeline (injecting bubbles) upon taking a branch.
* **Memory:**
    * Configurable Data Memory (4KB default).
    * Supports Byte (`LB`, `SB`), Half-word (`LH`, `SH`), and Word (`LW`, `SW`) access.
* **Visualization:** Color-coded terminal output showing the status of every stage per clock cycle.
* **CLI Interface:** Simple command-line arguments for input/output file management.

## 🛠️ Technical Implementation

### Supported Instructions
The simulator supports a wide range of instructions, including:
* **R-Type:** `ADD`, `SUB`, `AND`, `OR`, `XOR`, `SLL`, `SRL`, `SLT`, etc.
* **I-Type:** `ADDI`, `ANDI`, `ORI`, `LB`, `LH`, `LW`, `JALR`.
* **S-Type:** `SB`, `SH`, `SW`.
* **B-Type:** `BEQ`, `BNE`, `BLT`, `BGE`.
* **J-Type:** `JAL`.
* **M-Extension:** `MUL`, `MULH`, `DIV`, `REM`, etc.

### Pipeline Registers
State is maintained between stages using specific structures:
* `IFID_Reg`: Instruction Fetch / Instruction Decode
* `IDEX_Reg`: Instruction Decode / Execute
* `EXMO_Reg`: Execute / Memory Operation
* `MOWB_Reg`: Memory Operation / Write Back

## 🚀 Getting Started

### Prerequisites
* A C++ Compiler.
* Standard C++ libraries.

### Compilation
Compile the source code using `g++`:

```bash
g++ -o riscv_pipeline RISC-V_Pipeline.cpp
 ```

### Running
```bash
./riscv_pipeline [-i input_file] [-o output_file]
```

| Option | Description                             | Default Value          |
|--------|-----------------------------------------|------------------------|
| `-i`   | Path to the machine code file           | `machineCode.txt`      |
| `-o`   | Path to dump the final Register File    | `Terminal (cout)`      |
| `-h`   | Show help message                       | N/A                    |


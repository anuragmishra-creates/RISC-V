# RISC-V Assembler ⚙️

A robust C++ implementation of a RISC-V Assembler. This tool converts human-readable RISC-V assembly language (RV32IM) into 32-bit binary machine code, ready for execution on a simulator or processor.

## 📋 Overview

This project implements a **Two-Pass Assembler** to handle label resolution and code generation efficiently:

1.  **Pass 1 (Symbol Resolution):** Scans the input file to identify labels and map them to specific Program Counter (PC) addresses.
2.  **Pass 2 (Encoding):** Parses the instructions, resolves label addresses to relative offsets, handles pseudo-instructions, and generates the final 32-bit binary machine code.

## ✨ Key Features

* **Architecture:** Supports **RV32IM** (Base Integer + Multiplication/Division Extension).
* **Pseudo-Instruction Support:** Automatically expands common pseudo-instructions (e.g., `mv`, `li`, `j`, `ret`, `beqz`) into their base instruction equivalents.
* **Label Handling:** Full support for symbolic labels, allowing for easy branch and jump target definitions without manual offset calculation.
* **Comment Handling:** Automatically strips inline comments starting with `#`.
* **CLI Interface:** Simple command-line arguments for input/output file management.

## 🛠️ Technical Implementation

### Supported Instructions
The assembler supports a comprehensive set of instructions mapped to the standard RISC-V encoding formats:

* **R-Type:** `add`, `sub`, `sll`, `xor`, `srl`, `sra`, `or`, `and`.
* **I-Type:** `addi`, `xori`, `ori`, `andi`, `slli`, `srli`, `srai`, `jalr`.
* **S-Type:** `sb`, `sh`, `sw`.
* **L-Type:** `lb`, `lh`, `lw`, `lbu`, `lhu`.
* **B-Type:** `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`.
* **J-Type:** `jal`.
* **M-Extension:** `mul`, `div`, `divu`, `rem`, `remu`.

### Supported Pseudo-Instructions
The assembler simplifies coding by supporting these high-level mnemonics:
* `mv`, `li`, `nop`
* `j`, `jr`, `ret`
* `beqz`, `bnez`, `bltz`, `bgez`, `ble`, `bgt`
* `seqz`, `snez`, `sltz`, `sgtz`

## 🚀 Getting Started

### Prerequisites
* A C++ Compiler (GCC/G++ recommended).
* Standard C++ libraries.

### Compilation
Compile the source code using `g++`:

```bash
g++ -o riscv_assembler RISC-V_Assembler.cpp
```

### Running
```bash
./riscv_assembler [-i input_file] [-o output_file]
```

| Option | Description                             | Default Value          |
|--------|-----------------------------------------|------------------------|
| `-i`   | Path to the assembly code file          | `assemblyCode.txt`     |
| `-o`   | Path to save the machine code           | `machineCode.txt`      |
| `-h`   | Show help message                       | N/A                    |


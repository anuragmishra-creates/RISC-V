# RISC-V Assembler in C++

This project is a C++ implementation of a two-pass assembler for a subset of the RISC-V 32-bit instruction set architecture (ISA). It is designed to take a human-readable RISC-V assembly file as input and produce the corresponding 32-bit machine code in binary format.

This was developed as a project to gain a deeper understanding of computer architecture, instruction set design, and the process of how low-level code is translated into machine-executable instructions.

## ✨ Features

- **Two-Pass Design:** Implements a two-pass assembly process to correctly resolve forward-referenced labels in branch and jump instructions.
- **RV32IM Support:** Assembles a useful subset of the base integer instruction set (`RV32I`) and the integer multiplication and division extension (`M`).
- **Pseudo-Instruction Expansion:** Automatically expands common pseudo-instructions (e.g., `mv`, `li`, `j`, `ret`, `nop`, `beqz`) into base instructions.
- **Label Resolution:** Correctly calculates and encodes immediate offsets for branch (`B-type`) and jump (`J-type`) instructions that use labels.
- **Register ABI Support:** Recognizes both ABI names (e.g., `sp`, `ra`, `a0`) and architectural names (e.g., `x2`, `x1`, `x10`).

## 🛠️ Supported ISA Subset

The assembler currently supports the following instructions:

| Type       | Instructions                                                           |
| ---------- | ---------------------------------------------------------------------- |
| **R-Type** | `add`, `sub`, `sll`, `slt`, `sltu`, `xor`, `srl`, `sra`, `or`, `and`   |
| **M-Ext**  | `mul`, `div`, `divu`, `rem`, `remu`                                    |
| **I-Type** | `addi`, `slti`, `sltiu`, `xori`, `ori`, `andi`, `slli`, `srli`, `srai` |
| **Loads**  | `lb`, `lh`, `lw`, `lbu`, `lhu`                                         |
| **Stores** | `sb`, `sh`, `sw`                                                       |
| **Branch** | `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`                             |
| **Jump**   | `jal`, `jalr`                                                          |
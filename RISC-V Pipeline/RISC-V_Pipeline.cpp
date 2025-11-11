#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bitset>
#include <algorithm>
#include <fstream>
using namespace std;

#define BLACK "\033[90m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define MAGENTA "\033[95m"
#define CYAN "\033[96m"
#define WHITE "\033[97m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

struct ControlWord
{
    bool regRead, regWrite, memRead, memWrite, mem2Reg, branch, jump, ALUSrc;
    uint32_t ALUOp; // 2 bits

    ControlWord()
    {
        regRead = regWrite = memRead = memWrite = mem2Reg = branch = jump = ALUSrc = 0;
        ALUOp = 0;
    }
};

// Register File:
class RegisterFile
{
private:
    vector<uint32_t> GPR;

public:
    RegisterFile(uint32_t sp)
    {
        GPR.resize(32, 0);
        write(2, sp);
    }

    uint32_t read(uint32_t rsl)
    {
        if (rsl >= 32)
            return 0;
        return GPR[rsl];
    }

    void write(uint32_t rdl, uint32_t value)
    {
        if (rdl == 0 || rdl >= 32)
            return;
        GPR[rdl] = value;
    }
    void dump(const string &outputFileName)
    {
        static const string regNames[32] =
            {
                "zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
                "s0/fp", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
                "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
                "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

        // Decide output stream: cout or file
        ostream *out;
        ofstream file;

        if (outputFileName.empty())
        {
            out = &cout; // print to terminal
        }
        else
        {
            file.open(outputFileName);
            if (!file)
            {
                cerr << "Error: Failed to open output file: " << outputFileName << endl;
                return;
            }
            out = &file; // print to file
        }

        *out << "Register File State:\n";
        for (int i = 0; i < 32; i++)
        {
            // Example: x05 (t0)
            string reg = string("x") + (i < 10 ? "0" : "") + to_string(i) + " (" + regNames[i] + ")";

            if (out == &cout)
            {
                *out << CYAN << left << setw(12) << reg
                     << WHITE << " = " << RED << "0x"
                     << YELLOW << right << hex << setw(8) << setfill('0') << GPR[i]
                     << RESET << dec << setfill(' ') << endl;
            }
            else
            {
                *out << left << setw(12) << reg
                     << " = 0x"
                     << right << hex << setw(8) << setfill('0') << GPR[i]
                     << dec << setfill(' ') << endl;
            }
        }
    }
};

// Instruction Memory:
class InstructionMemory
{
private:
    vector<uint32_t> IM;

public:
    InstructionMemory(const string &filename)
    {
        ifstream file(filename);
        if (!file.is_open())
        {
            cerr << "Error: Cannot open the machine code file\n";
            exit(1);
        }

        string line;
        while (getline(file, line))
        {
            line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
            if (line.empty())
                continue;

            if (line.size() != 32)
            {
                cerr << "Error: Instruction must be 32 bits only. Received: " << line << "\n";
                exit(1);
            }

            for (char ch : line)
            {
                if (ch != '0' && ch != '1')
                {
                    cerr << "Instruction Memory: Instruction should contain only '0' and '1'. Received: " << line << "\n";
                    exit(1);
                }
            }
            uint32_t ins = bitset<32>(line).to_ulong();
            IM.push_back(ins);
        }
        file.close();
    }

    uint32_t read(uint32_t address) const
    {
        if (address / 4 >= IM.size())
        {
            cerr << "Error: Instruction memory out of bounds. PC= " << address << "\n";
            exit(1);
        }
        return IM[address / 4];
    }

    size_t size() const
    {
        return IM.size();
    }
};

class DataMemory
{
private:
    vector<uint8_t> DM;
    uint32_t baseAddr;
    int32_t signExtend(uint32_t val, int bits)
    {
        int shift = 32 - bits;
        return ((static_cast<int32_t>(val << shift)) >> shift);
    }

public:
    DataMemory(size_t bytes = 4096, uint32_t base = 0) : DM(bytes, 0), baseAddr(base)
    {
    }

    bool validAddress(uint32_t addr, size_t bytesCount)
    {
        if (addr < baseAddr)
            return false;
        uint32_t off = addr - baseAddr;
        if (off > DM.size())
            return false;
        if (off + bytesCount > DM.size())
            return false;
        return true;
    }

    // Word operations
    void writeWord(uint32_t addr, uint32_t value)
    {
        if (!validAddress(addr, 4))
        {
            cerr << "Data Memory: writeWord() invalid address 0x" << hex << addr << dec << "\n";
            return;
        }

        uint32_t off = addr - baseAddr;
        DM[off + 0] = (value & 0xFF);
        DM[off + 1] = ((value >> 8) & 0xFF);
        DM[off + 2] = ((value >> 16) & 0xFF);
        DM[off + 3] = ((value >> 24) & 0xFF);
    }
    uint32_t readWord(uint32_t addr)
    {
        if (!validAddress(addr, 4))
        {
            cerr << "Data Memory: readWord() invalid address 0x" << hex << addr << dec << "\n";
            return 0;
        }

        uint32_t off = addr - baseAddr;
        return ((uint32_t)DM[off + 0] << 0) |
               ((uint32_t)DM[off + 1] << 8) |
               ((uint32_t)DM[off + 2] << 16) |
               ((uint32_t)DM[off + 3] << 24);
    }

    // Byte operations:
    void writeByte(uint32_t addr, uint32_t value)
    {
        if (!validAddress(addr, 1))
        {
            cerr << "Data Memory: writeByte() invalid address 0x" << hex << addr << dec << "\n";
            return;
        }
        uint32_t off = addr - baseAddr;
        DM[off] = value;
    }
    uint32_t readByte(uint32_t addr, bool signedExt = false)
    {
        if (!validAddress(addr, 1))
        {
            cerr << "Data Memory: readByte() invalid address 0x" << hex << addr << dec << "\n";
            return 0;
        }
        uint32_t off = addr - baseAddr;
        uint8_t v = DM[off];
        if (signedExt)
            return static_cast<uint32_t>(signExtend(v, 8));
        return v;
    }

    // Half word operations:
    void writeHalf(uint32_t addr, uint16_t value)
    {
        if (!validAddress(addr, 2))
        {
            cerr << "Data Memory: writeHalf() invalid address 0x" << hex << addr << dec << "\n";
            return;
        }
        uint32_t off = addr - baseAddr;
        DM[off + 0] = (value & 0xFF);
        DM[off + 1] = ((value >> 8) & 0xFF);
    }
    uint32_t readHalf(uint32_t addr, bool signedExt = false)
    {
        if (!validAddress(addr, 2))
        {
            cerr << "Data Memory: readHalf() invalid address 0x" << hex << addr << dec << "\n";
            return 0;
        }
        uint32_t off = addr - baseAddr;
        uint16_t v = (uint16_t)DM[off] | ((uint16_t)DM[off + 1] << 8);

        if (signedExt)
            return static_cast<uint32_t>(signExtend(v, 16));
        return v;
    }

    // Dump:
    void dump(uint32_t start = 0, uint32_t end = 128)
    {
        cout << "Data Memory Dump (hex bytes):";
        for (uint32_t i = start; i < end && i < DM.size(); i++)
        {
            if (i % 16 == 0)
            {
                cout << "\n0x"
                     << setw(4) << setfill('0') << hex << uppercase << i
                     << " : ";
            }
            cout << setw(2) << setfill('0') << hex << uppercase << (int)DM[i] << " ";
        }
        cout << dec << endl; // decimal output
    }
};

ControlWord ControlUnit(uint32_t opcode)
{
    ControlWord CW;
    switch (opcode)
    {
    case 51: // R type
        CW.regRead = 1;
        CW.regWrite = 1;
        CW.memRead = 0;
        CW.memWrite = 0;
        CW.mem2Reg = 0;
        CW.branch = 0;
        CW.jump = 0;
        CW.ALUSrc = 0;
        CW.ALUOp = 2;
        break;

    case 19: // I type
        CW.regRead = 1;
        CW.regWrite = 1;
        CW.memRead = 0;
        CW.memWrite = 0;
        CW.mem2Reg = 0;
        CW.branch = 0;
        CW.jump = 0;
        CW.ALUSrc = 1;
        CW.ALUOp = 2;
        break;

    case 3: // Load type
        CW.regRead = 1;
        CW.regWrite = 1;
        CW.memRead = 1;
        CW.memWrite = 0;
        CW.mem2Reg = 1;
        CW.branch = 0;
        CW.jump = 0;
        CW.ALUSrc = 1;
        CW.ALUOp = 0;
        break;

    case 35: // Store type
        CW.regRead = 1;
        CW.regWrite = 0;
        CW.memRead = 0;
        CW.memWrite = 1;
        CW.mem2Reg = 0;
        CW.branch = 0;
        CW.jump = 0;
        CW.ALUSrc = 1;
        CW.ALUOp = 0; // Add action is used
        break;

    case 99: // B type
        CW.regRead = 1;
        CW.regWrite = 0;
        CW.memRead = 0;
        CW.memWrite = 0;
        CW.mem2Reg = 0;
        CW.branch = 1;
        CW.jump = 0;
        CW.ALUSrc = 0;
        CW.ALUOp = 1;
        break;

    case 111: // JAL (J type)
        CW.regRead = 0;
        CW.regWrite = 1;
        CW.memRead = 0;
        CW.memWrite = 0;
        CW.mem2Reg = 0;
        CW.branch = 0;
        CW.jump = 1;
        CW.ALUSrc = 0;
        CW.ALUOp = 2; // Does NOT matter
        break;

    case 103: // JALR
        CW.regRead = 1;
        CW.regWrite = 1;
        CW.memRead = 0;
        CW.memWrite = 0;
        CW.mem2Reg = 0;
        CW.branch = 0;
        CW.jump = 1;
        CW.ALUSrc = 1;
        CW.ALUOp = 0; // Matters
        break;

    default:
        cerr << "Control Unit: Unknown opcode: " << opcode << "\n";
        exit(1);
        break;
    }
    return CW;
}

uint32_t ALUControl(uint32_t ALUOp, uint32_t func7, uint32_t func3, uint32_t opcode)
{
    /*
        ALUOp: Action
        0: ADD
        1: SUB
        2: func3 determines the operation and func7 (if exists) breaks ties.
    */
    switch (ALUOp)
    {
    case 0: // Load and Store and JALR (ADD action)
        return 2;
        break;

    case 1: // Branch (SUB action)
        return 6;
        break;

    case 2: // R and I type
        bool _30thBitSet = (((func7 >> 5) & 1) != 0);
        bool _25thBitSet = ((func7 & 1) != 0); // For M extension
        bool RType = (opcode == 51);

        if (RType && _25thBitSet) // M Extension
        {
            if (func3 == 0)
                return 10;
            else if (func3 == 1)
                return 11;
            else if (func3 == 2)
                return 12;
            else if (func3 == 3)
                return 13;
            else if (func3 == 4)
                return 14;
            else if (func3 == 5)
                return 15;
            else if (func3 == 6)
                return 16;
            else if (func3 == 7)
                return 17;
            else
                return 2;
        }
        else // (non-M) R or I type
        {
            if (func3 == 0)
                return ((RType && _30thBitSet) ? 6 : 2);
            else if (func3 == 1)
                return 4;
            else if (func3 == 2)
                return 8;
            else if (func3 == 3)
                return 9;
            else if (func3 == 4)
                return 3;
            else if (func3 == 5)
                return ((_30thBitSet) ? 7 : 5);
            else if (func3 == 6)
                return 1;
            else if (func3 == 7)
                return 0;
            else
                return 2;
        }
    }
    return 2;
}

uint32_t ALU(uint32_t ALUSelect, uint32_t A, uint32_t B)
{
    /*
        ALUSelect: ALU Action
        0: AND
        1: OR
        2: ADD
        3: XOR
        4: SLL
        5: SRL
        6: SUB
        7: SRA
        8: SLT
        9: SLTU
        10: MUL
        11: MULH
        12: MULHSU
        13: MULHU
        14: DIV
        15: DIVU
        16: REM
        17: REMU
    */

    uint32_t result = 0;

    switch (ALUSelect)
    {
    case 0:
        result = A & B;
        break;

    case 1:
        result = A | B;
        break;

    case 2:
        result = A + B;
        break;

    case 3:
        result = A ^ B;
        break;

    case 4:
        result = A << (B & (0x1F));
        break;

    case 5:
        result = A >> (B & (0x1F));
        break;

    case 6:
        result = A - B;
        break;

    case 7:
        result = static_cast<int32_t>(A) >> (B & (0x1F));
        break;

    case 8:
        result = (static_cast<int32_t>(A) < static_cast<int32_t>(B)) ? 1 : 0;
        break;

    case 9:
        result = (A < B) ? 1 : 0;
        break;

    case 10:
        result = A * B;
        break;

    case 11: // mulh: High 32 bits of signed*signed
    {
        // This was your line 525
        int64_t aS = static_cast<int32_t>(A);
        int64_t bS = static_cast<int32_t>(B);
        int64_t resS = aS * bS;
        result = static_cast<uint32_t>(resS >> 32); // Upper 32 bits
        break;
    }
    case 12: // mulhsu: High 32 bits of signed*unsigned
    {
        // This was your line 534
        int32_t a_s32 = static_cast<int32_t>(A);
        uint64_t b_u64 = B;

        if (a_s32 >= 0)
        {
            // A is positive, same as mulhu but with a signed cast
            uint64_t res = static_cast<uint64_t>(a_s32) * b_u64;
            result = static_cast<uint32_t>(res >> 32);
        }
        else
        {
            // A is negative. Calculate product of abs(A) * B, then negate.
            uint64_t a_abs;
            if (a_s32 == INT32_MIN)
            {
                a_abs = static_cast<uint64_t>(INT32_MIN); // 2^31
            }
            else
            {
                a_abs = static_cast<uint64_t>(-a_s32);
            }

            uint64_t abs_prod = a_abs * b_u64;
            uint64_t neg_prod = ~abs_prod + 1; // Two's complement negation
            result = static_cast<uint32_t>(neg_prod >> 32);
        }
        break;
    }

    case 13: // mulhu: High 32 bits of unsigned*unsigned
    {
        // This was your line 543
        uint64_t aU = A;
        uint64_t bU = B;
        uint64_t resU = aU * bU;
        result = static_cast<uint32_t>(resU >> 32); // Upper 32 bits
        break;
    }

    case 14:
    {
        int32_t aS = static_cast<int32_t>(A);
        int32_t bS = static_cast<int32_t>(B);

        if (bS == 0) // Divide by zero
            result = 0xFFFFFFFF;
        else if (aS == 0x80000000 && bS == -1) // Overflow (INT_MIN/-1)
            result = 0x80000000;
        else
            result = static_cast<uint32_t>(aS / bS);
        break;
    }

    case 15:
    {
        if (B == 0)
            result = 0xFFFFFFFF;
        else
            result = A / B;
        break;
    }

    case 16: // REM (signed)
    {
        int32_t a_s = static_cast<int32_t>(A);
        int32_t b_s = static_cast<int32_t>(B);

        if (b_s == 0)                            // Divide by zero
            result = A;                          // Remainder is dividend
        else if (a_s == 0x80000000 && b_s == -1) // Overflow (INT_MIN % -1)
            result = 0;
        else
            result = static_cast<uint32_t>(a_s % b_s);
        break;
    }

    case 17: // REMU (unsigned)
    {
        if (B == 0)     // Divide by zero
            result = A; // Remainder is dividend
        else
            result = A % B;
        break;
    }

    default:
        result = 0;
    }
    return result;
}

int32_t signExtend(uint32_t val, int bits)
{
    int shift = 32 - bits;
    return ((static_cast<int32_t>(val << shift)) >> shift);
}

void prepareOpcodeAndFunctions(uint32_t instr, uint32_t &opcode, uint32_t &rdl, uint32_t &func3,
                               uint32_t &rsl1, uint32_t &rsl2, uint32_t &func7)
{
    opcode = instr & 0x7F;
    rdl = (instr >> 7) & 0x1F;
    func3 = (instr >> 12) & 0x07;
    rsl1 = (instr >> 15) & 0x1F;
    rsl2 = (instr >> 20) & 0x1F;
    func7 = (instr >> 25) & 0x7F;
}

bool branchTaken(uint32_t func3, uint32_t A, uint32_t B)
{
    switch (func3)
    {
    case 0: // BEQ
        return A == B;
    case 1: // BNE
        return A != B;
    case 4: // BLT
        return static_cast<int32_t>(A) < static_cast<int32_t>(B);
    case 5: // BGE
        return static_cast<int32_t>(A) >= static_cast<int32_t>(B);
    case 6: // BLTU
        return A < B;
    case 7: // BGEU
        return A >= B;
    default:
        return false;
    }
}

int32_t genImm(uint32_t instr, uint32_t opcode)
{
    int32_t imm = 0;
    if (opcode == 19 || opcode == 3 || opcode == 103) // I type or Load type or JALR
    {
        imm = (instr >> 20) & 0xFFF;
        imm = signExtend(imm, 12);
    }
    else if (opcode == 35) // Store type
    {
        imm = ((instr >> 25) << 5) | ((instr >> 7) & 0x1F);
        imm = signExtend(imm, 12);
    }
    else if (opcode == 111) // J type (JAL)
    {
        imm = ((instr >> 31) << 20) | (((instr >> 12) & 0xFF) << 12) | (((instr >> 20) & 0x1) << 11) | (((instr >> 21) & 0x3FF) << 1);
        imm = signExtend(imm, 21);
    }
    else if (opcode == 99) // B type
    {
        imm = ((instr >> 31) << 12) | (((instr >> 7) & 0x1) << 11) | (((instr >> 25) & 0x3F) << 5) | (((instr >> 8) & 0xF) << 1);
        imm = signExtend(imm, 13);
    }
    else
        imm = 0;

    return imm;
}

// Pipeline registers:
/*
    Note:
    Initally, valid=false; => NOP
    stall=false (because otherwise pipeline will be stuck as no stall=true can only be set from the right!)
*/

struct PC_Reg
{
    uint32_t value, TPC;

    PC_Reg()
    {
        value = 0;
        TPC = -1;
    }
};

struct IFID_Reg
{
    uint32_t DPC, IR;
    bool stall, valid;
    IFID_Reg()
    {
        DPC = IR = 0;
        stall = false, valid = false;
    }
};

struct IDEX_Reg
{
    ControlWord CW;
    uint32_t DPC;
    uint32_t rs1, rs2;
    int32_t imm;

    uint32_t opcode;
    uint32_t rdl;
    uint32_t func3;
    uint32_t rsl1, rsl2; // for operand forwarding
    uint32_t func7;

    bool stall, valid;

    IDEX_Reg()
    {
        CW = ControlWord();
        DPC = 0;
        rs1 = rs2 = 0;
        opcode = rdl = func3 = rsl1 = rsl2 = func7 = 0;
        stall = false, valid = false;
    }
};

struct EXMO_Reg
{
    ControlWord CW;
    uint32_t DPC;
    uint32_t ALUOut;
    uint32_t rs2; // Will be required for store

    uint32_t rdl;   // Will be required in Operand forwarding
    uint32_t func3; // Will be required for Load type determination

    bool stall, valid;

    EXMO_Reg()
    {
        CW = ControlWord();
        DPC = rdl = func3 = 0;
        ALUOut = 0;
        stall = false, valid = false;
    }
};

struct MOWB_Reg
{
    ControlWord CW;
    uint32_t DPC;
    uint32_t ALUOut, LDOut;
    uint32_t rdl; // Will be required in operand forwarding

    bool stall, valid;

    // To remember current state for MOWB before it is overwritten by EXMO (in MO stage):
    ControlWord CWOld;
    uint32_t ALUOutOld, LDOutOld, rdlOld;
    bool validOld;
    /*
        Why do we need old values?
        Reason: Suppose in EX, we have rsl2=X5,
                        in MO, we have rdl=X4,
                        in WB, we have rdl=X5.
        Now when we move sequentially from right to left, we get overwrite MOWB.rdl=X4.
        Now operand forwarding will not find that there is any issue and it will continue with stale value.
    */

    MOWB_Reg()
    {
        CW = ControlWord();
        DPC = 0;
        ALUOut = LDOut = rdl = 0;
        ALUOutOld = LDOutOld = rdlOld = 0;
        CWOld = ControlWord();
        stall = false, valid = false, validOld = false;
    }
};

bool programRunning = true;
bool insertBubble = false;
PC_Reg PC;
IFID_Reg IFID;
IDEX_Reg IDEX;
EXMO_Reg EXMO;
MOWB_Reg MOWB;

// Functions:
void InstructionFetch(InstructionMemory &IM)
{
    cout << "\n[IF Stage]" << endl;
    if (IFID.stall)
    {
        cout << "  IF: Stalled" << endl;
        return;
    }

    // Program is over but we might have to continue running till the pipeline is empty:
    if (PC.value >= 4 * IM.size())
    {
        // Bubble injection:
        IFID.valid = false;

        // Check if all pipeline stages are empty:
        if (!IDEX.valid && !EXMO.valid && !MOWB.valid)
            programRunning = false;
        return;
    }

    IFID.IR = IM.read(PC.value);
    IFID.DPC = PC.value;
    cout << "  IF: PC=0x" << hex << PC.value << " (dec: " << dec << PC.value << ")"
         << " IR=0x" << hex << IFID.IR << " (dec: " << dec << IFID.IR << ")" << endl;

    // PC Update logic: For next instruction and NOT the current instruction:
    if (PC.TPC != -1) // The normal flow is broken
    {
        PC.value = PC.TPC;
        PC.TPC = -1; // Reset the TPC so that normal flow is continued now
    }
    else // Normal flow
        PC.value = PC.value + 4;

    IFID.valid = true;
}

// Finds the load-use hazard in Decode stage before actual decoding starts
void HazardDetectionUnit()
{
    // If the instruction infront is Load instruction
    if (IDEX.valid && IFID.valid && IDEX.CW.memRead && IDEX.rdl != 0)
    {
        uint32_t rsl1 = (IFID.IR >> 15) & 0x1F;
        uint32_t rsl2 = (IFID.IR >> 20) & 0x1F;

        // Check if the to-be-decoded instruction (in IFID) needs the loaded value
        if (IDEX.rdl == rsl1 || IDEX.rdl == rsl2)
        {
            // Handled in Decode now: IFID.stall = true;  // Keep current instruction in IFID (stall Fetch)
            IDEX.stall = true;  // Keep current instruction in IDEX (stall Decode)
            IDEX.valid = false; // Insert bubble in IDEX (ensure NOP in EX in next cycle)
            cout << "Load-Use Hazard detected.\n";
        }
    }
}

void InstructionDecode(RegisterFile &RF)
{
    cout << "\n[ID Stage]" << endl;
    HazardDetectionUnit();

    if (IDEX.stall)
    {
        IFID.stall = true; // Left stage should also be stalled now
        return;
    }

    if (IFID.valid == false) // Bubble in IFID => NOP in Decode
    {
        // Propagate the bubble
        IDEX.valid = false;

        // Since bubble moves and is NOT stalled, we should signal left stage to move by removing the latter's stall!
        IFID.stall = false;
        return;
    }

    // Prepare opcode and functions:
    prepareOpcodeAndFunctions(IFID.IR, IDEX.opcode, IDEX.rdl, IDEX.func3, IDEX.rsl1, IDEX.rsl2, IDEX.func7);

    // Immediate generation:
    IDEX.imm = genImm(IFID.IR, IDEX.opcode);

    // Control Word Generation:
    IDEX.CW = ControlUnit(IDEX.opcode);

    // Read Register:
    IDEX.rs1 = 0, IDEX.rs2 = 0;
    if (IDEX.CW.regRead)
    {
        IDEX.rs1 = RF.read(IDEX.rsl1);
        IDEX.rs2 = RF.read(IDEX.rsl2);
    }
    IDEX.DPC = IFID.DPC;

    IFID.stall = false;
    IDEX.valid = true;
}

uint32_t ALUForwarder(int i)
{
    switch (i)
    {
    case 1:
        if (EXMO.valid && EXMO.CW.regWrite && EXMO.rdl != 0 && EXMO.rdl == IDEX.rsl1)
            return EXMO.ALUOut;
        else if (MOWB.validOld && MOWB.CWOld.regWrite && MOWB.rdlOld != 0 && MOWB.rdlOld == IDEX.rsl1)
            return ((MOWB.CWOld.mem2Reg) ? (MOWB.LDOutOld) : (MOWB.ALUOutOld));
        else
            return IDEX.rs1;
        break;

    case 2:
        if (IDEX.CW.ALUSrc == 1) // Forwarding never required, as imm is ALWAYS updated!
            return (static_cast<uint32_t>(IDEX.imm));
        else // Forwarding might be required as we want UPDATED rs2!
        {
            if (EXMO.valid && EXMO.CW.regWrite && EXMO.rdl != 0 && EXMO.rdl == IDEX.rsl2)
                return EXMO.ALUOut;
            else if (MOWB.validOld && MOWB.CWOld.regWrite && MOWB.rdlOld != 0 && MOWB.rdlOld == IDEX.rsl2)
                return ((MOWB.CWOld.mem2Reg) ? (MOWB.LDOutOld) : (MOWB.ALUOutOld));
            else
                return IDEX.rs2;
        }
        break;

    default:
        cout << "ALUForwarder(): Error! Called with invalid value of i = " << i << ".\n";
        return 0;
    }
    return 0;
}

uint32_t rs2Forwarder() // also called storedDataForwarder()
{
    /*
        (I)
        ADDI rs2, rs2, 1;
        SW rs2, off(rs1);

        (II)
        ADDI rs2, rs2, 1;
        XXXXXXXXXXXXXXXX; // Some other instruction
        SW rs2, off(rs1);

        Example I indicates the requirement of rs2Forwarder().
        Example II indicates to us that the rs2Forwarder() should be in Execute() and NOT MemoryOperation().
        If we do the otherway around, then we will will have to forward data from RF as well.

        (III)
        ADDI rs2, rs2, 1;
        ADDI rs2, rs2, 1;
        SW rs2, off(rs1);

        Example III indicates that we must borrow from EXMO first.
    */

    if (EXMO.valid && EXMO.CW.regWrite && EXMO.rdl != 0 && EXMO.rdl == IDEX.rsl2)
        return EXMO.ALUOut;
    else if (MOWB.validOld && MOWB.CWOld.regWrite && MOWB.rdlOld != 0 && MOWB.rdlOld == IDEX.rsl2)
        return ((MOWB.CWOld.mem2Reg) ? (MOWB.LDOutOld) : (MOWB.ALUOutOld));
    else
        return IDEX.rs2;
    return 0;
}

void Execute()
{
    cout << "\n[EX Stage]" << endl;
    if (EXMO.stall)
    {
        IDEX.stall = true; // Left stage should also be stalled now
        return;
    }

    if (IDEX.valid == false) // Bubble in IDEX => NOP in Execute
    {
        // Propagate the bubble
        EXMO.valid = false;

        // Since bubble moves and is NOT stalled, we should signal left stage to move by removing the latter's stall!
        IDEX.stall = false;
        return;
    }

    // Determine ALU Input:
    uint32_t alusrc1 = ALUForwarder(1);
    uint32_t alusrc2 = ALUForwarder(2);
    uint32_t &rs1 = alusrc1;
    uint32_t rs2 = rs2Forwarder();

    // ALU Select:
    uint32_t ALUSelect = ALUControl(IDEX.CW.ALUOp, IDEX.func7, IDEX.func3, IDEX.opcode);

    // ALU Execute:
    uint32_t ALUResult = ALU(ALUSelect, alusrc1, alusrc2);
    cout << "  EX: ALU op=" << dec << ALUSelect << " src1=0x" << hex << alusrc1
         << " (dec: " << dec << alusrc1 << ") src2=0x" << hex << alusrc2
         << " (dec: " << dec << alusrc2 << ") result=0x" << hex << ALUResult
         << " (dec: " << dec << ALUResult << ")" << endl;

    // Branch and jump handling:
    uint32_t BPC = static_cast<uint32_t>(static_cast<int32_t>(IDEX.DPC) + IDEX.imm); // (B and JAL)
    uint32_t JPC = (ALUResult & (~1u));                                              // Ignoring the odd bit (JALR)

    // If TPC is set to != -1, it means that we are taking a branch/jump
    if (IDEX.CW.branch && branchTaken(IDEX.func3, rs1, rs2)) // B
    {
        PC.TPC = BPC;
        insertBubble = true;
    }
    else if (IDEX.CW.jump && IDEX.opcode == 111) // JAL
    {
        PC.TPC = BPC;
        insertBubble = true;
    }
    else if (IDEX.CW.jump && IDEX.opcode == 103) // JALR
    {
        PC.TPC = JPC;
        insertBubble = true;
    }

    EXMO.DPC = IDEX.DPC;
    EXMO.CW = IDEX.CW;
    EXMO.ALUOut = ALUResult;
    EXMO.rdl = IDEX.rdl;
    EXMO.func3 = IDEX.func3; // For checking the load type in MO stage
    EXMO.rs2 = rs2;          // For store in MO in next stage

    IDEX.stall = false;
    EXMO.valid = true;
}

void MemoryOperation(DataMemory &DM)
{
    cout << "\n[MEM Stage]" << endl;
    if (MOWB.stall)
    {
        return;
    }

    // Store current values before overwriting them:
    MOWB.LDOutOld = MOWB.LDOut;
    MOWB.ALUOutOld = MOWB.ALUOut;
    MOWB.rdlOld = MOWB.rdl;
    MOWB.CWOld = MOWB.CW;
    MOWB.validOld = MOWB.valid;

    if (EXMO.valid == false) // Bubble in PC => NOP in Memory Operation
    {
        // Propagate the bubble
        MOWB.valid = false;

        // Since bubble moves and is NOT stalled, we should signal left stage to move by removing the latter's stall!
        EXMO.stall = false;
        return;
    }

    // Memory Read (Load) and Write (Store):
    uint32_t LDResult = 0;
    if (EXMO.CW.memRead)
    {
        cout << "  MEM: Reading from addr 0x" << hex << EXMO.ALUOut << " (dec: " << dec << EXMO.ALUOut << ")" << endl;
        if (EXMO.func3 == 0) // LB
            LDResult = DM.readByte(EXMO.ALUOut, true);
        else if (EXMO.func3 == 1) // LH
            LDResult = DM.readHalf(EXMO.ALUOut, true);
        else if (EXMO.func3 == 2) // LW
            LDResult = DM.readWord(EXMO.ALUOut);
        else if (EXMO.func3 == 4) // LBU
            LDResult = DM.readByte(EXMO.ALUOut, false);
        else if (EXMO.func3 == 5) // LHU
            LDResult = DM.readHalf(EXMO.ALUOut, false);
        else // No LWU
            LDResult = DM.readWord(EXMO.ALUOut);
    }
    MOWB.LDOut = LDResult;
    MOWB.ALUOut = EXMO.ALUOut;

    if (EXMO.CW.memWrite)
    {
        cout << "  MEM: Writing to addr 0x" << hex << EXMO.ALUOut << " (dec: " << dec << EXMO.ALUOut
             << ") value=0x" << hex << EXMO.rs2 << " (dec: " << dec << EXMO.rs2 << ")" << endl;
        if (EXMO.func3 == 0) // SB
            DM.writeByte(EXMO.ALUOut, static_cast<uint8_t>(EXMO.rs2 & 0xFF));
        else if (EXMO.func3 == 1) // SH
            DM.writeHalf(EXMO.ALUOut, static_cast<uint16_t>(EXMO.rs2 & 0xFFFF));
        else if (EXMO.func3 == 2) // SW
            DM.writeWord(EXMO.ALUOut, EXMO.rs2);
        else // Fallback
            DM.writeWord(EXMO.ALUOut, EXMO.rs2);
    }

    MOWB.CW = EXMO.CW;
    MOWB.DPC = EXMO.DPC;
    MOWB.rdl = EXMO.rdl;

    EXMO.stall = false;
    MOWB.valid = true;
}

void WriteBack(RegisterFile &RF)
{
    cout << "\n[WB Stage]" << endl;
    if (MOWB.valid == false) // Bubble in PC => NOP in WriteBack
    {
        // Since bubble moves and is NOT stalled, we should signal left stage to move by removing the latter's stall!
        MOWB.stall = false;
        return;
    }

    // Write Register:
    if (MOWB.CW.regWrite)
    {
        uint32_t writeVal = 0;
        if (MOWB.CW.jump) // JAL, JALR
            writeVal = MOWB.DPC + 4;
        else if (MOWB.CW.mem2Reg) // Load
            writeVal = MOWB.LDOut;
        else // R, I
            writeVal = MOWB.ALUOut;

        cout << "  WB: Writing value 0x" << hex << writeVal << " (dec: " << dec << writeVal
             << ") to register x" << dec << MOWB.rdl << endl;
        RF.write(MOWB.rdl, writeVal);
    }

    MOWB.stall = false;
}

void printUsage()
{
    cout << RED << "Usage:\n"
         << RESET;
    cout << BLUE << "  RISC-V_Pipeline [-i <inputfile>] [-o <outputfile>]\n\n";
    cout << "Options:\n";
    cout << "  -i <inputfile>   :  Input machine code file (default: machineCode.txt)\n";
    cout << "  -o <outputfile>  :  Output Final Register File (default: terminal)\n";
    cout << "  -h --help        :  Show this help message\n"
         << RESET;
}

int main(int argc, char *argv[])
{
    printf(BOLD RED "  RISC-V_Assembler (by anuragmishra-creates)\n" RESET);
    string inputFileName = "machineCode.txt", outputFileName = "";

    for (int i = 1; i < argc; i++)
    {
        string arg = argv[i];
        if (arg == "-i")
        {
            if (i + 1 < argc)
                inputFileName = argv[++i];
            else
            {
                cerr << RED << "Error: -i requires a filename.\n"
                     << RESET;
                return 1;
            }
        }
        else if (arg == "-o")
        {
            if (i + 1 < argc)
                outputFileName = argv[++i];
            else
            {
                cerr << RED << "Error: -o requires a filename.\n"
                     << RESET;
                return 1;
            }
        }
        else if (arg == "-h" || arg == "--help")
        {
            printUsage();
            return 0;
        }
        else
        {
            cerr << RED << "Unknown argument: " << arg << "\n"
                 << RESET;
            printUsage();
            return 1;
        }
    }

    cout << MAGENTA << "   >>> Pipeline Started <<<\n"
         << RESET;
    cout << CYAN << "Input File : " << GREEN << inputFileName << "\n"
         << RESET;
    cout << CYAN << "Output File: " << GREEN << outputFileName << "\n"
         << RESET;

    // Load Instruction memory:
    InstructionMemory IM(inputFileName);
    cout << "Loaded " << IM.size() << " number of instructions.\n";
    RegisterFile RF(4096);
    DataMemory DM(4096, 0);

    // Setting input parameter (a0/x10) as 4 (temporary):
    RF.write(10, 2);

    uint32_t instrCount = IM.size();
    uint32_t cycle = 0;

    while (programRunning)
    {
        cout << BLUE << "\n===================== Cycle " << dec << ++cycle << " =====================" << RESET << endl;
        WriteBack(RF);
        MemoryOperation(DM);
        Execute();
        InstructionDecode(RF);
        InstructionFetch(IM);

        if (insertBubble) // NOP in the next cycle preparation
        {
            IFID.valid = false;   // NOP in Decode in the next cycle
            IDEX.valid = false;   // NOP in Execute in the next cycle
            insertBubble = false; // Bubble injection is done!
        }

        // Failsafe
        if (cycle > 1000)
        {
            cerr << RED << "Simulation timed out with " << cycle << " cycles\n"
                 << RESET;
            break;
        }
    }

    cout << MAGENTA << "\n   >>> Pipeline Ended <<<\n"
         << RESET;
    cout << GREEN << "Execution finished with " << cycle << " cycles\n"
         << RESET;
    RF.dump(outputFileName);
    return 0;
}
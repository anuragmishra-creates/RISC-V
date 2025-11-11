#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <vector>
#include <bitset>
#include <fstream>
using namespace std;

#define RESET "\033[0m"
#define RED "\033[91m"
#define GREEN "\033[92m"
#define YELLOW "\033[93m"
#define BLUE "\033[94m"
#define MAGENTA "\033[95m"
#define CYAN "\033[96m"
#define WHITE "\033[97m"
#define BOLD "\033[1m"

// Creating the Hash Maps
static unordered_map<string, uint32_t> regMap =
    {
        {"x0", 0},
        {"x1", 1},
        {"x2", 2},
        {"x3", 3},
        {"x4", 4},
        {"x5", 5},
        {"x6", 6},
        {"x7", 7},
        {"x8", 8},
        {"x9", 9},
        {"x10", 10},
        {"x11", 11},
        {"x12", 12},
        {"x13", 13},
        {"x14", 14},
        {"x15", 15},
        {"x16", 16},
        {"x17", 17},
        {"x18", 18},
        {"x19", 19},
        {"x20", 20},
        {"x21", 21},
        {"x22", 22},
        {"x23", 23},
        {"x24", 24},
        {"x25", 25},
        {"x26", 26},
        {"x27", 27},
        {"x28", 28},
        {"x29", 29},
        {"x30", 30},
        {"x31", 31},
        {"zero", 0},
        {"ra", 1},
        {"sp", 2},
        {"gp", 3},
        {"tp", 4},
        {"t0", 5},
        {"t1", 6},
        {"t2", 7},
        {"s0", 8},
        {"fp", 8},
        {"s1", 9},
        {"a0", 10},
        {"a1", 11},
        {"a2", 12},
        {"a3", 13},
        {"a4", 14},
        {"a5", 15},
        {"a6", 16},
        {"a7", 17},
        {"s2", 18},
        {"s3", 19},
        {"s4", 20},
        {"s5", 21},
        {"s6", 22},
        {"s7", 23},
        {"s8", 24},
        {"s9", 25},
        {"s10", 26},
        {"s11", 27},
        {"t3", 28},
        {"t4", 29},
        {"t5", 30},
        {"t6", 31}};

// Stores the addresses of the labels
unordered_map<string, int> labelMap;

// R TYPE
struct RSpec
{
    uint32_t func7;
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, RSpec> rMap =
    {
        {"add", {0x00, 0x0, 0x33}},
        {"sub", {0x20, 0x0, 0x33}},
        {"sll", {0x00, 0x1, 0x33}},
        {"slt", {0x00, 0x2, 0x33}},
        {"sltu", {0x00, 0x3, 0x33}},
        {"xor", {0x00, 0x4, 0x33}},
        {"srl", {0x00, 0x5, 0x33}},
        {"sra", {0x20, 0x5, 0x33}},
        {"or", {0x00, 0x6, 0x33}},
        {"and", {0x00, 0x7, 0x33}},
        // M-extension:
        {"mul", {0x01, 0x0, 0x33}},
        {"div", {0x01, 0x4, 0x33}},
        {"divu", {0x01, 0x5, 0x33}},
        {"rem", {0x01, 0x6, 0x33}},
        {"remu", {0x01, 0x7, 0x33}}};
uint32_t encodeR(uint32_t rd, uint32_t rs1, uint32_t rs2, RSpec &spec)
{
    return (spec.func7 << 25) | (rs2 << 20) | (rs1 << 15) | (spec.func3 << 12) | (rd << 7) | (spec.opcode);
}

// I Arith
struct IArithSpec
{
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, IArithSpec> iAMap =
    {
        {"addi", {0x0, 0x13}},
        {"slti", {0x2, 0x13}},
        {"sltiu", {0x3, 0x13}},
        {"xori", {0x4, 0x13}},
        {"ori", {0x6, 0x13}},
        {"andi", {0x7, 0x13}}};
uint32_t encodeIArith(uint32_t rd, uint32_t rs1, uint32_t imm, IArithSpec &spec)
{
    return ((imm & 0xFFF) << 20) | (rs1 << 15) | (spec.func3 << 12) | (rd << 7) | (spec.opcode);
}

struct IShiftSpec
{
    uint32_t func7;
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, IShiftSpec> iSMap =
    {
        {"slli", {0x00, 0x1, 0x13}},
        {"srli", {0x00, 0x5, 0x13}},
        {"srai", {0x20, 0x5, 0x13}},
};
uint32_t encodeIShift(uint32_t rd, uint32_t rs1, uint32_t shamt, IShiftSpec &spec)
{
    return (spec.func7 << 25) | ((shamt & 0x1F) << 20) | (rs1 << 15) | (spec.func3 << 12) | (rd << 7) | (spec.opcode);
}

struct BSpec
{
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, BSpec> bMap =
    {
        {"beq", {0x0, 0x63}},
        {"bne", {0x1, 0x63}},
        {"blt", {0x4, 0x63}},
        {"bge", {0x5, 0x63}},
        {"bltu", {0x6, 0x63}},
        {"bgeu", {0x7, 0x63}}};
uint32_t encodeB(uint32_t rs1, uint32_t rs2, uint32_t imm, BSpec &spec)
{
    uint32_t imm12 = (imm >> 12) & 0x01;
    uint32_t imm10to5 = (imm >> 5) & 0x3F;
    uint32_t imm11 = (imm >> 11) & 0x01;
    uint32_t imm4to1 = (imm >> 1) & 0x0F;

    return (imm12 << 31) | (imm10to5 << 25) | (rs2 << 20) | (rs1 << 15) | (spec.func3 << 12) | (imm4to1 << 8) | (imm11 << 7) | spec.opcode;
}

// S Spec
struct SSpec
{
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, SSpec> sMap =
    {
        {"sb", {0x0, 0x23}},
        {"sh", {0x1, 0x23}},
        {"sw", {0x2, 0x23}}};
uint32_t encodeS(uint32_t rs1, uint32_t rs2, uint32_t imm, SSpec &spec)
{
    uint32_t imm4to0 = imm & 0x1F;
    uint32_t imm11to5 = (imm >> 5) & 0x7F;

    return (imm11to5 << 25) | (rs2 << 20) | (rs1 << 15) | (spec.func3 << 12) | (imm4to0 << 7) | (spec.opcode);
}

// LSpec
struct LSpec
{
    uint32_t func3;
    uint32_t opcode;
};
static unordered_map<string, LSpec> lMap =
    {
        {"lb", {0x0, 0x03}},
        {"lh", {0x1, 0x03}},
        {"lw", {0x2, 0x03}},
        {"lbu", {0x4, 0x03}},
        {"lhu", {0x5, 0x03}}};
uint32_t encodeL(uint32_t rd, uint32_t rs1, uint32_t imm, LSpec &spec)
{
    return ((imm & 0xFFF) << 20) | (rs1 << 15) | (spec.func3 << 12) | (rd << 7) | spec.opcode;
}

struct JALSpec
{
    uint32_t opcode = 0x6F;
};
static unordered_map<string, JALSpec> jalMap =
    {
        {"jal", {0x6F}}};
uint32_t encodeJAL(uint32_t rd, uint32_t imm, JALSpec &spec)
{
    // imm->[20:0]
    uint32_t imm20 = (imm >> 20) & 0x1;
    uint32_t imm10to1 = (imm >> 1) & 0x3FF;
    uint32_t imm11 = (imm >> 11) & 0x1;
    uint32_t imm19to12 = (imm >> 12) & 0xFF;

    return (imm20 << 31) | (imm10to1 << 21) | (imm11 << 20) | (imm19to12 << 12) | (rd << 7) | spec.opcode;
}

// JALRSpec
struct JALRSpec
{
    uint32_t func3 = 0;
    uint32_t opcode = 0x67;
};
static unordered_map<string, JALRSpec> jalrMap =
    {
        {"jalr", {0x0, 0x67}}};
uint32_t encodeJALR(uint32_t rs1, uint32_t rd, uint32_t imm, JALRSpec &spec)
{
    return ((imm & 0xFFF) << 20) | (rs1 << 15) | (spec.func3 << 12) | (rd << 7) | (spec.opcode);
}

/* Parse Instructions*/
// Removes comments as they start with '#' (if present)
string stripComment(const string &line)
{
    size_t pos = line.find("#");
    if (pos != string::npos)
        return line.substr(0, pos);
    return line;
}

// Splits on tabs and spaces
vector<string> tokenize(const string &line)
{
    vector<string> tokens;
    string tok;
    for (char ch : line)
    {
        if (isspace(ch) || ch == ',' || ch == '(' || ch == ')')
        {
            if (!tok.empty())
            {
                // transform(tok.begin(),tok.end(),tok.begin(),::tolower);
                tokens.push_back(tok);
                tok.clear();
            }
        }
        else
            tok += ch;
    }
    if (!tok.empty())
    {
        // transform(tok.begin(),tok.end(),tok.begin(),::tolower);
        tokens.push_back(tok);
    }
    return tokens;
}

vector<string> expandPseudo(const vector<string> &tokens)
{
    if (tokens.empty())
        return tokens;

    string mne = tokens[0];

    if (mne == "mv")
    {
        if (tokens.size() == 3)
            return {"addi", tokens[1], tokens[2], "0"};
        cerr << "Error: mv requires 2 operands\n";
        return {};
    }
    else if (mne == "li")
    {
        if (tokens.size() == 3)
            return {"addi", tokens[1], "x0", tokens[2]};
        cerr << "Error: li requires 2 operands\n";
        return {};
    }
    else if (mne == "j")
    {
        if (tokens.size() == 2)
            return {"jal", "x0", tokens[1]};
        cerr << "Error: j requires 1 operand\n";
        return {};
    }
    else if (mne == "jr")
    {
        if (tokens.size() == 2)
            return {"jalr", "x0", "0", tokens[1]};
        cerr << "Error: jr requires 1 operand\n";
        return {};
    }
    else if (mne == "ret")
    {
        if (tokens.size() == 1)
            return {"jalr", "x0", "0", "ra"};
        cerr << "Error: ret requires NO operands\n";
        return {};
    }
    else if (mne == "nop")
    {
        if (tokens.size() == 1)
            return {"addi", "x0", "x0", "0"};
        cerr << "Error: nop requires NO operands\n";
        return {};
    }
    else if (mne == "ble")
    {
        if (tokens.size() == 4)
            return {"bge", tokens[2], tokens[1], tokens[3]};
        cerr << "Error: ble requires 3 operands\n";
        return {};
    }
    else if (mne == "bgt")
    {
        if (tokens.size() == 4)
            return {"blt", tokens[2], tokens[1], tokens[3]};
        cerr << "Error: bgt requires 3 operands\n";
        return {};
    }
    else if (mne == "beqz")
    {
        if (tokens.size() == 3)
            return {"beq", tokens[1], "x0", tokens[2]};
        cerr << "Error: beqz requires 2 operands\n";
        return {};
    }
    else if (mne == "bnez")
    {
        if (tokens.size() == 3)
            return {"bne", tokens[1], "x0", tokens[2]};
        cerr << "Error: bnez requires 2 operands\n";
        return {};
    }
    else if (mne == "bgez")
    {
        if (tokens.size() == 3)
            return {"bge", tokens[1], "x0", tokens[2]};
        cerr << "Error: bgez requires 2 operands\n";
        return {};
    }
    else if (mne == "bltz")
    {
        if (tokens.size() == 3)
            return {"blt", tokens[1], "x0", tokens[2]};
        cerr << "Error: bltz requires 2 operands\n";
        return {};
    }
    else if (mne == "seqz")
    {
        if (tokens.size() == 3)
            return {"sltiu", tokens[1], tokens[2], "1"};
        cerr << "Error: seqz requires 2 operands\n";
        return {};
    }
    else if (mne == "snez")
    {
        if (tokens.size() == 3)
            return {"sltu", tokens[1], "x0", tokens[2]};
        cerr << "Error: snez requires 2 operands\n";
        return {};
    }
    else if (mne == "sltz")
    {
        if (tokens.size() == 3)
            return {"slt", tokens[1], tokens[2], "x0"};
        cerr << "Error: sltz requires 2 operands\n";
        return {};
    }
    else if (mne == "sgtz")
    {
        if (tokens.size() == 3)
            return {"slt", tokens[1], "x0", tokens[2]};
        cerr << "Error: sgtz requires 2 operands\n";
        return {};
    }
    return tokens; // Unchanged if NOT a pseudo-instruction
}

bool validReg(const string &tok)
{
    return (regMap.find(tok) != regMap.end());
}

bool parseInstructionLine(const string &line, uint32_t &machineCode, int pc)
{
    // Remove comments:
    string filtered = stripComment(line);
    if (filtered.empty())
        return false;

    // Break filtered into tokens (handles commas, parenthesis, spaces, etc)
    vector<string> tokens = tokenize(filtered);
    if (tokens.empty())
        return false;

    // Expand pseudo-instructions:
    tokens = expandPseudo(tokens);
    string mne = tokens[0];
    if (rMap.find(mne) != rMap.end())
    {
        RSpec spec = rMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[2]) || !validReg(tokens[3]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        uint32_t rs1 = regMap[tokens[2]];
        uint32_t rs2 = regMap[tokens[3]];
        machineCode = encodeR(rd, rs1, rs2, spec);
        return true;
    }
    else if (iAMap.find(mne) != iAMap.end())
    {
        IArithSpec spec = iAMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[2]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        uint32_t rs1 = regMap[tokens[2]];
        uint32_t imm = stoi(tokens[3]);
        machineCode = encodeIArith(rd, rs1, imm, spec);
        return true;
    }
    else if (iSMap.find(mne) != iSMap.end())
    {
        IShiftSpec spec = iSMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[2]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        uint32_t rs1 = regMap[tokens[2]];
        uint32_t shamt = stoi(tokens[3]);
        machineCode = encodeIShift(rd, rs1, shamt, spec);
        return true;
    }
    else if (lMap.find(mne) != lMap.end())
    {
        LSpec spec = lMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[3]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        int32_t imm = stoi(tokens[2]);
        uint32_t rs1 = regMap[tokens[3]];
        machineCode = encodeL(rd, rs1, imm, spec);
        return true;
    }
    else if (sMap.find(mne) != sMap.end())
    {
        SSpec spec = sMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[3]))
        {
            return false;
        }
        uint32_t rs2 = regMap[tokens[1]];
        int32_t imm = stoi(tokens[2]);
        uint32_t rs1 = regMap[tokens[3]];
        machineCode = encodeS(rs1, rs2, imm, spec);
        return true;
    }
    else if (bMap.find(mne) != bMap.end())
    {
        BSpec spec = bMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[2]))
        {
            return false;
        }
        uint32_t rs1 = regMap[tokens[1]];
        uint32_t rs2 = regMap[tokens[2]];
        int32_t imm;

        // Handle label or immediate
        if (labelMap.find(tokens[3]) != labelMap.end())
            imm = labelMap[tokens[3]] - pc;
        else if (isdigit(tokens[3][0]) || tokens[3][0] == '-')
            imm = stoi(tokens[3]);
        else
        {
            cerr << "Unknown label: " << tokens[3] << "\n";
            return false;
        }
        if (imm % 2)
        {
            cerr << "Branch target NOT aligned: " << tokens[3] << "\n";
            return false;
        }
        machineCode = encodeB(rs1, rs2, imm, spec);
        return true;
    }
    else if (jalMap.find(mne) != jalMap.end())
    {
        JALSpec spec = jalMap[mne];
        if (tokens.size() != 3 || !validReg(tokens[1]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        int32_t imm;

        // Handle label or immediate
        if (labelMap.find(tokens[2]) != labelMap.end())
            imm = labelMap[tokens[2]] - pc;
        else if (isdigit(tokens[2][0]) || tokens[2][0] == '-')
            imm = stoi(tokens[2]);
        else
        {
            cerr << "Unknown label: " << tokens[2] << "\n";
            return false;
        }
        if (imm % 2)
        {
            cerr << "Jump target NOT aligned: " << tokens[2] << "\n";
            return false;
        }
        machineCode = encodeJAL(rd, imm, spec);
        return true;
    }
    else if (jalrMap.find(mne) != jalrMap.end())
    {
        JALRSpec spec = jalrMap[mne];
        if (tokens.size() != 4 || !validReg(tokens[1]) || !validReg(tokens[3]))
        {
            return false;
        }
        uint32_t rd = regMap[tokens[1]];
        int32_t imm = stoi(tokens[2]);
        uint32_t rs1 = regMap[tokens[3]];
        machineCode = encodeJALR(rs1, rd, imm, spec);
        return true;
    }
    cerr << "Unknown instruction with the mnemonic \' " << mne << " \'\n";
    return false;
}

void printUsage()
{
    cout << RED << "Usage:\n"
         << RESET;
    cout << BLUE << "  RISC-V_Assembler [-i <inputfile>] [-o <outputfile>]\n\n";
    cout << "Options:\n";
    cout << "  -i <inputfile>   :  Input assembly file (default: assemblyCode.txt)\n";
    cout << "  -o <outputfile>  :  Output machine code file (default: machineCode.txt)\n";
    cout << "  -h --help        :  Show this help message\n"
         << RESET;
}

int main(int argc, char *argv[])
{
    printf(BOLD RED "  RISC-V_Assembler (by anuragmishra-creates)\n" RESET);
    string inputFileName = "assemblyCode.txt", outputFileName = "machineCode.txt";
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

    cout << MAGENTA << "   >>> Assembler Started <<<\n"
         << RESET;
    cout << CYAN << "Input File : " << GREEN << inputFileName << "\n"
         << RESET;
    cout << CYAN << "Output File: " << GREEN << outputFileName << "\n"
         << RESET;

    // Real assembler code starts here:
    int pc = 0;
    vector<string> allLines;
    string line;

    ifstream inFile(inputFileName);
    if (!inFile)
    {
        cerr << "Error : Could NOT open the input file: 'assemblyCode.txt'\n";
        return 1;
    }

    // First pass to find all the labels and their addresses
    while (getline(inFile, line))
    {
        string filtered = stripComment(line);
        if (filtered.empty())
            continue;

        vector<string> tokens = tokenize(filtered);
        if (tokens.empty())
            continue;

        string lineToStore = filtered; // Stored for pass 2
        bool instructionFoundInCurrentLine = true;

        // Check if the line starts with label (end with ':')
        if (tokens[0].back() == ':')
        {
            string label = tokens[0].substr(0, (int)tokens[0].size() - 1);
            labelMap[label] = pc;
            tokens.erase(tokens.begin()); // Remove the label token (like .L2)
            if (tokens.empty())
                instructionFoundInCurrentLine = false;
            else
            {
                size_t colonIndex = filtered.find(":");
                lineToStore = filtered.substr(colonIndex + 1);
            }
        }

        if (instructionFoundInCurrentLine)
        {
            allLines.push_back(lineToStore);
            pc += 4; // Note: labels and empty lines are not given any PC
        }
    }
    inFile.close();

    // Open Output File:
    ofstream outFile(outputFileName);
    if (!outFile)
    {
        cerr << "Error: Could NOT open the output file: 'machineCode.txt'!\n";
        return 1;
    }

    // Second pass to generate the instructions of size 32 bits each:
    pc = 0;
    for (auto &l : allLines)
    {
        uint32_t machineCode;
        if (parseInstructionLine(l, machineCode, pc))
            outFile << bitset<32>(machineCode) << "\n";
        else
            outFile << "# There was some error while converting here.\n";
        pc += 4;
    }
    outFile.close();

    cout << MAGENTA << "\n   >>> Assembler Ended <<<\n"
         << RESET;
    cout << YELLOW << "The Assembly Code has been successfully converted to machine code.\n"
         << RESET;

    return 0;
}
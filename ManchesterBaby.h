//
// Created by 杨睿哲 on 2025-11-26.
//

#ifndef ASSIGNMENT1_MANCHESTERBABY_H
#define ASSIGNMENT1_MANCHESTERBABY_H
#include <vector>
#include <string>
#include <cstdint>
using namespace std;

class ManchesterBaby {
private:
    // addressing modes
    enum AddressingMode {
        DIRECT=0,
        INDIRECT=1,
        IMMEDIATE=2,
        RESERVED=3 // not used
    };
    //instruction set opcodes
    enum Opcode {
        JMP=0,
        JRP=1,
        LDN=2,
        STO=3,
        SUB=4,
        sub=5,
        CMP=6,
        STP=7
    };

    // structure of instructions
    struct instruction {
        Opcode opcode;   // 4-bit opcode
        AddressingMode mode;    // 2-bit addressing mode
        int32_t address;  // 8-bit address
    };
    
    instruction currInst; // current instruction
    uint8_t memorySize = 32; // memory size(32-64)
    vector<int32_t> memory; // 32-bit memory
    int32_t accumulator;    // 32-bit accumulator
    uint8_t pc;           // program counter register
    int32_t pi;              // program instruction register
    bool isTerminated = false; // flag to indicate if the program has terminated
    int32_t toInt(const string& binaryStr); // convert binary string to int32_t
    string toBinary(int32_t value); // convert int32_t to binary string

public:
    ManchesterBaby();
    int getAccumulator() const;
    void setMemorySize(uint8_t size);
    bool getCode(const string& filename);
    int32_t fetch();
    void decode();
    void execute();
    void cycle();
    void getStatus();

};


#endif //ASSIGNMENT1_MANCHESTERBABY_H

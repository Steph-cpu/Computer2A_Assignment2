//
// Created by 杨睿哲 on 2025-11-26.
//

#ifndef ASSIGNMENT1_MANCHESTERBABY_H
#define ASSIGNMENT1_MANCHESTERBABY_H
#include <vector>
#include <string>
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
        LDN=0,
        LOAD=1,
        STO=2,
        ADD=3,
        SUB=4,
        JMP=5,
        JNZ=6,
        STP=63
    };
    
    struct instruction {
        Opcode opcode;   // 6-bit opcode ,操作码
        AddressingMode mode;    // 2-bit addressing mode ,寻址方式
        uint8_t address;  // 24-bit address ,地址码
    };
    
    instruction currInst; // current instruction ,当前指令
    vector<int32_t> memory; // 32-bit memory ,主存储器
    int32_t accumulator;    // 32-bit accumulator ,累加器
    uint8_t pi;           // program instruction register ,程序指令寄存器
    uint8_t memorySize = 32; // memory size ,存储器大小 (扩展为 64 行)
    bool isTerminated = false; // flag to indicate if the program has terminated ,程序是否结束的标志
    int32_t toInt(const string& binaryStr); // convert binary string to int32_t
    string toBinary(int32_t value); // convert int32_t to binary string
public:
    friend void test01();
    ManchesterBaby();
    int getAccumulator() const;
    void setMemorySize(uint8_t size);
    bool getCode(const string& filename); // load machine code from file
    string fetch();
    void decode();
    void execute();
    void cycle();
    // run f-d-e cycle()
    bool isHalted() const;
    void getStatus();

};


#endif //ASSIGNMENT1_MANCHESTERBABY_H

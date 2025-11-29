//
// Created by 杨睿哲 on 2025-11-26.
//

#include "ManchesterBaby.h"
#include <cstdint>
#include <fstream>
#include <iostream>

using namespace std;

//convert 32-bit binary string (little-endian) to signed 32-bit value
int32_t ManchesterBaby::toInt(const string &binaryStr) {
    uint32_t value = 0;
    const size_t n = binaryStr.size();
    for (size_t i = 0; i < n; ++i) {
        char c = binaryStr[i];
        if (c == '1') {
            // set bit i if there is a '1'
            value |= (1u << i);
        }
    }
    return static_cast<int32_t>(value);
}

// convert signed 32-bit value to 32-bit binary string (little-endian)
string ManchesterBaby::toBinary(int32_t value) {
    uint32_t u = static_cast<uint32_t>(value); // work with raw 32-bit pattern
    string result(32, '0');
    for (int i = 0; i < 32; ++i) {
        result[i] = (u & (1u << i)) ? '1' : '0';
    }
    return result;
}

// get the value of accumulator
int ManchesterBaby::getAccumulator() const {
    return accumulator;
}

//constructor: assign every element to 0
ManchesterBaby::ManchesterBaby()
        : currInst{STP, DIRECT, 0},
          accumulator(0),
          pi(0),
          memorySize(32),
          memory(memorySize, 0) {}


// set memory size (32-64)
void ManchesterBaby::setMemorySize(uint8_t size) {
    // check if the size is valid
    if (size<32||size>64) {
        cout<<"Invalid setting! Memory size should be between 32 and 64!"<<endl;
        return;
    }
    memorySize=size;
    memory.assign(memorySize, 0);  //resize memory and initialize to 0
}

// get machine code from file
bool ManchesterBaby::getCode(const string &filename) {
    ifstream file (filename);
    // check if the file is opened successfully
    if (!file.is_open()) {
        cout<<"The file can not be opened!"<<endl;
        return false;
    }
    int count=0;    //count the number of lines read
    string line;    //store each line temporally
    //traverse to get each line of the file
    while (getline(file,line)&&static_cast<uint8_t>(count)<memorySize) {
        if (line.size()!=32) {
            cout<<"Wrong machine code for line "<<count+1<<", which should be 32 bits!"<<endl;
            file.close();
            return false;
        }
        //traverse string to check if it contains only 0 and 1
        for (char c:line) {
            if (c!='0'&& c!='1') {
                cout<<"Wrong machine code for line "<<count+1<<", which should contain only 0 and 1!"<<endl;
                file.close();
                return false;
            }
        }
        memory[count]=toInt(line);
        //cout << "Memory[" << count << "]: " << line << " -> " << toBinary(memory[count]) << endl; 这行测试的时候也可以不用，直接测decode就行了
        line.clear();   //clear the temporary string
        count++;    //update the number of lines read
    }
    file.close();
    return true;
}

// fetch the program instruction
int32_t ManchesterBaby::fetch() {
    // check if pi is out of border
    if (pi>=memorySize) {
        isTerminated=true;
        return 0;
    }
    return memory[++pi];    // fetch the increased pi (starts from 1)
}

void ManchesterBaby::decode() {
    int32_t instruction = fetch();
    if (isTerminated) return;   // if fetch caused termination, exit the program
	// bit 0-7: address (8 bits)
    currInst.address = instruction & ((1 << 8) - 1);
    // bit 8-9: addressing mode (2 bits)
    currInst.mode = static_cast<AddressingMode>((instruction >> 8) & ((1 << 2) - 1));
    // bit 13-16: opcode (4 bits)
    currInst.opcode = static_cast<Opcode>((instruction >> 13) & ((1 << 4) - 1));

    /*
    // Debug output
	cout << instruction << " | ";
    cout << "Instruction: " << toBinary(instruction) << " | ";
    cout << "Opcode: " << currInst.opcode << " | ";
    cout << "Mode: " << currInst.mode << " | ";
    cout << "Address: " << (int)currInst.address << endl;
    */

}

// execute the instruction
void ManchesterBaby::execute() {
    if (isTerminated) return;   // check if the program has already terminated
    int32_t operand = 0;    // operand after addressing mode operation
    // resolve operand based on addressing mode
    switch (currInst.mode) {
        case IMMEDIATE: {
            operand = currInst.address; // immediate addressing: low 8 bits are value
            break;
        }
        case DIRECT: {
            int32_t addr = currInst.address;    // direct addressing: low 8 bits are address
            // check if address is valid
            if (addr < 0 || addr >= memorySize) {
                cout << "Invalid DIRECT address: " << addr << endl;
                isTerminated = true;    // update the termination flag
                return;
            }
            operand = memory[addr];     // fetch the value from memory
            break;
        }
        case INDIRECT: {
            int32_t addr = currInst.address;    // indirect addressing: low 8 bits are address of address
            if (addr < 0 || addr >= memorySize) {
                cout << "Invalid INDIRECT base address: " << addr << endl;
                isTerminated = true;
                return;
            }
            int32_t realAddr = memory[addr];    // fetch the real address from memory
            // check if the real address is valid
            if (realAddr < 0 || realAddr >= memorySize) {
                cout << "Invalid INDIRECT target address: " << realAddr << endl;
                isTerminated = true;
                return;
            }
            operand = memory[realAddr];     // fetch the value from the real address
            break;
        }
        // cases for unsupported and invalid  addressing mode
        case RESERVED:
        default:
            cout << "Invalid addressing mode: " << static_cast<int>(currInst.mode) << endl;
            isTerminated = true;    // terminate the program
            return;
    }

    // execute based on opcode
    switch (currInst.opcode) {
        case LDN:
            accumulator = -operand;     // Load negative to accumulator
            break;
        case sub:
        case SUB:
            accumulator -= operand;     // ACC = ACC - operand
            break;
        case STO: {
            int32_t addr = currInst.address;
            // check if address is valid
            if (addr >= 0 && addr < memorySize) {
                memory[addr] = accumulator;     // store accumulator to address
            } else {
                cout << "Invalid store address: " << addr << endl;
                isTerminated = true;
            }
            break;
        }
        case JMP: {
            int32_t target = operand;
            // check if the target address is valid
            if (target >= 0 && target < memorySize) {
                pi = static_cast<uint8_t>(target); // jump to target address
            } else {
                cout << "Invalid jump target: " << target << endl;
                isTerminated = true;
            }
            break;
        }
        case JRP: {
            int32_t newPi = static_cast<int32_t>(pi) + operand;
            // check if the new pi is valid
            if (newPi >= 0 && newPi < memorySize) {
                pi = static_cast<uint8_t>(newPi); // update pi
            } else {
                cout << "Invalid relative jump target: " << newPi << endl;
                isTerminated = true;
            }
            break;
        }
        case CMP:
            // if accumulator < 0, skip next instruction
            if (accumulator < 0) {
                // check if the next pi is valid
                if (pi < memorySize) {
                    ++pi;
                } else {
                    isTerminated = true;
                }
            }
            break;
        case STP:
            isTerminated = true;    // terminate the program
            break;
        default:
            cout << "Unknown opcode: "
                 << static_cast<int>(currInst.opcode) << endl;
            isTerminated = true;
            break;
    }
}

// run fetch-decode-execute cycle until termination
void ManchesterBaby::cycle() {
    while (!isTerminated) {
        decode();   // call decode() function to decode
        execute();  // call execute() function to execute
    }
}

// print the current status of the Manchester Baby
void ManchesterBaby::getStatus() {
    cout<<"The current status of the Manchester Baby is:"<<endl;
    cout<<"Accumulator: "<<toBinary(accumulator)<<endl;
    cout<<"Program Instruction Register: "<<(int)pi<<endl;
    cout<<(isTerminated?"The program has terminated.": "The program is still running.")<<endl;
    // print the memory
    for (int i=0;i<memorySize;i++) cout<<"Memory["<<i<<"]: "<<toBinary(memory[i])<<endl;
}

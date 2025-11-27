//
// Created by 杨睿哲 on 2025-11-26.
//

#include "ManchesterBaby.h"
#include <fstream>
#include <iostream>

using namespace std;

//convert 32-bit binary string to signed 32-bit value
int32_t ManchesterBaby::toInt(const string &binaryStr) {
    uint32_t value = 0;
    for (char c : binaryStr) {
        value <<= 1;                 // shift left to make room for next bit
        if (c == '1') value |= 1u;   // set lowest bit if current char is '1'
    }
    return static_cast<int32_t>(value);
}

// convert signed 32-bit value to 32-bit binary string
string ManchesterBaby::toBinary(int32_t value) {
    uint32_t u = static_cast<uint32_t>(value); // work with raw 32-bit pattern
    string result(32, '0');
    for (int i = 31; i >= 0; --i) {
        result[i] = (u & 1u) ? '1' : '0';     // set bit based on lowest bit of u
        u >>= 1;                               // move to next bit
    }
    return result;
}

int ManchesterBaby::getAccumulator() {
    return accumulator;
}

//constructor: assign every element to 0
ManchesterBaby::ManchesterBaby() {
    // explicitly use 32-bit zero values
    accumulator = static_cast<int32_t>(0);
    pi = static_cast<uint8_t>(0);
    // ensure memory has memorySize elements, all initialized to 32-bit 0
    memory.assign(memorySize, 0);
}

void ManchesterBaby::setMemorySize(uint8_t size) {
    if (size<32||size>64) {
        cout<<"Invalid setting! Memory size should be between 32 and 64!"<<endl;
        return;
    }
    memorySize=size;
    memory.assign(memorySize, 0); //resize memory and initialize to 0
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
        line.clear();   //clear the temporary string
        count++;    //update the number of lines read
    }
    file.close();
    return true;
}

// fetch the program instruction
string ManchesterBaby::fetch() {
    if (pi>=memorySize) {
        isTerminated=true;
        return string(32,0);
    }
    return toBinary(memory[pi++]);
}

void ManchesterBaby::decode() {
    const string instructionStr=fetch();
    currInst.opcode=static_cast<Opcode>(toInt(instructionStr.substr(0,6)));
    currInst.mode=static_cast<AddressingMode>(toInt(instructionStr.substr(6,2)));
    currInst.address=static_cast<uint8_t>(toInt(instructionStr.substr(8,24)));
}

void ManchesterBaby::execute() {

}

void ManchesterBaby::cycle() {
    while (!isHalted()) {
        decode();
        execute();
    }
}




bool ManchesterBaby::isHalted() {
    return isTerminated;
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

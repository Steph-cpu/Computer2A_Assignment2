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
    vector<int32_t> memory; // 32-bit memory ,主存储器
    int32_t accumulator;    // 32-bit accumulator ,累加器
    uint8_t pi;           // program instruction register ,程序指令寄存器
    const uint8_t memorySize = 32; // memory size ,存储器大小
    bool isTerminated = false; // flag to indicate if the program has terminated ,程序是否结束的标志
    int32_t toInt(const string& binaryStr); // convert binary string to int32_t
    string toBinary(int32_t value); // convert int32_t to binary string
public:
    ManchesterBaby();
    bool getCode(const string& filename); // load machine code from file
    string fetch();
    //decode()
    //execute()
    // run f-d-e cycle()
    void getStatus();

};


#endif //ASSIGNMENT1_MANCHESTERBABY_H
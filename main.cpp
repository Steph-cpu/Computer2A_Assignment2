#include <iostream>
#include "ManchesterBaby.h"
using namespace std;
void test02() {
    ManchesterBaby mb1;
    string file="/Users/yangruizhe/Desktop/BabyTest1-MC.txt";   //换成你自己的文件路径
    mb1.getCode(file);
    mb1.cycle();
    cout<<"Test02 Accumulator: "<<mb1.getAccumulator()<<endl;   //结果应该直接输出1646
}
int main() {
    test02();
    return 0;

}

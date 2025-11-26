#include <iostream>

#include "ManchesterBaby.h"
using namespace std;
void test01() {
    ManchesterBaby mb;
    mb.getStatus();
    string file="/Users/yangruizhe/Desktop/BabyTest1-MC.txt";
    bool res=mb.getCode(file);
    cout<<res<<endl;
    mb.getStatus();
}

int main() {
    test01();
    return 0;
}

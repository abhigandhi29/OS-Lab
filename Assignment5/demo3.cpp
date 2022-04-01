//Group 1
//OS Assignment 5
//19CS10031 - Gandhi Abhishek Rajesh
//19CS10051 - Sajal Chhamunya

#include <iostream>
#include "memlab.h"
using namespace std;
// #include <chrono>
// using namespace std::chrono;



void printNum(Ptr k) {
    startScope();
    int t; getVar(k,t);
    for(int i=0;i<t;i++){
        cout<<i<<" ";
    }cout<<endl;
    endScope();
}

int main() {
    cout<<"Enter a number: \n";
    int k; cin>>k;
    // auto start = high_resolution_clock::now();
    createMem(250 * 1024 * 1024/4);  // 250MB
    startScope();
    
    Ptr t = createVar(Type::INT);
    assignVar(t,k);
    printNum(t);
    endScope();
    // auto stop = high_resolution_clock::now();
    // auto duration = duration_cast<microseconds>(stop - start);
    // cout << duration.count()/1e6 << endl;
    freeMem();
}
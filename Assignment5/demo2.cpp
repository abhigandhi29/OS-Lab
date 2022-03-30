#include <iostream>
#include "memlab.h"
using namespace std;

Ptr fibonacci(Ptr k){
    startScope();
    int t; getVar(k, t);
    if(t==0){
        endScope();
        Ptr final = createVar(Type::INT);
        assignVar(final,1);
        return final;
    }
    t--;
    Ptr temp = createVar(Type::INT);
    assignVar(temp,t);
    //int ans = (t+1)*
    Ptr p = fibonacci(temp);
    int fib;
    getVar(p,fib);
    int ans = fib*(t+1);
    endScope();
    Ptr final = createVar(Type::INT);
    assignVar(final,ans);
    return final;
}

Ptr fibonacciProduct(Ptr k) {
    startScope();
    cout << "creating array of size k" << endl;
    int r;
    getVar(k,r);
    Ptr arr = createArr(r+1, k.type);
    cout << "array created" << endl;
    for (int i = 0; i <= r; i++) {
        startScope();
        Ptr p1 = createVar(Type::INT);
        assignVar(p1,i);
        Ptr ans = fibonacci(p1);
        int fib; getVar(ans,fib);
        //cout<<fib<<endl;
        assignArr(arr,i,fib);
        endScope();
    }
    //freeElem(arr);
    int mul = 1;
    for(int i = 0;i<=r;i++){
        int temp; getArr(arr,i,temp);
        mul *= temp;
        //cout<<temp<<" ";
    }
    //cout<<endl;
    endScope();
    Ptr final_ans = createVar(Type::INT);
    assignVar(final_ans,mul);
    return final_ans;
}

int main() {
    createMem(250 * 1024 * 1024);  // 250MB
    startScope();
    cout<<"Enter a number: ";
    int k; cin>>k;
    Ptr t = createVar(Type::INT);
    assignVar(t,k);
    Ptr mul = fibonacciProduct(t);
    int multi; getVar(mul, multi);
    cout<<"Multiplication is: "<<multi<<endl;
    endScope();
    freeMem();
}
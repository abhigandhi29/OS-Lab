#include <iostream>
#include "memlab.h"
using namespace std;

void randArr(Ptr x, Ptr y) {
    cout << "creating random array" << endl;
    Ptr arr = createArr(50000, x.type);
    cout << "array created" << endl;
    for (int i = 0; i < 50000; i++) {
        startScope();
        int r = rand() % 26;
        Ptr p1 = createVar(Type::INT);
        assignVar(p1, r);
        getVar(p1, r);
        char c = 'a' + r;
        assignArr(arr, i, c);
        endScope();
    }
    freeElem(arr);
}

int main() {
    createMem(250 * 1024 * 1024 /4);  // 250MB
    for (int i = 0; i < 10; i++) {
        cout << "called for " << i << endl;
        startScope();
        Ptr x = createVar(Type::CHAR);
        Ptr y = createVar(Type::CHAR);
        randArr(x, y);
        cout << "done" << i << endl;
        endScope();
        usleep(50 * 1000);
    }
    freeMem();
}
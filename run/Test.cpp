#include<unistd.h>
#include <cstdlib>
#include<stdio.h>
using namespace std;
int main(){
    freopen("test.out","w",stderr);
    system("g++ add.cpp -o add");
}

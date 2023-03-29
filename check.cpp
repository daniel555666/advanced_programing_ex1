#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "stdio.h"
#include "errno.h"
#include "stdlib.h"
#include "unistd.h"
#include <string.h>
#include <iterator>
#include <list>
using namespace std;

#include <iostream>
#include <stack>

int main(){
    list<char *> myList;
    myList.push_back("hey");    
    printf("%s",myList.back());

        
}
#include <iostream>

int main(int argc, char ** argv) 
{
   /*alignas(int)*/ short data[sizeof(int)/2];
   int *myInt = new(data) int;
   *myInt = 34;

   std::cout << *reinterpret_cast<int*>(data) << std::endl;
}

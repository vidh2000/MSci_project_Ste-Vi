#include<iostream>
#include<thread>
#include <string>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <stack>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <chrono>

#include <omp.h>
#include <stdio.h>




int main()
{
    #pragma omp parallel
    printf("Hello from thread %d!\n", omp_get_thread_num());
}

// void myFunc()
// {
//     std::cout << "So, we're in!" << std::endl;
// }

// class F
// {
//     public:
//         void operator()()
//         {
//             for (int i=0; i>-10; i--){
//                 std::cout << "Starting at t1 " << i <<std::endl;
//             }
//         }
// };

// int main()
// {
//     // Oversubsubscription
    
//     unsigned nb_threads = std::thread::hardware_concurrency();
//     std::cout << "Number of threads = " << nb_threads << std::endl;
//     //myFunc();
//     F functor;

//     //std::thread t1(myFunc); // t1 starts running
//     std::thread t1((F())); // t1 starts running (MOST VEXING SYNTAX) 
    
//     //std::thread t1(functor); 
//     // Using Resource Acquisition is Initalization (RAII)
//     // using wrapper class that wraps around the thread t1
//     // wrapper w(t1);


//     try
//     {
//         for (int i=0; i<10; i++)
//         {
//             std::cout << "msg from main: " << i << std::endl;
//         }
//     }
//     catch(...)
//     {
//         t1.join();
//         throw;
//     }
    
//     std::cout << "Parent thread ID" << std::this_thread::get_id() << std::endl; //parent thread ID
//     std::cout << "Chid thread ID" << t1.get_id() << std::endl;

   
    
//     //!! can only detach/join the specific thread only ones
//     //t1.detach(); // t1 will run freely (deamon process)
//     if (t1.joinable())
//     {
//         t1.join(); //crash
//     }

//     return 0;
// }




















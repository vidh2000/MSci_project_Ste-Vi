#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <set>
#include <stack>
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <random>
//#include <boost/asio.hpp>

#include "functions.h"
#include "vecfunctions.h"
//#include "causet.h"
//#include "embeddedcauset.h"
//#include "shapes.h"
//#include "spacetimes.h"



int main(){
std::cout << "Hello World" << std::endl;

std::set<int> s1 = {1,2,3,4};
std::set<int> s2 = {2,4};

std::set<int> s = set_intersection(s1,s2);
print_set(s);

std::vector<int> a;
a = distinct_randint1(100,3);
print_vector(a);

/*
int DIM = 4;
int N = 10000;
vector<vector<double>> coords = generate_2Dvector(N,DIM,0,2);
//std::cout << "This file works" << std::endl;

auto start = high_resolution_clock::now();

Causet c(coords,"cmatrix");

auto stop = high_resolution_clock::now();
double duration = duration_cast<microseconds>(stop - start).count();
std::cout << "Time taken by function in D=" << DIM << ": "
         << duration/pow(10,6) << " seconds" << std::endl;
*/

};  

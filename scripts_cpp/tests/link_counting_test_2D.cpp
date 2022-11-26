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

#include "../causets_cpp/sprinkledcauset.h"
#include "../causets_cpp/shapes.h"
#include "../causets_cpp/spacetimes.h"

#include "../causets_cpp/functions.h"
#include "../causets_cpp/vecfunctions.h"

using namespace std::chrono;


// Sprinkle into 2D into a cube by default.. Other files will do other dims etc.

int card = 1000;
int dim = 3;
double edge = 4;
std::vector<double> center (dim, edge/2);
std::vector<double> edges = {1,2,3,4};
double radius = 3.0;
double myduration = 3;

// Parameters
bool poisson = false;
bool make_matrix = true;
bool special = false;
bool use_transitivity = true;
bool make_sets = false;
bool make_links = true;
const char* sets_type = "future";



int main(){

auto start = high_resolution_clock::now();

std::vector<const char*> names = {"cube"};

for (const char* name : names)
{
    std::cout<<"\n\n============= Sprinkling into "<<name<<" ====================\n";
    std::cout << "N = " << card << ", dim = " << dim << "\n" << std::endl;

    CoordinateShape shape(dim,name,center,radius,edge,edges,0.0,myduration);
    Spacetime S = Spacetime();
    S.BlackHoleSpacetime(dim);
    SprinkledCauset C(card, S, shape, poisson,
                        make_matrix, special, use_transitivity,
                        make_sets, make_links,sets_type);
    
    // // SAVE CAUSET (only sets_type="all with links" works)
    // std::cout << "Generated the causet... Saving ->" << std::endl;
    // std::string path_file_str = "../../data/blackhole"+std::to_string(dim)
    //                            +"D_N"+std::to_string(card)+".txt";
    // const char* path_file = path_file_str.c_str();
    // C.save_causet(path_file);

    // Count links
    double t_f = edge;
    int N_links = C.count_links(t_f);
    std::cout << "Number of links over horizon = " << N_links << std::endl;
}

auto stop = high_resolution_clock::now();
    double duration = duration_cast<microseconds>(stop - start).count();
    std::cout << "Time taken: "
            << duration/pow(10,6) << " seconds" << std::endl;
}
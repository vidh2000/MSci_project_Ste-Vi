/// \authors Vid Homsak, Stefano Veroni
/// \date 31/10/2022

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

//using namespace std::chrono;


#include "functions.h"
#include "MyVecFunctions.h"
#include "causet.h"
#include "embeddedcauset.h"
#include "sprinkledcauset.h"
#include "shapes.h"
#include "spacetimes.h"

using std::vector;
using std::set;
using std::unordered_set;

// For Vid (can"t locate headers for some reason)
// Path: D:\Documents\Sola\Imperial College London\Year 4\MSci project\
            Project\causets_code\causets_cpp\"header".h...


/**
 * @brief Construct a new Sprinkled Causet object. 
 * 
 * @param card: int, number of events to sprinkle
 * @param spacetime: Spacetime.
 * @param shape: CoordinateShape.
 * @param poisson: bool, if true card is used as average of a Poisson
 * distribution to get a new cardinality, otherwise card is used.
 * @param make_matrix: bool, if true make matrix.
 * @param special: bool, if true (and make_matrix) make special Cmatrix
 * @param use_transitivity: bool, if true use also transitivity to establish
 * causality relations. 
 * @param make_sets: bool, if true make set (see sets_type)
 * @param make_links: bool, if true make links (see sets_type)
 * @param method: const char* specifying
 * - "intensity": card is the average of Poisson distribution used to extract 
 * number of events to sprinkle
 * - "card": the number of events to sprinkle is fixed to card
 * @param sets_type: const char* specifying the type of set:
 * - "past": make _past_links
 * - "future": make _future_links
 * @param seed: int, for coordinate generation.
 */
SprinkledCauset::SprinkledCauset(int card,
                                 Spacetime spacetime, 
                                 CoordinateShape shape, 
                                 bool poisson,// = false,
                                 bool make_matrix,// = true,
                                 bool special,// = false,
                                 bool use_transitivity,// = true,
                                 bool make_sets,// = false,
                                 bool make_links,// = false,
                                 const char* sets_type,// = "past",
                                 int seed)// = 0)
{
    if (poisson)
        {_intensity = card*1;}
    
    _coords = sprinkle_coords(card, shape, seed);
    _size = _coords.size();
    _spacetime = spacetime;
    _shape = shape; 
    
    this->sort_coords(0, false);
    if (make_matrix)
        {this->make_cmatrix("coordinates", special, use_transitivity,
                                make_sets, make_links, sets_type);}
    
    else if (make_sets)
        {
            if (make_links)
            {
                if (sets_type == "past")
                    {this->make_all_pasts("coordinates");}
                else if (sets_type == "future")
                    {this->make_all_futures("coordinates");}
            }
            else
            {
                if (sets_type == "past")
                    {this->make_pasts("coordinates");}
                else if (sets_type == "future")
                    {this->make_futures("coordinates");}
            }
        }

    else if (make_links)
        {
            if (sets_type == "past")
                    {this->make_past_links("coordinates");}
            else if (sets_type == "future")
                {this->make_fut_links("coordinates");}   
        }

    else
        {
            throw std::invalid_argument("At least one among make_matrix, \
                                    make_sets and make_links must be true");
        }
}



/**
 * @brief Determines number of points to sprinkle via Poisson distribution or
 *        set as a constant integer and calls the sprinkle_coords method with
 *        the given parameters.
 * 
 * @param count : number (or average) number of points to sprinkle.
 * @param shape : CoordinateShape where to sprinkle
 * @param poisson : bool, if true count is average of Poisson distribution
 * from which get the effective count
 * @param seed : int, default is no seed.
 * @return vector<vector<double>> coordinates, with ith entry being coordinates
 * of ith point.
 */
vector<vector<double>> SprinkledCauset::sprinkle( int count, 
                                                    CoordinateShape shape,
                                                    bool poisson,// = false,
                                                    int seed)// = 0)
{
    if (count<0)
        {   throw std::invalid_argument(
                "The sprinkle cardinality has to be a non-negative integer.");
        }
    if (poisson)
        {
            std::default_random_engine generator;
            std::poisson_distribution<int> distribution(count);
            count = distribution(generator);
        }
    vector<vector<double>> coords = SprinkledCauset::sprinkle_coords
                                                      (count,shape,seed); 
    return coords;
}   


/**
 * @brief Generate or ~sprinkle~ "count" coordinates into some "shape"
 * 
 * @param count : fixed number number of points to sprinkle.
 * @param shape : CoordinateShape where to sprinkle
 * @param seed : int, default is no seed.
 * @return vector<vector<double>> coordinates, with ith entry being coordinates
 * of ith point.
 */
vector<vector<double>> SprinkledCauset::sprinkle_coords(int count, 
                                                    CoordinateShape shape,
                                                    int seed)// = 0)
{
    if (count < 0)
    {   throw std::invalid_argument(
            "The sprinkle cardinality has to be a non-negative integer.");
    }
    vector<vector<double>> coords (count);

    // Define mersenne_twister_engine Random Generator (with seed)
    if (seed==0)
        {std::random_device rd;
        seed = rd();}  
    std::mt19937 gen(seed);

    
    if ((shape._name == "cube") || (shape._name == "cuboid"))
    {
        // vector<double> low; "
        // vector<double> high;

        //Generate Coordinates
        for (int i = 0; i<shape._dim; i++)
        {
            double low  = shape._center[i] - shape.Edges()[i]/2;
            double high = shape._center[i] + shape.Edges()[i]/2;
            std::uniform_real_distribution<> dis(low,high);
            for (int j=0; j<count; j++)
            {
                coords[i][j] = dis(gen);
            }
        }
    }

    else if ((shape._name == "ball") || (shape._name == "cylinder") ||
             (shape._name == "diamond") || (shape._name == "bicone"))
    {
        // Create circle based sprinkle:
        bool isCylindrical = shape._name == "cylinder";
        bool isDiamond =(shape._name=="diamond")||(shape._name=="bicone");

        int d = shape._dim;
        double R = shape.Parameter("radius");
       
        if (d==2 && isDiamond)
        {
            //pick "count" random coordinate tuples uniformly:
            vector<vector<double>> uv;
            std::uniform_real_distribution<> dis(-1.0,1.0);
            for (int i = 0; i<count; i++)
            {
                double u = dis(gen);
                double v = dis(gen);
                coords[i][0] = (u + v)*R/2;
                coords[i][1] = (u - v)*R/2;
            } 
        }

        else
        {
            int n_different = (shape._name == "ball")? 0 : 1;
            int n_rad = d - n_different;
            
            if (isCylindrical)
            {   
                //Get time done separately
                double t_low = shape._center[0]-shape.Parameter("duration")/2;
                double t_high= shape._center[0]+shape.Parameter("duration")/2;
                std::uniform_real_distribution<> cyltime(t_low, t_high);
                for (int i = 0; i<count; i++)
                    {coords[i][0] = cyltime(gen);} 
            }

            // prepare coordinates and scaling factor, where to accumulate all
            // scaling factors to apply at the end
            double r_scaling = 0;
            std::uniform_real_distribution<> uni(0, 1);
            for (int i = 0; i<count; i++)
            {
                //1)First generate random numbers on sphere of radius 1
                std::normal_distribution<> gauss(0,1);
                double r2 = 0;
                for (int a = n_different; a<n_rad; a++)
                {
                    double x_a = gauss(gen);
                    coords[i][a] = x_a;
                    r2 += x_a*x_a;
                }
                // 1/r2 brings the points on a sphere of radius 1
                r_scaling = 1/r2;
                // 2)Scale points radially inside n_rad-ball
                r_scaling *= std::pow(R, 1/n_rad);
                if (isDiamond)
                {
                    //Set time properly
                    double k = uni(gen);
                    int t_sign = 1;
                    double droot_k;
                    if (k<0.5)
                    {
                        t_sign = -1;
                        droot_k = std::pow(k*2, 1/d); 
                    }
                    else if (k>=0.5)
                    {
                        droot_k = std::pow((k-0.5)*2, 1/d); 
                    }
                    r_scaling *= droot_k;
                    coords[i][0] = t_sign * (1-droot_k) * R;
                }
                for (int a = n_different; a<n_rad; a++)
                    {coords[i][a] *= r_scaling;}
            }
        }
    }
    return coords;
}




int main(){
    std::cout << "sprinkledcauset.cpp WORKS! :)";
}



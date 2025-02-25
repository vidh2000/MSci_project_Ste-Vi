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
#include <string.h>
#include <vector>
#include <chrono>
#include <unordered_set>
#include <random>
#include <stdint.h>

#include "functions.h"
#include "vecfunctions.h"
#include "causet.h"

using std::vector;
using std::set;
using std::unordered_set;

/**
 * @brief   Find the ordering fraction of an interval: 
            The ratio of actual relations over possible ones in such interval.
 * 
 * @param A Causet : the entire interval. 
 * @param mode string : Use as denominator:
- 'choose' -> |A|(|A|-1)/2, i.e. |A| choose 2 (Default).
- 'n2'     -> (|A|^2)/2.
 *            
 * @return  Ordering fraction of Alexandrov Interval. 
 * This is nrelations / (N choose 2)
 */
double Causet::ord_fr(Causet & A, const char* denominator) // = "choose"
{
    if (A._CMatrix.size())
    {
        return Causet::ord_fr(A._CMatrix, denominator);    
    }
    else if (A._pasts.size())
    {
        return Causet::ord_fr(A._pasts,denominator);
    }
    else if (A._futures.size())
    {
        return Causet::ord_fr(A._futures,denominator);
    }
    else{
        const char* errormsg = "Provided Causet is empty! It has no _CMatrix \
                                nor pasts-future sets. Size matters... ;)";
        std::cout << errormsg << " Returning ord_fr = 0.0" << std::endl;
        return 0.0;
    }
}

/**
 * @brief   Ordering fraction determined from a CMatrix.
 *          See above description above all ord_fr definitions
 */
double Causet::ord_fr(vector<vector<int>>  & M,
                        const char* denominator)// = "choose",
{
    if (strcmp(denominator, "choose")!=0 || strcmp(denominator, "n2")!=0)
    {
        std::cout<<"Param 'denominator' must be 'choose' or 'n2'"<<std::endl;
        throw std::invalid_argument("Param 'denominator' must be \
                                'choose' or 'n2'");
    }
    int N = M.size();
    int nrelations = 0;
    for (int j; j<N; j++){
        for (int i; i<j;j++){
            if (M[i][j] != 0){
                nrelations +=1;
            }
        }
    }
    double fr = 2 * nrelations/ (N* (N - (strcmp(denominator,"choose")==0)));
    return fr;
}

/**
 * @brief   Ordering fraction determined from sets of pasts (or also futures).
 *          See above description above all ord_fr definitions. 
 */
template<typename SET>
double Causet::ord_fr(vector<SET> & A_pasts,
                const char* denominator)// = "choose",
{
    if (strcmp(denominator,"choose")!=0 || strcmp(denominator,"n2")!=0)
    {
        std::cout<<"Param 'denominator' must be 'choose' or 'n2'"<<std::endl;
        throw std::invalid_argument("Param 'denominator' must be \
                                'choose' or 'n2'");
    }
    int N = A_pasts.size();
    int nrelations = 0;
    for (auto e_i : A_pasts)
    {
        nrelations += e_i.size();
    }
    double fr = 2 * nrelations/ (N* (N - (strcmp(denominator,"choose")==0)));
    return fr;
}

/**
 * @brief   Find the ordering fraction of an interval between a and b: 
            the ratio of actual relations over possible in such interval.
 * 
 * @param a: integer "causetevent", a<b wanted
 * @param b: integer "causetevent", a<b wanted
 */
double Causet::ord_fr(int a, int b,
                const char* denominator, // = "choose"
                bool from_matrix) // = true
{
    if (strcmp(denominator, "choose")!=0 && strcmp(denominator, "n2")!=0)
    {
        std::cout << "Param 'denominator' was " << denominator << std::endl; 
        std::cout<<"Param 'denominator' must be 'choose' or 'n2'"<<std::endl;
        throw std::invalid_argument("Param 'denominator' must be \
                                    'choose' or 'n2'");
    }
    
    if (a>b){
        return ord_fr(b, a, denominator, from_matrix);
    }

    double nrelations = 0;
    double N;
    bool have_sets = _pasts.size() && _futures.size();

    if (_CMatrix.size() && (from_matrix || !have_sets))
    {
        if (_CMatrix[a][b] == 0){
            return 0.;
        }

        // Get all elements in the Alexander interval [A,B]
        std::vector<int> A = {a};

        // Find all elements in the past of a and future of b
        for (int i = a; i<b;i++)
        {
            if ((_CMatrix[i][b] != 0) && (_CMatrix[a][i] != 0)){
                A.push_back(i);}
        }
        // Also append b
        A.push_back(b);
        // N = size of Alexander interval
        N = A.size();
        for (int i: A)
        {
            for (int j: A)
            {
                if (_CMatrix[i][j] != 0){
                    nrelations +=1;}
            }
        }
    }      

    else if (have_sets)
    {
        if (_futures[a].find(b)!=_futures[a].end()){
        return 0;
        }

        std::cout << "In pasts and futures ord_fr\n";
        std::unordered_set<int> A = set_intersection(_futures[a],_pasts[b]);
        A.insert(a);
        A.insert(b);
        N = A.size();
        for (auto e: A){
            //std::cout << "add " << (set_intersection(_futures[e],A)).size() << std::endl;
            nrelations += (set_intersection(_futures[e],A)).size();
        }
    }
    else
    {
        const char* errormsg = "Provided Causet is empty! It has no _CMatrix \
                                nor pasts-future sets. Size matters... ;)";
        std::cout << errormsg << " Returning ord_fr = 0.0" << std::endl;
        return 0.0;
        }

    double fr = 2 * nrelations / (N * (N - (strcmp(denominator,"choose")==0)));
    return fr;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// MM DIMENSION ESTIMATOR
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

double Causet::MM_drelation(double d)
    /*
    Dimension function to be solved for with
    order fraction for MM-dimension estimation.
    */
{
    double a = std::tgamma(d+1);
    double b = std::tgamma(d/2);
    double c = 4* std::tgamma(3*d/2);
    return a*b/c;
}

/**
 * @brief Use Myrheim-Meyers dimensional estimator to compute the 
          fractal dimension (not necesseraly int).
          Only intended to work with a Causet with
          _pasts and _futures vector<set<int>> objects existing. Could also
          work with CMatrix. Doesn't work for spacetime of dimnsion >=10.
        
 * 
 * @param method: str 
            - 'random': randomly sample.
            - 'big': take all events with no past, all with no future
                     and apply estimator ro their combinations.
 * 
 * @param Nsamples: int 
            Times to iterate procedure to then average on if method "random"
            or, also, if it is big.
            Default is 20.
 * 
 * @param size_min: int\n
            Minimum size of Alexandrov Sets on which you apply estimators.
            Default is 20 elements.
 * 
 * @param size_max: int\n
            Maximum size of Alexandrov Sets on which you apply estimators.
            Default and highly recommended is 1e9.
 * 
 * @return
        - dimension estimate: float
        - dimension std: float
 */
vector<double> Causet::MMdim_est(const char* method,// = "random",
                                int Nsamples,// = 20,
                                int size_min,// = vecmin({1000,_size/2})
                                double size_max,// = 1e9
                                bool from_matrix) //true
{
    // std::cout << "NOTE (not error, chill): MMd works only in " <<
    //                          "flat spacetime" << std::endl;

    // Variables to be used
    int* N = &_size;
    vector<double> destimates;

    if (strcmp(method,"random")==0)
    {
        int fails = 0;
        int successes = 0;
        while (Nsamples>0)
        {
            if (fails>= 100000 && successes == 0)
            {
                std::cout << "Found 0/100000 OK Alexandrov intervals. \
Causet portion too small. Returning {-1,-1} values.";
                vector<double> returnerr = {-1,-1};
                return returnerr;
            }
            // Define mersenne_twister_engine Random Gen. (with random seed)
            std::random_device rd;
            int seed = rd();
            std::mt19937 gen(seed);
            std::uniform_real_distribution<> dis(0,*N);
            // Pick two random elements
            int e1 = (int) dis(gen), e2 =(int) dis(gen);
            int a; int b;
            if (e1 == e2){
                fails += 1;
                continue;
            }
            else if (e1 < e2){
                a = e1;
                b = e2;
            }
            else if (e1>e2){
                a = e2;
                b = e1;
            }
            else{
                fails += 1;
                continue;
            }
            
            int n = IntervalCard(a, b);
            if (n >= size_min && n<= size_max) 
            {
                successes += 1;
                double fr_i = this->ord_fr(a,b,"choose",from_matrix);
                //std::cout << "fr_i = " << fr_i << std::endl;
                if (fr_i ==1)
                {
                    destimates.push_back(1);
                    Nsamples --;
                }
                else
                {
                    //Order fraction correction for MMestimator
                    double fr_i = fr_i * (n-1)/n;

                    // Define function whose root needs to be found
                    auto MM_to_solve = [fr_i](double d){
                        return Causet::MM_drelation(d) - fr_i/2;};

                    double dmin = 0.75;
                    double dmax = 10;
                    // Estimate dimension of Causet
                    double d_i = bisection(MM_to_solve,dmin,dmax);
                    destimates.push_back(d_i);
                    Nsamples --;
                }
            }
            else
            {
                fails +=1;
                continue;
            }
        }
    }

    else if (strcmp(method, "big")==0)
    {
        vector<int> As = {};
        vector<int> Bs = {};
        // for (int e = 0; e<*N; e++)
        // {
        //     if (_pasts[e].size() == 0){
        //         As.push_back(e);
        //     }
        //     else if (_futures[e].size() == 0){
        //         Bs.push_back(e);
        // }
        for (int e = 0; e<*N; e++)
        {
            // Find if e is maximal element
            bool edge_element = true;
            for (int j=e+1; j<*N; j++)
            {
                if (_CMatrix[e][j]!=0){
                    edge_element = false;
                    break;}
            }
            if (edge_element) {
                 Bs.push_back(e);
            }
            else {
                // Find if e is minimal element
                edge_element = true;
                for (int i=0; i<e; i++){
                    if (_CMatrix[i][e]!=0){
                        edge_element = false;
                        break;
                        }
                    }
                if (edge_element) {
                    As.push_back(e);
                }
            }
        }
        int counter = 0;
        for (int i=0; i<As.size(); i++)
        {
            if (counter > Nsamples) break;
            for (int j=Bs.size()-1; j>-1; j--)
            {   
                int a = As[i];
                int b = Bs[j];
                if (a == b) continue;
                double n = (double)IntervalCard(a, b)*1.0;
                //std::cout << "n = " << n << std::endl;
                if (n >= size_min && n<= size_max) 
                {
                    counter++;
                    double fr_i = this->ord_fr(a,b,"choose",from_matrix);
                    //std::cout << "fr_i =" << fr_i << std::endl; 
                    if (fr_i ==1.0){
                        destimates.push_back(1);
                    }
                    else
                    {
                        //Order fraction correction for MMestimator
                        fr_i *= ((double)n-1.0)/(double)n;

                        // Define function whose root needs to be found
                        auto MM_to_solve = [fr_i](double d){
                            return Causet::MM_drelation(d) - fr_i/2;};

                        double dmin = 0.9;
                        double dmax = 10;
                        // Estimate dimension of Causet
                        double d_i = bisection(MM_to_solve,dmin,dmax);
                        //std::cout << "dim est = " << d_i << std::endl;
                        if (d_i != dmin && d_i != dmax)
                        destimates.push_back(d_i);
                    }
                }
            }
        }
    }

    else
    {
        const char* errormsg = "'method' parameter must be 'random' or 'big'";
        std::cout<<errormsg<<std::endl;
        throw std::invalid_argument(errormsg);
    }

    // Return mean and std of dimension estimate result
    double sum = std::accumulate(std::begin(destimates),
                                 std::end(destimates), 0.0);
    double mean = sum/destimates.size();

    double accum = 0.0;
    std::for_each(std::begin(destimates), std::end(destimates),
                    [&](const double d)
                    {
                    accum += (d - mean) * (d - mean);
                    });

    double stdev = sqrt(accum / (destimates.size()));
    vector<double> result = {mean, stdev};
    return result;
    
}   
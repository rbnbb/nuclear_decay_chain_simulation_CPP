#ifndef NUCLIDE_H
#define NUCLIDE_H

#include<cctype>
#include<iomanip>
#include<fstream>
#include<iostream>
#include<random>
#include<sqlite3.h>
#include<vector>
#include<unordered_map>
#include<exception>
#include<algorithm>

struct event_t{ //holds info on radioactive decay event
  double t; //time elapsed in seconds from begging of simulation
  double energy;
  std::string info; //human-readable description of event (ex:" At t=?   235U ----> 231Th  +  4He  4.678 Mev)
};

void sort_events_chrnologically(std::vector<event_t> & events);

//represents a nuclide and contains related info
//can call SQLite3 database to update info after a decay
class Nuclide {
  int t_Z,t_A;  // Z and A numbers that describe the nuclide
  double t_weight, t_half_life; //weight in atomic mass units and half-life in seconds
  bool t_is_stable;
  std::string t_symbol,t_decay_mode;
  double t_time=0.0;  //the time elapsed from the beginning of the simulation in seconds
  sqlite3* DB; //SQLite3 database object
  std::mt19937_64 gen;   //Mersweene Twister pseudorandom number generator. It has a period of 2^19937-1. It returns integers.
  std::uniform_real_distribution<> dis; //takes random integers from mt19937 and returns a real number in a specified interval
  std::vector<event_t> *t_events; //for storing decay events history, taken as argument by the class constructor

  //struct used strictly for extracting data from SQLite3 database nuclides.db
  struct db_extract_t{ 
      int Z,A;
      bool is_stable;
      double half_life;
      double weight; // weight in atomic mass units
      std::string symbol,decay_mode;
  };
  //struct that describes the decay modes of a nuclide
  struct decay_mode_t{
    std::string decay_type;
    double branch_frac; //probability that the isotope should decay using current mode
    double Q; //Q energy value in MeV
  };

  public:  
  Nuclide(int Z,int A,std::vector<event_t>* events);
  std::string get_name() const{ 
    return std::to_string(t_A)+t_symbol;
  }
  void decay_chain(); //runs the decay simulation for current nuclide until a stable element is reached by repeated calls to Nuclide::m_decay()

  private:
  void m_update(int Z,int A);//function that assigns class members good values by querying database
  bool m_decay(); //runs single decay. Returns 1 if decay product is stable and 0 if not
  std::string m_decay_info(const double Q,std::string parent,std::string products);
  double m_time_from_half_life(double half_life); //generates a random time (in seconds) for disintegration according to exponential distribution of given half-life
  std::vector<decay_mode_t> m_decay_modes();//returns decay_mode information processed from Nuclide::t_decay_mode string
  std::string m_format_time(double t);//takes time in seconds and return human readable std::string
  static int m_sql_callback(void* data_name, int num_cols, char** col_values, char** col_names);//function taken as argument by sqlite3_exec(). Structure explained in sqlite3 documentation
};

#endif

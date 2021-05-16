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

struct event_t{ //holds info on radioactive decay
  double t;
  double energy;
  std::string info; 
};

class Nuclide {
  int t_Z,t_A;
  double t_weight, t_half_life; //weight in atomic mass units and half-life in seconds
  bool t_is_stable;
  std::string t_symbol,t_decay_mode;
  double t_time=0.0;  //the time elapsed from the beginning of the simulation in seconds
  sqlite3* DB; //SQLite3 database object
  std::mt19937_64 gen;   //Mersweene Twister pseudorandom number generator. It has a period of 2^19937-1. It returns integers.
  std::uniform_real_distribution<> dis; //takes random integers from mt19937 and returns a real number in a specified interval
  std::vector<event_t> *t_events; //for storing decay events information, taken as argument by the class constructor
  //structure used strictly for extracting data from SQLite database
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
    double branch_frac;
    double Q;
  };

  public:  
  Nuclide(int Z,int A,std::vector<event_t>* events);
  std::string name() const{
    return std::to_string(t_A)+t_symbol;
  }
  void decay_chain(); //runs the decay simulation until a stable element is reached

  private:
  static int m_sql_callback(void* data_name, int num_cols, char** col_values, char** col_names);//function taken as argument by sqlite3_exec(). Structure explained in sqlite3
  void m_update(int Z,int A);//function that assigns class members good values by querying database
  double m_time_from_half_life(double half_life); //generates a random time for disintegration according to exponential distribution of given half-life
  
  std::string m_format_time(double t);//takes time in seconds and return human readable std::string
  std::string m_decay_info(const double Q,std::string parent,std::string products);
  bool m_decay();
};

#endif

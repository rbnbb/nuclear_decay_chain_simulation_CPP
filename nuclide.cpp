#include<iomanip>
#include<fstream>
#include<iostream>
#include<random>
#include<sqlite3.h>
#include <vector>
using namespace std;
//simplify checking state of the program for undefined behaviour
void xassert(bool all_is_good_condition, std::string error_message){
  if(!all_is_good_condition){
      std::cerr<<error_message<<std::endl;
      std::terminate();
  } 
}

//holds multiple events
struct event{
  double t;
  double energy;
  string info; 
};
class Nuclide {
  int t_Z,t_A;
  double t_weight, t_half_life; //weight in atomic mass units and half-life in seconds
  bool t_is_stable;
  string t_symbol,t_decay_mode;
  double t_time=0.0;  //the time elapsed from the beginning of the simulation in seconds
  sqlite3* DB; //SQLite3 database object
  ofstream log = ofstream("nuclide_history.out"); //write nuclide change to a file
  std::mt19937_64 gen;   //Mersweene Twister pseudorandom number generator it has a period of 2^19937-1. It returns integers
  std::uniform_real_distribution<> dis; //takes random integers from mt19937 and returns a real number in a specified interval
  //structure used strictly for extracting data from SQLite database
  struct db_extract_t{ 
      int Z,A;
      bool is_stable;
      double half_life;
      double weight; // weight in atomic mass units
      string symbol,decay_mode;
  };
  //struct that describes a decay mode.
  struct decay_mode_t{
    string decay_type;
    double branch_frac;
    double Q;
  };
  /*function taken as argument by sqlite3_exec(). Structure explained in sqlite3
  documentation. data_name is used to assign query response to external
  varables c-style; num_cols= number of columns of the table; col_values =
  pointer (~used as array) to c-style string for table entries; col_names
  pointer to c style string of column names. */
  static int m_sql_callback(void* data_name, int num_cols, char** col_values, char** col_names)
  {
      char space_char=' ';
      //going through col_values and ensuring there are no null pointers that could cause crashes.
      for (int i = 0; i < num_cols; i++) {
          /* printf("col_values: %s ; col_names: %s\n",col_values[i],col_names[i]); */
          /* printf("%s = %s\n", col_names[i], col_values[i] ? col_values[i] : "NULL"); */
          if(col_values[i]==NULL){
              col_values[i]=&space_char;
          }
      }
      //recovering struct data from outside the function using the argument data_name
      db_extract_t* nuclide_dat=(db_extract_t*) data_name;
      //assigning values to all nuclide_dat members using the query result
      nuclide_dat->Z=atoi(col_values[0]);  
      nuclide_dat->A=atoi(col_values[1]);  
      nuclide_dat->symbol=string(col_values[2]);  
      nuclide_dat->weight=atof(col_values[3]);
      nuclide_dat->is_stable=bool(atoi(col_values[4]));
      nuclide_dat->half_life=atof(col_values[5]);
      nuclide_dat->decay_mode=string(col_values[6]);  
      //print for debugging
      /* printf("num_cols: %i\n",num_cols); */
      /* printf("%i, %i, %f, %i, %f, \n",nuclide_dat->Z,nuclide_dat->A,nuclide_dat->weight,nuclide_dat->is_stable,nuclide_dat->half_life); */
      /* printf("%s, %s, %s, %s \n",nuclide_dat->symbol.c_str(),nuclide_dat->betam.c_str(),nuclide_dat->betap.c_str(),nuclide_dat->alpha.c_str()); */
      return 0;
  }
  public:  
  Nuclide(int Z,int A) {
    if(!(0<Z&&Z<118&&0<A&&A<294)){
      printf("(Z=%i,A=%i) is invalid input for Nuclide constructor\n Database query aborted\n",Z,A);
    }
    else {
      int rc=0; //return code
      rc=sqlite3_open("nuclides.db",&DB);
      xassert(rc==SQLITE_OK,"in Nuclide constructor: Error opening DB");
      m_update(Z,A);
      log<<"At t="<<setw(14)<<t_time<<"s   "<<A<<t_symbol<<'\n';
      //setting up random number generation 
      std::random_device rd; 
      gen= mt19937_64(rd());   //seeding mersweene twister engine with a random_device seed
      dis=std::uniform_real_distribution<>(0.0,1.0); //initializing uniform distribution dis
    }
  }
  /*prints info on class state*/
  void info(){
    printf("%i%s: Z=%i, A=%i, m=%f u, is_stable=%i, hl=%f, decays=%s\n",t_A,t_symbol.c_str(),t_Z,t_A,t_weight,t_is_stable,t_half_life,t_decay_mode.c_str());
  }
  double rand_num(){
    return dis(gen);
  }
  void decay_chain(){
    bool is_decay_chain_over=m_decay();
    int i=0;
    while(!is_decay_chain_over&&i<100){
      is_decay_chain_over=m_decay();
      i++;
    }
    xassert(i<100,"in Nuclide::decay_chain(): more than 100 calls to m_decay. Terminating...");
    log<<"====end====\n\n";
  }
  private:
  //function that assigns class members good values by querying database
  void m_update(int Z,int A){ 
    string sql_instruction="SELECT * FROM nuclides WHERE Z="+to_string(Z)+" AND A="+to_string(A); 
    db_extract_t data;
    int rc = sqlite3_exec(DB, sql_instruction.c_str(), m_sql_callback, &data, NULL); //database  query with sqlite3_exec
    xassert(rc==SQLITE_OK,"in Nuclide::m_update(): SELECT operation failed "); 
    t_Z=data.Z; t_A=data.A; 
    t_is_stable=data.is_stable;
    t_symbol=data.symbol;
    t_decay_mode=data.decay_mode;
    t_weight=data.weight;
    t_half_life=data.half_life;
  }
  //takes time in seconds and return human readable string
  string m_format_time(double t){
    int secs_in_year=30672000;
    int secs_in_day=86400;
    int secs_in_hour=3600;
    if(t>secs_in_year){ //if more than a year
      int years=t/(double)secs_in_year; 
      return to_string(years)+"y";
    }else if(t>secs_in_day){
      int days=t/(double)secs_in_day; 
      t-=days*(double)secs_in_day;
      int hours=t/(double)secs_in_hour; 
      return to_string(days)+"d "+to_string(hours)+"h";
    }else if(t>secs_in_hour){
      int hours=t/(double)secs_in_hour; 
      t-=hours*(double)secs_in_hour;
      int min=t/60.0;  
      return to_string(hours)+"h "+to_string(min)+"min";
    }else if(t>0.5){
      return to_string(t)+"s";
    }else {
      return to_string(t*1000)+"ms";
    } 

  }
  /* //generate random time of disintegration according to exponential
   * distribution with half-life specified in argument */
  double m_time_from_half_life(double half_life){
    xassert(half_life!=0,"in Nuclide::m_time_from_half_life(): half_life=0");
    double lambda=0.6931471805599453/half_life; //lambda=ln(2)/t_{1/2}=decay constant
    std::exponential_distribution<> d(lambda); //using std class for generating the numbers
    double t=d(gen);//time in seconds
    return t;
  }
  bool m_decay(){
    if(t_is_stable){  //stable element
      log<<t_A<<t_symbol<<" is stable.\n";
      return 1; //decay chain ends at stable isotope
    }
    else if(t_half_life>15336000000){  //half-life longer than 500 years in seconds
      log<<t_A<<t_symbol<<" has a long half life of "<<t_half_life/3600/24/355<<" years.\nYou'll have to wait a while."; 
      return 1; //decay chain ends at isotope with long half-life
    }
    else{ //here check decay modes of nuclide and select one.
      /* first extract decay mode information from the string in the database
       * which has structure: "num_decay_modes|decay_symbol:
       * branch_fraction,Q-value;decay_symbol: branch_fraction,Q-value;etc.",
       * for example: "2|A: 1.0, 8.348; EC: 5e-06, 3.479" */
      string s=t_decay_mode;
      //find number of decays and cut it from the beginning of string s
      const short num_decays=stoi(s.substr(0,s.find("|")));
      s=s.substr(s.find("|")+1,string::npos);
      std::vector<decay_mode_t> decays(num_decays); //contains decay info for current nuclide
      for(short i=0;i<num_decays;i++)
      {
        //extract info from string s and cut it from the string
        short pos=s.find(":");
        decays[i].decay_type=s.substr(0,pos);
        s=s.substr(pos+1,string::npos);
        pos=s.find(",");
        decays[i].branch_frac=stof(s.substr(0,pos));
        s=s.substr(pos+1,string::npos);
        pos=s.find(";");
        decays[i].Q=stof(s.substr(0,pos));
        s=s.substr(pos+1,string::npos);
        /* cout<<decays[i].decay_type<<" "<<decays[i].branch_frac<<" "<<decays[i].Q<<" "; */ 
        /* cout<<"_"<<s<<"_\n"; */
      } 
      //now select which decay path to go on by comparing branch fraction with  random number between 0 and 1
      double rand_num=dis(gen); //random number between 0 and 1 for selecting decay path
      double cum_prob=0.0; //cumulative probability 
      for(auto d:decays){
        cum_prob+=d.branch_frac;
        if(rand_num<cum_prob){
          string s=d.decay_type;
          if(s=="A"){
            m_alpha(d.Q);
          }else if(s=="B-"){
            m_beta_minus(d.Q);
          }else if(s=="EC"){
            m_beta_plus(d.Q);
          }else if(s=="N"){
            m_neuton(d.Q);
          }else if(s=="P"){
            m_proton(d.Q);
          }else if(s=="SF"){
            m_spontenous_fission(d.Q);
            return 1; //decay chain ends at spontaneous fission
          }else {xassert(1,"in Nuclide::m_decay(): d.decay_type has illegal value");}
          return 0;  //decay chain continues after
        }
      }
      xassert(1,"in Nuclide::m_decay(): unstable isotope but no decay mode selected");
      return 1; //this line is never executed
    }
  }
  void m_log_decay(const double Q,string parent,string products){
    string daughter=to_string(t_A)+t_symbol;
    log<<"At t="<<setw(15)<<m_format_time(t_time)<<"   "<<setw(5)<<parent<<" ----> "<<setw(5)<<daughter<<"  +  "<<products<<setw(10)<<Q<<" Mev\n";
  }
  void m_alpha(double Q) {
    string parent=to_string(t_A)+t_symbol;
    string products="4He"; //alpha particle is 4He nucleus
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    t_Z = t_Z-2;
    t_A = t_A-4;
    m_update(t_Z,t_A);
    m_log_decay(Q,parent,products);
  }
  void m_beta_minus (double Q) {
    string parent=to_string(t_A)+t_symbol;
    string products="electron(B-) + antineutrino"; 
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    t_Z = t_Z+1; // a neutron become a proton
    m_update(t_Z,t_A);
    m_log_decay(Q,parent,products);
  }
  void m_beta_plus (double Q) {
    string parent=to_string(t_A)+t_symbol;
    string products="positron(B+) + neutrino"; 
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    t_Z = t_Z-1; // a proton captures an electron to become a neutron
    m_update(t_Z,t_A);
    m_log_decay(Q,parent,products);
  }
  void m_spontenous_fission(double Q){
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    log<<"At t="<<setw(8)<<t_time<<setw(10)<<t_A<<setw(2)<<t_symbol<<" -SF->   *  + ** "<<setw(10)<<Q<<" Mev\n";
  }
  void m_proton(double Q) {
    string parent=to_string(t_A)+t_symbol;
    string products="1p"; 
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    t_A = t_A-1; //a proton is ejected
    t_Z = t_Z-1;
    m_update(t_Z,t_A);
    m_log_decay(Q,parent,products);
  }
  void m_neuton(double Q) {
    string parent=to_string(t_A)+t_symbol;
    string products="1n"; //alpha particle is 4He nucleus
    double t=m_time_from_half_life(t_half_life);
    t_time+=t;
    t_A = t_A-1; //nucleus ejects a neutron
    m_update(t_Z,t_A);
    m_log_decay(Q,parent,products);
  }
};

int main() {
  Nuclide n(92,238);
  n.decay_chain();
  /* for(int i=0;i<=2000;i++) { */
  /*   cout<<n.callt(89090)<<"\n"; */
  /* } */
}

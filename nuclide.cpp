#include"nuclide.h"
using namespace std;

//this namespace contains only const variables whose values are given by laws of physics
namespace physics{
  struct decay_phy_t{ //holds info on a type of radioactive decay
    string name;
    int dZ, dA; //the change in atomic numbers Z and A brought by the decay
    string products; //lists products other that daughter nuclide
  };
  const decay_phy_t alpha={"alpha",2,4,"4He"};
  const decay_phy_t proton={"proton emission",1,1,"1p"};
  const decay_phy_t proton2={"double proton emission",2,2,"2p"};
  const decay_phy_t neutron={"neutron emission",0,1,"1n"};
  const decay_phy_t neutron2={"double neutron emission",0,2,"2n"};
  const decay_phy_t betam={"beta minus",-1,0,"electron(B-) + antineutrino"};
  const decay_phy_t betap={"beta plus",+1,0,"positron(B+) + neutrino"};
  const decay_phy_t sf={"spontaneous fission",0,0,"spontaneously fissions"};

  //maps the string used to identify a decay in the sqlite3 database to appropriate decay_phy_t structure. This allows to write shorter more general code in Nuclide::m_decay()
  const std::unordered_map<string,decay_phy_t> radioactive_decays={
    {"A",alpha},
    {"P",proton},
    {"2P",proton2},
    {"N",neutron},
    {"2N",neutron2},
    {"B-",betam},
    {"EC",betap},
    {"SF",sf}
  };
};

//check the state of the program for undefined behaviour and terminate if undefined behaviour found
void xassert(bool all_is_good_condition, std::string error_message){
  if(!all_is_good_condition){
      std::cerr<<error_message<<std::endl;
      std::terminate();
  } 
}


Nuclide::Nuclide(int Z,int A,vector<event_t>* events) {
  if(!(0<=Z&&Z<=118&&0<A&&A<=295)){
    printf("(Z=%i,A=%i) is invalid input for Nuclide constructor\n Database query aborted\n",Z,A);
  }
  else {
    int rc=0; //return code
    rc=sqlite3_open("nuclides.db",&DB);
    xassert(rc==SQLITE_OK,"in Nuclide constructor: Error opening DB");
    m_update(Z,A);
    t_events=events; 
    //setting up random number generation 
    std::random_device rd; 
    gen= mt19937_64(rd());   //seeding mersweene twister engine with a random_device seed
    dis=std::uniform_real_distribution<>(0.0,1.0); //initializing uniform distribution dis
  }
}

void Nuclide::decay_chain(){
  bool is_decay_chain_over=m_decay();
  int i=0;
  while(!is_decay_chain_over&&i<500){
    is_decay_chain_over=m_decay();
    i++;
  }
  xassert(i<500,"in Nuclide::decay_chain(): more that 500 calls to m_decay()");
}

  /*function taken as argument by sqlite3_exec(). Structure explained in sqlite3
  documentation. data_name is used to assign query response to external
  varables c-style; num_cols= number of columns of the table; col_values =
  pointer (~used as array) to c-style string for table entries; col_names
  pointer to c style string of column names. */
int Nuclide::m_sql_callback(void* data_name, int num_cols, char** col_values, char** col_names) {
    char space_char=' ';
    //going through col_values and ensuring there are no null pointers that could cause crashes.
    for (int i = 0; i < num_cols; i++) {
        if(col_values[i]==NULL){
            col_values[i]=&space_char;
        }
    }
    //recovering struct data from outside the function using the argument data_name
    db_extract_t* nuclide_dat=(db_extract_t*) data_name;
    //assigning good values to all Nuclide class members (accessed through nuclide_dat) using the query result
    nuclide_dat->Z=atoi(col_values[0]);  
    nuclide_dat->A=atoi(col_values[1]);  
    nuclide_dat->symbol=string(col_values[2]);  
    nuclide_dat->weight=atof(col_values[3]);
    nuclide_dat->is_stable=bool(atoi(col_values[4]));
    nuclide_dat->half_life=atof(col_values[5]);
    nuclide_dat->decay_mode=string(col_values[6]);  
    return 0;
}

void Nuclide::m_update(int Z,int A){ 
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

double Nuclide::m_time_from_half_life(const double half_life){
  xassert(half_life!=0,"in Nuclide::m_time_from_half_life(): half_life=0");
  double lambda=0.6931471805599453/half_life; //lambda=ln(2)/t_{1/2}=decay constant
  std::exponential_distribution<> d(lambda); //using std class for generating the numbers
  double t=d(gen);//time in seconds
  return t;
}

//takes time in seconds and return human readable string
string Nuclide::m_format_time(double t){
  const int secs_in_year=31557600;
  const int secs_in_day=86400;
  const int secs_in_hour=3600;
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

string Nuclide::m_decay_info(const double Q,const string parent,const string products){
  string daughter=to_string(t_A)+t_symbol;
  std::stringstream s; 
  s<<"At t="<<setw(15)<<m_format_time(t_time)<<setw(8)<<parent<<" ----> "<<setw(5)<<daughter<<"  +  "<<products<<setw(10)<<Q<<" Mev\n";
  return s.str();
}

/* extract decay mode information from the string in the database
 * which has structure: "num_decay_modes|decay_symbol:
 * branch_fraction,Q-value;decay_symbol: branch_fraction,Q-value;etc.",
 * for example: "2|A: 1.0, 8.348; EC: 5e-06, 3.479" */
std::vector<Nuclide::decay_mode_t> Nuclide::m_decay_modes(){
    string s=t_decay_mode;
    //find number of decays and cut it from the beginning of string s
    short num_decays=stoi(s.substr(0,s.find("|")));
    std::vector<decay_mode_t> decays(num_decays); //contains decay info for current nuclide
    s=s.substr(s.find("|")+1,string::npos);
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
    } 
    s=s.substr(s.find("|")+1,string::npos);
   return decays; 
}

bool Nuclide::m_decay(){
  if(t_is_stable){  
    return 1; //decay chain ends at stable isotope
  }
  else{ //here check decay modes of nuclide and select one.
    std::vector<decay_mode_t> decays=m_decay_modes();
    //now select which decay path to go on by comparing branch fraction with  random number between 0 and 1
    double rand_num=dis(gen); //random number between 0 and 1 for selecting decay path
    double cum_prob=0.0; //cumulative probability 
    for(auto d:decays){
      cum_prob+=d.branch_frac;
      if(rand_num<cum_prob){
        string s=d.decay_type;
        s.erase(std::remove(s.begin(), s.end(),' '),s.end()); //dropping white spaces from decay type specifier
        string parent=to_string(t_A)+t_symbol;
        physics::decay_phy_t this_decay;
        try{ this_decay=physics::radioactive_decays.at(s);
        } catch (std::out_of_range e) { cerr<<"in Nuclide::m_decay: string s=_"<<s<<"_ is not a key in Physics::radioactive_decays\n"; }
        double t=m_time_from_half_life(t_half_life);
        t_time+=t;
        t_Z = t_Z-this_decay.dZ;
        t_A = t_A-this_decay.dA;
        m_update(t_Z,t_A);
        string decay_log=m_decay_info(d.Q,parent,this_decay.products);
        event_t e={t_time,d.Q,decay_log};
        t_events->push_back(e);
        return 0;
      }
    }
    xassert(1,"in Nuclide::m_decay(): unstable isotope but no decay mode selected");
    return 1; //this line is never executed
  }
}

void sort_events_chrnologically(vector<event_t> & events){
  bool is_sorted=0;
  while(!is_sorted){
    is_sorted=1;
    for(int i=1;i<events.size();i++){
      if(events[i-1].t>events[i].t){
        event_t x=events[i-1];
        events[i-1]=events[i];
        events[i]=x;
        is_sorted=0;
       }
      }
   }
}

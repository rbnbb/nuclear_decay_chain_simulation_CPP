#include <exception>
#include <fstream>
#include <iostream>
#include <sqlite3.h>
#include <stdexcept>
using namespace std;
//simplify checking state of the program for undefined behaviour
void xassert(bool all_is_good_condition, std::string error_message){
  if(!all_is_good_condition){
      std::cerr<<error_message<<std::endl;
      std::terminate();
  } 
}
class Nuclide {
  sqlite3* DB;
  int t_Z,t_A;
  double t_weight, t_half_life;
  bool t_is_stable;
  string t_symbol,t_decay_mode;
  //structure used strictly for extracting data from SQLite database
  struct db_extract_t{ 
      int Z,A;
      bool is_stable;
      double half_life;
      double weight; // weight in atomic mass units
      string symbol,decay_mode;
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
    }
  }
  /*prints info on class state*/
  void info(){
    printf("%i%s: Z=%i, A=%i, m=%f u, is_stable=%i, hl=%f, decays=%s\n",t_A,t_symbol.c_str(),t_Z,t_A,t_weight,t_is_stable,t_half_life,t_decay_mode.c_str());
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
  void m_alpha () {
    t_A = t_A-2;
    t_Z = t_Z-4;
    m_update(t_Z,t_A);
  }
  void m_proton () {
    t_A = t_A-1;
    t_Z = t_Z-1;
  }
  void m_duble_proton () {
    t_A = t_A-2;
    t_Z = t_Z-2;
  }
  void m_neuton () {
    t_A = t_A-1;
  }
  void m_duble_neutron () {
    t_A = t_A-2;
  }
  void m_beta_minus () {
    t_Z = t_Z+1;
  }
  void m_beta_plus () {
    t_Z = t_Z-1;
  }
  void m_duble_beta_minus () {
    t_Z = t_Z+2;
  }
  void m_electron () {
    t_Z = t_Z-1;
  }
  void m_duble_electron () {
    t_Z = t_Z-2;
  }
  void m_duble_positron () {
    t_Z = t_Z-2;
  }
};

int main() {
  Nuclide n(59,141);
  n.info();
  /* cout<<"Enter atomic number:"; */
  /* cin>>t_Z; */
  /* while(t_Z < 0 || t_Z > 118) { */
  /*   cout<<"Stupid value try again (0-118):"; */
  /*   cin>>t_Z; */
  /*   } */
  /* cout<<"Enter mass number:"; */
  /* cin>>t_A; */
  /* while(t_A < 1 || t_A > 295) { */
  /*   cout<<"Stupid value try again (1-295):"; */
  /*   cin>>t_A; */
  /*   } */
}

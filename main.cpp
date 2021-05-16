#include"nuclide.h"
using namespace std;

struct initial_nuclides_t{
  int Z,A;
  int num; //number of isotopes with given Z and A
};

//given initial composition of a (very small) sample of radioactive material as an argument runs the simulation and prints results to file 
void simulate(vector<initial_nuclides_t> nuclides){
  vector<event_t> events;
  events.reserve(500);
  stringstream first_line;
  first_line<<"At t=0"<<setw(15)<<"   ";
  for(auto n:nuclides){
    first_line<<n.num<<" x ";
    for(int i=1;i<=n.num;i++){
      Nuclide x(n.Z,n.A,&events);
      if(i==1){
        first_line<<x.get_name()<<"        ";
      }
      x.decay_chain();
    }
  }
  sort_events_chrnologically(events);
  ofstream out_human_readable("results_decay_hist.out");
  ofstream out_tQ_table("results_tq_table.csv");
  out_human_readable<<first_line.str()<<'\n';
  for(auto e: events){
    out_human_readable<<e.info; 
    out_tQ_table<<setw(15)<<e.t<<";"<<setw(10)<<e.energy<<";\n"; 
  }
  out_human_readable.close();
  out_tQ_table.close();
  cout<<"DONE!\n";
}

//processes user input and passes it to the simulation via the initial_nuclides_t struct
vector<initial_nuclides_t> userinput(){ 
  cout<<"=====Nuclear decay chains simulator 2021=====\nWrite 'exit' to terminate \n\nPlease enter the initial conditions carefully\nType the number of different isotopes (nuclides) in the simulation\n";
  vector<initial_nuclides_t> initial_isotopes;
  string input;
  while(input!="exit"){
  try{
  cin>>input;
  int num_unique_isotopes;
  num_unique_isotopes=stoi(input);
  initial_isotopes.reserve(num_unique_isotopes);
  for(int i=1;i<=num_unique_isotopes;i++){
    initial_nuclides_t x; 
    cout<<"Now type Z and A numbers for isotope number "<<i<<", in that order, separated by space:\n";
    cin>>x.Z>>x.A;
    cout<<"Now type the number of such isotopes to simulate\n";
    cin>>x.num;
    if(0<x.num&&x.num<500){
        initial_isotopes.push_back(x);
    }else{
        cout<<"Invalid input. Try again.\n";
        break;
    }

  }
  break;
  } catch(...){
    if(input!="exit")
      cerr<<"Bad input. Restarting...\n";
  }
  }
  if(input=="exit"){
      cout<<"Exited successfully\n";
  } else {
  cout<<"Initial conditions correctly introduced\nSimulations results can be found in human readable format in results_decay_hist.out.\nA table of times and decay energy can be found in file results_tq_table than ca be used for more quantitative analyses\n\n";}
  return initial_isotopes;
};

int main() {
  simulate(userinput());
}

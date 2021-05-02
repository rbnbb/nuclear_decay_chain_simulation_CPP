#include<iostream>
#include<sqlite3.h>
#include<chrono>
using namespace std;
//structure used strictly for extracting data from SQLite database
struct db_extract_t{
    int Z,A;
    bool is_stable;
    double half_life;
    double weight; // weight in atomic mass units
    string symbol,betam,alpha,betap;
};
//function taken as argument by sqlite_exec(). Structure explained in sqlite3
//documentation. data_name is used to assign query response to external
//varables c-style; num_cols= number of columns of the table; col_values =
//pointer (~used as array) to c-style string for table entries; col_names
//pointer to c style string of column names. 
static int sql_callback(void* data_name, int num_cols, char** col_values, char** col_names)
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
    nuclide_dat->alpha=string(col_values[6]);  
    nuclide_dat->betap=string(col_values[7]);  
    nuclide_dat->betam=string(col_values[8]);  
    //print for debugging
    /* printf("num_cols: %i\n",num_cols); */
    /* printf("%i, %i, %f, %i, %f, \n",nuclide_dat->Z,nuclide_dat->A,nuclide_dat->weight,nuclide_dat->is_stable,nuclide_dat->half_life); */
    /* printf("%s, %s, %s, %s \n",nuclide_dat->symbol.c_str(),nuclide_dat->betam.c_str(),nuclide_dat->betap.c_str(),nuclide_dat->alpha.c_str()); */
    return 0;
}
int main(){
    sqlite3* DB;
    int exit=0;
    exit=sqlite3_open("nuclides.db",&DB);
    string sql_instruction("SELECT * FROM nuclides WHERE A=254 AND Z=103");

    if(exit){
        std::cerr<<"Error opening DB "<<sqlite3_errmsg(DB)<<std::endl;
        return(-1);
    }
    /* else std::cout<<"Opened Database successfully!"<<std::endl; */
    db_extract_t data;
    int rc = sqlite3_exec(DB, sql_instruction.c_str(), sql_callback, &data, NULL);
  
    if (rc != SQLITE_OK)
        cerr << "Error SELECT" << endl;
    /* else { cout << "Operation OK!" << endl; } */ 
    sqlite3_close(DB);
    return(0);
}

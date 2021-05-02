#include <fstream>
#include <iostream>
#include <string>
#include <sqlite3.h>


using namespace std;

class Nuclide {
    char t_name[3];  
    int t_A;
    int t_Z;
    public:  
    Nuclide () {
        cout<<"Enter atomic number:";
        cin>>t_Z;
        while(t_Z < 0 || t_Z > 118) {
            cout<<"Stupid value try again (0-118):";
            cin>>t_Z;
            }
        cout<<"Enter mass number:";
        cin>>t_A;
        while(t_A < 1 || t_A > 295) {
            cout<<"Stupid value try again (1-295):";
            cin>>t_A;
            }
    }
    void m_update () {

    }
    void m_alpha () {
        t_A = t_A-2;
        t_Z = t_Z-4;
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
    void m_show_Z () {
        cout<<t_Z;
    }
    void m_show_A () {
        cout<<t_A;
    }
};



int main() {
    Nuclide aaa;
    aaa.m_show_A();
}


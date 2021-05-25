# Nuclear decay chain siulator in c++ with SQLite Database

### Why use it
You should use it if you need a SQLite database of 3341 isotopes to use in a C++ nuclear simulation. Class with interface to database is included

### Intro
This c++ software simulates nuclear decay chains using a database of decay modes and half lifes "nuclides.db".
The results are written to 2 output files that contain the history of decays that the input sample underwent.

The code is centered around the class Nuclide that is used to manage the decay chains and it is the portable part of the software declared in nuclide.h. The implementation details of the class are in nuclide.cpp and main.cpp offers a convenient console application that simulates the evolution of a sample given by the user.

### Requirements
Other that the Standard Library, this source code requires SQLite3 - C++ bindings available at https://sqlite.org/download.html. If sqlite3 library is installed adding the flag -l sqlite3 to g++ compiler should suffice. 

### Database
The file "nuclides.db" is a SQLite3 database that contains information on 3341 isotopes.
The data used originated from International Atomic Energy Agency's Nudat2 database available online at: https://www.nndc.bnl.gov/nudat2/indx_sigma.jsp
The "nuclide.db" file used was produced with the help of a python script available at https://github.com/jhykes/nuclide-data. The script used is distributed under the license: "Copyright (c) 2011, Joshua M. Hykes All rights reserved." (full license at https://github.com/jhykes/nuclide-data/blob/master/LICENSE)
The script was only used in the creation of the database for convenience and plays no other role in this piece of software. 
### Coding conventions
This c++ source code was written trying to adhere to C++ Core Guidelines available at https://github.com/isocpp/CppCoreGuidelines . In particular preferring Standard Library containers to C style arrays and writing the code such that a minimum of commentaries are required to explain its function.
#### Conventions used:
- naming style is snake_case
- Classes are capitalised
- private methodes of classes are prefixed with m_
- private members of classes are prefixed with t_
- the atomic number Z appearers before mass number A in function arguments

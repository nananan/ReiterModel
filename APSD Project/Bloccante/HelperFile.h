#pragma one

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

class HelperFile {

private:
    string line;

public:
    HelperFile() {}

    void readFile(float &alpha, float &beta, float &gamma, int &percentual, int &margolus) {
        ifstream myfile ("value.txt");
        int cont = 0;
        if (myfile.is_open())
        {
            while ( getline (myfile,line) )
            {
                switch(cont) {
                    case 0:
                        alpha = atof(line.c_str());
                        break;
                    case 1:
                        beta = atof(line.c_str());
                        break;
                    case 2:
                        gamma = atof(line.c_str());
                        break;
                    case 3:
                        percentual = atoi(line.c_str());
                        break;
                    case 4:
                        margolus = atoi(line.c_str());
                        break;
                }

                cont++;
            }
            myfile.close();
        }

        else cout << "Unable to open file"; 

    }

};

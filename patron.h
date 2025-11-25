#ifndef PATRON_H
#define PATRON_H

#include <string>
#include <vector>
//#include "item.h"

class Patron {
    public:
        std::vector<int> holds;      // set of items this patron is waiting for
        std::vector<int> catalogue;  // set of items this patron borrowed

        int getID();			   // return patron ID
        bool getAccountStatus();	   // confirm patron account status
        std::string getName();		   // return patron username
        Patron(int id, std::string name);  // patron constructor

    private:
            int id;            // patron id number
            bool status;       // patron account status (false == blocked, true == open)
            std::string name;  // patron name
};

#endif


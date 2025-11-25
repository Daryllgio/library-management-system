#include "patron.h"


Patron::Patron(int id, std::string name) {
    this->id = id;
    this->status = true;
    this->name = name;
}


int Patron::getID() { return id; }

bool Patron::getAccountStatus() { return status; }

std::string Patron::getName() { return name; }


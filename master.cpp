#include "main.h"
#include "master.h"

int Master::YODA;
int Master::ZET;
int Master::ID;

Master::Master(int ID_){
        ID = ID_;
}


void Master::setYODA(int value){
        YODA = value;
}

void Master::setZET(int value){
        ZET = value;
}

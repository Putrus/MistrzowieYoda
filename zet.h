#ifndef ZET_H
#define ZET_H
#include "master.h"
class Zet : public Master{
public:
        static int requestsForRefilled;

public:
        Zet(int ID);

        void mainLoop();
};



#endif

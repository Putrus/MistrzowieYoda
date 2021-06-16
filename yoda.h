#ifndef YODA_H
#define YODA_H
#include "master.h"
#include <vector>
#include <pthread.h>


class Yoda : public Master{
public:
        //statyczne zmienne ktore bedziemy wspoldzielic miedzy watkiem komunikacyjnym a glownym
        static std::vector<int> lamportTable;
        static bool isSent;
        static bool zetSent;
        static pthread_mutex_t lamportMutex;
        static pthread_mutex_t isSentMutex;
        static pthread_mutex_t energyMutex;
        static int lamportClock;
        static int energy;
public:
        Yoda(int ID);
        static void lamport(int value, bool set);
        static void decrementEnergy();
        void mainLoop();
};



#endif

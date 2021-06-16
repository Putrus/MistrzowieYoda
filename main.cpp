#include "main.h"
#include "master.h"
#include "zet.h"
#include "yoda.h"

void *startComThreadYoda(void *ptr);

void *startComThreadZet(void *ptr);


void *function(void *ptr){
        while(true)
{    
        printf("Funkcja!\n");
}
}


int main(int argc, char **argv){

        int threadID;
        int size;
        MPI_Init(&argc, &argv);
        MPI_Status status;

        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &threadID);
    
        Master::setYODA(size / PROPORTION);
        Master::setZET(size - Master::YODA);    
    
        pthread_t threadMaster; 
    
        if(threadID < Master::YODA){
                Yoda yoda(threadID);
                pthread_create(&threadMaster, NULL, startComThreadYoda, 0); 
                yoda.mainLoop();
        }
        else{
                Zet zet(threadID);
                pthread_create(&threadMaster, NULL, startComThreadZet, 0); 
                zet.mainLoop();
        }

    
    
        return 0;
}

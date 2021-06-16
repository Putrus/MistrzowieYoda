#include "zet.h"
#include "main.h"

int Zet::requestsForRefilled;


Zet::Zet(int ID) : Master(ID){

}



void Zet::mainLoop(){
        printf("ID %d ZET - patrze sobie w pepek\n", ID);
        while(true){
                if(requestsForRefilled == YODA){
                        //losuje ile bedzie uzupelnial energie
                        int timeRef = std::rand() % ZET + 1;
                        printf("ID %d ZET - uzupelniam energie\n", ID);
                        sleep(timeRef);
                        printf("ID %d ZET - uzupelnilem energie, wracam patrzec sobie w pepek\n", ID);
                        for(int i = 0; i < YODA; i++){
                                //pierwszy argument niewazny interesuje nas, ze energia zostala uzupelniona
                                MPI_Send(&requestsForRefilled, 1, MPI_INT, i, REFILLED_DONE, MPI_COMM_WORLD);
                        }
                        requestsForRefilled = 0;
                }
        }
}






void * startComThreadZet(void *ptr){
        MPI_Status status;
        int msg;
        while(true){
                MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                switch(status.MPI_TAG){
                        case NEED_TO_BE_REFILLED:
                                Zet::requestsForRefilled++;
                        break;
                        default:
                        break;
                }
        }
}
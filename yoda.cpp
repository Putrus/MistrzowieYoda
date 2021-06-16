include "yoda.h"
#include "main.h"

int Yoda::lamportClock;
int Yoda::energy;

std::vector<int> Yoda::lamportTable;
bool Yoda::isSent;
bool Yoda::zetSent;
pthread_mutex_t Yoda::lamportMutex;
pthread_mutex_t Yoda::isSentMutex;
pthread_mutex_t Yoda::energyMutex;

Yoda::Yoda(int ID) : Master(ID){
        for(int i = 0 ;i < Master::YODA; i++){
                lamportTable.push_back(-1);
        }
        isSent = false;
        zetSent = false;    
        lamportMutex = PTHREAD_MUTEX_INITIALIZER;
        isSentMutex = PTHREAD_MUTEX_INITIALIZER;
        energyMutex = PTHREAD_MUTEX_INITIALIZER;
        energy = ZET;
}


void Yoda::mainLoop(){
        printf("ID %d P(%d) YODA - jestem mistrzem YODA, zaczynamy!\n", ID, lamportTable[ID]); 
        while(true){
                pthread_mutex_lock(&isSentMutex);
                if(!isSent){
                        //jesli nie zostala wyslana prosba o wejscie do nadhiperprzestrzenii to zwiekszamy zegar o 1 i wysylamy do innych procesow
                        lamport(0, false);
                        for(int i = 0; i < YODA; i++){
                                if(i != ID){
                                        MPI_Send(&lamportClock, 1, MPI_INT, i, REQ, MPI_COMM_WORLD);
                                }
    
                        }
                        //wyslalismy juz wszystkim i mozemy wyswietlic info, ze czekamy na dostep
                        printf("ID %d P(%d) YODA - oczekuje na wejscie do nadhiperprzestrzenii\n", ID, lamportTable[ID]);
                        isSent = true;
                }
                pthread_mutex_unlock(&isSentMutex);
                //sprawdzamy pozycje tego Yody w tabeli priorytetowej
                int position = YODA;
                pthread_mutex_lock(&lamportMutex);
                for(int i = 0; i < YODA; i++){
                        if((lamportTable[i] > lamportTable[ID]) || (lamportTable[i] == lamportTable[ID] && ID < i)){
                                position--;
                        }
                }
                pthread_mutex_unlock(&lamportMutex);

                //wejscie do nadhipeprzestrzenii
                if(position <= energy && isSent){
                        //jesli pozycja mniejsza od ilosci energii i prosby zostaly wyslane do wszystkich to mozemy wejsc do nadhipeprzestrzenii
                        printf("ID %d P(%d) YODA - dostalem dostep do nadhiperprzestrzenii\n", ID, lamportTable[ID]);

                        //losowanie jak dlugo bedzie czerpal energie
                        int timeRel = std::rand() % ZET + 1;
                        for(int i = 0; i < YODA; i++){
                                //wysylamy do wszystkich release ze energia zostala zuzyta
                                //pierwszy parametr niewazny, wysylamy cokolwiek
                                MPI_Send(&energy, 1, MPI_INT, i, RELEASE, MPI_COMM_WORLD);
                        }
                        //zwiekszamy zegar lamporta
                        lamport(0, false);
                        for(int i = 0; i < YODA; i++){
                                if(i != ID){
                                        //wysylamy do reszty procesow zgode nasza z wartoscia zegara lamporta
                                        MPI_Send(&lamportClock, 1, MPI_INT, i, ACK, MPI_COMM_WORLD);
                                }
                        }
                        //ponownie ustawiamy isSent na false, poniewaz chcemy ponownie wyslac zgody
                        isSent = false;
                        //czerpie energie przez timeRel czasu;
                        sleep(timeRel);
                        printf("ID %d P(%d) YODA - pobralem 1 energie z nadhiperprzestrzenii, wracam do oczekiwania na wejscie\n", ID, lamportTable[ID]);
                }

                //jesli energia po pobraniu jest rowna 0
                pthread_mutex_lock(&energyMutex);
                if(energy == 0 && !zetSent){
                        printf("ID %d P(%d) YODA - energia jest rowna 0, wysylam prosbe o uzupelnienie do mistrzow ZET\n", ID, lamportTable[ID]);
                        for(int i = YODA; i < YODA + ZET; i++){
                                //kolejny raz prametr 1 nie jest wazny
                                MPI_Send(&energy, 1, MPI_INT, i , NEED_TO_BE_REFILLED, MPI_COMM_WORLD);
                        }
                        //ustawiamy zmienna zetSent na true, poniewaz wyslalismy juz prosbe o uzupelnienie energii
                        zetSent = true;
                }
                pthread_mutex_unlock(&energyMutex);
        }

}


void Yoda::lamport(int value, bool set){
        pthread_mutex_lock(&lamportMutex);
        if(set){
                //sprawdzamy czy value wieksza od wczesniejszej wartosci clocka i jesli tak to ustawiamy i zwiekszamy o 1
                if(value > lamportClock){
                        //ustawiamy zegar na value i zwiekszamy o 1
                        lamportClock = value + 1;
                }
                else{
                        //tylko zwiekszamy o 1
                        lamportClock++;
                }
        }
        else{
                //zwiekszamy zegar o jeden i przypisujemy go dla wartosci w tabeli
                lamportClock++;
                lamportTable[ID] = lamportClock;
        }
        pthread_mutex_unlock(&lamportMutex);
}

void Yoda::decrementEnergy(){
        energy--;
}


void * startComThreadYoda(void *ptr){
        MPI_Status status;
        int msg = 0;
        //ilosc uzupelnionej energii przez msitrzow ZET gdy juz jest zuzyta
        int energyRefilled = 0;
        while(true){
                MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                switch(status.MPI_TAG){
                        case REQ:
                                pthread_mutex_lock(&Yoda::isSentMutex);
                                //sprawdzamy czy otrzymany priorytet jest najwiekszy, ustawiamy go jako nowy zegar i zwiekszamy o 1 lub zwiekszamy tylko zegar
                                                                  Yoda::lamport(msg, true);
                                if(!Yoda::isSent){
                                        //jesli nie wyslalismy to zwiekszamy zegar o 1 
                                        Yoda::lamport(0, false);
                                        //wysylamy zgode i wartosc naszego zegara do innych procesow
                                        MPI_Send(&Yoda::lamportClock, 1, MPI_INT, status.MPI_SOURCE, ACK, MPI_COMM_WORLD);
                                }
                                pthread_mutex_unlock(&Yoda::isSentMutex);

                        break;
                        case ACK:
                                //jesli otrzymalismy zgode ACK to sprawdzamy czy otrzymany priorytet jest najwiekszy jesli tak to ustawiamy go jako zegar i zwiekszamy o jeden
                                Yoda::lamport(msg, true);
                                //ustawiamy dla procesu od ktorego otrzymalismy wiadomosc ta wartosc priorytetu
                                Yoda::lamportTable[status.MPI_SOURCE] = msg;
                        break;
                        case RELEASE:
                                //informacja ze ktos zuzyl 1 energie
                                pthread_mutex_lock(&Yoda::energyMutex);
                                Yoda::decrementEnergy();
                                pthread_mutex_unlock(&Yoda::energyMutex);
                        break;
                        case REFILLED_DONE:
                                //informacja, ze energie uzupelnil 1 z mistrzow ZET
                                energyRefilled++;
                                if(energyRefilled == Yoda::ZET){
                                        //jesli energia zostala uzupelniona to ustawiamy ja na ilosc poczatkowa(ZET) i ustawiamy zetSent na false
                                        pthread_mutex_lock(&Yoda::energyMutex);
                                        Yoda::energy = Yoda::ZET;
                                        energyRefilled = 0;
                                        Yoda::zetSent = false;
                                        pthread_mutex_unlock(&Yoda::energyMutex);

                                }

                        break;
                        default:

                        break;


                }

        }


}
                                                                                                                                 
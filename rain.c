#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>


#define SIZE 8
#define SHIP_NUM 10
#define AMMO 10


//testing


/* Pseudocode
Make the canon fire every 1 seconds, make it move based on the directions (a specific char, say 'a' for left or 'd' for right)

Make aliens, 'O' chars come in from random points of the top grid every 1 seconds. 

Use pthreads for each alien ship and missile 

thread displays 'X' when missile hits, and exits (this would require a semaphore as it would mean occupying a spot in the matrix for more than the same time for both missiles and ships (dropped for now))

between levels, give a time of 3 seconds if the user wants to continue or not (for next version)
*/

void startClock();
long getCurrentTime();
time_t globalClock;
void createMatrix();
void* displayMatrix();

void PrepareFleet();

void PrepareMissiles();
void moveCanon();
void* fireCanon();
void* attackEarth(void*);
int checkWin();

int canon_position;
char matrix[SIZE][SIZE];        
int gameOver;
int win;


typedef struct ship{

    pthread_t handle;
    int tid;                    // goes from 1 to SHIP_NUM
    unsigned int startTime;
    char display;
    unsigned int speed;
    int destroyed;


} ship;

typedef struct missile{

    pthread_t handle;
    int fireFrom;
    unsigned int startTime;
    char display;
    unsigned int speed;


} missile;

ship fleet[SHIP_NUM];     
missile ammunition[AMMO];


int main(){

    startClock();

    system("clear");

    gameOver = 0;
    canon_position = 1;             //starting position of the canon

    createMatrix();

    PrepareFleet();

    PrepareMissiles();            

    long startingTime = getCurrentTime();
    long last_time = startingTime;

    pthread_t matrix;
    pthread_create(&(matrix), NULL, displayMatrix, NULL); 


    for(int k = 0; k < AMMO && !gameOver; k++){

        // long waitUntil = last_time + 1;  //s.startTime;
        // last_time = waitUntil;       

        while(getCurrentTime() < getCurrentTime() + 1)
                        ;			// busy wait to launch a ship every second
    
        if(k < SHIP_NUM )
            pthread_create(&(fleet[k].handle), NULL, attackEarth, (void*) &(fleet[k]));

        pthread_create(&(ammunition[k].handle), NULL, fireCanon, (void*) &(ammunition[k])); 

        if(getCurrentTime() >=  startingTime + SHIP_NUM + SIZE/2)
            win = checkWin();
        
        if(win)
            break;     

    }

    for(int a = 0; a < AMMO; a++){                  // can optimize here. If you broke away in the previous loop, you shouldn't pthread_join for all AMMO

        if(a < SHIP_NUM)
            pthread_join(fleet[a].handle, NULL);

        pthread_join(ammunition[a].handle, NULL);

    }

    pthread_join(matrix, NULL);

    if(gameOver)
        printf("\nALIENS HAVE INVADED THE PLANET, EARTH IS LOST. FOLLOW ELON TO MARS\n");

    if(win)
        printf("\nYou've won!! Send Elon back to Mars!!\n");
    
    return 0;

}

int checkWin(){

    int destroyed_all = 1;

    for(int i = 0; i < SHIP_NUM; i++){
         destroyed_all = destroyed_all && fleet[i].destroyed;  //( a * would've also worked here instaad of an &&)
    }

    return destroyed_all;
}

void PrepareFleet(){

    for(int i = 0; i < SHIP_NUM; i++){      // make the ships

        ship* s;
        s = (ship*)malloc(sizeof(ship));
        s->tid = i + 1;
        s->destroyed = 0;

        if(i == 0)
            s->startTime = getCurrentTime() + 1;            

        else {
            s->startTime = (&fleet[i - 1])->startTime ;     // A ship sails every second
            s->speed = 1;                                   
        }

        fleet[i] = *s;
    }
}

void PrepareMissiles(){
    
    for(int i = 0; i < AMMO; i++){      // make the ships

        missile* m;
        m = (missile*)malloc(sizeof(missile));
 
        if(i == 0)
            m->startTime = getCurrentTime() + 1;

        else {
            m->startTime = (&ammunition[i - 1])->startTime ;     // A missile sails every second
            m->speed = 1;
        }

        ammunition[i] = *m;
    }
}


void* displayMatrix(){                          

    while(!gameOver){ 

        system("clear");

        for(int i = 0; i < SIZE; i++){
            for(int j = 0; j < SIZE; j++){

                if(j == SIZE - 1)
                    printf("%c\n", matrix[i][j]);

                else
                    printf("%c", matrix[i][j]);
            }
        }
    }
    pthread_exit(0);

    return NULL;
}


void createMatrix(){                                   


    for(int i = 0; i < SIZE; i++){
        matrix[i][0] = '|';
        matrix[i][SIZE - 1] = '|';

        matrix[0][i] = '-';
        matrix[SIZE - 2][i] = '-';          // leave the last one to display the canon

    }

    for(int i = 1; i < SIZE - 2; i++){

        for(int j = 1; j < SIZE - 1; j++){
            matrix[i][j] = ' ';
        }
    }
}

void* attackEarth(void* s){

    if(gameOver){
        pthread_exit(0);
        return NULL;
    }

    int j = 1; // OVERRIDDING FOR NOW else should be j = rand() % SIZE;      // attack starting position
    int i = 1;                  // height of the ship from ground

    if(j == 0)
        j++;
    if(j == SIZE - 1)
        j--;

    while(i != SIZE - 2 && matrix[i + 1][j] != '^' && !gameOver){
        matrix[i][j] = 'O';

        long waitUntil = getCurrentTime() + 1;  //s->speed
        while(!gameOver && getCurrentTime() < waitUntil)
             ;   // busy wait
        if(!gameOver){
            matrix[i][j] = ' ';
            i++;
        }
    }

    if(matrix[i + 1][j] == '^'){
         matrix[i][j] = 'X';
        ((ship*)s) ->destroyed = 1;

        pthread_exit(0);
        return NULL;
    }

    if(!gameOver && i == SIZE - 2)
          gameOver = 1;
    

    pthread_exit(0);
    return NULL;
    

}

void* fireCanon(void* m){

    if(gameOver ||  win){
        pthread_exit(0);
        return NULL;
    }

    while(!gameOver && !win){

        //moveCanon();
        // if(moveCanon() == 0)
        //     continue;
            
        int position = canon_position;

        for(int k = SIZE - 3; !gameOver && !win && k > 1 ; k--){
            
            if(matrix[k - 1][position] == 'O'){
                 matrix[k][position] = ' ';   
                 // I'll need semaphores for this!!!
                // matrix[k - 1][position] = 'X';

                // while (getCurrentTime() < getCurrentTime() + 1);

                // matrix[k - 1][position] = ' ';
                
                pthread_exit(0);
                return NULL;
            }
            
            matrix[k][position] = '^';

            long waitUntil = getCurrentTime() + ((missile*)m)->speed;  //m->speed
            while(!gameOver && getCurrentTime() < waitUntil)
                ;   // busy wait
            if(!gameOver && !win)
                matrix[k][position] = ' '; 
        }
    
        pthread_exit(0);
        return NULL;
    }
    pthread_exit(0);
    return NULL;
}

void moveCanon(){

        char movement;
        scanf("%c\n", &movement);

        while(1){

            if(canon_position == 0 && movement == 'a')        // can't move beyond the matrix
                continue;
            
            if(canon_position == SIZE - 1 && movement == 'd')        // can't move beyond the matrix
                continue;
            
            if(movement == 'a'){                              // move left
                canon_position == canon_position - 1;
                break;
            }

            if(movement == 'd'){                              // move right
                canon_position == canon_position + 1;
                break;
            }
        }
}

void startClock()
{
	globalClock = time(NULL);
}

long getCurrentTime()
{
	time_t now;
	now = time(NULL);
	return now-globalClock;
}
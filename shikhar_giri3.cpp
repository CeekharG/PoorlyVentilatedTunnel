/***********************************************************************************************************
Name - Shikhar Giri
PeopleSoft ID - 1782555
Section - M/W(4:00-5:30)
Instructor - Jehan-François Pâris
Course - Fundamentals of Operating Systems(COSC 3360)
Assignment 3 - The Poorly Ventilated Tunnel 
Due Wednesday, April 29, 2020 at 11:59:59 pm

Description: This assignment familiarize with the use of pthreads, pthread mutexes and pthread condition variables.

To Compile: g++ shikhar_giri.cpp -lpthread -fpermissive -std=c++11
            ./a.out  < filename.txt 
**********************************************************************************************************/

#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <cstdlib>

#define COUNT 128

using namespace std;

typedef struct _CARINFO
{
    int id;
    long long int arrive;
    long long int cross;
    int id_NB;
    int id_SB;
    string dir;
} CARINFO;

void *carThread(void *arg);
void *tunnelThread(void *arg);

static pthread_mutex_t routeLock;
static pthread_mutex_t lock;
static pthread_cond_t condition;

static string direction = "";
static int MaxNcarsInTunnel = 0;
static int count = 0;
static int nCarsInTunnel = 0;
static int nNBCarsInTunnel = 0;
static int nSBCarsInTunnel = 0;
static int southBoundCars = 0;
static int northBoundCars = 0;
static int waitCarsCount = 0;
static int nNBcars = 0, nSBcars = 0;

// Thread for cars
void *carThread(void *arg)
{
    CARINFO *pCar = (struct _CARINFO *)arg;
    bool delayed = false;

    // Arrival
    pthread_mutex_lock(&lock);
    if (pCar->dir == "N"){
        cout << endl;
        cout << "Northbound car # " << pCar->id_NB << " arrives at the tunnel." << endl;}
    else{
        cout << endl;
        cout << "Southbound car # " << pCar->id_SB << " arrives at the tunnel." << endl;}

    if (pCar->dir != direction || nCarsInTunnel == MaxNcarsInTunnel ||
        ((pCar->dir == "N") && (nNBCarsInTunnel == nNBcars)) ||
        ((pCar->dir == "S") && (nSBCarsInTunnel == nSBcars)))
    {
        if ((pCar->dir == direction && nCarsInTunnel == MaxNcarsInTunnel) ||
            (pCar->dir == direction && nNBCarsInTunnel == nNBcars) ||
            (pCar->dir == direction && nSBCarsInTunnel == nSBcars))
        {
            delayed = true;
            if (pCar->dir == "N"){
                cout << "Northbound car # " << pCar->id_NB << " has to wait." << endl;
                cout << endl;
            }
            else{
                cout << "Southbound car # " << pCar->id_SB << " has to wait." << endl;
                cout << endl;
            }
        }
    }

    while (pCar->dir != direction || nCarsInTunnel == MaxNcarsInTunnel ||
           ((pCar->dir == "N") && (nNBCarsInTunnel == nNBcars)) ||
           ((pCar->dir == "S") && (nSBCarsInTunnel == nSBcars)))
    {
        pthread_cond_wait(&condition, &lock);
    }

    if (delayed == true)
    {
        waitCarsCount++;
        delayed == false;
    }
    nCarsInTunnel++;
    if (pCar->dir == "N")
    {
        cout << "Northbound car # " << pCar->id_NB << " enters the tunnel." << endl;
        nNBCarsInTunnel++;
    }
    else
    {
        cout << "Southbound Car # " << pCar->id_SB << " enters the tunnel." << endl;
        nSBCarsInTunnel++;
    }
    pthread_mutex_unlock(&lock);

    sleep(pCar->cross);

    // Tunnel exit
    pthread_mutex_lock(&lock);
    nCarsInTunnel--;

    if (direction == "N")
        pthread_cond_broadcast(&condition);
    else if (direction == "S")
        pthread_cond_broadcast(&condition);

    if (pCar->dir == "N")
    {
        cout << "Northbound car # " << pCar->id_NB << " exits the tunnel." << endl;
        nNBCarsInTunnel--;
        //northBoundCars++;
    }
    else
    {
        cout << "Southbound car # " << pCar->id_SB << " exits the tunnel." << endl;
        nSBCarsInTunnel--;
        //southBoundCars++;
    }
    pthread_mutex_unlock(&lock);

    pthread_exit(NULL);
}

// Thread for tunnel
void *tunnelThread(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&routeLock);
        direction = "S";
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&routeLock);

        pthread_mutex_lock(&routeLock);
        direction = "N";
        pthread_cond_broadcast(&condition);
        pthread_mutex_unlock(&routeLock);
    }
}

// Main program
int main()
{
    pthread_t carThreads[COUNT] = {0};
    pthread_t threadID;
    CARINFO cars[COUNT] = {0};
    int i = 1, j = 1;

    pthread_mutex_init(&routeLock, NULL);
    pthread_mutex_init(&lock, NULL);
    condition = PTHREAD_COND_INITIALIZER;

    // Get info from input file
    cin >> MaxNcarsInTunnel >> nNBcars >> nSBcars;

    // Echo first 3 lines of input
    cout << "Maximum number of cars in the tunnel: " << MaxNcarsInTunnel << endl;
    cout << "Maximum number of northbound cars: " << nNBcars << endl;
    cout << "Maximum number of southbound cars: " << nSBcars << endl;
    cout << endl;

    // Create threads
    pthread_create(&threadID, NULL, tunnelThread, NULL);

    // Get further info from input file
    while (cin >> cars[count].arrive >> cars[count].dir >> cars[count].cross)
    {
        cars[count].id = count + 1;

        if (cars[count].dir == "N")
        {
            cars[count].id_NB = i++;
            northBoundCars++;
        }
        else
        {
            cars[count].id_SB = j++;
            southBoundCars++;
        }
        count++;
    }
    // create threads
    for (unsigned int i = 0; i < count; i++)
    {
        sleep(cars[i].arrive);
        pthread_create(&carThreads[i], NULL, carThread, (void *)&cars[i]);
    }

    // Wait for termination
    for (int i = 0; i < count; i++)
        pthread_join(carThreads[i], NULL);

    // Display summary
    cout << endl;
    cout << northBoundCars << " northbound car(s) crossed the tunnel." << endl;
    cout << southBoundCars << " southbound car(s) crossed the tunnel." << endl;
    cout << waitCarsCount << " car(s) had to wait." << endl;

    pthread_mutex_destroy(&routeLock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&condition);

    return 0;
}
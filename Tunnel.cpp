#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <cstdlib>

#define COUNT 128

using namespace std;

typedef struct _CARINFO {
	int id; unsigned int arrive; unsigned int cross;
	int id_NB; int id_SB;
	string dir;
}CARINFO;

void *carThread(void * arg); 
void *tunnelThread(void * arg); 

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
void *carThread(void * arg) 
{
	CARINFO * pCar = (struct _CARINFO *) arg;
	string tdir = pCar->dir;
	int tid_NB = pCar->id_NB;
	int tid_SB = pCar->id_SB;
	unsigned int tcross = pCar->cross;	
	bool delayed = false;

	// Arrival
	pthread_mutex_lock(&lock);
	if (tdir == "N")
		cout << "Northbound car # " << tid_NB << " arrives at the tunnel." << endl;
	else
		cout << "Southbound car # " << tid_SB << " arrives at the tunnel." << endl;		

	pthread_mutex_unlock(&lock);
	
	pthread_mutex_lock(&lock);
	if (   nCarsInTunnel == MaxNcarsInTunnel 	 || 
		((tdir == "N") && (nNBCarsInTunnel == nNBcars)) 	  	 ||
	        ((tdir == "S") && (nSBCarsInTunnel == nSBcars)) 
	      )
	{
		pthread_mutex_lock(&routeLock);
		if ((tdir == direction && nCarsInTunnel == MaxNcarsInTunnel) ||
		    (tdir == direction && nNBCarsInTunnel == nNBcars) ||
		    (tdir == direction && nSBCarsInTunnel == nSBcars)) 
		{
			delayed = true;
			if (tdir == "N")
				cout << "Northbound car # " << tid_NB << " has to wait." << endl;
			else
				cout << "Southbound car # " << tid_SB << " has to wait." << endl;	
		}
		pthread_mutex_unlock(&routeLock);
	}

	while (   nCarsInTunnel == MaxNcarsInTunnel 	 || 
		((tdir == "N") && (nNBCarsInTunnel == nNBcars)) 	  	 ||
	        ((tdir == "S") && (nSBCarsInTunnel == nSBcars)) 
	      )
	{
		pthread_cond_wait(&condition, &lock);
	}

	if (delayed == true) {
		waitCarsCount++;
		delayed == false;
	}
	nCarsInTunnel++;
	if (tdir == "N") {
		cout << "Northbound car # " << tid_NB << " enters the tunnel." << endl;
		nNBCarsInTunnel++;
	} else {
		cout << "Southbound Car # " << tid_SB << " enters the tunnel." << endl;
		nSBCarsInTunnel++;
	}
	pthread_mutex_unlock(&lock);
	sleep(tcross);


	// Tunnel exit
	pthread_mutex_lock(&lock);
	nCarsInTunnel--;
	pthread_mutex_unlock(&lock);

	pthread_mutex_lock(&routeLock);
	if (direction == "N") { 
		pthread_cond_broadcast(&condition);
	}
	else if (direction == "S")
		pthread_cond_broadcast(&condition);
	pthread_mutex_unlock(&routeLock);

	pthread_mutex_lock(&lock);
	if (tdir == "N")
	{
		cout << "Northbound car # " << tid_NB << " exits the tunnel." << endl;
		nNBCarsInTunnel--;	
		//northBoundCars++;
	}
	else
	{
		cout << "Southbound car # " << tid_SB << " exits the tunnel." << endl;
		nSBCarsInTunnel--;
		//southBoundCars++;
	}
	pthread_mutex_unlock(&lock);

	pthread_exit(NULL);
}

// Thread for tunnel
void * tunnelThread(void * arg)
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

	// Create threads	
	pthread_create(&threadID, NULL, tunnelThread, NULL);

	// Get further info from input file
	while (cin >> cars[count].arrive >> cars[count].dir >> cars[count].cross)
	{
		cars[count].id = count + 1;	

		if (cars[count].dir == "N") {
			cars[count].id_NB = i++;
			northBoundCars++;
		} else {
			cars[count].id_SB = j++;
			southBoundCars++;
		}
		count++;		
	}
	// create threads
	for (int i = 0; i < count; i++)
	{
		sleep(cars[i].arrive);
		pthread_create(&carThreads[i], NULL, carThread, (void*)&cars[i]);
	}

	// Wait for termination
	for (int i = 0; i < count; i++)
		pthread_join(carThreads[i], NULL);

	// Display summary
	cout << northBoundCars << " northbound car(s) crossed the tunnel." << endl;
	cout << southBoundCars << " southbound car(s) crossed the tunnel." << endl;
	cout << waitCarsCount << " car(s) had to wait." << endl;

	pthread_mutex_destroy(&routeLock);
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&condition);

	return 0;
}

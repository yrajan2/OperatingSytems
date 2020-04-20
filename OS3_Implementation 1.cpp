#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include<iomanip>
using namespace std;
//This program implements Queue transformer (Size: 5)
// Include Frame Generator Function
double* generate_frame_vector(int l);

//Declare Threads
void* camera(void* arg);
void* transformer(void* arg);
void* estimator(void* arg);

double* copy_ptr(double* a, int s)
{
	double* ret = (double*)malloc(s * sizeof(double));
	int i;
	for (i = 0; i < s; i++)
	{
		ret[i] = a[i];
	}
	return ret;
}

//Declare Semaphores
sem_t cacheEl; // Number of elements in cache ready for transformation
sem_t TREl; // Number of elements in tempRecorder ready for MSE calculation
sem_t cacheSpace; // Number of empty spaces in cache
sem_t TRSpace; // Number of empty spaces in tempRecorder


bool allProcessed = false; // Declare flag for all frames processed
int gctr = 0; //global counter

//Make Nodes
struct frames
{
	double* fr;
};

//Make Cache Queue
static struct frames cache[6]; //Implemented circular queue and therefore we need size 6 for imitating a queue of size 5
static int Cfront = 0;
static int Crear = 0;

//Make TempRecorder Queue
static struct frames tempRecorder[6]; //Implemented circular queue and therefore we need size 6 for imitating a queue of size 5
static int TRfront = 0;
static int TRrear = 0;

int main(int argc, char* argv[]) {
	int interval = 0;
	if (argc == 2) //check for correct input
	{
		interval = atoi(argv[1]);
		//Initialize Semaphores
		sem_init(&cacheEl, 0, 0);
		sem_init(&TREl, 0, 0);
		sem_init(&cacheSpace, 0, 5);
		sem_init(&TRSpace, 0, 5);
		int c1 = 0;
		int t1 = 0;
		int e1 = 0;
		pthread_t cam;
		pthread_t trans;
		pthread_t est;
		c1 = pthread_create(&cam, NULL, camera, (void*)&interval);
		if (c1)
		{
			cout << "Error when creating camera thread!" << endl;
			exit(-1);
		}
		t1 = pthread_create(&trans, NULL, transformer, NULL);
		if (t1)
		{
			cout << "Error when creating transformer thread!" << endl;
			exit(-1);
		}
		e1 = pthread_create(&est, NULL, estimator, NULL);
		if (e1)
		{
			cout << "Error when creating estimator thread!" << endl;
			exit(-1);
		}
		c1 = pthread_join(cam, NULL); //wait for camera thread to end
		t1 = pthread_join(trans, NULL);//wait for transformer thread to end
		e1 = pthread_join(est, NULL);//wait for estimator thread to end
		pthread_exit(NULL);
	}
}

void* camera(void* arg)
{
	int interval = *(int*)arg;

	while (true)
	{
		sem_wait(&cacheSpace); //Reduce Space in Cache
		double* f = generate_frame_vector(8);
		if (f == NULL)
		{
			allProcessed = true; //Signal all entries processed
			break;
		}
		else
		{
			gctr++;
			cache[Crear].fr = f;
			Crear++;
			Crear = Crear % 6;
			sleep(interval);
			sem_post(&cacheEl); //Signal transformer -- Update elements in cache
			
		}
	}
	pthread_exit(NULL); //camera thread ended 
}

void* transformer(void* arg)
{
	int tctr = 0;
	int CTfront = 0;
	while (true)
	{
		sem_wait(&cacheEl); //Wait for camera's signal to check if there are any elements in the cache
		sem_wait(&TRSpace); //Wait for estimator's signal equal to the number of empty spaces in temp recorder
		tempRecorder[TRrear].fr = copy_ptr(cache[CTfront].fr, 8);
		tctr++;

		TRrear++;
		TRrear = TRrear % 6;
		
		for (int i = 0; i < 8; i++)
		{
			double rem = 0;
			tempRecorder[CTfront].fr[i] *= 10;
			rem = remainder(tempRecorder[CTfront].fr[i], 1);
			if (rem <= 0.5)
			{
				tempRecorder[CTfront].fr[i] = tempRecorder[CTfront].fr[i] - rem;
			}
			else
			{
				tempRecorder[CTfront].fr[i] = tempRecorder[CTfront].fr[i] + (1 - rem);
			}
			tempRecorder[CTfront].fr[i] /= 10;
			
		}
		sleep(3);
		sem_post(&TREl); //Signal estimator (update element Count in tempRecord)
		//Check all entries processed
		if (allProcessed && tctr == gctr)
		{
			break;
		}
		CTfront++;
		CTfront = CTfront % 6;
		
	}
	pthread_exit(NULL); //transformer thread ended 
}

void* estimator(void* arg)
{
	int ectr = 0;
	while (true)
	{
		sem_wait(&TREl); //Wait for transformer's signal to check if there are elements in tempRecorder
		double MSE = 0;

		for (int i = 0; i < 8; i++)
		{
			double temp;
			temp = (cache[Cfront].fr[i] - tempRecorder[TRfront].fr[i]);
			temp *= temp;
			MSE += temp;
		}
		MSE /= 8;
		ectr++;
		cout << "mse = " << fixed << setprecision(6) << MSE << endl;
		cache[Cfront].fr = NULL;
		Cfront++;
		TRfront++;
		Cfront = Cfront % 6;
		TRfront = TRfront % 6;
		sem_post(&cacheSpace); //Increase empty spaces in cache
		sem_post(&TRSpace); //Increase empty spaces in TempRecorder
		if (allProcessed && ectr == gctr) //Check for all frames processed
		{
			break;
		}
		
	}
	pthread_exit(NULL); //estimator thread ended 
}
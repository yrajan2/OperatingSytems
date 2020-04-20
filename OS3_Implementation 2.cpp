#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include<iomanip>
using namespace std;
//This program implemenst transformer as a single double* array
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
sem_t trans; // Make estimator wait for transformer
sem_t cacheSpace; // Number of empty spaces in cache
sem_t est; //Make Transformer for estimator to calculate MSE


// Declare flag for all frames processed
bool allProcessed = false;
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
double* tempRecorder= new double(); //Implemented circular queue and therefore we need size 6 for imitating a queue of size 5


int main(int argc, char* argv[]) {
	int interval = 0;
	if (argc == 2) //check for correct input
	{
		interval = atoi(argv[1]);
		//Initialize Semaphores
		sem_init(&cacheEl, 0, 0);
		sem_init(&trans, 0, 0);
		sem_init(&cacheSpace, 0, 5);
		sem_init(&est, 0, 1);
		
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
			sem_wait(&cacheSpace); //Reduce Space in Cache
		}
	}
	pthread_exit(NULL); //camera thread ended 
}

void* transformer(void* arg)
{
	int tctr = 0;
	while (true)
	{
		
		
		sem_wait(&cacheEl); //frames present in cache
		sem_wait(&est);
		
		tempRecorder = copy_ptr(cache[Cfront].fr, 8);
		tctr++;

		for (int i = 0; i < 8; i++)
		{
			double rem = 0;
			tempRecorder[i] *= 10;
			rem = remainder(tempRecorder[i], 1);
			if (rem <= 0.5)
			{
				tempRecorder[i] = tempRecorder[i] - rem;
			}
			else
			{
				tempRecorder[i] = tempRecorder[i] + (1 - rem);
			}
			tempRecorder[i] /= 10;

		}
		sleep(3);
		
		sem_post(&trans);//signal estimator 
		
		if (allProcessed && tctr == gctr)
		{
			break;
		}
		

	}
	pthread_exit(NULL); //transformer thread ended 
}

void* estimator(void* arg)
{
	
	int ectr = 0;
	while (true)
	{
		
		double MSE = 0;
		sem_wait(&trans); //begin MSE calculation
		
		for (int i = 0; i < 8; i++)
		{
			double temp;
			temp = (cache[Cfront].fr[i] - tempRecorder[i]);
			temp *= temp;
			MSE += temp;
		}
		
		
		MSE /= 8;
		ectr++;
		cout << "mse = " << fixed << setprecision(6) << MSE << endl;
		cache[Cfront].fr = NULL;
		Cfront++;
		Cfront = Cfront % 6;
		sem_post(&est); // complete MSE calculation and allow transformer to work now
		
		sem_post(&cacheSpace); //Increase empty spaces in cache

		if (allProcessed && ectr == gctr) //Check for all frames processed
		{
			break;
		}
		
	}
	pthread_exit(NULL); //estimator thread ended 
}
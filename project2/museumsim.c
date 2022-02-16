#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "museumsim.h"

//
// In all of the definitions below, some code has been provided as an example
// to get you started, but you do not have to use it. You may change anything
// in this file except the function signatures.
//

struct shared_data
{
	// Add any relevant synchronization constructs and shared state here.
	// For example:
	pthread_mutex_t mutex;
	int tickets;
	int visitorsWaiting;
	int visitorsInside;
	int guidesInside;
	int guidesWaiting;
	int numVisitors;

	pthread_cond_t waitingGuide;
	pthread_cond_t visitorReady;
	pthread_cond_t guideReady;
	pthread_cond_t visitorArrived;
	pthread_cond_t visitorLeft;
	pthread_cond_t guideArrived;
};

static struct shared_data shared;

/**
 * Set up the shared variables for your implementation.
 * 
 * `museum_init` will be called before any threads of the simulation
 * are spawned.
 */
void museum_init(int num_guides, int num_visitors)
{
	pthread_mutex_init(&shared.mutex, NULL);
	shared.tickets = MIN(VISITORS_PER_GUIDE * num_guides, num_visitors);
	shared.numVisitors = num_visitors;
	shared.visitorsWaiting = 0;
	shared.visitorsInside = 0;
	shared.guidesInside = 0;
	shared.guidesWaiting = 0;

	pthread_cond_init(&shared.waitingGuide, NULL);
	pthread_cond_init(&shared.visitorReady, NULL);
	pthread_cond_init(&shared.guideReady, NULL);
	pthread_cond_init(&shared.visitorArrived, NULL);
	pthread_cond_init(&shared.visitorLeft, NULL);
	pthread_cond_init(&shared.guideArrived, NULL);
}

/**
 * Tear down the shared variables for your implementation.
 * 
 * `museum_destroy` will be called after all threads of the simulation
 * are done executing.
 */
void museum_destroy()
{
	//Remeber to destroy everything
	//almost like freeing after mallocing
	pthread_mutex_destroy(&shared.mutex);

	pthread_cond_destroy(&shared.waitingGuide);
	pthread_cond_destroy(&shared.visitorReady);
	pthread_cond_destroy(&shared.guideReady);
	pthread_cond_destroy(&shared.visitorLeft);
	pthread_cond_destroy(&shared.visitorArrived);
	pthread_cond_destroy(&shared.guideArrived);
}

/**
 * Implements the visitor arrival, touring, and leaving sequence.
 */
void visitor(int id)
{

	pthread_mutex_lock(&shared.mutex); //LOCK
	{
		//the visitor is going to be coming regardless
		visitor_arrives(id);

		//this part decides if they can do the rest.
		//if there are enough tickets then the visitor is allowed in
		if (shared.tickets > 0)
		{
			shared.visitorsWaiting += 1;				 //puts them ina waiting room
			shared.tickets -= 1;						 //one less ticket
			shared.visitorsInside += 1;					 //one more person inside fear covid
			pthread_cond_signal(&shared.visitorArrived); //let the guide know that I AM
		}
		else
		{
			//if there is no tickets left then the visitor leaves.
			visitor_leaves(id);

			pthread_mutex_unlock(&shared.mutex); //LOCK
			return;
			//i had this return statement but it BROKE because the whole prgram just stopped
		}

		//sometimes the guide isnt here yet so wait for him to get there
		while (shared.guidesInside == 0)
		{ //while instead of if to protect from spurious wakeup
			pthread_cond_wait(&shared.guideArrived, &shared.mutex);
		}

		//let the guide know that we are ready to tour
		pthread_cond_signal(&shared.visitorReady);

		//wait for guide to let us know we can tour now
		pthread_cond_wait(&shared.guideReady, &shared.mutex);
	}
	pthread_mutex_unlock(&shared.mutex); //UNLOCK

	visitor_tours(id); //tours

	pthread_mutex_lock(&shared.mutex); //LOCK
	{
		//IDK if the lock fucks it up or not?
		//better output when the leave was inside the lock;
		visitor_leaves(id);			//right after touring, visitor leaves
		shared.visitorsInside -= 1; //1 less lonely girllll

		//if theres no one inside the museum
		if (shared.visitorsInside == 0)
		{ //let the guide know they can leave
			pthread_cond_broadcast(&shared.visitorLeft);
		}
	}
	pthread_mutex_unlock(&shared.mutex); //UNLOCK
}

/**
 * Implements the guide arrival, entering, admitting, and leaving sequence.
 */
void guide(int id)
{
	pthread_mutex_lock(&shared.mutex); //LOCK
	{
		//guide will arrive for work no matter what
		guide_arrives(id);

		//lets the guide inside if they are allowed inside
		if (shared.guidesInside < 2)
		{ //if theres less than 2 people in the museum
			//let this guide in
			guide_enters(id);
			shared.guidesInside += 1;
		}
		else
		{ //this means that there are 2 guides inside already
			while (shared.guidesInside == 2)
			{
				//we will wait until they leave for our shift
				pthread_cond_wait(&shared.waitingGuide, &shared.mutex);
			}

			guide_enters(id);
			shared.guidesInside += 1;
		}
		//once the guides are inside let the visitors know we are inside.

		pthread_cond_signal(&shared.guideArrived);

		//ask if them bitches are ready?
		pthread_cond_wait(&shared.visitorReady, &shared.mutex);

		//this is basically a var to keep track of if the person served 10 people
		int canServe = 0;
		while (shared.numVisitors != 0 && canServe != 10)
		{
			//theres 2 cases, either everyone in the WR OR, 10 people. whichever one is least

			if (shared.visitorsWaiting == 0) //this waits for atleast 1 person inside the WR
			{
				pthread_cond_wait(&shared.visitorArrived, &shared.mutex);
			}

			//once we know that people are inside, admit them
			guide_admits(id);

			//do the necessary decrements
			shared.numVisitors -= 1;
			shared.visitorsWaiting -= 1;
			canServe += 1;

			pthread_cond_signal(&shared.guideReady); //signal that guide is DONE
		}

		//wait for there to be NO visitors inside
		pthread_cond_wait(&shared.visitorLeft, &shared.mutex);

		if (shared.visitorsInside == 0) //double check, this made the code work better
		{
			while (shared.guidesInside != 0) //guides must leave together so this waits for them
			{
				shared.guidesInside -= 1;
				guide_leaves(id);
				//LEAVE together
			}
		}

		if (shared.guidesWaiting > 0)
		{ //incase there was guides waiting this lets them inside.
			pthread_cond_broadcast(&shared.waitingGuide);
		}
	}
	pthread_mutex_unlock(&shared.mutex); //UNLOCK
}

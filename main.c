#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include "queue.h"
#include "gel.h"
#define ARRIVAL 1
#define DEPARTURE 2

static int recieved;
static int dropped;
static int MAXBUFFER;
static int qLength;
static double arrivalRate;
static double serviceRate;
static double clk; 
static double timeBusy;
static double timePrev;
static double timeQ;
static queue_t myQ;
static dll_t gel;

double negative_exponentially_distributed_time (double rate)
{
    double u;
    u = drand48();
    return ((-1/rate)*log(1-u));
}

int initialization()
{
    dropped = 0;
    qLength = 0;
    clk = 0;
    timeBusy = 0;
    timePrev = 0;
    timeQ = 0;
    double timeS;
    double timeL;

    printf("\n");
    printf("***********************************************\n");
    printf("                INITIALIZING\n");
    printf("             Getting Parameters\n");
    printf("***********************************************\n");
    printf("\n");

    printf("Arrival rate:\n >");
	scanf("%lf",&arrivalRate);
	printf("Arrival Rate: %.3f \n", arrivalRate);
    if (arrivalRate < 0 || arrivalRate > 1)
    {
        printf("initialization(): invalid arrivalRate\n");
        return -1;
    }
    printf("\n");
    printf("Service rate:\n >");
	scanf("%lf",&serviceRate);
	printf("Service Rate: %.3f \n", serviceRate);
    if (serviceRate < 0 || serviceRate > 1)
    {
        printf("initialization(): invalid arrivalRate\n");
        return -1;
    }
    printf("\n");
    printf("MAXBUFFER:\n >");
	scanf("%d",&MAXBUFFER);
	printf("MAXBUFFER: %d \n", MAXBUFFER);
    if (MAXBUFFER < 0)
    {
        printf("initialization(): invalid MAXBUFFER\n");
        return -1;
    }
    printf("\n");

    srand(time(NULL));
    myQ = queue_create();
    gel = gel_create();
    
    /* 
     * Gernerating the first event
     * Listing in GEL
     */
    event_t newA;
    newA = (event_t)malloc(sizeof(struct event));
    newA->timeS = negative_exponentially_distributed_time(arrivalRate);
    newA->timeL = negative_exponentially_distributed_time(serviceRate);
    timeBusy += newA->timeL;
    newA->type = ARRIVAL;    
    int result = gel_insert(gel, newA);
    if (result)
    {
        printf("initialization(): First event failed\n");
        return -1;
    }

    return 0;
}

int main()
{
    event_t tempE;

    printf("\n\n\n\n\n");
    printf("|*************************************************|\n");
    printf("|           ECS 152 / EEC 173A PROJECT I          |\n");
    printf("| Simulation Analysis of IEEE 802.x Based Network |\n");
    printf("|                                                 |\n");
    printf("|                   AUTHORS:                      |\n");
    printf("|                  Kaiqi Jin                      |\n");
    printf("|                  Jiaxin Zhao                    |\n");
    printf("|                  Jiayi Li                       |\n");
    printf("|*************************************************|\n");
    printf("\n\n");
    
    if(initialization())
    {
        printf("int main(): initialization() failed\n");
        return -1;
    }

    printf("\n");
    printf("***********************************************\n");
    printf("            INITIALIZATION SUCCESS\n");
    printf("            Start Reciving Packages\n");
    printf("***********************************************\n");
    printf("\n");


    for (int i = 0; i < 10000; i++)
    {
        if (gel_length(gel) == 0)
        {
            printf("gel is empty\n");
            printf("System Clock: %.3f\n", clk);
            break;
        }
        if (gel_remove(gel, &tempE))
        {
            printf("int main(): failed to get the first event in gel\n");
            return -1;
        }

        clk = tempE->timeS;
        timeQ += qLength * (clk - timePrev);
        timePrev = clk;

        /*
         * When tempE is an ARRIVAL event
         * Process a upcoming event
         * Create an ARRIVAL event
         * Pushing into gel
         */
        if(tempE->type == ARRIVAL)
        {
            recieved++;
            event_t newA;
            newA = (event_t)malloc(sizeof(struct event));
            newA->timeS = clk + negative_exponentially_distributed_time(arrivalRate);
            newA->timeL = negative_exponentially_distributed_time(serviceRate);
            timeBusy += newA->timeL;
            newA->type = ARRIVAL;
            if (gel_insert(gel, newA))
            {
                printf("int main(): Pushing new ARRIVAL event failed\n");
                return -1;
            }

            /*
             * Deal with current event
             */

            /*
             * If myQ is empty
             * Create a DEPARTURE event
             * Pushing into gel
             */
            if (qLength == 0) 
            {
                qLength ++;
                event_t newD;
                newD = (event_t)malloc(sizeof(struct event));
                newD->timeS = clk + tempE->timeL;
                newD->timeL = 0;
                newD->type = DEPARTURE;
                if (gel_insert(gel, newD))
                {
                    printf("int main(): Pushing new DEPARTURE event failed\n");
                    return -1;
                }
            }
            /*
             * If myQ has elemnts
             * Pushing the DEPARTURE event into myQ
             * If myQ is full, drop it
             */
            else
            {
                if (qLength < MAXBUFFER + 1)
                {
                    event_t newD;
                    newD = (event_t)malloc(sizeof(struct event));
                    newD->timeS = 0;
                    newD->timeL = tempE->timeL;
                    newD->type = DEPARTURE;
                    if (queue_enqueue(myQ, newD))
                    {
                        printf("int main(): Fail to enqueue myQ\n");
                        return -1;
                    }
                    qLength++;
                }
                else
                {
                    dropped++;
                    timeBusy -= tempE->timeL;
                    timeQ -= clk - tempE->timeS;
                }
            }  
            free(tempE);
        }
        /*
         * When tempE is a DEPARTURE event
         * Dequeue myQ and push the event into gel
         */
        else 
        {
            qLength--;
            free(tempE);
            if(qLength != 0)
            {
                
                event_t newD;
                if (queue_dequeue(myQ, (void**)&newD))
                {
                    printf("int main(): Fail to dequeque myQ\n");
                    return -1;
                }
                newD->timeS = clk + newD->timeL;
                if (gel_insert(gel, newD))
                {
                    printf("int main(): Pushing new DEPARTURE event failed\n");
                    return -1;
                }
            }
        }
    }
    printf("\n");
    printf("***********************************************\n");
    printf("                FINISH RUNNING\n");
    printf("               Ready for Output\n");
    printf("***********************************************\n");
    printf("\n");
    printf("Arrival Rate:                   %.3f\n", arrivalRate);
    printf("Serivce Rate:                   %.3f\n", serviceRate);
    printf("MAXBUFFER:                      %d\n", MAXBUFFER);
    printf("-----------------------------------------------\n");
    printf("System Clock:                   %.3f\n", clk);
    printf("Server Busy Time:               %.3f\n", timeBusy);
    printf("Server Usage Percentage:        %.3f%%\n", (timeBusy/clk)*100);
    printf("Package Recieved:               %d\n", recieved);
    printf("Package Dropped:                %d\n", dropped);
    printf("Package Loss:                   %.3f%%\n", ((double)dropped/(double)recieved)*100);
    printf("Total waiting time in Queue:    %.3f\n", timeQ);
    printf("Average Queue length:           %.3f\n", (timeQ-timeBusy)/clk);
    printf("Average Queue time:             %.3f\n", (timeQ-timeBusy)/timeBusy);
    printf("\n");
    printf("***********************************************\n");
    printf("                CLEARING AND EXIT\n");    
    printf("***********************************************\n");
    printf("\n\n\n");

    /*
     *  CLEARING AND EXIT
     */

    while (queue_length(myQ) != 0) 
    {
        if (queue_dequeue(myQ, (void**)&tempE))
        {
            printf("int main(): Fail to dequeue\n");
            return -1;
        }
        free(tempE);
    }
    
    while (gel_length(gel) != 0) 
    {
        if (gel_remove(gel, &tempE))
        {
            printf("int main(): Fail to remove the first event in gel\n");
            return -1;
        }
        free(tempE);
    }

    gel_destroy(gel);
    queue_destroy(myQ);

    return 0;
}
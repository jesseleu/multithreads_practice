#include <iostream> // cout
#include <sstream>  // stringstream
#include <string>   // string
#include "Shop_org.h"

Shop_org::Shop_org(int Barbers, int nChairs) //constructor
{
    if(Barbers <= 0) //at least need 1 barber
    {
        Barbers = DEFAULT_BARBERS;
    }
    if(nChairs < 0) //at least need 0 chair
    {
        nChairs = DEFAULT_CHAIRS;
    }
    this->nBarbers = Barbers;
    this->max = nChairs;
    pthread_mutex_init(&mutex, NULL);
    
    for(int i = 0; i < nBarbers; i++)
    {
        barberList[i].id = i;
        pthread_cond_init(&barberList[i].cond_barber_sleeping, NULL);
        pthread_cond_init(&barberList[i].cond_barber_paid, NULL);
    }
}

Shop_org::Shop_org( ) //constructor
{
    this->nBarbers = DEFAULT_BARBERS;
    this->max = DEFAULT_CHAIRS;
    
    pthread_mutex_init(&mutex, NULL);
    
    
    for(int i = 0; i < nBarbers; i++)
    {
        barberList[i].id = i;
        pthread_cond_init(&barberList[i].cond_barber_sleeping, NULL);
        pthread_cond_init(&barberList[i].cond_barber_paid, NULL);
    }
}

void  Shop_org::Customer::init()  // customer init thread cond
{
    pthread_cond_init(&cond_customers_waiting, NULL);
    pthread_cond_init(&cond_customers_sit, NULL);
}
string Shop_org::int2string(int i)
{
    stringstream out;
    out << i;
    return out.str( );
}

void Shop_org::print(int person, string message)
{
    cout << (( person > 0 ) ? "customer[" : "barber  [")
    << abs(person) << "]: " << message << endl;
}

int Shop_org::visitShop(int customerId)
{
    pthread_mutex_lock(&mutex);
    customerList[customerId] = Customer(); // create new customer
    customerList[customerId].id = customerId; // put into list
    customerList[customerId].init();
    int barberId = UNSET;
    if(waiting_chairs.size() == max && sleepingBarbers.empty())
    {
        print(customerId,"leaves the shop because of no available waiting chairs.");
        nDropsOff++;
        pthread_mutex_unlock(&mutex);
        return UNSET;
    }

    if(sleepingBarbers.empty()) // no available barber
    {
        waiting_chairs.push(customerId);
        print( customerId, "takes a waiting chair. # waiting seats available = "
              + int2string( (int)(max - waiting_chairs.size( ) )));
        
        while (customerList[customerId].myBarber == UNSET) //wait for pair
        {
            pthread_cond_wait(&customerList[customerId].cond_customers_waiting, &mutex);
        }
        barberId = customerList[customerId].myBarber;
    }
    else  //there are sleeping barbers in a queue
    {
        barberId = sleepingBarbers.front();
        sleepingBarbers.pop();
        customerList[customerId].myBarber = barberId; //pair
        barberList[barberId] . myCustomer = customerId;
    }
    
    print(customerId, "moves to the service chair[" + int2string(barberId) + "], # waiting seats available = "
          + int2string((int) (max-waiting_chairs.size())));
    
    
    customerList[customerId].serving = true; //sit down & tell barber
    pthread_cond_signal(&(barberList[barberId].cond_barber_sleeping));
    pthread_mutex_unlock(&mutex);
    return barberId;
}

void Shop_org::leaveShop(int customerId, int barberId)
{
    pthread_mutex_lock(&mutex);
    print(customerId, "wait for barber[" + int2string(barberId) + "] to be done with hair-cut  ");
    
    while(customerList[customerId].myBarber != UNSET) //sit and wait
    {
        pthread_cond_wait(&customerList[customerId].cond_customers_sit, &mutex);
    }
    
    print(customerId, "says good-bye to the barber[" +  int2string(barberId) + "]");
    
    customerList[customerId].paid = true;
    pthread_cond_signal(&barberList[barberId].cond_barber_paid); //paid
    pthread_mutex_unlock(&mutex);
}

void Shop_org::helloCustomer(int barberId)
{
    pthread_mutex_lock(&mutex);
    if (barberList[barberId].myCustomer == UNSET) // if no customer
    {
        print(- barberId, "sleeps because of no customers.");
        sleepingBarbers.push(barberId);
        while (barberList[barberId].myCustomer == UNSET)
        {
            pthread_cond_wait(&(barberList[barberId].cond_barber_sleeping), &mutex);//sleep
        }
    }
    
    while( ! customerList[barberList[barberId].myCustomer].serving) //wait for customer to sit
    {
        pthread_cond_wait(&(barberList[barberId].cond_barber_sleeping), &mutex);
    }
    
    print( - barberId,
          "starts a hair-cut service for customer[" + int2string( barberList[barberId].myCustomer ) + "]");
    
    pthread_mutex_unlock(&mutex);
}

void Shop_org::byeCustomer(int barberId)
{
    pthread_mutex_lock(&mutex);
    print( - barberId, "says he's done with a hair-cut service for customer[" +
          int2string(barberList[barberId].myCustomer)  + "]");
    
    customerList[barberList[barberId].myCustomer].serving = false;
    customerList[barberList[barberId].myCustomer].myBarber = UNSET;
    pthread_cond_signal(&customerList[barberList[barberId].myCustomer].cond_customers_sit); // tell the customer it is done
    while( ! customerList[barberList[barberId].myCustomer].paid) //wait for my customer to pay
    {
        pthread_cond_wait(&(barberList[barberId].cond_barber_paid), &mutex);
    }
    barberList[barberId].myCustomer = UNSET;
    
    print(barberId, "calls in another customer");
    if( ! waiting_chairs.empty()) // call next customer
    {
        int customerId = waiting_chairs.front();
        waiting_chairs.pop();
        barberList[barberId].myCustomer = customerId;
        customerList[customerId].myBarber = barberId;
        pthread_cond_signal(&customerList[customerId].cond_customers_waiting);
    }
    
    pthread_mutex_unlock(&mutex);
}


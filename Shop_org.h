#ifndef _SHOP_ORG_H_
#define _SHOP_ORG_H_
#include <pthread.h>
#include <queue> 
#include <map>
#include <iostream>
using namespace std;
#define DEFAULT_CHAIRS 3 // default chair
#define DEFAULT_BARBERS 1 //default barber
#define UNSET -999

class Shop_org
{
public:
    Shop_org(int nBarbers, int nChairs);
    Shop_org( );
    int visitShop(int customerId);
    
    void leaveShop(int customerId, int barberId);
    void helloCustomer(int barberId);
    void byeCustomer(int barberId);
    string int2string(int i);
    void print(int person, string message) ;
    int nDropsOff = 0;
    
private:
    int nBarbers;
    int max;
    struct Barber
    {
        
        int id;
        pthread_cond_t cond_barber_sleeping;
        pthread_cond_t cond_barber_paid;
        int myCustomer = UNSET;
    };
    
    struct Customer
    {
        int id;
        pthread_cond_t cond_customers_waiting;
        pthread_cond_t cond_customers_sit;
        void init();
        bool wait = true;
        bool paid = false;
        bool serving = false;
        int myBarber = UNSET;
    };
    
    
    map<int, Barber> barberList; 
    map<int, Customer> customerList;

    queue<int> waiting_chairs;
    queue<int> sleepingBarbers;
    
    pthread_mutex_t mutex;
    
    
};

#endif

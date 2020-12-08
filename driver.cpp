#include <iostream>    // cout
#include <sys/time.h> // gettimeofday
#include <unistd.h> //usleep
#include "Shop_org.h"
#define UNSET -999
using namespace std;


void *barber(void *);
void *customer(void *);

class ThreadParam
{
public:
    ThreadParam(Shop_org *shop, int id, int serviceTime) :
    shop(shop), id(id), serviceTime(serviceTime) { };
    Shop_org *shop;
    int id;
    int serviceTime;
};

int main(int argc, char *argv[])
{
    if ( argc != 5 ) {
      cerr << "invalid command line arg" << endl;
      return -1;
    }
     int nBarbers = atoi( argv[1] );
     int nChairs = atoi( argv[2] );
     int nCustomers = atoi( argv[3] );
     int serviceTime = atoi( argv[4] );
    
    pthread_t barber_thread[nBarbers]; //array barber thread
    pthread_t customer_threads[nCustomers];//array cus thread
    Shop_org shop(nBarbers, nChairs);
    
    for(int i = 0; i < nBarbers; i++)
    {
        ThreadParam *param = new ThreadParam( &shop, i, serviceTime );
        pthread_create( &barber_thread[i], NULL, barber, (void *)param );
    }
    for(int i = 0; i < nCustomers; i++)
    {
        usleep(rand( ) % 1000);
        ThreadParam *param = new ThreadParam(&shop, i + 1, 0);
        pthread_create(&customer_threads[i], NULL, customer, (void *)param);
    }
    
    for( int i = 0; i < nCustomers; i++)
        pthread_join(customer_threads[i], NULL);
    
    for(int i = 0; i < nBarbers; i++)
        pthread_cancel(barber_thread[i]);
    cout << "# customers who didn't receive a service = " << shop.nDropsOff
    << endl;
    
    return 0;
}


void *barber(void *arg)
{
    ThreadParam &param = *(ThreadParam *)arg;
    Shop_org &shop = *(param.shop);
    int id = param.id;
    int serviceTime = param.serviceTime;
    delete &param;
    
    
    while(true)
    {
        shop.helloCustomer(id);
        usleep(serviceTime);
        shop.byeCustomer(id);
    }
    return NULL;
}


void *customer(void *arg)
{
    ThreadParam &param = *(ThreadParam *)arg;
    Shop_org &shop = *(param.shop);
    int id = param.id;
    delete &param;
    
    int barber = shop.visitShop(id);
    if (barber != UNSET) // if succeed
        shop.leaveShop(id, barber);
    
    return NULL;
}

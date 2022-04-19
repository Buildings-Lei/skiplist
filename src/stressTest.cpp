#include <iostream>
#include<string>
#include<pthread.h>
#include<vector>
#include<mutex>
#include "skiplist.h"
using namespace std;
using namespace MyDatebase;
#define NUM_THREADS 8
#define TEST_COUNT 100000
static skiplist<int, std::string> skip(18);
static vector<int> veckey;
static std::mutex mymtx;

void *insertElement(void* threadid) {
    long long tid;
    tid = reinterpret_cast<long long>(threadid);
    std::cout << tid << std::endl;
    int tmp = TEST_COUNT/NUM_THREADS;
    for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
        int key = rand() % TEST_COUNT;
        mymtx.lock();
        skip.insertElement(key, "a");
        veckey.push_back(key);
        mymtx.unlock();
    }
    pthread_exit(NULL);

}

void *getElement(void* threadid) {
    long long tid;
    tid = reinterpret_cast<long long>(threadid);
    std::cout << tid << std::endl;
    int tmp = TEST_COUNT/NUM_THREADS;
    for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
        skip.searchElement(rand() % TEST_COUNT);
    }
    pthread_exit(NULL);
}

void *delElement( void* threadid ){
    long long tid;
    tid = reinterpret_cast<long long>(threadid);
    std::cout << tid << std::endl;
    int tmp = TEST_COUNT/NUM_THREADS;
    for (int i=tid*tmp, count=0; count<tmp; i++) {
        count++;
        mymtx.lock();
        if( veckey.size() > 0){
            int key = veckey.back();
            veckey.pop_back();
            skip.deleteElement(key);
        }
        mymtx.unlock();
    }
    pthread_exit(NULL);
}
int main()
{
    srand (time(NULL));
        {

            pthread_t threads[NUM_THREADS * 2];
            int rc;
            int i;

            auto start = std::chrono::high_resolution_clock::now();

            for( i = 0; i < NUM_THREADS; i++ ) {
                std::cout << "main() : creating thread, " << i << std::endl;
                rc = pthread_create(&threads[2*i], NULL, insertElement, reinterpret_cast<void*>(2*i) );
                rc = rc || pthread_create(&threads[2*i + 1], NULL, delElement, reinterpret_cast<void *>(2*i + 1));
                if (rc) {
                    std::cout << "Error:unable to create thread," << rc << std::endl;
                    exit(-1);
                }
            }

            void *ret;
            for( i = 0; i < NUM_THREADS*2; i++ ) {
                if (pthread_join(threads[i], &ret) !=0)  {
                    perror("pthread_create() error");
                    exit(3);
                }
            }
            auto finish = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = finish - start;
            std::cout << "insert elapsed:" << elapsed.count() << std::endl;
        }
        cout<<" elements : "<<skip.size()<<endl;
         {
             pthread_t threads[NUM_THREADS];
             int rc;
             int i;
             auto start = std::chrono::high_resolution_clock::now();

             for( i = 0; i < NUM_THREADS; i++ ) {
                 std::cout << "main() : creating thread, " << i << std::endl;
                 rc = pthread_create(&threads[i], NULL, insertElement, reinterpret_cast<void *>(i));

                 if (rc) {
                     std::cout << "Error:unable to create thread," << rc << std::endl;
                     exit(-1);
                 }
             }

             void *ret;
             for( i = 0; i < NUM_THREADS; i++ ) {
                 if (pthread_join(threads[i], &ret) !=0 )  {
                     perror("pthread_create() error");
                     exit(3);
                 }
             }

             auto finish = std::chrono::high_resolution_clock::now();
             std::chrono::duration<double> elapsed = finish - start;
             std::cout << "get elapsed:" << elapsed.count() << std::endl;
         }
        cout<<" elements : "<<skip.size()<<endl;
        skip.display_list();
        pthread_exit(NULL);
    return 0;
}

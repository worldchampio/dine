#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <assert.h>
#include <random>
#include <cmath>

/*
Dining Philosophers implementation

This was an exercise in using threads, mutexes, dynamic memory allocation and general OOP. 
Currently, the only limiting factor of the table size (no. of philosophers) is entries in 
the vector placed in main() holding the names. By expanding it, every other aspect is 
written scaleable in terms of memory and instantiation. This was done taking the input 
number in a for loop and casting the index to a string, creating n diners:
diner 1, diner 2, ..., diner n

The simulation time is driven by the integer (1-50) returned from Philosopher::rng(),
which is also passed to the chrono::sleep_for() function so that every time any given 
philosopher eats/thinks, the duration will be random. As a result, time t stored in
Table class is written to by all diners, which is terrible practice -> fix
*/

// Holds fork (mutex) and public ptr
class Fork
{
    std::mutex fork;
public:
    std::mutex *fork_pointer = &fork;
};

// Impl. a diner, taking in name, fork ptrs then logs eat/think time
class Philosopher
{
    std::string name = {""};
    std::mutex* ptr_l;
    std::mutex* ptr_r;

    // Stores the sum of all time spent eating/thinking
    volatile int eattime   = 0;
    volatile int thinktime = 0;

    /*
    Stores the individual calls to rng() to drive simulationtime
    Named temp because the number is always overwritten next iteration
    */
    volatile int eattime_temp   = 0;
    volatile int thinktime_temp = 0;

    int eatcounter   = 0;
    int thinkcounter = 0;
    bool eating = false;

    int rng() {//Private as its only called within the class
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1,50);
        return dist6(rng);
    }

public:
    Philosopher(std::string _name, std::mutex* _ptr_l, std::mutex* _ptr_r ){
        ptr_l = _ptr_l;
        ptr_r = _ptr_r;
        name  = _name;
    }

    ~Philosopher(){
        std::cout << name <<" ate for " 
                  << (static_cast< float >(eattime))/1000 
                  << "s ("<<eatcounter<<" times), thought for " 
                  << (static_cast< float >(thinktime))/1000<<"s ("
                  << thinkcounter << " times). \n";
    }

    void think(){
        std::cout << name << " started thinking.\n";
     
        thinkcounter++;
        thinktime_temp = rng();
        thinktime += thinktime_temp;
    
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime_temp));
        std::cout << name << " stopped thinking.\n";    
    }

    bool get_fork(){ // Attempt to lock both mutexes, release all if either lock fails
        if ((ptr_r->try_lock() && ptr_l->try_lock())==true) {
            eating=true;
            return eating;
        } else {
            release_fork();
            return false;
        }
    }
    
    void release_fork(){
       ptr_r->unlock(); ptr_l->unlock();
    }

    void eat(){
        /*
        Attempt to get forks->eat, and think if it cannot eat
        */
        if(!get_fork()){
            think();
        } else {
            assert(eating==true); // Will abort execution if mutex lock works incorrectly
            
            std::cout << name << " started eating.\n";
            
            eatcounter++;
            eattime_temp = rng();
            eattime += eattime_temp;

            std::this_thread::sleep_for(std::chrono::milliseconds(eattime_temp));
            
            std::cout << name << " stopped eating.\n";
            release_fork();

            eating = false;
        } 
    }
    
    int get_eattime() {
        return eattime_temp;
    }

    int get_thinktime() {
        return thinktime_temp;
    }
};

//Table keeps track of all diners, forks and starts/ends the meal
class Table
{
    std::vector<std::string> names;
    std::vector<Fork*> forklist_ptr;
    std::vector<Philosopher*> phillist_ptr;
    std::vector<std::thread*> thread_ptr;
    std::vector<int> range;
    
    unsigned int N=0;
    volatile float t = 0;
    float tf= 0;

public:
    Table(unsigned int _N){
        N = _N;
        tf = 1.5*N; // scale simtime with no. of diners

        /*
        Name and fork creation
        Alternatively, a vector of strings could be passed
        to the constructor with actual names, althought its
        length will limit the simulation scalability, ie. number of diners.
        */
        for (int i=0; i<N; i++) {
            names.push_back("Diner " + std::to_string(i+1));
            forklist_ptr.push_back(new Fork);
            range.push_back(i); // Make this vector to use as for-loop range
        }

        /* 
        Create philosopher objects once forks are done
        Forks are implemented as circular, first philosopher uses left N-1 and right 0, 
        all the others use left(j-1) and right (j), the last philosopher uses
        left(j-1) and right(jmax=N-1) -> first and last share, for N=5:
        
        obj | i | left | right
        ----+---+------+------
        ph1 | 0 | N-1  | 0
        ph2 | 1 | 0    | 1
        ph3 | 2 | 1    | 2
        ph4 | 3 | 2    | 3
        ph5 | 4 | 3    | N-1
        ----+---+------+------
        */
        for (auto j : range) {
            if (j==0) { // exception for first instantiation
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[N-1]->fork_pointer,forklist_ptr[j]->fork_pointer));
            } else {
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j-1]->fork_pointer,forklist_ptr[j]->fork_pointer));
            }
        }

        /* 
        Start meal by spawning N threads with 
        pointer to callable Philosopher::eat and pointer to 
        philosopher objects
        */
        while (t<tf){ 
            /* Dynamic thread spawning*/
            for (auto i : range) {
                thread_ptr.push_back( new std::thread{&Philosopher::eat,phillist_ptr[i]});
                
                // Update simulation time

                t += static_cast< float >(phillist_ptr[i]->get_thinktime() + phillist_ptr[i]->get_eattime())/1000;
            }
        }
    }

    ~Table() {
        /* Garbage collection */

        // Join all threads first
        for (auto thr : range) {
            thread_ptr[thr]->join();
        }

        // This seems to stop weird bug when destructors are called
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Deallocate forks, philosophers and threads
        for (auto ptrs : range) {
            delete forklist_ptr[ptrs];
            delete phillist_ptr[ptrs];
            delete thread_ptr[ptrs];
        }
        std::cout << "\nMeal lasted " 
                  << t << "s, " << t-tf << "s overtime.\n";
    }
};

// Ensure only valid ints are input
int integerInput(const std::string& message) { 
    int out=0;
    while(true)
    {
        std::cout << message << std::endl;

        int value(0);

        if (std::cin >> value && abs(value)<21 ) // max 20 diners
        {
          return abs(value);
        }

        std::cerr << "Invalid or above limit, enter again:" << std::endl;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
    }
}

int main()
{
    Table table( integerInput("How many diners? [2-20]") );
    return 0;
}


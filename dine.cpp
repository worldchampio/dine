#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <assert.h>
#include <random>

/*
Dining Philosophers implementation

This was an exercise in using threads, mutexes, dynamic memory allocation and general OOP. 
Currently, the only limiting factor of the table size (no. of philosophers) is entries in 
the vector placed in main() holding the names. By expanding it, every other aspect is 
written scaleable in terms of memory and instantiation. This was done taking the input 
number in a for loop and casting the index to a string, creating n diners:
diner 1, diner 2, ..., diner n

The simulation time is driven by the integer (1-100) returned from Philosopher::rng(),
which is also passed to the chrono::sleep_for() function so that every time any given 
philosopher eats/thinks, the duration will be random. As a result, time t stored in
Table class is written to by all diners, which is terrible practice -> fix????

*/

class Fork
{
    std::mutex fork;
public:
    std::mutex *fork_pointer = &fork;
};

class Philosopher // impl. a diner, taking in name, fork ptrs then logs eat/think time
{
    std::string name;
    std::mutex* ptr_l;
    std::mutex* ptr_r;

    // Stores the sum of all time spent eating/thinking
    int eattime   = 0;
    int thinktime = 0;

    // Stores the individual calls to rng() to drive simulationtime
    int eattime_temp   = 0;
    int thinktime_temp = 0;

    int eatcounter   = 0;
    int thinkcounter = 0;
    bool eating = false;

    int rng() {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1,40);
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
        thinktime+=thinktime_temp;
    
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime_temp));
        std::cout << name << " stopped thinking.\n";    
    }

    bool get_fork(){ // Attempt to lock both mutexes. Release any mutex aquired upon failure.
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
        This implementation is messy, so:

        The philosopher will attempt to lock the mutexes 
        corresponding to the forks available from its position.
        If it cant, it will think. Once it has thought, it will attempt
        to eat again, on repeat. The aim was to make eating prioritized
        over thinking.
        */
        if(!get_fork()){
            think();
        } else {
            assert(eating==true); // Will abort execution if mutex lock works incorrectly

            eatcounter++;
            std::cout << name << " started eating.\n";
            eattime_temp = rng();
            std::this_thread::sleep_for(std::chrono::milliseconds(eattime_temp));
            std::cout << name << " stopped eating.\n";
            eattime += eattime_temp;
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

class Table //Table keeps track of all diners, forks and starts/ends the meal
{    
    std::vector<std::string> names;
    std::vector<Fork*> forklist_ptr;
    std::vector<Philosopher*> phillist_ptr;
    std::thread* thread_ptr;

    unsigned int N=0;
    float t = 0;
    float tf= 0;

public:
    Table(std::vector<std::string> _names, unsigned int _N, float _tf){
        N = _N;
        tf = _tf;
        /* This section dynamically insantiates the required classes so that 
            I dont have to explicitly create a bunch of objects myself
        */
        for (int i=0; i<N; i++) {
            names.push_back(_names[i]);
            forklist_ptr.push_back(new Fork);
        }

        /* Create philosopher objects once forks are done
            i: 0, 1, 2, ... , N-1
            i=0   -> must share with last = N-1
            i=N-1 -> must share with first = 0
        */
        for (int j=0; j<N; j++) {
            if (j==0) { // exception for first instantiation
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[N-1]->fork_pointer,forklist_ptr[j]->fork_pointer));

            } else if(j==N-1){ // exception for last
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[0]->fork_pointer,forklist_ptr[j]->fork_pointer));
            
            } else {
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j-1]->fork_pointer,forklist_ptr[j]->fork_pointer));
            }
        }

        while (t<tf){ 

            /* Dynamic thread spawning*/
            for (int i=0;i<N;i++) {
                thread_ptr = new std::thread{&Philosopher::eat,phillist_ptr[i]};

                t+= static_cast< float >(phillist_ptr[i]->get_thinktime() + phillist_ptr[i]->get_eattime())/1000;
            }
            thread_ptr->join();
        }
    }

    ~Table(){
        /* Garbage collection
        Iterate through the list of pointers to philisopher 
        and fork objects, and delete
        */
        for (int i=0; i<N; i++) {
            delete phillist_ptr[i];
            delete forklist_ptr[i];
        }
        std::cout << "\n\nMeal lasted " 
                  << t << "s, " << t-tf << "s overtime.\n";
    }
};

int main() 
{
    std::vector<std::string> names_;
    
    int num;
    
    std::cout << "How many diners? "; 
    std::cin >> num;
    
    float simtime = 1.5*num;

    for (int i=0; i<num; i++) {
        names_.push_back("Diner "+std::to_string(i+1));
    }

    Table table(names_, num, simtime);

    return 0;
}

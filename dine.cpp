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

*/

class Fork // Holds fork (mutex) and public ptr 
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

    /*
    Stores the individual calls to rng() to drive simulationtime
    Named temp because the number is always overwritten next iteration
    */
    int eattime_temp   = 0;
    int thinktime_temp = 0;

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

class Table //Table keeps track of all diners, forks and starts/ends the meal
{
    std::vector<std::string> names;
    std::vector<Fork*> forklist_ptr;
    std::vector<Philosopher*> phillist_ptr;
    std::vector<int> index;

    std::thread* thread_ptr;
    
    unsigned int N=0;
    float t = 0;
    float tf;

public:
    Table(unsigned int _N){
        N = _N;
        tf = 1.5*N;

        /*
        Automatic name creation
        Alternatively, a vector of strings could be passed
        to the constructor with actual names, althought its
        length will limit the simulation scalability, ie. number of diners.
        */
        for (int i=0; i<N; i++) {
            names.push_back("Diner "+std::to_string(i+1));
            forklist_ptr.push_back(new Fork);
            index.push_back(i); // Make this vector to use as for-loop range
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
        for (auto j : index) {
            if (j==0) { // exception for first instantiation
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[N-1]->fork_pointer,forklist_ptr[j]->fork_pointer));
            } else {
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j-1]->fork_pointer,forklist_ptr[j]->fork_pointer));
            }
        }

        /* 
        Start meal by calling spawning N threads with 
        callable Philosopher::eat and pointer to 
        philosopher objects
        */        
        while (t<tf){ 
            /* Dynamic thread spawning*/
            for (auto i : index) {
                thread_ptr = new std::thread{&Philosopher::eat,phillist_ptr[i]};

                // Update simulation time
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
        for (auto i : index) {
            delete phillist_ptr[i];
            delete forklist_ptr[i];
        }
        std::cout << "\nMeal lasted " 
                  << t << "s, " << t-tf << "s overtime.\n";
    }
};

int main()
{
    int num;
    
    std::cout << "How many diners: "; 
    std::cin >> num;
  
    Table table(num);

    return 0;
}

#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <random>

/*
Dining Philosophers implementation

*/

int rng() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,100);
    return dist6(rng);
}

class Fork{
public:
    std::mutex fork;
    std::mutex *fork_pointer = &fork;
};

class Philosopher{

    std::string name;
    std::mutex* ptr_l;
    std::mutex* ptr_r;
    int eattime=0;
    int thinktime=0;
    int eattime_temp=0;
    int thinktime_temp=0;

public:
    Philosopher(std::string _name, std::mutex* _ptr_l, std::mutex* _ptr_r ){
        ptr_l = _ptr_l;
        ptr_r = _ptr_r;
        name  = _name;
    }

    ~Philosopher(){
        std::cout << name <<" left the table, ate for " 
                  << (static_cast< float >(eattime))/1000 
                  << "s, thought for " 
                  << (static_cast< float >(thinktime))/1000<<"s.\n";
    }

    void think(){
        std::cout << name << " started thinking.\n";
        thinktime_temp = rng();
        std::this_thread::sleep_for(std::chrono::milliseconds(thinktime_temp));
        std::cout << name << " stopped thinking.\n";
        thinktime+=thinktime_temp;
    }
    
    bool get_fork(){
        if ((ptr_l->try_lock() && ptr_r->try_lock())==true) {
            return true;
        } else {
            return false;
        }
    }
    
    void release_fork(){
        ptr_l->unlock(); ptr_r->unlock();
    }

    void eat(){
        if (!get_fork()){
            think();
        } else {
            std::cout << name << " started eating.\n";
            eattime_temp = rng();
            std::this_thread::sleep_for(std::chrono::milliseconds(eattime_temp));
            std::cout << name << " stopped eating.\n";
            eattime += eattime_temp;
        }

        release_fork();
    }

    int get_eattime() {
        return eattime_temp;
    }

    int get_thinktime() {
        return thinktime_temp;
    }
};

class Table{

    std::vector<std::string> names;
    std::vector<Fork*> forklist_ptr;
    std::vector<Philosopher*> phillist_ptr;
    std::thread* test;

    unsigned int N=0;
    float t = 0;
    float tf= 0;

public:

    Table(std::vector<std::string> _names, unsigned int _N, float _tf){
        N = _N;
        tf = _tf;
        /* 
        This section dynamically insantiates the required classes so that 
        I dont have to explicitly create a bunch of objects myself
        */
        for (int i=0; i<N; i++) {
            names.push_back(_names[i]);
            forklist_ptr.push_back(new Fork);
        }

        // Create philosopher objects once forks are done
        for (int j=0; j<N; j++) {
            if (j==0) { // exception for first instantiation
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j]->fork_pointer,forklist_ptr[N-1]->fork_pointer));    
            } else {
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j]->fork_pointer,forklist_ptr[j-1]->fork_pointer));
            }
        }

        while (t<tf){ 

            /* Dynamic thread spawning*/
            for (int i=0;i<N;i++) {
                test = new std::thread{&Philosopher::eat,phillist_ptr[i]};

                t+= static_cast< float >(phillist_ptr[i]->get_thinktime() + phillist_ptr[i]->get_eattime())/1000;
            }
            test->join();
        }
    }

    ~Table(){
        // Garbage collection
        for (int i=0; i<N; i++) {
            delete phillist_ptr[i];
            delete forklist_ptr[i];
        }
        std::cout << "\n\nTable was smashed. Meal lasted " 
                  << t << "s, " << t-tf << "s overtime.\n";
    }
};

int main() {
    std::vector<std::string> names_ = {"En","To","Tre","Fire","Fem","Seks","Syv","8","Ni","Ti"};
    int num;
    std::cout << "How many diners? "; std::cin>>num;
    float simtime = 6.0;

    Table table(names_, num, simtime);

    return 0;
}

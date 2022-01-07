#include <iostream>
#include <chrono>
#include <mutex>
#include <thread>
#include <string>
#include <vector>
#include <random>

/*
Dining Philosophers implementation

#TODO:
    la tråden leve hele programscopet -> provoser deadlock


*/

int rng() {
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,40);
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
    int eattime, thinktime, eattime_temp, thinktime_temp;

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
    
    void get_fork(){
        ptr_l->lock(); ptr_r->lock();
    }
    
    void release_fork(){
        ptr_l->unlock(); ptr_r->unlock();
    }

    void eat(){
        get_fork();
        {
            std::cout << name << " started eating.\n";
            eattime_temp = rng();
            std::this_thread::sleep_for(std::chrono::milliseconds(eattime_temp));
            std::cout << name << " stopped eating.\n";
            eattime += eattime_temp;
        }
        release_fork();
        think();
    }

    int get_eattime() {
        return eattime_temp;
    }

    int get_thinktime() {
        return thinktime_temp;
    }
    
    void meal(){
        std::thread thread(&Philosopher::eat, this);
        thread.join();
    }
};

class Table{
    std::vector<std::string> names;
    std::vector<Fork*> forklist_ptr;
    std::vector<Philosopher*> phillist_ptr;
    unsigned int N;
    float t = 0;
    float tf;

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

        for (int j=0; j<N; j++) {
            if (j==0) { // exception for first instantiation
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j]->fork_pointer,forklist_ptr[N-1]->fork_pointer));    
            } else {
                phillist_ptr.push_back(new Philosopher(names[j],forklist_ptr[j]->fork_pointer,forklist_ptr[j-1]->fork_pointer));
            }
        }

        while (t<tf){
            for (int i=0; i<N; i++) {
                phillist_ptr[i]->meal();
                t+= static_cast< float >(phillist_ptr[i]->get_thinktime() + phillist_ptr[i]->get_eattime())/1000; 
            }
        }
    }

    ~Table(){
        // Garbage collection
        for (int i=0; i<N; i++) {
        delete phillist_ptr[i];
        delete forklist_ptr[i];
        }
        std::cout << "\n\nTable was smashed.\n";
    }
};

int main() {
    std::vector<std::string> names_ = {"En","To","Tre","Fire","Fem","Seks","Syv","8","Ni","Ti"};

    unsigned int num;
    float simtime;

    std::cout << "\nSimtime: "; std::cin >> simtime;
    std::cout << "\nNumber of diners: "; std::cin >> num;

    Table table(names_, num, simtime);

    return 0;
}

#include <iostream>
#include <thread>
#include <mutex>  // "Fork"
#include <chrono> //thread sleep
#include <vector>
#include <string>
#include <random> //RNG generation

int rng() {
    // RNG: 1-500
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,40);
    return dist6(rng);
}

struct Fork{         // Hold forks
    std::mutex  fork;
    std::mutex* fork_ptr=&fork;
}f1,f2,f3,f4,f5;

class Philosopher{
    
    std::string name;
    std::mutex *ptr_r, *ptr_l;

    float t_eat = 0.0;
    float t_think = 0.0;

    int rngtime_think=0;
    int rngtime_eat=0;

public:
    Philosopher(std::string _name, std::mutex* _ptr_r, std::mutex* _ptr_l){
        name  = _name;
        ptr_r = _ptr_r;
        ptr_l = _ptr_l;
    }

    float get_eattime(){
        return t_eat/1000;
    }

    float get_thinktime(){
        return t_think/1000;
    }

    // Temp get-functions
    float get_eat_temp(){
        return static_cast< float >(rngtime_eat);
    }

    float get_think_temp(){
        return static_cast< float >(rngtime_think);
    }

    void think(){
        std::cout << name << " is thinking\n";

        rngtime_think = rng();    // Pass to rng-generator
        t_think += rngtime_think; // Store thinking time

        std::this_thread::sleep_for(std::chrono::milliseconds(rngtime_think));
    }

    void get_forks() {
        ptr_l->lock(); ptr_r->lock();
    }

    void put_forks() {
        ptr_l->unlock(); ptr_r->unlock();
    }

    void eat() {
    /*
        Attempt to eat, locking the appropriate mutex to deny adjacent diners
        access. When finished eating, release mutex locks and think 
    */

        rngtime_eat = rng();   //pass to rng-generator
        t_eat += rngtime_eat;  //store eating time

        this->get_forks();

        std::cout << name << " started eating\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(rngtime_eat));
        std::cout << name <<" is done eating\n";

        this->put_forks();

        this->think();     
    }

    void meal(){
        /*
        Create a thread and call eat() from Philosopher class
        */
        std::thread thread(&Philosopher::eat, this);
        thread.join();
    }
};

void dine() {
    /*
    Hold diner names, instantiate Philosopher() with pointers to appropriate mutexes and
    commence meal for tf (6) seconds.
    */
    std::vector<std::string> names = {"Descartes","Nietszche","Robespierre","Franklin","Hamilton"};
                            // Right fork   Left fork
    Philosopher desc(names[0], f1.fork_ptr, f5.fork_ptr);
    Philosopher hami(names[4], f5.fork_ptr, f4.fork_ptr);
    Philosopher robe(names[2], f3.fork_ptr, f2.fork_ptr);
    Philosopher niet(names[1], f2.fork_ptr, f1.fork_ptr);
    Philosopher fran(names[3], f4.fork_ptr, f3.fork_ptr);


    float t = 0.0;
    float tf= 3.0;

    while (t<tf) {
        // Start meal
        robe.meal();
        desc.meal();
        niet.meal();
        hami.meal();
        fran.meal();

        // Synchronize simtime with eat/think-time
        t+=(desc.get_think_temp()+desc.get_eat_temp() + niet.get_think_temp()+niet.get_eat_temp() + robe.get_think_temp()+robe.get_eat_temp() + fran.get_think_temp()+fran.get_eat_temp() + hami.get_think_temp()+hami.get_eat_temp())/1000;

        // Kill simulation and print individual times (assess starvation)
        if (t>0.999999*tf) {
            std::cout << "************************************************\n";
            std::cout << names[0] << " ate for "<<desc.get_eattime()<<"s, thought for "<<desc.get_thinktime()<<"s.\n";
            std::cout << names[1] << " ate for "<<niet.get_eattime()<<"s, thought for "<<niet.get_thinktime()<<"s.\n";
            std::cout << names[2] << " ate for "<<robe.get_eattime()<<"s, thought for "<<robe.get_thinktime()<<"s.\n";
            std::cout << names[3] << " ate for "<<fran.get_eattime()<<"s, thought for "<<fran.get_thinktime()<<"s.\n";
            std::cout << names[4] << " ate for "<<hami.get_eattime()<<"s, thought for "<<hami.get_thinktime()<<"s.\n";
            std::cout << "************************************************\n";
            std::cout << "\nSum eating time: \t" << std::to_string(desc.get_eattime()+niet.get_eattime()+robe.get_eattime()+fran.get_eattime()+hami.get_eattime())<<"s.\n";
            std::cout << "Sum thinking time: \t" << std::to_string(desc.get_thinktime()+niet.get_thinktime()+robe.get_thinktime()+fran.get_thinktime()+hami.get_eattime())<<"s.\n";
            std::cout << "Simtime: \t" << tf << "s.\n"
                      << "Difference: \t" << t-tf << "s.\n";
            exit(0);
        } 
    }

}

int main() {

    dine();

    return 0;
}
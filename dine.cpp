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
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,100);
    return dist6(rng);
}

class Fork{
public:
    std::mutex fork;
    std::mutex* fork_ptr=&fork;
};

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

    ~Philosopher(){
        std::cout << name << " ate for "<< get_eattime() <<"s, thought for "<< get_thinktime()<<"s.\n";
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

        std::cout << name << " is done thining\n";
    }

    void get_forks() {
        ptr_l->lock(); 
        ptr_r->lock();
    }

    void put_forks() {
        ptr_l->unlock(); 
        ptr_r->unlock();
    }

    void eat() {
    /*
        Attempt to eat, locking the appropriate mutex to deny adjacent diners
        access. When finished eating, release mutex locks and think.

        rng() is called to make the philosopher randomly eat or think 
    */
        if (rng()>50) {
            this->think();
        } else {
            this->get_forks();

            rngtime_eat = rng();   //pass to rng-generator
            t_eat += rngtime_eat;  //store eating time

            std::cout << name << " started eating\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(rngtime_eat));

            this->put_forks();
            std::cout << name <<" is done eating\n";   
        }
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
    
    // Construct five forks
    Fork f1; Fork f2; Fork f3; Fork f4; Fork f5;

                            // Right fork   Left fork
    Philosopher desc(names[0], f1.fork_ptr, f5.fork_ptr);
    Philosopher niet(names[1], f2.fork_ptr, f1.fork_ptr);
    Philosopher robe(names[2], f3.fork_ptr, f2.fork_ptr);
    Philosopher fran(names[3], f4.fork_ptr, f3.fork_ptr);
    Philosopher hami(names[4], f5.fork_ptr, f4.fork_ptr);

    float t = 0.0;
    float tf= 4.0;

    while (t<tf) {
        // Start meal
        robe.meal();
        desc.meal();
        niet.meal();
        hami.meal();
        fran.meal();

        // Drive simtime with eat/think-time
        t+=(desc.get_think_temp()+desc.get_eat_temp() + niet.get_think_temp()+niet.get_eat_temp() + robe.get_think_temp()+robe.get_eat_temp() + fran.get_think_temp()+fran.get_eat_temp() + hami.get_think_temp()+hami.get_eat_temp())/1000;
    }
    // Print results after while-loop
    std::cout << "\nSum eating time: \t" << std::to_string(desc.get_eattime()+niet.get_eattime()+robe.get_eattime()+fran.get_eattime()+hami.get_eattime())<<"s.\n";
    std::cout << "Sum thinking time: \t" << std::to_string(desc.get_thinktime()+niet.get_thinktime()+robe.get_thinktime()+fran.get_thinktime()+hami.get_eattime())<<"s.\n";
    std::cout << "Simtime: \t\t" << tf << "s.\n"
              << "Runtime: \t\t" << t << "s.\n"
              << "Difference: \t\t" << t-tf << "s.\n\n";

}

int main() {

    dine();

    return 0;
}
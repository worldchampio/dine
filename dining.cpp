#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <string>
#include <random>

int rng() {
    // RNG: 1-500
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,50);
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

    float get_eat_temp(){
        return static_cast< float >(rngtime_eat);
    }

    float get_think_temp(){
        return static_cast< float >(rngtime_think);
    }

    void think(){
        std::cout << name << " is thinking\n";

        // Store thinking time
        rngtime_think = rng();
        t_think += rngtime_think;

        std::this_thread::sleep_for(std::chrono::milliseconds(rngtime_think));
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
        
        this->think();

        // Store eating time
        rngtime_eat = rng();
        t_eat += rngtime_eat;

        this->get_forks();

        std::cout << name << " started eating\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(rngtime_eat));
        std::cout << name <<" is done eating\n";

        this->put_forks();
        
    }

    void meal(){
        std::thread thread(&Philosopher::eat, this);
        thread.join();
    }
};


void dine() {
    std::vector<std::string> names = {"Descartes","Nietszche","Robespierre","Franklin","Hamilton"};
                            // Right fork   Left fork
    Philosopher desc(names[0], f1.fork_ptr, f5.fork_ptr);
    Philosopher niet(names[1], f2.fork_ptr, f1.fork_ptr);
    Philosopher robe(names[2], f3.fork_ptr, f2.fork_ptr);
    Philosopher fran(names[3], f4.fork_ptr, f3.fork_ptr);
    Philosopher hami(names[4], f5.fork_ptr, f4.fork_ptr);

    float t = 0.0;
    float dt=0.1;
    float tf= 6.0;

    while (t<tf) {
        desc.meal();
        niet.meal();
        robe.meal();
        fran.meal();
        hami.meal();

        t+=(desc.get_think_temp()+desc.get_eat_temp() + niet.get_think_temp()+niet.get_eat_temp() + robe.get_think_temp()+robe.get_eat_temp() + fran.get_think_temp()+fran.get_eat_temp() + hami.get_think_temp()+hami.get_eat_temp())/1000;

        std::cout << "TIME: " << t << "\n";
        if (t>0.9999*tf) {
            std::cout << names[0] << " spiste i "<<desc.get_eattime()<<"s, tenkte i "<<desc.get_thinktime()<<"s.\n";
            std::cout << names[1] << " spiste i "<<niet.get_eattime()<<"s, tenkte i "<<niet.get_thinktime()<<"s.\n";
            std::cout << names[2] << " spiste i "<<robe.get_eattime()<<"s, tenkte i "<<robe.get_thinktime()<<"s.\n";
            std::cout << names[3] << " spiste i "<<fran.get_eattime()<<"s, tenkte i "<<fran.get_thinktime()<<"s.\n";
            std::cout << names[4] << " spiste i "<<hami.get_eattime()<<"s, tenkte i "<<hami.get_thinktime()<<"s.\n";

            std::cout << "Sum spisetid: " << std::to_string(desc.get_eattime()+niet.get_eattime()+robe.get_eattime()+fran.get_eattime()+hami.get_eattime())<<"\n";
            std::cout << "Sum tenketid: " << std::to_string(desc.get_thinktime()+niet.get_thinktime()+robe.get_thinktime()+fran.get_thinktime()+hami.get_eattime())<<"\n";

            exit(1);
        }
    }

}

int main() {

    dine();

    return 0;
}
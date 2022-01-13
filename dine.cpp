#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <assert.h>
#include <random>

// Hold forks and public fork ptrs
class Fork{
    std::mutex fork;
public:
    std::mutex* f_ptr = &fork;
};

/* Holds name, fork ptrs, lifethread and time data for eating/thinking. 
std::thread lifethread with callable Philisopher::start() initiates 
meal participation. 
*/
class Philosopher{
    std::mutex * ptr_l, *ptr_r;
    std::string name = "";

    volatile float t = 0.0;
    volatile float t_end = 0.0;
    
    volatile int t_think = 0;
    volatile int t_think_sum = 0;
    volatile int n_think = 0;
    volatile int t_eat = 0;
    volatile int t_eat_sum = 0;
    volatile int n_eat = 0;

    bool is_eating = false;

    int rng() {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(1,50);
        return dist6(rng);
    }

public:
    std::thread lifethread;

    Philosopher(std::string _name, float _t_end, std::mutex* _ptr_l, std::mutex* _ptr_r){
        name = _name;
        t_end= _t_end;
        ptr_l = _ptr_l;
        ptr_r = _ptr_r;
    }

    ~Philosopher(){        
        std::cout << name << " ate for " << static_cast<float>(t_eat_sum)/1000<< "s, (" 
                  << n_eat << " times), thought for " << static_cast<float>(t_think_sum)/1000 
                  << "s, \t(" << n_think <<" times)"<<std::endl;
        
    }

    bool get_forks(){
        if (this->ptr_r->try_lock() && this->ptr_l->try_lock()){
            is_eating = true;
            return is_eating;
        } else {
            is_eating = false;
            return is_eating;
        }
    }
    void release_forks(){
        ptr_r->unlock(); ptr_l->unlock();
    }

    void think(){
        std::cout << name << " started thinking"<<std::endl;

        t_think = rng();
        t_think_sum += t_think;
        n_think++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(t_think));

        std::cout << name << " stopped thinking"<<std::endl;
    }

    void eat(){
        assert(is_eating==true);
        std::cout << name << " started eating"<<std::endl;
        t_eat = rng();
        t_eat_sum += t_eat;
        n_eat++;
        std::this_thread::sleep_for(std::chrono::milliseconds(t_eat));

        std::cout << name << " stopped eating"<<std::endl;
        
        release_forks(); 
        is_eating = false;
    }

    void join_table() {
        while (t<t_end) {
            if (!get_forks()) {
                think();
                release_forks();
            } else {
                eat();
            }
            t += static_cast<float>(t_think + t_eat)/1000;
        }
    }

    void start(){
        this->lifethread = std::thread(&Philosopher::join_table,this);
        if (t>t_end) {
            this->lifethread.join();
        }
    }

};

// Stores N Forks and N Philosophers, starts meal with Philosopher::lifethread
class Table{
    std::vector<Fork*> fork_vec;
    std::vector<Philosopher*> phil_vec;
    std::vector<int> range;
    unsigned int N;

public:
    Table(unsigned int _N){
        N = _N;

        for (int k=0; k<N; k++) {
            range.push_back(k);
            fork_vec.push_back(new Fork);
        }

        for (auto i : range) {
            if (i==0) {
                phil_vec.push_back(new Philosopher("Diner "+std::to_string(i+1),N*1.5,fork_vec[N-1]->f_ptr, fork_vec[i]->f_ptr));
            } else {
                phil_vec.push_back(new Philosopher("Diner "+std::to_string(i+1),N*1.1,fork_vec[i-1]->f_ptr, fork_vec[i]->f_ptr));
            }
        }

        for (auto j : range) {
            phil_vec[j]->start();
        }
    }

    ~Table(){
        // Allow all threads to join before garbage collection
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for (auto d : range) {
            phil_vec[d]->lifethread.join();
            delete fork_vec[d];
            delete phil_vec[d];
        }
    }
};

/*  DINING PHILOSOPHERS 

    Table(N)                                    - constructs N philosophers and N forks,
                                                  where forks are passed thusly:
                                                  Phil(fork[i-1],fork[i]) to create
                                                  a circular sharing of forks, to provoke
                                                  deadlock.

    Philosopher( name, t_end, ptr_r, ptr_l )    - constructs 1 philosopher with reference
                                                  to the pointer of corresponding to its 
                                                  index and the index before it as right/left fork.
    Fork()                                      - holds a fork(mutex) and public ptr to it.

    DEADLOCK SOLUTION

    Two countermeasures are implemented.
    1)  Any given diner will always attempt to lock its right (highest index) fork.
    
    2)  Should right or left fork acquisiton fail, the diner releases ALL forks,
        then executes think() and tries again.
*/

int main() {
    int num;
    std::cout << "How many diners: ";
    std::cin >> num;

    Table table(num);
    
    return 0;
}

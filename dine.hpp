#pragma once

#include <mutex>
#include <thread>
#include <iostream>
#include <memory>
#include <vector>

static std::atomic<double> t;
static std::atomic<int> thinks;
static std::atomic<int> eats;

class Fork{
    std::mutex fork;
public:
    // Pointer to mutex stored in object
    std::mutex* f_ptr{ &fork };
};

/* 
Holds name, fork ptrs, lifethread and time data for eating/thinking. 
std::thread lifethread with callable Philisopher::start() initiates 
meal participation. 
*/
class Philosopher{
public:
    std::thread lifethread;
    Philosopher(std::string name, double t_end, std::mutex* ptr_l, std::mutex* ptr_r);
    ~Philosopher();
    void start();
private:
    std::mutex* ptr_l;
    std::mutex* ptr_r;
    std::string name{ "" };
    double      t_end{ 0.0 };
    /*
    As each Philosopher object tracks its own times/counts,
    these variables do not need to be atomic
    */
    int  t_think_sum{ 0 };
    int  n_think{ 0 };
    int  t_eat_sum{ 0 };
    int  n_eat{ 0 };
    bool is_eating{false};

    /**
    Think for a random amount of time.
    @return total time spent 'thinking'.
     */
    int think();
    /**
    Eat for a random amount of time.
    @return total time spent 'eating'.
     */
    int eat();
    /**
     Return a random integer between given limits.
     Limits default to (1-200).
     @param min Lower limit for random number.
     @param max Upper limit for random number.
     @return Random number in range(min, max)
     */
    int rng(int min, int max);
    /**
     Attempt to lock the mutexes the Philosopher has pointers to.
     @return Success status.
     */
    bool get_forks();
    // Unlock mutexes if they were locked.
    void release_forks();
    // Attempt to eat() and think() for simulation duration.
    void join_table();
};

// Stores N Forks and N Philosophers, starts meal with Philosopher::lifethread
class Table
{
public:
    Table(int N);
    ~Table();
private:
    std::vector<std::unique_ptr<Fork>>fork_vec;
    std::vector<std::unique_ptr<Philosopher>> phil_vec;
    std::vector<int> range;
    float simtime{ 0 };
    unsigned int N{ 0 };
};

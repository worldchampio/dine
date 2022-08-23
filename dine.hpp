#pragma once

#include <mutex>
#include <thread>
#include <iostream>
#include <memory>
#include <vector>

/* 
Global scope atomic double t
that all Philosophers write to
*/
static std::atomic<double> t;


// Hold forks and public fork ptrs
class Fork{
    std::mutex fork;
public:
    std::mutex* f_ptr{&fork};
};

/* 
Holds name, fork ptrs, lifethread and time data for eating/thinking. 
std::thread lifethread with callable Philisopher::start() initiates 
meal participation. 
*/
class Philosopher{
    std::mutex     *ptr_l, *ptr_r;
    std::string     name{ "" };
    double          t_end{ 0.0 };
    /*
    As each Philosopher object tracks its own times/counts,
    these variables do not need to be atomic
    */
    int t_think{ 0 };
    int t_think_sum{ 0 };
    int n_think{ 0 };
    int t_eat{ 0 };   
    int t_eat_sum{ 0 };
    int n_eat{ 0 };
    bool is_eating{false};

    int rng();
public:
    std::thread lifethread;
    Philosopher(std::string _name, double _t_end, std::mutex* _ptr_l, std::mutex* _ptr_r);
    ~Philosopher();
    bool get_forks();
    void release_forks();
    void think();
    void eat();
    void join_table();
    void start();
};

// Stores N Forks and N Philosophers, starts meal with Philosopher::lifethread
class Table
{
    std::vector<std::unique_ptr<Fork>>fork_vec;
    std::vector<std::unique_ptr<Philosopher>> phil_vec;
    std::vector<int> range;
    float simtime{ 0 };
    unsigned int N{ 0 };
public:
    Table(int N);
    ~Table();
};

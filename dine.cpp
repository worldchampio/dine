#include <chrono>
#include <string>
#include <limits>
#include <cmath>
#include <iostream>
#include <random>
#include <atomic>
#include "dine.hpp"

int Philosopher::rng() 
{
    std::random_device  dev;
    std::mt19937        rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,200);
    return dist6(rng);
}

Philosopher::Philosopher(std::string _name, double _t_end, std::mutex* _ptr_l, std::mutex* _ptr_r) :
name{_name},
t_end{_t_end},
ptr_l{_ptr_l},
ptr_r{_ptr_r}
{
}

Philosopher::~Philosopher()
{
    std::cout << name << " ate for " << static_cast<double>(t_eat_sum)/1000<< "s, (" 
                << n_eat << " times), thought for " << static_cast<double>(t_think_sum)/1000 
                << "s, (" << n_think <<" times)"<<std::endl;
}

bool Philosopher::get_forks()
{
    is_eating = (this->ptr_r->try_lock() && this->ptr_l->try_lock()) ? true : false;
    return is_eating;
}

void Philosopher::release_forks()
{
    ptr_r->unlock(); ptr_l->unlock();
}

void Philosopher::think()
{
    std::cout << name << " started thinking"<<std::endl;
    t_think = rng();
    t_think_sum += t_think;
    n_think++;
    std::this_thread::sleep_for(std::chrono::milliseconds(t_think));
    std::cout << name << " stopped thinking"<<std::endl;
}

void Philosopher::eat()
{
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

void Philosopher::join_table() 
{
    while (t_all.t <t_end) 
    {
        /*
        The solution to the deadlock is implemented here
        by making any philosopher failing to lock BOTH
        forks release ANY it locked.
        */
        (!get_forks()) ? (think(), release_forks()) : eat();
        t_all.t.store( 
            t_all.t.load(std::memory_order_relaxed) + static_cast<double>(t_think + t_eat)/1000, 
            std::memory_order_relaxed);   
    }
}

void Philosopher::start()
{
    this->lifethread = std::thread(&Philosopher::join_table,this);
    // Checking joinable to avoid ugly destructor prints
    if (t_all.t>t_end && lifethread.joinable()) 
    {
        this->lifethread.join();
    }
}

Table::Table(int N)
{
    // Scale simtime with amount of diners
    simtime = static_cast<float>(N)*1.5;

    // First loop to create the range and forks
    for (int i=0; i<N; i++) 
    {
        range.push_back(i);
        fork_vec.push_back(std::make_unique<Fork>());
    }

    /* 
    As any instantiation of a Philosopher requires two Forks,
    the loops must run in sequence.
    */
    for (const auto& i : range) 
    {
        if (i==0) 
        {
            phil_vec.push_back(
                std::make_unique<Philosopher>("Diner "+std::to_string(i+1),simtime,fork_vec[N-1]->f_ptr, fork_vec[i]->f_ptr));
        } 
        else 
        {
            phil_vec.push_back(
                std::make_unique<Philosopher>("Diner "+std::to_string(i+1),simtime,fork_vec[i-1]->f_ptr, fork_vec[i]->f_ptr));
        }
    }

    /* Call start for every Philosopher in phil_vec,
    starting Philosopher::lifethread with callable
    Philosopher::eat
    */
    for (const auto& Philosopher : phil_vec)
    {
        Philosopher->start();
    }
}

Table::~Table()
{        
    // Final calls to join, if they were not executed in ~Philosopher()
    for (const auto& Philosopher : phil_vec)
    {
        Philosopher->lifethread.join();
    }

    std::cout <<"+------------------------------------------+\n" 
                <<"Simulation time: "<<t_all.t<<"s, "<<t_all.t - simtime<<"s overtime.\n"
                <<"+------------------------------------------+\n";
}

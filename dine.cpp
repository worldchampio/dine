#include <chrono>
#include <string>
#include <limits>
#include <cmath>
#include <iostream>
#include <random>
#include <atomic>
#include "dine.hpp"

int Philosopher::rng(int min=1, int max=200) 
{
    std::random_device  dev;
    std::mt19937        rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(min,max);
    return dist6(rng);
}

Philosopher::Philosopher(std::string name, double t_end, std::mutex* ptr_l, std::mutex* ptr_r) :
name{name},
t_end{t_end},
ptr_l{ptr_l},
ptr_r{ptr_r}
{}

Philosopher::~Philosopher()
{
    std::cout << name 
    << " ate for " << static_cast<double>(t_eat_sum)/1000<< "s, (" << n_eat 
    << " times), thought for " << static_cast<double>(t_think_sum)/1000           
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

int Philosopher::think()
{
    std::cout << name << " started thinking"<<std::endl;
    const auto t_think{ rng() };
    t_think_sum += t_think;
    n_think++;
    std::this_thread::sleep_for(std::chrono::milliseconds(t_think));
    std::cout << name << " stopped thinking"<<std::endl;
    return t_think;
}

int Philosopher::eat()
{
    assert(is_eating==true);
    std::cout << name << " started eating"<<std::endl;
    const auto t_eat{ rng() };
    t_eat_sum += t_eat;
    n_eat++;
    std::this_thread::sleep_for(std::chrono::milliseconds(t_eat));
    std::cout << name << " stopped eating"<<std::endl;
    release_forks(); 
    is_eating = false;
    return t_eat;
}

void Philosopher::join_table() 
{
    while (t <t_end) 
    {
        int t_think{ 0 }; 
        int t_eat{ 0 };
        if(!get_forks())
        {
            t_think = think();
            thinks.store(thinks.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
            release_forks();
        } 
        else 
        {
            t_eat = eat();
            eats.store(eats.load(std::memory_order_relaxed) + 1, std::memory_order_relaxed);
        }
        t.store(t.load(std::memory_order_relaxed) + static_cast<double>(t_think + t_eat)/1000, std::memory_order_relaxed);   
    }
}

void Philosopher::start()
{
    this->lifethread = std::thread(&Philosopher::join_table,this);
    if (t>t_end && lifethread.joinable()) 
        this->lifethread.join();
}

Table::Table(int N)
{
    // Scale simtime with amount of diners
    simtime = static_cast<float>(N)*1.5;

    // Create Forks and range
    for (int i=0; i<N; i++) 
    {
        range.push_back(i);
        fork_vec.push_back(std::make_unique<Fork>());
    }
    // Make Philosophers
    for (const auto& i : range) 
        phil_vec.push_back(
            std::make_unique<Philosopher>("Diner "+std::to_string(i+1), simtime, fork_vec[i==0 ? N-1 : i-1]->f_ptr,fork_vec[i]->f_ptr)
        );
    for (const auto& Philosopher : phil_vec)
        Philosopher->start();
}

Table::~Table()
{        
    // Final calls to join, if they were not executed in ~Philosopher()
    for (const auto& Philosopher : phil_vec)
        Philosopher->lifethread.join();
    std::cout <<"+----------------------------------------------------------+\n" 
              <<"Simulation time: "<< t <<"s, "<<t - simtime<<"s overtime. "<<thinks<<" thinks, "<<eats<<" eats. \n"
              <<"+----------------------------------------------------------+"<<std::endl;
}

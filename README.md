# Dining philosophers

## Quickstart

Clone and execute `build`. Must have `g++`-compiler.

## Description
    Table(N)                                    - constructs N philosophers and N forks,
                                                  where forks are passed thusly:
                                                  Phil(fork[i-1],fork[i]) to create
                                                  a circular sharing of forks, to provoke
                                                  deadlock. Responsible for garbage-
                                                  collection at the end of simulation.

    Philosopher( name, t_end, ptr_r, ptr_l )    - constructs a philosopher with references
                                                  to the pointers corresponding to its index 
                                                  and the index before it as right/left fork.

        Philosopher::start()                    - starts lifethread with callable join_table() 
                                                  and pointer to itself

        Philosopher::join_table()               - tries to lock fork right & left.
                                                  if successful, eat() is called. if not,
                                                  think() is called on a while loop limited
                                                  by t_end, incremented by eat/think time.

    Fork()                                      - holds a fork(mutex) and public ptr to it.

    DEADLOCK/STARVATION SOLUTION

    Two countermeasures are implemented.
    1)  Any given diner will always attempt to lock its right (highest index) fork first.
    
    2)  Should right or left fork acquisiton fail, the diner releases ALL forks,
        then executes think() and tries again.


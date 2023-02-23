#include <iostream>
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <string>

using namespace std;

// ==== Debug Function

std::recursive_mutex print_mtx;

template <typename T>
void threadsafe_debug(T t)
{
    cout << t << endl ;
}

template<typename T, typename... Args>
void threadsafe_debug(T t, Args... args)
{
    std::lock_guard<std::recursive_mutex> lock(print_mtx);
    cout << t << " ";
    threadsafe_debug(args...) ;
}

// ==== End Debug

//dungeon class
class Dungeon{
    private:
        std::mutex mtx;
        bool has_cupcake;
        bool cleared;
        int ticket;
        int num_guests;
    public:

        Dungeon(int num_guests) : cleared(false), has_cupcake(true), ticket(-1), num_guests(num_guests) {
            //constructor
        }

        void declare_finished(){
            cleared = true;
        }

        bool is_cleared(){
            return cleared;
        }

        void set_next_ticket(std::string thread_name){
            //randomizes next ticket
            int previous_ticket = ticket;
            ticket = rand()% num_guests;
            threadsafe_debug(thread_name, "Set next ticket from", previous_ticket, "to", ticket);
        }

        int get_ticket(){
            return ticket;
        }

        void cupcake_room(std::string thread_name, bool refill_cupcake, bool consume_cupcake, bool* refilled_cupcake, bool* ate_cupcake){
            bool refilled = false;
            //bool ate = false;
            //mtx.lock();
            threadsafe_debug(thread_name, "is visiting the cupcake room");
            if ( has_cupcake){
                threadsafe_debug(thread_name, "found the cupcake");
                if (consume_cupcake){
                    threadsafe_debug(thread_name, "and ate the cupcake");
                    has_cupcake = false;
                    *ate_cupcake = true;
                }
            }
            if (!has_cupcake && refill_cupcake){
                threadsafe_debug(thread_name, "refilled the cupcake");
                refilled = true;
                has_cupcake = true;
            }
            //mtx.unlock();

            *refilled_cupcake =  refilled;
            //*ate_cupcake = ate;
            threadsafe_debug(thread_name, "is leaving the cupcake room");
            set_next_ticket(thread_name);
        }

        void nothing_room(std::string thread_name){
            threadsafe_debug(thread_name, "says there's nothing here");
        }

};


void guest(Dungeon& dungeon, std::string thread_name, bool is_leader, int total_members, int my_id){
    int times_refilled = 0;
    bool has_eaten = false;
    //threadsafe_debug(thread_name,"has id of", my_id);
    while ( !dungeon.is_cleared() ){
        if ( ! (dungeon.get_ticket() == my_id) ) {
            //simulate doing other stuff
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        bool refilled_cupcake = false;

        //threadsafe_debug(thread_name, "Has Eaten [Before]", has_eaten);
        dungeon.cupcake_room(thread_name, is_leader, !has_eaten, &refilled_cupcake, &has_eaten);
        //threadsafe_debug(thread_name, "Has Eaten [After]", has_eaten);
        if (refilled_cupcake) times_refilled++;
        if (times_refilled >= total_members) dungeon.declare_finished();
    }
    threadsafe_debug(thread_name, "finished exploring the dungeon");
}

int main()
{
    std::vector<std::thread> threads;
    int num_threads = 50;

    Dungeon labyrinth(num_threads);
    labyrinth.set_next_ticket("<main_thread>");

    auto start = std::chrono::high_resolution_clock::now();
    threadsafe_debug("Starting Threads. Num threads:", num_threads);

    //leader/counter Guest
    threads.push_back( std::thread(guest, std::ref(labyrinth), "Thread-1 (Leader)", true, num_threads, 0) );
    //the rest
    for(int i = 1; i < num_threads; i++){
        threads.push_back( std::thread(guest, std::ref(labyrinth), "Thread-" + std::to_string(i+1), false, num_threads, i) );
    }
    threadsafe_debug("All Threads started, waiting for finish.");
    for(int i = 0; i < num_threads; i++){
        threads[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    threadsafe_debug("Threads Finished");

    threadsafe_debug("Total Execution Time:", std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count(), "milliseconds");

}

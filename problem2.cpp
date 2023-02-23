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
void threadsafe_debug(T t, Args... args) // recursive variadic function
{
    //std::lock_guard<std::recursive_mutex> lock(print_mtx);
    cout << t << " ";
    threadsafe_debug(args...) ;
}

// ==== End Debug

class Dungeon{
    private:
        std::mutex mtx;
        std::mutex vase_room_door;
        bool has_cupcake;
        bool cleared;
        int ticket;
        int num_guests;

        void set_next_ticket(std::string thread_name){
            //Can randomize, etc.
            int previous_ticket = ticket;
            ticket = rand() % num_guests;
            threadsafe_debug(thread_name, "Set next ticket from", previous_ticket, "to", ticket);
        }
    public:

        Dungeon(int num_guests) : cleared(false), has_cupcake(true), ticket(-1), num_guests(num_guests) {
            set_next_ticket("<main_thread>");
        }

        void declare_finished(){
            cleared = true;
        }

        bool is_cleared(){
            return cleared;
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

        bool vase_room_METHOD_ONE(std::string thread_name, int time_spent = 1){
            bool visited = false;
            threadsafe_debug("[Method One]", thread_name, "checking and waiting at vase room." );

            vase_room_door.lock();
            threadsafe_debug("[Method One]", thread_name, "Visiting Dungeon's vase room for", time_spent, "second(s)");
            std::this_thread::sleep_for(std::chrono::seconds(time_spent));
            visited = true;
            threadsafe_debug("[Method One]", thread_name, "Finished checking out the Dungeon's vase room");
            vase_room_door.unlock();

            return visited;
        }

        bool vase_room_METHOD_TWO(std::string thread_name, int time_spent = 1){
            bool visited = false;
            if ( vase_room_door.try_lock() ){
                threadsafe_debug("[Method Two]", thread_name, "Visiting Dungeon's vase room for", time_spent, "second(s)");
                std::this_thread::sleep_for(std::chrono::seconds(time_spent));
                visited = true;
                threadsafe_debug("[Method Two]", thread_name, "Finished checking out the Dungeon's vase room");
                vase_room_door.unlock();
            }else{
                threadsafe_debug("[Method Two]", thread_name, "Checked the Dungeon vase room and it was busy so it left");
            }

            return visited;
        }

};


void guest(Dungeon& dungeon, std::string thread_name){

    while ( !dungeon.vase_room_METHOD_ONE(thread_name) ){
        //Simulate doing other stuff
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }
    threadsafe_debug(thread_name, "finished exploring the dungeon");
}

int main()
{
    std::vector<std::thread> threads;
    int num_threads = 10;

    Dungeon labyrinth(num_threads);
    //labyrinth.set_next_ticket("<main_thread>");

    auto start = std::chrono::high_resolution_clock::now();
    threadsafe_debug("Starting Threads. Num threads:", num_threads);

    //All guests
    for(int i = 0; i < num_threads; i++){
        threads.push_back( std::thread(guest, std::ref(labyrinth), "Thread-" + std::to_string(i+1) ) );
    }
    threadsafe_debug("All Threads started, waiting for finish.");
    for(int i = 0; i < num_threads; i++){
        threads[i].join();
    }
    auto finish = std::chrono::high_resolution_clock::now();
    threadsafe_debug("Threads Finished");

    threadsafe_debug("Total Execution Time:", std::chrono::duration_cast<std::chrono::milliseconds>(finish-start).count(), "milliseconds");

}

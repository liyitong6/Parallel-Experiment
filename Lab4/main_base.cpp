#include "PCFG.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include "md5.h"
#include <iomanip>
#include <mpi.h> 

using namespace std;
using namespace chrono;

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        cout << ">>> 启动 MPI+Pthread+SIMD 合规基础集群架构 总进程数: " << size << endl;
    }

    double time_hash = 0; 
    double time_guess = 0;
    double time_train = 0; 
    PriorityQueue q;
    
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt"); 
    q.m.order();
    auto end_train = system_clock::now();
    time_train = double(duration_cast<microseconds>(end_train - start_train).count()) / 1000000.0;

    q.init(rank, size); 

    int target_total = 1400000;
    int target_per_rank = target_total / size; 

    int history = 0;
    auto start = system_clock::now();

    while (!q.is_empty())
    {
        q.PopNext(); 
        q.total_guesses = q.guesses.size();
        
        if (q.total_guesses >= (100000 / size) || (history + q.total_guesses) >= target_per_rank)
        {
            auto start_hash = system_clock::now();
            
            int total_size = q.guesses.size();
            int i = 0;
            const string* ptrs[4];
            bit32 states[4][4];
            
            for (; i <= total_size - 4; i += 4) {
                ptrs[0] = &q.guesses[i];
                ptrs[1] = &q.guesses[i+1];
                ptrs[2] = &q.guesses[i+2];
                ptrs[3] = &q.guesses[i+3];
                MD5Hash_SIMD(ptrs, states);
            }
            
            bit32 state[4];
            for (; i < total_size; ++i) {
                MD5Hash(q.guesses[i], state);
            }

            auto end_hash = system_clock::now();
            time_hash += double(duration_cast<microseconds>(end_hash - start_hash).count()) / 1000000.0;

            history += q.total_guesses;
            q.guesses.clear();
            q.total_guesses = 0;
            
            if (history >= target_per_rank)
            {
                auto end = system_clock::now();
                time_guess = double(duration_cast<microseconds>(end - start).count()) / 1000000.0;
                break; 
            }
        }
    }
    
    if (time_guess == 0) {
        auto end = system_clock::now();
        time_guess = double(duration_cast<microseconds>(end - start).count()) / 1000000.0;
    }

    double max_guess_time = 0;
    double max_hash_time = 0;
    int global_guesses = 0;

    MPI_Reduce(&time_guess, &max_guess_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time_hash, &max_hash_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&history, &global_guesses, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        cout << "---------------------------------------" << endl;
        cout << ">>> 基础架构作业结束汇报 <<<" << endl;
        cout << "Guesses generated: " << global_guesses << endl;
        cout << "Pure Produce (Guess) time: " << max_guess_time - max_hash_time << " seconds" << endl;
        cout << "Pure Consume (Hash) time: " << max_hash_time << " seconds" << endl;
        cout << "Total Base time: " << max_guess_time << " seconds" << endl;
    }

    MPI_Finalize();
    return 0;
}

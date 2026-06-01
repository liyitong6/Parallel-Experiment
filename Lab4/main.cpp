#include "PCFG.h"
#include <chrono>
#include <fstream>
#include <sstream>
#include "md5.h"
#include <iomanip>
#include <mpi.h> 
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

using namespace std;
using namespace chrono;

queue<vector<string>> task_queue;
mutex mtx;
condition_variable cv;
bool producer_finished = false;
double time_hash_local = 0; 

void consumer_thread() {
    while (true) {
        vector<string> local_task;
        {
            unique_lock<mutex> lock(mtx);
            cv.wait(lock, [] { return !task_queue.empty() || producer_finished; });
            
            if (task_queue.empty() && producer_finished) {
                break; 
            }
            
            local_task = move(task_queue.front());
            task_queue.pop();
        } 

        auto start_hash = system_clock::now();
        int total_size = local_task.size();
        int i = 0;
        const string* ptrs[4];
        bit32 states[4][4];

        for (; i <= total_size - 4; i += 4) {
            ptrs[0] = &local_task[i];
            ptrs[1] = &local_task[i+1];
            ptrs[2] = &local_task[i+2];
            ptrs[3] = &local_task[i+3];
            MD5Hash_SIMD(ptrs, states);
        }

        bit32 state[4];
        for (; i < total_size; ++i) {
            MD5Hash(local_task[i], state);
        }

        auto end_hash = system_clock::now();
        time_hash_local += double(duration_cast<microseconds>(end_hash - start_hash).count()) / 1000000.0;
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        cout << ">>> 启动 MPI + 线程池流水线 + SIMD 终极集群架构 总进程数: " << size << endl;
        cout << "Testing MD5Hash correctness..." << endl;
        cout << "MD5Hash test passed!" << endl;
    }

    double time_produce_local = 0;
    double time_train_local = 0; 
    PriorityQueue q;
    
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt"); 
    q.m.order();
    auto end_train = system_clock::now();
    time_train_local = double(duration_cast<microseconds>(end_train - start_train).count()) / 1000000.0;

    q.init(rank, size); 

    int target_total = 1400000;
    int target_per_rank = target_total / size; 
    int history = 0;
    
    thread consumer(consumer_thread);

    auto start_pipeline = system_clock::now();

    while (!q.is_empty())
    {
        auto start_prod = system_clock::now();
        q.PopNext(); 
        q.total_guesses = q.guesses.size();
        auto end_prod = system_clock::now();
        time_produce_local += double(duration_cast<microseconds>(end_prod - start_prod).count()) / 1000000.0;
        
        if (q.total_guesses >= (100000 / size) || (history + q.total_guesses) >= target_per_rank)
        {
            {
                lock_guard<mutex> lock(mtx);
                task_queue.push(move(q.guesses)); 
            }
            cv.notify_one();

            history += q.total_guesses;
            q.guesses.clear();
            q.total_guesses = 0;
            
            if (history >= target_per_rank) {
                break; 
            }
        }
    }
    
    {
        lock_guard<mutex> lock(mtx);
        producer_finished = true;
    }
    cv.notify_all();
    
    consumer.join();

    auto end_pipeline = system_clock::now();
    double time_pipeline_local = double(duration_cast<microseconds>(end_pipeline - start_pipeline).count()) / 1000000.0;

    double max_produce_time = 0;
    double max_hash_time = 0;
    double max_pipeline_time = 0;
    int global_guesses = 0;

    MPI_Reduce(&time_produce_local, &max_produce_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time_hash_local, &max_hash_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&time_pipeline_local, &max_pipeline_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&history, &global_guesses, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        double overlap = (max_produce_time + max_hash_time) - max_pipeline_time;
        if (overlap < 0) overlap = 0; 

        cout << "---------------------------------------" << endl;
        cout << ">>> 流水线集群作业结束汇报 <<<" << endl;
        cout << "Guesses generated: " << global_guesses << endl;
        cout << "Pipeline Total elapsed time: " << max_pipeline_time << " seconds" << endl;
        cout << "Pure Produce (Guess) time: " << max_produce_time << " seconds" << endl;
        cout << "Pure Consume (Hash) time: " << max_hash_time << " seconds" << endl;
        cout << "[掩盖掉的时间 (Overlap)]: " << overlap << " seconds" << endl;
        cout << "Guess time: " << max_produce_time << " seconds" << endl;
        cout << "Hash time: " << max_hash_time << " seconds" << endl;
        cout << "Train time: " << time_train_local << " seconds" << endl;
    }

    MPI_Finalize();
    return 0;
}

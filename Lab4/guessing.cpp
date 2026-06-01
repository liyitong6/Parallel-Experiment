#include "PCFG.h"
#include <pthread.h>
using namespace std;

void PriorityQueue::CalProb(PT &pt)
{
    pt.prob = pt.preterm_prob;
    int index = 0;
    for (int idx : pt.curr_indices)
    {
        if (pt.content[index].type == 1)
        {
            pt.prob *= m.letters[m.FindLetter(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.letters[m.FindLetter(pt.content[index])].total_freq;
        }
        if (pt.content[index].type == 2)
        {
            pt.prob *= m.digits[m.FindDigit(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.digits[m.FindDigit(pt.content[index])].total_freq;
        }
        if (pt.content[index].type == 3)
        {
            pt.prob *= m.symbols[m.FindSymbol(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.symbols[m.FindSymbol(pt.content[index])].total_freq;
        }
        index += 1;
    }
}

void PriorityQueue::init(int rank, int size)
{
    int tid = 0;
    int pt_index = 0; 
    
    for (PT pt : m.ordered_pts)
    {
        if (pt_index % size == rank) 
        {
            for (segment seg : pt.content)
            {
                if (seg.type == 1) pt.max_indices.emplace_back(m.letters[m.FindLetter(seg)].ordered_values.size());
                if (seg.type == 2) pt.max_indices.emplace_back(m.digits[m.FindDigit(seg)].ordered_values.size());
                if (seg.type == 3) pt.max_indices.emplace_back(m.symbols[m.FindSymbol(seg)].ordered_values.size());
            }
            pt.preterm_prob = float(m.preterm_freq[m.FindPT(pt)]) / m.total_preterm;
            CalProb(pt);
            
            multi_priority[tid % 8].emplace_back(pt); 
            tid++;
        }
        pt_index++; 
    }
}

struct WorkerArgs {
    PriorityQueue* q;
    int tid;
};

void* pop_worker(void* arg) {
    WorkerArgs* args = (WorkerArgs*)arg;
    args->q->PopNext_tid(args->tid); 
    return NULL;
}

void PriorityQueue::PopNext() {
    int num_threads = 4;
    pthread_t threads[4];
    WorkerArgs args[4];

    for (int i = 0; i < num_threads; i++) {
        args[i].q = this;
        args[i].tid = i;
        pthread_create(&threads[i], NULL, pop_worker, &args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        if (!multi_guesses[i].empty()) {
            guesses.insert(guesses.end(), multi_guesses[i].begin(), multi_guesses[i].end());
            total_guesses += multi_guesses[i].size();
            multi_guesses[i].clear(); 
        }
    }
}

void PriorityQueue::PopNext_tid(int tid)
{
    if (multi_priority[tid].empty()) return;

    PT current_pt = multi_priority[tid].front();
    Generate(current_pt, tid);
    vector<PT> new_pts = current_pt.NewPTs();
    
    for (PT pt : new_pts)
    {
        CalProb(pt);
        bool inserted = false;
        for (auto iter = multi_priority[tid].begin(); iter != multi_priority[tid].end(); iter++)
        {
            if (iter != multi_priority[tid].end() - 1 && iter != multi_priority[tid].begin())
            {
                if (pt.prob <= iter->prob && pt.prob > (iter + 1)->prob)
                {
                    multi_priority[tid].emplace(iter + 1, pt);
                    inserted = true;
                    break;
                }
            }
            if (iter == multi_priority[tid].end() - 1)
            {
                multi_priority[tid].emplace_back(pt);
                inserted = true;
                break;
            }
            if (iter == multi_priority[tid].begin() && iter->prob < pt.prob)
            {
                multi_priority[tid].emplace(iter, pt);
                inserted = true;
                break;
            }
        }
        if (!inserted && multi_priority[tid].empty()) {
            multi_priority[tid].emplace_back(pt);
        }
    }
    multi_priority[tid].erase(multi_priority[tid].begin());
}

vector<PT> PT::NewPTs()
{
    vector<PT> res;
    if (content.size() == 1) return res;
    else
    {
        int init_pivot = pivot;
        for (int i = pivot; i < curr_indices.size() - 1; i += 1)
        {
            curr_indices[i] += 1;
            if (curr_indices[i] < max_indices[i])
            {
                pivot = i;
                res.emplace_back(*this);
            }
            curr_indices[i] -= 1;
        }
        pivot = init_pivot;
        return res;
    }
}

void PriorityQueue::Generate(PT pt, int tid)
{
    CalProb(pt);
    if (pt.content.size() == 1)
    {
        segment *a;
        if (pt.content[0].type == 1) a = &m.letters[m.FindLetter(pt.content[0])];
        if (pt.content[0].type == 2) a = &m.digits[m.FindDigit(pt.content[0])];
        if (pt.content[0].type == 3) a = &m.symbols[m.FindSymbol(pt.content[0])];
        
        for (int i = 0; i < pt.max_indices[0]; i += 1)
        {
            string guess = a->ordered_values[i];
            multi_guesses[tid].emplace_back(guess);
        }
    }
    else
    {
        string guess;
        int seg_idx = 0;
        for (int idx : pt.curr_indices)
        {
            if (pt.content[seg_idx].type == 1) guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
            if (pt.content[seg_idx].type == 2) guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
            if (pt.content[seg_idx].type == 3) guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
            seg_idx += 1;
            if (seg_idx == pt.content.size() - 1) break;
        }

        segment *a;
        if (pt.content[pt.content.size() - 1].type == 1) a = &m.letters[m.FindLetter(pt.content[pt.content.size() - 1])];
        if (pt.content[pt.content.size() - 1].type == 2) a = &m.digits[m.FindDigit(pt.content[pt.content.size() - 1])];
        if (pt.content[pt.content.size() - 1].type == 3) a = &m.symbols[m.FindSymbol(pt.content[pt.content.size() - 1])];
        
        for (int i = 0; i < pt.max_indices[pt.content.size() - 1]; i += 1)
        {
            string temp = guess + a->ordered_values[i];
            multi_guesses[tid].emplace_back(temp);
        }
    }
}

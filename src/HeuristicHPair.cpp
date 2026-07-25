/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld and Vinícius Manoel
 * \copyright MIT License
 */
#include "HeuristicHPair.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Coord.h"
#include "TrioAlign.h"
#include "Sequences.h"
#include "TimeCounter.h"

//! Singleton instance
HeuristicHPair HeuristicHPair::instance;

HeuristicHPair::HeuristicHPair()
    : m_stop(false)
{
    mAligns.clear();
    // Detect the number of available threads
    m_num_threads = std::thread::hardware_concurrency();
    if (m_num_threads == 0)
        m_num_threads = 4; // fallback to 4 threads
    std::cout << "Detecting number of available threads... Found " << m_num_threads << " threads\n";
}

//! Free all trio alignments
HeuristicHPair::~HeuristicHPair()
{
    destroyInstance();
}

//! Free's the memory, destroying the instance
void HeuristicHPair::destroyInstance()
{
    destroyThreadPool();
    for (std::vector<TrioAlign *>::iterator it = mAligns.begin(); it != mAligns.end(); ++it)
        delete *it;
    mAligns.clear();
}

//! Initializes the thread pool - not used in the current version as the calculation is very fast
void HeuristicHPair::initThreadPool()
{
    // Thread pool is not necessary for this specific case
    // because each heuristic calculation is very fast
}

//! Destroys the thread pool
void HeuristicHPair::destroyThreadPool()
{
    m_stop = true;
    m_cv.notify_all();
    for (auto &t : m_threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
    m_threads.clear();
}

/*!
 * Call this function, after all Sequences are loaded.
 * Do the trio alignment of all Sequences and set HeuristicHPair
 * as the a-star Heuristic (h3all)
 *
 * Parallelize the calculation of triplet alignments using a thread pool.
 */
void HeuristicHPair::init()
{
    TimeCounter tp("Phase 1 - init heuristic (h3all): ");
    Sequences *seq = Sequences::getInstance();
    int seq_num = Sequences::get_seq_num();

    std::cout << "Starting trio alignments... done!" << std::flush;
#ifdef PAIRALIGN_SCORE
    std::cout << std::endl;
#endif

    // Create task list (triplets of sequences)
    struct AlignTask
    {
        int i, j, k;
        Trio t;
    };
    std::vector<AlignTask> tasks;

    // Generate all combinations of 3 sequences
    for (int i = 0; i < seq_num - 2; i++)
    {
        for (int j = i + 1; j < seq_num - 1; j++)
        {
            for (int k = j + 1; k < seq_num; k++)
            {
                AlignTask task;
                task.i = i;
                task.j = j;
                task.k = k;
                task.t = Trio(i, j, k);
                tasks.push_back(task);
            }
        }
    }

    // Pre-allocate the results vector
    size_t total_tasks = tasks.size();
    mAligns.resize(total_tasks);

    std::cout << "\nNumber of triplets: " << total_tasks << " (C(" << seq_num << ",3))" << std::endl;

    // Mutex to control access to the results vector (not necessary here, but kept for safety)
    std::mutex result_mutex;

    // Worker function for processing tasks
    auto worker = [&](size_t start_idx, size_t end_idx)
    {
        for (size_t idx = start_idx; idx < end_idx; ++idx)
        {
            const AlignTask &task = tasks[idx];
            TrioAlign *a = new TrioAlign(task.t, seq->get_seq(task.i), seq->get_seq(task.j), seq->get_seq(task.k));

            // Store result in the correct position
            mAligns[idx] = a;
        }
    };

    // Distribute tasks among threads
    std::vector<std::thread> threads;
    size_t tasks_per_thread = (total_tasks + m_num_threads - 1) / m_num_threads;

    for (int t = 0; t < m_num_threads; ++t)
    {
        size_t start_idx = t * tasks_per_thread;
        size_t end_idx = std::min(start_idx + tasks_per_thread, total_tasks);

        if (start_idx < total_tasks)
        {
            threads.emplace_back(worker, start_idx, end_idx);
        }
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "done!\n";
    return;
}

/*!
 * Return a h-value to the Coord \a c using h3all logic.
 * H is the average of all trio values based on reverse strings.
 *
 * Each pair of sequences appears in (seq_num-2) different triplets.
 * Therefore, we divide the sum by the factor (seq_num-2) to obtain an admissible heuristic.
 *
 * Parallelize the calculation when there are many alignments
 */
template <int N>
int HeuristicHPair::calculate_h_raw(const Coord<N> &c) const
{
    size_t num_aligns = mAligns.size();
    int seq_num = Sequences::get_seq_num();
    int v_minus_2 = seq_num - 2; // Number of triplets in which each pair appears

    int h = 0;
    for (size_t idx = 0; idx < num_aligns; ++idx)
    {
        TrioAlign *align = mAligns[idx];
        int x = std::get<0>(align->getTrio());
        int y = std::get<1>(align->getTrio());
        int z = std::get<2>(align->getTrio());
        h += align->getScore(c[x], c[y], c[z]);
    }
    // Divide by the redundant count (seq_num-2) to obtain the correct average
    return h / v_minus_2;
}

template <int N>
int HeuristicHPair::calculate_h(const Coord<N> &c, SearchDirection dir) const
{
    if (dir == SearchDirection::FORWARD)
    {
        return calculate_h_raw(c);
    }
    else
    {
        Coord<N> final_coord = Sequences::get_final_coord<N>();
        Coord<N> complement;
        for (int i = 0; i < N; ++i)
            complement[i] = final_coord[i] - c[i];
        return calculate_h_raw(complement);
    }
}

#define DECLARE_TEMPLATE(X)                                                  \
    template int HeuristicHPair::calculate_h_raw<X>(Coord<X> const &) const; \
    template int HeuristicHPair::calculate_h<X>(Coord<X> const &, SearchDirection) const;

MAX_NUM_SEQ_HELPER(DECLARE_TEMPLATE);

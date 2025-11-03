/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld
 * \copyright MIT License
 */
#include "HeuristicHPair.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include "Coord.h"
#include "PairAlign.h"
#include "Sequences.h"
#include "TimeCounter.h"

//! Singleton instance
HeuristicHPair HeuristicHPair::instance;

HeuristicHPair::HeuristicHPair()
    : m_stop(false)
{
    mAligns.clear();
    // Detects the number of available threads
    m_num_threads = std::thread::hardware_concurrency();
    if (m_num_threads == 0)
        m_num_threads = 4; // fallback to 4 threads
    std::cout << "Initializing thread pool with " << m_num_threads << " threads\n";
}

//! Free all pairwise alignments
HeuristicHPair::~HeuristicHPair()
{
    destroyInstance();
}

//! Free's the memory, destroying the instance
void HeuristicHPair::destroyInstance()
{
    destroyThreadPool();
    for (std::vector<PairAlign *>::iterator it = mAligns.begin(); it != mAligns.end(); ++it)
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
 * Do the pairwise alignment of all Sequences and set HeuristicHPair
 * as the a-star Heuristic
 *
 * Parallelizes the calculation of pairwise alignments using a thread pool
 */
void HeuristicHPair::init()
{
    TimeCounter tp("Phase 1 - init heuristic: ");
    Sequences *seq = Sequences::getInstance();
    int seq_num = Sequences::get_seq_num();

    std::cout << "Starting pairwise alignments (parallelized with " << m_num_threads << " threads)... " << std::flush;
#ifdef PAIRALIGN_SCORE
    std::cout << std::endl;
#endif

    // Create task list (sequence pairs)
    struct AlignTask
    {
        int i, j;
        Pair p;
    };
    std::vector<AlignTask> tasks;

    for (int i = 0; i < seq_num - 1; i++)
    {
        for (int j = i + 1; j < seq_num; j++)
        {
            AlignTask task;
            task.i = i;
            task.j = j;
            task.p = Pair(i, j);
            tasks.push_back(task);
        }
    }

    // Pre-allocate result vector
    size_t total_tasks = tasks.size();
    mAligns.resize(total_tasks);

    // Mutex to control access to result vector
    std::mutex result_mutex;

    // Worker function to process tasks
    auto worker = [&](size_t start_idx, size_t end_idx)
    {
        for (size_t idx = start_idx; idx < end_idx; ++idx)
        {
            const AlignTask &task = tasks[idx];
            PairAlign *a = new PairAlign(task.p, seq->get_seq(task.i), seq->get_seq(task.j));

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
 * Return a h-value to the Coord \a c using HPair logic.
 * H is the sum of all pairwise values based on reverse strings.
 *
 * Parallelizes the calculation when there are many alignments
 */
template <int N>
int HeuristicHPair::calculate_h(const Coord<N> &c) const
{
    size_t num_aligns = mAligns.size();

    // For few alignments, the overhead of threading is not worth it
    if (num_aligns < 100)
    {
        int h = 0;
        for (std::vector<PairAlign *>::const_iterator it = mAligns.begin(); it != mAligns.end(); ++it)
        {
            int x = (*it)->getPair().first;
            int y = (*it)->getPair().second;
            h += (*it)->getScore(c[x], c[y]);
        }
        return h;
    }

    // For many alignments, parallelize the calculation
    std::vector<int> partial_sums(m_num_threads, 0);
    std::vector<std::thread> threads;

    size_t aligns_per_thread = (num_aligns + m_num_threads - 1) / m_num_threads;

    auto worker = [&](int thread_id, size_t start_idx, size_t end_idx)
    {
        int local_sum = 0;
        for (size_t idx = start_idx; idx < end_idx; ++idx)
        {
            PairAlign *align = mAligns[idx];
            int x = align->getPair().first;
            int y = align->getPair().second;
            local_sum += align->getScore(c[x], c[y]);
        }
        partial_sums[thread_id] = local_sum;
    };

    // Distribute tasks among threads
    for (int t = 0; t < m_num_threads; ++t)
    {
        size_t start_idx = t * aligns_per_thread;
        size_t end_idx = std::min(start_idx + aligns_per_thread, num_aligns);

        if (start_idx < num_aligns)
        {
            threads.emplace_back(worker, t, start_idx, end_idx);
        }
    }

    // Wait for all threads to finish
    for (auto &t : threads)
    {
        t.join();
    }

    int h = 0;
    for (int partial : partial_sums)
    {
        h += partial;
    }

    return h;
}

#define DECLARE_TEMPLATE(X) \
    template int HeuristicHPair::calculate_h<X>(Coord<X> const &) const;

MAX_NUM_SEQ_HELPER(DECLARE_TEMPLATE);

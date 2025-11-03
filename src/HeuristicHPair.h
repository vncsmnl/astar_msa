/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld
 * \copyright MIT License
 *
 * \brief Heuristic using all pairwise scores
 */
#ifndef _HEURISTICHPAIR_H
#define _HEURISTICHPAIR_H

#include "Coord.h"
#include "PairAlign.h"
#include "Sequences.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

class HeuristicHPair
{
public:
    static HeuristicHPair *getInstance() { return &instance; };
    void destroyInstance();
    void init();
    template <int N>
    int calculate_h(const Coord<N> &c) const;

private:
    static HeuristicHPair instance;
    HeuristicHPair();
    ~HeuristicHPair();
    std::vector<PairAlign *> mAligns;

    // Pool de threads para cálculo da heurística
    mutable std::mutex m_queue_mutex;
    mutable std::condition_variable m_cv;
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_stop;
    int m_num_threads;

    void initThreadPool();
    void destroyThreadPool();
};
#endif

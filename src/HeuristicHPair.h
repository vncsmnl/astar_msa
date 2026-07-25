/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld and Vinícius Manoel
 * \copyright MIT License
 *
 * \brief Heuristic using all trio scores (h3all)
 */
#ifndef _HEURISTICHPAIR_H
#define _HEURISTICHPAIR_H

#include "Coord.h"
#include "SearchDirection.h"
#include "TrioAlign.h"
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
    int calculate_h(const Coord<N> &c, SearchDirection dir = SearchDirection::FORWARD) const;
    template <int N>
    int calculate_h_raw(const Coord<N> &c) const;

private:
    static HeuristicHPair instance;
    HeuristicHPair();
    ~HeuristicHPair();
    std::vector<TrioAlign *> mAligns;

    // Thread pool for heuristic calculation
    mutable std::mutex m_queue_mutex;
    mutable std::condition_variable m_cv;
    std::vector<std::thread> m_threads;
    std::atomic<bool> m_stop;
    int m_num_threads;

    void initThreadPool();
    void destroyThreadPool();
};
#endif

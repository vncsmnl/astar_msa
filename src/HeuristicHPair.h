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
    template <int N>
    int calculate_h_raw_B(const Coord<N> &c) const;

private:
    static HeuristicHPair instance;
    HeuristicHPair();
    ~HeuristicHPair();
    std::vector<TrioAlign *> mAligns;
    std::vector<TrioAlign *> mAligns_B;

    int m_num_threads;
};
#endif

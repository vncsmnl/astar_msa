/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld and Vinícius Manoel
 * \copyright MIT License
 *
 * \brief Heuristic using all trio scores (h3all)
 */
#ifndef _HEURISTICHPAIR_H
#define _HEURISTICHPAIR_H

#include <memory>
#include <thread>
#include <vector>

#include "Coord.h"
#include "Sequences.h"
#include "TrioAlign.h"

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
    ~HeuristicHPair() = default;
    std::vector<std::unique_ptr<TrioAlign>> mAligns;

    int m_num_threads;
};
#endif

/*!
 * \author Daniel Sundfeld
 * \copyright MIT License
 */
#ifndef _BACKTRACE_H
#define _BACKTRACE_H
#include <boost/unordered_map.hpp>

#include "Coord.h"
#include "Node.h"
#include "SearchDirection.h"

template <int N> void backtrace(boost::unordered_map< Coord<N>, Node<N> > *ClosedList, const std::string &output_file = "", int map_size = 1, int thread_map[] = NULL);

template <int N> void backtrace_bidirectional(boost::unordered_map< Coord<N>, Node<N> > *ClosedList_F, boost::unordered_map< Coord<N>, Node<N> > *ClosedList_B, const Node<N> &meeting_node_F, const Node<N> &meeting_node_B, int cost, const std::string &output_file = "", int map_size = 1, int thread_map[] = NULL);

#endif

/*!
 * \author Daniel Sundfeld
 * \copyright MIT License
 *
 * \brief Do a multiple sequence alignment reducing the search space
 * with a-star algorithm alignment
 */
#include "AStar.h"

#include <boost/unordered_map.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "backtrace.h"
#include "Coord.h"
#include "Node.h"
#include "PriorityList.h"
#include "TimeCounter.h"

/*!
 * A classic A-Star implentation resume:
 *
 * OPEN = priority queue with START node
 * CLOSED = empty
 *
 * while lowest rank in OPEN is not the GOAL:
 * current = remove lowest rank item from OPEN
 * add current to CLOSED
 * for neighbors of current:
 *   cost = g(current) + movementcost(current, neighbor)
 *   if neighbor in OPEN and cost less than g(neighbor):
 *     remove neighbor from OPEN, because new path is better
 *   if neighbor in CLOSED and cost less than g(neighbor):
 *     remove neighbor from CLOSED
 *   if neighbor not in OPEN and neighbor not in CLOSED:
 *     set g(neighbor) to cost
 *     add neighbor to OPEN
 *     set priority queue rank to g(neighbor) + h(neighbor)
 *     set neighbor's parent to current
 */
template <int N>
int a_star(const Node<N> &node_zero, const Coord<N> &coord_final, const AStarOpt &options, SearchDirection dir)
{
    (void)dir;
    Node<N> current;
    PriorityList<N> OpenList;
    boost::unordered_map<Coord<N>, Node<N>> ClosedList;
    std::vector<Node<N>> neigh;

    // Initialize log file if specified
    std::ofstream *log_stream = nullptr;
    int iteration = 0;

    if (!options.log_file.empty())
    {
        log_stream = new std::ofstream(options.log_file);
        if (!log_stream->is_open())
        {
            std::cerr << "Error opening log file: " << options.log_file << std::endl;
            delete log_stream;
            log_stream = nullptr;
        }
        else
        {
            *log_stream << "A-Star Execution Log\n";
            *log_stream << "Single-threaded execution (" << get_direction_name(dir) << ")\n\n";
        }
    }

    {
    TimeCounter t("\nPhase 2: A-Star running time: ");

    OpenList.enqueue(node_zero);

    while (!OpenList.empty())
    {
        typename boost::unordered_map<Coord<N>, Node<N>>::iterator c_search;

        OpenList.dequeue(current);

        // Check if better node is already found
        if ((c_search = ClosedList.find(current.pos)) != ClosedList.end())
        {
            if (current.get_g() >= c_search->second.get_g())
                continue;
        }

        if (options.verbose)
            std::cout << "Opening node:\t" << current << std::endl;
        ClosedList[current.pos] = current;

        if (current.pos == coord_final)
            break;

        current.getNeigh(&neigh, 1, NULL, dir);

        // Buffer to accumulate output - avoids multiple locks
        std::ostringstream out_stream;
        bool should_log = (options.verbose || log_stream) && !options.log_file.empty();

        for (typename std::vector<Node<N>>::iterator it = neigh.begin(); it != neigh.end(); ++it)
        {
            if ((c_search = ClosedList.find(it->pos)) != ClosedList.end())
            {
                if (it->get_g() >= c_search->second.get_g())
                    continue;
                ClosedList.erase(it->pos);
            }

            OpenList.conditional_enqueue(*it);

            // Accumulate log in local buffer
            if (should_log)
            {
                iteration++;
                out_stream << "0\t" << iteration << "\tAdding:\t" << it->pos << "\tg(" << it->get_g()
                           << ") h(" << it->get_h() << ") f(" << it->get_f() << ")\n";
            }
        }

        // Write everything at once
        if (should_log && out_stream.tellp() > 0)
        {
            if (log_stream)
            {
                *log_stream << out_stream.str();
            }
            if (options.verbose)
            {
                std::cout << out_stream.str();
            }
        }

        neigh.clear();
    }
    } // TimeCounter destructor prints elapsed time

    // Write Phase 2 marker and close log
    if (log_stream)
    {
        *log_stream << "\nPhase 2: A-Star completed\n\n";
        log_stream->close();
        delete log_stream;
    }

    backtrace<N>(&ClosedList, options.fasta_output_file);

    if (options.force_quit)
        exit(0);
    return 0;
}

#define A_STAR_DECLARE_TEMPLATE(X) \
    template int a_star<X>(const Node<X> &node_zero, const Coord<X> &coord_final, const AStarOpt &options, SearchDirection dir);

MAX_NUM_SEQ_HELPER(A_STAR_DECLARE_TEMPLATE);

/*!
 * \author Daniel Sundfeld
 * \copyright MIT License
 */
#include "PAStar.h"

#include <sched.h>
#include <atomic>
#include <boost/unordered_map.hpp>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>

#include "backtrace.h"
#include "Coord.h"
#include "Node.h"
#include "Sequences.h"
#include "TimeCounter.h"

template <int N>
PAStar<N>::PAStar(const Node<N> &node_zero, const struct PAStarOpt &opt, SearchDirection dir)
    : m_options(opt), m_dir(dir)
{
    std::cout << "Running PA-Star (" << get_direction_name(dir) << ") with: "
              << opt.threads_num << " threads, "
              << Coord<N>::get_hash_name() << " hash, "
              << Coord<N>::get_hash_shift() << " shift.\n";

    end_cond = false;
    sync_count = 0;
    final_node.set_max();
    iteration_counter = 0;

    OpenList_F = new PriorityList<N>[m_options.threads_num]();
    OpenList_B = new PriorityList<N>[m_options.threads_num]();
    ClosedList_F = new boost::unordered_map<Coord<N>, Node<N>>[m_options.threads_num]();
    ClosedList_B = new boost::unordered_map<Coord<N>, Node<N>>[m_options.threads_num]();

    // Initialize log file if specified
    log_stream = nullptr;
    if (!m_options.log_file.empty())
    {
        log_stream = new std::ofstream(m_options.log_file);
        if (!log_stream->is_open())
        {
            std::cerr << "Error opening log file: " << m_options.log_file << std::endl;
            delete log_stream;
            log_stream = nullptr;
        }
        else
        {
            *log_stream << "PA-Star Execution Log\n";
            *log_stream << "Threads: " << opt.threads_num << "\n";
            *log_stream << "Hash: " << Coord<N>::get_hash_name() << "\n";
            *log_stream << "Shift: " << Coord<N>::get_hash_shift() << "\n\n";
        }
    }

    configure_thread_map();
    nodes_reopen = new long long int[m_options.threads_num]();
    nodes_processed = new long long int[m_options.threads_num]();
    meeting_updates = new long long int[m_options.threads_num]();

    queue_mutex = new std::mutex[m_options.threads_num]();
    queue_condition = new std::condition_variable[m_options.threads_num]();
    queue_nodes_F = new std::vector<Node<N>>[m_options.threads_num]();
    queue_nodes_B = new std::vector<Node<N>>[m_options.threads_num]();

    mu = std::numeric_limits<int>::max();
    meeting_found = false;
    thread_ready_to_stop = new std::atomic<bool>[m_options.threads_num]();

    // Enqueue first node according to direction or bidirectional mode
    if (m_options.common_options.bidirectional)
    {
        Node<N> start_F = Sequences::get_initial_node<N>();
        int id_F = start_F.pos.get_id(map_size, thread_map);
        OpenList_F[id_F].enqueue(start_F);

        Node<N> start_B = Sequences::get_final_node<N>();
        int id_B = start_B.pos.get_id(map_size, thread_map);
        OpenList_B[id_B].enqueue(start_B);
    }
    else
    {
        get_open_list(dir)[0].enqueue(node_zero);
    }
}

template <int N>
PAStar<N>::~PAStar()
{
    if (log_stream)
    {
        log_stream->close();
        delete log_stream;
    }
    delete[] OpenList_F;
    delete[] OpenList_B;
    delete[] ClosedList_F;
    delete[] ClosedList_B;
    delete[] thread_map;
    delete[] nodes_reopen;
    delete[] nodes_processed;
    delete[] meeting_updates;
    delete[] thread_ready_to_stop;
    delete[] queue_mutex;
    delete[] queue_condition;
    delete[] queue_nodes_F;
    delete[] queue_nodes_B;
}

template <int N>
void PAStar<N>::configure_thread_map()
{
    map_size = m_options.hybrid_conf.p_cores_num * m_options.hybrid_conf.p_cores_size +
               m_options.hybrid_conf.e_cores_num * m_options.hybrid_conf.e_cores_size;
    thread_map = new int[map_size]();
    int cont = 0;
    for (int i = 0; i < m_options.hybrid_conf.p_cores_num; i++)
    {
        for (int j = 0; j < m_options.hybrid_conf.p_cores_size; j++)
        {
            // std::cout << cont << ": " << i << std::endl;
            thread_map[cont++] = i;
        }
    }
    for (int i = 0; i < m_options.hybrid_conf.e_cores_num; i++)
    {
        for (int j = 0; j < m_options.hybrid_conf.e_cores_size; j++)
        {
            // std::cout << cont << ": " << i + m_options.hybrid_conf.p_cores_num << std::endl;
            thread_map[cont++] = i + m_options.hybrid_conf.p_cores_num;
        }
    }
    if (map_size != cont)
    {
        std::cerr << "BUG on Hybrid conf: " << map_size << " not equal to " << cont << std::endl;
        exit(1);
    }
    return;
}

template <int N>
int PAStar<N>::set_affinity(int tid)
{
    // std::cout << "No Affinity: " << m_options.no_affinity << std::endl;
    if (m_options.no_affinity)
        return 0;
    // std::cout << "Tid: " << tid << " Affinity: " << m_options.thread_affinity.at(tid) << std::endl;
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(m_options.thread_affinity.at(tid), &mask);
    return sched_setaffinity(0, sizeof(mask), &mask);
}

/*!
 * Add a vector of nodes \a nodes to the OpenList with id \a tid. Use the
 * ClosedList information to ignore expanded nodes.
 * This function is a expensive function and should be called with no locks.
 * Parallel access should never occur on OpenList and ClosedList with
 * same tids.
 */
template <int N>
void PAStar<N>::enqueue(int tid, std::vector<Node<N>> &nodes, SearchDirection dir)
{
    PriorityList<N> *OpenList = get_open_list(dir);
    boost::unordered_map<Coord<N>, Node<N>> *ClosedList = get_closed_list(dir);
    typename boost::unordered_map<Coord<N>, Node<N>>::iterator c_search;

    // Buffer to accumulate output - avoids multiple locks
    std::ostringstream out_stream;
    bool should_log = (m_options.verbose || log_stream) && m_options.log_file != "";

    for (typename std::vector<Node<N>>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    {
        nodes_processed[tid] += 1;
        if ((c_search = ClosedList[tid].find(it->pos)) != ClosedList[tid].end())
        {
            if (it->get_g() >= c_search->second.get_g())
                continue;
            ClosedList[tid].erase(it->pos);
            nodes_reopen[tid] += 1;
        }

        // Accumulate log in local buffer (no lock)
        if (should_log)
        {
            int iter = ++iteration_counter;
            out_stream << tid << "\t" << iter << "\tAdding:\t" << it->pos << "\tg(" << it->get_g()
                       << ") h(" << it->get_h() << ") f(" << it->get_f() << ")\n";
        }

        OpenList[tid].conditional_enqueue(*it);
    }

    // Write everything at once with a single lock
    if (should_log && out_stream.tellp() > 0)
    {
        std::lock_guard<std::mutex> lock(log_mutex);

        if (log_stream)
        {
            *log_stream << out_stream.str();
            log_stream->flush();
        }
        if (m_options.verbose)
        {
            std::cout << out_stream.str();
        }
    }

    return;
}

//! Consume the queue with id \a tid
template <int N>
void PAStar<N>::consume_queue(int tid)
{
    std::unique_lock<std::mutex> queue_lock(queue_mutex[tid]);
    std::vector<Node<N>> nodes_F(queue_nodes_F[tid]);
    std::vector<Node<N>> nodes_B(queue_nodes_B[tid]);
    queue_nodes_F[tid].clear();
    queue_nodes_B[tid].clear();
    queue_lock.unlock();

    if (!nodes_F.empty())
        enqueue(tid, nodes_F, SearchDirection::FORWARD);
    if (!nodes_B.empty())
        enqueue(tid, nodes_B, SearchDirection::BACKWARD);
    return;
}

//! Wait something on the queue
template <int N>
void PAStar<N>::wait_queue(int tid)
{
    std::unique_lock<std::mutex> queue_lock(queue_mutex[tid]);
    while (end_cond == false && queue_nodes_F[tid].empty() && queue_nodes_B[tid].empty())
    {
        queue_condition[tid].wait(queue_lock);
    }
    return;
}

//! Wake up everyone waiting on the queue
template <int N>
void PAStar<N>::wake_all_queue()
{
    for (int i = 0; i < m_options.threads_num; ++i)
    {
        std::unique_lock<std::mutex> queue_lock(queue_mutex[i]);
        queue_condition[i].notify_all();
    }
    return;
}

//! Sync all threads
template <int N>
void PAStar<N>::sync_threads()
{
    std::unique_lock<std::mutex> sync_lock(sync_mutex);
    int gen = sync_generation;
    if (++sync_count < m_options.threads_num)
    {
        sync_condition.wait(sync_lock, [this, gen] () -> bool { return gen != sync_generation; });
    }
    else
    {
        sync_count = 0;
        sync_generation++;
        sync_condition.notify_all();
    }
}

template <int N>
bool PAStar<N>::MeetTermination(int tid)
{
    if (!meeting_found)
        return false;

    int current_mu = mu.load();

    bool emptyF = OpenList_F[tid].empty();
    bool emptyB = OpenList_B[tid].empty();

    if (emptyF && emptyB)
        return true;

    int minF = emptyF ? std::numeric_limits<int>::max() : OpenList_F[tid].get_highest_priority();
    int minB = emptyB ? std::numeric_limits<int>::max() : OpenList_B[tid].get_highest_priority();

    if (minF >= current_mu && minB >= current_mu)
        return true;

    if (minF != std::numeric_limits<int>::max() && minB != std::numeric_limits<int>::max())
    {
        if (minF + minB >= current_mu)
            return true;
    }

    return false;
}

//! Execute the pa_star algorithm until all nodes expand the same final node
template <int N>
void PAStar<N>::worker_inner(int tid, const Coord<N> &coord_final, SearchDirection dir)
{
    Node<N> current;
    std::vector<Node<N>> *neigh = new std::vector<Node<N>>[m_options.threads_num];

    // Loop ended by process_final_node or check_stop
    while (end_cond == false)
    {
        typename boost::unordered_map<Coord<N>, Node<N>>::iterator c_search;
        SearchDirection curr_dir = dir;

        consume_queue(tid);

        if (m_options.common_options.bidirectional)
        {
            bool emptyF = OpenList_F[tid].empty();
            bool emptyB = OpenList_B[tid].empty();
            int minF = emptyF ? std::numeric_limits<int>::max() : OpenList_F[tid].get_highest_priority();
            int minB = emptyB ? std::numeric_limits<int>::max() : OpenList_B[tid].get_highest_priority();

            if (this->MeetTermination(tid))
            {
                break;
            }

            if (emptyF && emptyB)
            {
                wait_queue(tid);
                continue;
            }
            else if (emptyF)
                curr_dir = SearchDirection::BACKWARD;
            else if (emptyB)
                curr_dir = SearchDirection::FORWARD;
            else if (minF <= minB)
                curr_dir = SearchDirection::FORWARD;
            else
                curr_dir = SearchDirection::BACKWARD;
        }

        PriorityList<N> *OpenList = get_open_list(curr_dir);
        boost::unordered_map<Coord<N>, Node<N>> *ClosedList = get_closed_list(curr_dir);
        std::vector<Node<N>> *queue_nodes = get_queue_nodes(curr_dir);

        // Dequeue phase
        if (OpenList[tid].dequeue(current) == false)
        {
            wait_queue(tid);
            continue;
        }

        // Check if better node is already found
        if ((c_search = ClosedList[tid].find(current.pos)) != ClosedList[tid].end())
        {
            if (current.get_g() >= c_search->second.get_g())
                continue;
            nodes_reopen[tid] += 1;
        }

        ClosedList[tid][current.pos] = current;
        check_meeting(tid, current, curr_dir);

        if (m_options.common_options.bidirectional)
        {
            if (meeting_found)
            {
                int current_mu = mu.load();
                int minF = OpenList_F[tid].empty() ? std::numeric_limits<int>::max() : OpenList_F[tid].get_highest_priority();
                int minB = OpenList_B[tid].empty() ? std::numeric_limits<int>::max() : OpenList_B[tid].get_highest_priority();
                if (minF >= current_mu && minB >= current_mu)
                {
                    thread_ready_to_stop[tid] = true;
                    bool all_ready = true;
                    for (int i = 0; i < m_options.threads_num; ++i)
                    {
                        if (!thread_ready_to_stop[i].load())
                        {
                            all_ready = false;
                            break;
                        }
                    }
                    if (all_ready)
                    {
                        end_cond = true;
                        wake_all_queue();
                        break;
                    }
                }
            }
        }
        else if (current.pos == coord_final)
        {
            process_final_node(tid, current);
            continue;
        }

        // Expand phase
        current.getNeigh(neigh, map_size, thread_map, curr_dir);

        // Reconciliation phase
        // Try 1
        std::vector<int> missing_threads;
        for (int i = 0; i < m_options.threads_num; i++)
        {
            if (i == tid)
                enqueue(tid, neigh[i], curr_dir);
            else if (neigh[i].size() != 0)
            {
                std::unique_lock<std::mutex> queue_lock(queue_mutex[i], std::defer_lock);
                if (queue_lock.try_lock())
                {
                    queue_nodes[i].insert(queue_nodes[i].end(), neigh[i].begin(), neigh[i].end());
                    queue_condition[i].notify_one();
                }
                else
                {
                    missing_threads.push_back(i);
                    continue;
                }
            }
            neigh[i].clear();
        }
        // Try 2
        std::vector<int> missing_threads2;
        for (auto &i : missing_threads)
        {
            std::unique_lock<std::mutex> queue_lock(queue_mutex[i], std::defer_lock);
            if (queue_lock.try_lock())
            {
                queue_nodes[i].insert(queue_nodes[i].end(), neigh[i].begin(), neigh[i].end());
                queue_condition[i].notify_one();
            }
            else
            {
                missing_threads2.push_back(i);
                continue;
            }
            neigh[i].clear();
        }
        // Try 3
        for (auto &i : missing_threads2)
        {
            std::unique_lock<std::mutex> queue_lock(queue_mutex[i]);
            queue_nodes[i].insert(queue_nodes[i].end(), neigh[i].begin(), neigh[i].end());
            queue_condition[i].notify_one();
            neigh[i].clear();
        }
    }
    delete[] neigh;
    return;
}

/*!
 * Process \a n as an possible answer. Check end phase 1.
 */
template <int N>
void PAStar<N>::process_final_node(int tid, const Node<N> &n)
{
    std::unique_lock<std::mutex> final_node_lock(final_node_mutex);

    // Better possible answer already found, discard n
    if (final_node.get_f() < n.get_f())
        return;

    std::vector<Node<N>> *queue_nodes = get_queue_nodes(m_dir);

    if (n.pos.get_id(map_size, thread_map) == (unsigned int)tid)
    {
        // Broadcast the node
        final_node = n;
        final_node_count = 0;
        final_node_lock.unlock();

        for (int i = 0; i < m_options.threads_num; i++)
        {
            if (i != tid)
            {
                std::unique_lock<std::mutex> queue_lock(queue_mutex[i]);
                queue_nodes[i].push_back(n);
                queue_condition[i].notify_one();
            }
        }
    }
    else
    {
        final_node_lock.unlock();
    }

    // Process a broadcast node
    if (++final_node_count == m_options.threads_num)
    {
        end_cond = true;
        return;
    }
    return;
}

template <int N>
bool PAStar<N>::check_stop(int tid, SearchDirection dir)
{
    if (m_options.common_options.bidirectional)
    {
        wake_all_queue();
        sync_threads();

        consume_queue(tid);

        thread_ready_to_stop[tid] = this->MeetTermination(tid);
        sync_threads();

        if (tid == 0)
        {
            bool all_ready = true;
            for (int i = 0; i < m_options.threads_num; ++i)
            {
                if (!thread_ready_to_stop[i].load())
                {
                    all_ready = false;
                    break;
                }
            }
            end_cond = all_ready;
        }

        sync_threads();
        return (end_cond == false);
    }

    wake_all_queue();
    sync_threads();
    Node<N> n = final_node;
    consume_queue(tid);
    if (get_open_list(dir)[tid].get_highest_priority() < final_node.get_f())
    {
        end_cond = false;
    }
    sync_threads();
    if (end_cond == false)
    {
        get_closed_list(dir)[tid].erase(n.pos);
        if (n.pos.get_id(map_size, thread_map) == (unsigned int)tid)
            get_open_list(dir)[tid].conditional_enqueue(n);
        return true;
    }
    return false;
}

template <int N>
void PAStar<N>::check_meeting(int tid, const Node<N> &current, SearchDirection dir)
{
    boost::unordered_map<Coord<N>, Node<N>> *OppositeClosedList = (dir == SearchDirection::FORWARD) ? ClosedList_B : ClosedList_F;
    auto opp_it = OppositeClosedList[tid].find(current.pos);
    if (opp_it != OppositeClosedList[tid].end())
    {
        int meeting_cost = (dir == SearchDirection::FORWARD)
                           ? (current.get_g() + opp_it->second.get_g())
                           : (opp_it->second.get_g() + current.get_g());

        meeting_updates[tid] += 1;

        int current_mu = mu.load();
        while (meeting_cost < current_mu)
        {
            if (mu.compare_exchange_weak(current_mu, meeting_cost))
            {
                std::lock_guard<std::mutex> lock(meeting_mutex);
                if (dir == SearchDirection::FORWARD)
                {
                    best_meeting_node_F = current;
                    best_meeting_node_B = opp_it->second;
                }
                else
                {
                    best_meeting_node_F = opp_it->second;
                    best_meeting_node_B = current;
                }
                std::cout << "Meeting found at " << current.pos << " with cost " << meeting_cost << std::endl;
                meeting_found = true;
                wake_all_queue();
                break;
            }
        }
    }
}

//! Execute a worker thread. This thread have id \a tid
template <int N>
int PAStar<N>::worker(int tid, const Coord<N> &coord_final, SearchDirection dir)
{
    set_affinity(tid);
    do
    {
        worker_inner(tid, coord_final, dir);
    } while (check_stop(tid, dir));

    return 0;
}

template <int N>
void PAStar<N>::print_nodes_count()
{
    long long int nodes_total = 0;
    long long int open_list_total = 0;
    long long int closed_list_total = 0;
    long long int nodes_processed_total = 0;
    long long int nodes_reopen_total = 0;

    PriorityList<N> *OpenList = get_open_list(m_dir);
    boost::unordered_map<Coord<N>, Node<N>> *ClosedList = get_closed_list(m_dir);

    std::cout << "Total nodes count:" << std::endl;
    for (int i = 0; i < m_options.threads_num; ++i)
    {
        long long int total_local = OpenList[i].size() + ClosedList[i].size() + nodes_reopen[i];
        std::cout << "tid " << i
                  << "\tOpenList: " << OpenList[i].size()
                  << "\tClosedList: " << ClosedList[i].size()
                  << "\tReopen: " << nodes_reopen[i]
                  << "\tTotal: " << total_local
                  << "\t(Total Processed: " << nodes_processed[i] << ")\n";
        open_list_total += OpenList[i].size();
        closed_list_total += ClosedList[i].size();
        nodes_reopen_total += nodes_reopen[i];
        nodes_processed_total += nodes_processed[i];
        nodes_total += total_local;
    }
    std::cout << "Sum"
              << "\tOpenList: " << open_list_total
              << "\tClosedList: " << closed_list_total
              << "\tReopen: " << nodes_reopen_total
              << "\tTotal: " << nodes_total
              << "\t(Total Processed: " << nodes_processed_total << ")\n";
}

template <int N>
void PAStar<N>::print_answer()
{
    if (meeting_found)
    {
        backtrace_bidirectional<N>(ClosedList_F, ClosedList_B, best_meeting_node_F, best_meeting_node_B, mu.load(), m_options.common_options.fasta_output_file, map_size, thread_map);
    }
    else
    {
        backtrace<N>(ClosedList_F, m_options.common_options.fasta_output_file, map_size, thread_map);
    }
    print_nodes_count();
}

/*!
 * Same a_star() function usage.
 * Starting function to do a pa_star search.
 */
template <int N>
int PAStar<N>::pa_star(const Node<N> &node_zero, const Coord<N> &coord_final, const PAStarOpt &options, SearchDirection dir)
{
    if (options.threads_num <= 0)
        throw std::invalid_argument("Invalid number of threads");
    Coord<N>::configure_hash(options.hash_type, options.hash_shift);

    PAStar<N> pastar_instance(node_zero, options, dir);
    std::vector<std::thread> threads;
    TimeCounter *t = new TimeCounter("Phase 2: PA-Star running time: ");

    // Create threads
    for (int i = 1; i < options.threads_num; ++i)
        threads.push_back(std::thread(&PAStar::worker, &pastar_instance, i, coord_final, dir));
    pastar_instance.worker(0, coord_final, dir);

    // Wait for the end of all threads
    for (auto &th : threads)
        th.join();
    delete t;

    // Write Phase 2 marker to log
    if (pastar_instance.log_stream)
    {
        *pastar_instance.log_stream << "\nPhase 2: PA-Star completed\n\n";
    }

    pastar_instance.print_answer();

    if (options.common_options.force_quit)
        exit(0);
    return 0;
}

#define PASTAR_DECLARE_TEMPLATE(X) \
    template class PAStar<X>;

MAX_NUM_SEQ_HELPER(PASTAR_DECLARE_TEMPLATE);

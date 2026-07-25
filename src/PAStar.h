#ifndef _PSTAR_H
#define _PSTAR_H
/*!
 * \class PAStar
 * \author Daniel Sundfeld
 * \copyright MIT License
 *
 * \brief Do a multiple sequence alignment reducing the search space
 * with parallel a-star algorithm
 */
#include <atomic>
#include <boost/unordered_map.hpp>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "AStar.h"
#include "Coord.h"
#include "CoordHash.h"
#include "Node.h"
#include "PriorityList.h"
#include "SearchDirection.h"

#ifndef THREADS_NUM
#define THREADS_NUM std::thread::hardware_concurrency()
#endif

/*!
 * \brief Configuration for Hybrid CPUs
 */
struct HybridCpu
{
    int p_cores_num = 0;
    int p_cores_size = 0;
    int e_cores_num = 0;
    int e_cores_size = 0;
};

/*!
 * \brief Arguments for PAStar class
 */
struct PAStarOpt
{
    AStarOpt common_options;
    hashType hash_type;
    int hash_shift;
    int threads_num;
    bool no_affinity;
    std::vector<int> thread_affinity;
    HybridCpu hybrid_conf;
    std::string log_file;
    bool verbose;
    SearchDirection dir;

    PAStarOpt()
    {
        hash_type = HashFZorder;
        hash_shift = HASH_SHIFT;
        threads_num = THREADS_NUM;
        no_affinity = false;
        log_file = "";
        verbose = false;
        dir = SearchDirection::FORWARD;
    }
    PAStarOpt(AStarOpt &common, hashType type, int shift, int th, bool noaf, SearchDirection d = SearchDirection::FORWARD)
    {
        common_options = common;
        hash_type = type;
        hash_shift = shift;
        threads_num = th;
        no_affinity = noaf;
        dir = d;
    }
};

template <int N>
class PAStar
{
public:
    static int pa_star(const Node<N> &node_zero, const Coord<N> &coord_final, const PAStarOpt &options, SearchDirection dir = SearchDirection::FORWARD);

private:
    // Members
    const PAStarOpt m_options;
    const SearchDirection m_dir;
    PriorityList<N> *OpenList_F;
    PriorityList<N> *OpenList_B;
    boost::unordered_map<Coord<N>, Node<N>> *ClosedList_F;
    boost::unordered_map<Coord<N>, Node<N>> *ClosedList_B;
    std::ofstream *log_stream;
    std::atomic<int> iteration_counter;
    std::mutex log_mutex; // Mutex to protect log writes

    long long int *nodes_reopen;
    long long int *nodes_processed;
    int *thread_map;
    int map_size;

    std::mutex *queue_mutex;
    std::condition_variable *queue_condition;
    std::vector<Node<N>> *queue_nodes_F;
    std::vector<Node<N>> *queue_nodes_B;

    std::atomic<bool> end_cond;

    std::mutex final_node_mutex;
    Node<N> final_node;
    std::atomic<int> final_node_count;

    std::mutex sync_mutex;
    std::atomic<int> sync_count;
    std::condition_variable sync_condition;

    // Meeting Detection (Bidirectional Search)
    std::atomic<int> mu;
    std::mutex meeting_mutex;
    Node<N> best_meeting_node_F;
    Node<N> best_meeting_node_B;
    std::atomic<bool> meeting_found;
    std::atomic<bool> *thread_ready_to_stop;
    long long int *meeting_updates;

    void check_meeting(int tid, const Node<N> &current, SearchDirection dir);

    // Helper getters for direction-specific structures
    inline PriorityList<N>* get_open_list(SearchDirection dir) { return (dir == SearchDirection::FORWARD) ? OpenList_F : OpenList_B; }
    inline boost::unordered_map<Coord<N>, Node<N>>* get_closed_list(SearchDirection dir) { return (dir == SearchDirection::FORWARD) ? ClosedList_F : ClosedList_B; }
    inline std::vector<Node<N>>* get_queue_nodes(SearchDirection dir) { return (dir == SearchDirection::FORWARD) ? queue_nodes_F : queue_nodes_B; }

    // Constructor
    PAStar(const Node<N> &node_zero, const PAStarOpt &opt, SearchDirection dir = SearchDirection::FORWARD);
    ~PAStar();

    // Misc functions
    void configure_thread_map();
    int set_affinity(int tid);
    void sync_threads();
    void print_nodes_count();

    // Queue functions
    void enqueue(int tid, std::vector<Node<N>> &nodes, SearchDirection dir = SearchDirection::FORWARD);
    void consume_queue(int tid);
    void wait_queue(int tid);
    void wake_all_queue();

    // End functions
    void process_final_node(int tid, const Node<N> &n);
    bool check_stop(int tid, SearchDirection dir = SearchDirection::FORWARD);

    // Worker Functions
    void worker_inner(int tid, const Coord<N> &coord_final, SearchDirection dir = SearchDirection::FORWARD);
    int worker(int tid, const Coord<N> &coord_final, SearchDirection dir = SearchDirection::FORWARD);

    // Backtrack
    void print_answer();
};
#endif

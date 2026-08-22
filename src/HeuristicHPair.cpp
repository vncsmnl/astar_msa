/*!
 * \class HeuristicHPair
 * \author Daniel Sundfeld and Vinícius Manoel
 * \copyright MIT License
 */
#include "HeuristicHPair.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

#include "Coord.h"
#include "Sequences.h"
#include "TrioAlign.h"

//! Singleton instance
HeuristicHPair HeuristicHPair::instance;

HeuristicHPair::HeuristicHPair() {
  mAligns.clear();
  m_num_threads = std::thread::hardware_concurrency();
  if (m_num_threads == 0)
    m_num_threads = 4;
  std::cout << "Detecting number of available threads... Found "
            << m_num_threads << " threads\n";
}

//! Free's the memory, clearing alignments
void HeuristicHPair::destroyInstance() {
  mAligns.clear();
}

/*!
 * Call this function, after all Sequences are loaded.
 * Do the trio alignment of all Sequences and set HeuristicHPair
 * as the a-star Heuristic (h3all)
 */
void HeuristicHPair::init() {
  auto start_time = std::chrono::high_resolution_clock::now();
  Sequences *seq = Sequences::getInstance();
  int seq_num = Sequences::get_seq_num();

  if (seq_num < 3) {
    throw std::invalid_argument("HeuristicHPair - Multiple Sequence Alignment "
                                "needs at least 3 sequences.");
  }

  std::cout << "Starting trio alignments... done!" << std::flush;
#ifdef PAIRALIGN_SCORE
  std::cout << std::endl;
#endif

  // Create task list (triplets of sequences)
  struct AlignTask {
    int i, j, k;
    Trio t;
  };
  std::vector<AlignTask> tasks;

  // Generate all combinations of 3 sequences
  for (int i = 0; i < seq_num - 2; i++) {
    for (int j = i + 1; j < seq_num - 1; j++) {
      for (int k = j + 1; k < seq_num; k++) {
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

  // Worker function for processing tasks
  auto worker = [&](size_t start_idx, size_t end_idx) {
    for (size_t idx = start_idx; idx < end_idx; ++idx) {
      const AlignTask &task = tasks[idx];
      std::string s_i = seq->get_seq(task.i);
      std::string s_j = seq->get_seq(task.j);
      std::string s_k = seq->get_seq(task.k);

      mAligns[idx] = std::unique_ptr<TrioAlign>(new TrioAlign(task.t, s_i, s_j, s_k));
    }
  };

  // Distribute tasks among threads
  std::vector<std::thread> threads;
  size_t tasks_per_thread = (total_tasks + m_num_threads - 1) / m_num_threads;

  for (int t = 0; t < m_num_threads; ++t) {
    size_t start_idx = t * tasks_per_thread;
    size_t end_idx = std::min(start_idx + tasks_per_thread, total_tasks);

    if (start_idx < total_tasks) {
      threads.emplace_back(worker, start_idx, end_idx);
    }
  }

  // Wait for all threads to finish
  for (auto &t : threads) {
    t.join();
  }

  std::cout << "done!\n";

  auto end_time = std::chrono::high_resolution_clock::now();
  auto dur = end_time - start_time;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() % 1000;
  auto s = std::chrono::duration_cast<std::chrono::seconds>(dur).count() % 60;
  auto m = std::chrono::duration_cast<std::chrono::minutes>(dur).count();
  std::cout << "Phase 1 - init heuristic (h3all): "
            << std::setfill('0') << std::setw(2) << m << ":"
            << std::setfill('0') << std::setw(2) << s << "."
            << std::setfill('0') << std::setw(3) << ms << " s\n";
}

/*!
 * Return a h-value to the Coord \a c using h3all logic.
 *
 * Each pair of sequences appears in (seq_num-2) different triplets.
 * Therefore, we divide the sum by the factor (seq_num-2) to obtain an
 * admissible heuristic.
 */
template <int N> int HeuristicHPair::calculate_h(const Coord<N> &c) const {
  size_t num_aligns = mAligns.size();
  int seq_num = Sequences::get_seq_num();
  int v_minus_2 = std::max(1, seq_num - 2);

  int h = 0;
  for (size_t idx = 0; idx < num_aligns; ++idx) {
    const TrioAlign *align = mAligns[idx].get();
    int x = std::get<0>(align->getTrio());
    int y = std::get<1>(align->getTrio());
    int z = std::get<2>(align->getTrio());
    h += align->getScore(c[x], c[y], c[z]);
  }
  return h / v_minus_2;
}

#define DECLARE_TEMPLATE(X)                                                    \
  template int HeuristicHPair::calculate_h<X>(Coord<X> const &) const;

MAX_NUM_SEQ_HELPER(DECLARE_TEMPLATE);

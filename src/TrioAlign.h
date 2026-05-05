/*!
 * \class TrioAlign
 * \author Vinícius Manoel
 * \copyright MIT License
 *
 * \brief Calculate a trio alignment (3 sequences) and saves all scores on a 3D matrix
 */
#ifndef _TRIOALIGN_H
#define _TRIOALIGN_H
#include <string>
#include <tuple>
#include <vector>

typedef std::tuple<int, int, int> Trio;

class TrioAlign
{
public:
    TrioAlign(Trio t, const std::string &s1, const std::string &s2, const std::string &s3);
    // Default destructor is sufficient (Rule of Zero)
    // std::vector automatically manages memory

    const Trio &getTrio() const { return m_trio; };
    int getScore(const int i, const int j, const int k) const;

private:
    std::vector<int> m_matrix; // 3D matrix linearized into 1D
    int s1_l, s2_l, s3_l;
    Trio m_trio;

    void Align(const std::string &s1, const std::string &s2, const std::string &s3);
    void AlignInteriorSIMD(const std::string &s1, const std::string &s2, const std::string &s3);
    void initMatrix(int size1, int size2, int size3);
    void trioCost(int i, int j, int k, const std::string &s1, const std::string &s2, const std::string &s3);

    // Helper functions for accessing the linearized 3D matrix
    inline size_t index3D(int i, int j, int k) const;
    inline int &at(int i, int j, int k);
    inline int at(int i, int j, int k) const;
};
#endif

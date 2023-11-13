#ifndef ELLPACK_HPP
#define ELLPACK_HPP

//
// Copyright (C) 2016 SAM (D-MATH) @ ETH Zurich
// Author(s): lfilippo <filippo.leonardi@sam.math.ethz.ch>
// Contributors: tille, jgacon, dcasati
// This file is part of the NumCSE repository.
//

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using Triplet = Eigen::Triplet<double>;
using Triplets = std::vector<Triplet>;
using Vector = Eigen::VectorXd;
using index_t = std::ptrdiff_t;

/**
 * @brief Class representing a sparse matrix in Ellpack format
 *
 */
/* SAM_LISTING_BEGIN_0 */
class EllpackMat {
 public:
  EllpackMat(const Triplets &triplets, index_t m, index_t n);
  double operator()(index_t i, index_t j) const;
  void mvmult(const Vector &x, Vector &y) const;
  void mtvmult(const Vector &x, Vector &y) const;
  index_t get_maxcols() const;

 private:
  std::vector<double> val;  //< Vector containing values
  // corresponding to entries in 'col'
  std::vector<index_t> col;  //< Vector containing column
  // indices of the entries in 'val'.
  // The position of a cell in 'val' and 'col'
  // is determined by its row number and original position in 'triplets'
  index_t maxcols;  //< Number of non-empty columns
  index_t m, n;     //< Number of rows, number of columns
};
/* SAM_LISTING_END_0 */

index_t EllpackMat::get_maxcols() const { return maxcols; }

/**
 * @brief Retrieve value of cell $(i,j)$
 *
 * @param i Row index
 * @param j Column index
 * @return double  Value of cell $(i,j)$
 */
/* SAM_LISTING_BEGIN_1 */
double EllpackMat::operator()(index_t i, index_t j) const {
  assert(0 <= i && i < m && 0 <= j && j < n && "Index out of bounds!");

  for (index_t l = i * maxcols; l < (i + 1) * maxcols; ++l) {
    if (col.at(l) == j) return val.at(l);
  }
  return 0;
}
/* SAM_LISTING_END_1 */

/**
 * @brief Constructor of class EllpackMat from vector of triplets
 *
 * @param triplets Vector of Eigen triplets
 * @param m Number of rows of matrix $A$
 * @param n Number of columns of matrix $A$
 */
/* SAM_LISTING_BEGIN_2 */
EllpackMat::EllpackMat(const Triplets &triplets, index_t m, index_t n)
    : m(m), n(n) {
  // TODO: (2-15.a) implement the constructor for the class EllpackMat
  // START

  // Find maxcols, number of non-empty columns
  // It is assumed that all triplets uniquely define one entry
  std::vector<unsigned int> counters(m);
  // Fill counters with 0
  std::fill(counters.begin(), counters.end(), 0);

  // counters especially for handling repeated index
  std::vector<std::vector<unsigned int>> counters_col(m);

  // 'maxcols' starts with no entry for each row
  maxcols = 0;
  // Loop over each triplet and increment the corresponding counter, updating
  // maxcols if necessary
  for (const Triplet &tr : triplets) {
    std::vector<unsigned int> &col_idx = counters_col[tr.row()];
    // if it's not a repeated index, then check if maxcols needs to be updated
    if (std::find(col_idx.begin(), col_idx.end(), tr.col()) == col_idx.end()) {
      // bookkeep the col of new(not repeated) triplet
      col_idx.push_back(tr.col());
      if (++counters[tr.row()] > maxcols) {
        maxcols = counters[tr.row()];
      }
    }
  }
  std::cout << "Maxcols: " << maxcols << std::endl;

  // Reserve space
  col.resize(m * maxcols, -1);
  val.resize(m * maxcols, 0);

  // Loop over each triplet and find a column where to put the value
  for (const Triplet &tr : triplets) {
    assert(0 <= tr.row() && tr.row() < m && 0 <= tr.col() && tr.col() < n &&
           "Index out of bounds!");

    index_t l;
    for (l = tr.row() * maxcols; l < (tr.row() + 1) * maxcols; ++l) {
      // Store the current triplet in the first
      // empty position between 'tr.row()*maxcols'
      // and '(tr.row()+1)*maxcols - 1'
      if (col[l] == -1) {
        col[l] = tr.col();
        val[l] = tr.value();
        break;
      }

      // if it's a repeated index, just sum up
      if (col[l] == tr.col()) {
        val[l] += tr.value();
        break;
      }
    }
    assert(l < (tr.row() + 1) * maxcols &&
           "You did not reserve enough columns!");
  }
  // END
}
/* SAM_LISTING_END_2 */

/**
 * @brief Compute $y = Ax$ exploiting the Ellpack format
 *
 * @param x Vector of matrix-vector multiplication
 * @param y Vector from $y = Ax$
 */
/* SAM_LISTING_BEGIN_3 */
void EllpackMat::mvmult(const Vector &x, Vector &y) const {
  assert(x.size() == n && "Incompatible vector x size!");
  assert(y.size() == m && "Incompatible vector y size!");
  // TODO: (2-15.b) implement the multiplication $\cob{A*x}$ using the
  // class EllpackMat, with optimal complexity.

  // START
  for (index_t i = 0; i < m; ++i) {
    y(i) = 0;
    for (index_t l = i * maxcols; l < (i + 1) * maxcols; ++l) {
      if (col[l] == -1) break;
      y(i) += x(col[l]) * val[l];
    }
  }
  // END
}
/* SAM_LISTING_END_3 */

/**
 * @brief Compute $y = A_t x$ exploiting the Ellpack format, where $A$ is
 * transposed
 *
 * @param x Vector of matrix-vector multiplication
 * @param y Vector from $y = Ax$
 */
/* SAM_LISTING_BEGIN_4 */
void EllpackMat::mtvmult(const Vector &x, Vector &y) const {
  assert(x.size() == m && "Incompatible vector x size!");
  assert(y.size() == n && "Incompatible vector y size!");
  // TODO: (2-15.c) implement the multiplication $\cob{A^{\top}*x}$ using the
  // class EllpackMat, with optimal complexity.

  // START
  y = Vector::Zero(n);
  for (index_t i = 0; i < m; ++i) {
    for (index_t l = i * maxcols; l < (i + 1) * maxcols; ++l) {
      if (col[l] == -1) break;
      y(col[l]) += x(i) * val[l];
    }
  }
  // END
}
/* SAM_LISTING_END_4 */

#endif

//
// Copyright (C) 2016 SAM (D-MATH) @ ETH Zurich
// Author(s): lfilippo <filippo.leonardi@sam.math.ethz.ch>
// Contributors: tille, jgacon, dcasati
// This file is part of the NumCSE repository.
//

#include <Eigen/Core>
#include <Eigen/SparseCore>
#include <iomanip>
#include <iostream>

using Triplet = Eigen::Triplet<double>;
using Triplets = std::vector<Triplet>;
using Vector = Eigen::VectorXd;
using index_t = std::ptrdiff_t;

#include "ellpack.hpp"

/* SAM_LISTING_BEGIN_5 */
int main() {
  // Vector of triplets
  Triplets triplets;

  // Data
  constexpr unsigned int m = 3, n = 6;
  constexpr unsigned int ntriplets = 9;

  // Reserve space for triplets
  triplets.reserve(ntriplets);

  // Fill in some triplets
  triplets.push_back(Triplet(1, 2, 4));
  triplets.push_back(Triplet(0, 0, 5));
  triplets.push_back(Triplet(1, 2, 6));  // repeated index
  triplets.push_back(Triplet(2, 5, 7));
  triplets.push_back(Triplet(0, 4, 8));
  triplets.push_back(Triplet(0, 0, 1));  // repeated index
  triplets.push_back(Triplet(1, 3, 9));
  triplets.push_back(Triplet(2, 2, 10));
  triplets.push_back(Triplet(1, 3, 2));  // repeated index
  triplets.push_back(Triplet(2, 1, 11));
  triplets.push_back(Triplet(1, 0, 12));

  // Build Ellpack matrix
  EllpackMat E(triplets, m, n);
  Eigen::SparseMatrix<double> A(m, n);
  A.setFromTriplets(triplets.begin(), triplets.end());

  std::cout << " ------------- Test: print  E ---------------- " << std::endl;
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      std::cout << std::setw(4) << E(i, j) << " ";
    }
    std::cout << std::endl;
  }

  std::cout << " ------------- Test of y = E*x --------------- " << std::endl;
  Vector x(n);
  x << 4, 5, 6, 7, 8, 9;

  Vector Ex = Vector::Zero(m);
  E.mvmult(x, Ex);
  std::cout << "Ellpack E*x =" << std::endl << Ex << std::endl;
  std::cout << "L^2 norm of difference to Eigen sparse multiplication ||E*x - "
               "A*x|| = "
            << std::endl
            << (Ex - A * x).norm() << std::endl;

  std::cout << " ------------- Test of y = A^t*x ------------- " << std::endl;
  x.resize(m);
  x << 1, 2, 3;

  Vector Etx = Vector::Zero(n);
  E.mtvmult(x, Etx);
  std::cout << "Ellpack E^t*x =" << std::endl << Etx << std::endl;
  std::cout
      << "L^2 norm of difference to Eigen sparse multiplication ||E^t*x - "
         "A^t*x|| = "
      << std::endl
      << (Etx - A.transpose() * x).norm() << std::endl;
}
/* SAM_LISTING_END_5 */
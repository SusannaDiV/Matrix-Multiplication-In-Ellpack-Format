
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "copy.hpp"
#include "doctest.h"

// includes for test data
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <random>
#include <vector>

struct TestData {
  TestData() {
    m = 70;
    n = 100;

    // repeated ratio
    r = 0.1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> row(0, m - 1);
    std::uniform_int_distribution<> col(0, n - 1);
    std::uniform_real_distribution<> val(0., 100.);

    // fill 5 percent with non-zero entries
    const std::size_t ntriplets = 0.05 * (m * n);
    // reserve for normal random data(ntriplets) and artificially repeated
    // data(r * ntriplets)
    triplets.reserve((1 + r) * ntriplets);
    std::uniform_int_distribution<> repeated_idx(0, ntriplets - 1);

    // fill with random values in random rows and columns
    for (std::size_t i = 0; i < ntriplets; ++i) {
      triplets.push_back(Eigen::Triplet<double>(row(gen), col(gen), val(gen)));
    }

    // make repeated index
    for (std::size_t i = 0; i < r * ntriplets; ++i) {
      int idx{repeated_idx(gen)};
      int row{triplets[idx].row()}, col{triplets[idx].col()};

      triplets.push_back(Eigen::Triplet<double>(row, col, val(gen)));
    }

    // shuffle triplets
    auto rng = std::default_random_engine{};
    std::shuffle(std::begin(triplets), std::end(triplets), rng);
  }

  std::size_t m, n;
  double r;
  std::vector<Eigen::Triplet<double> > triplets;
};

TestData data;

TEST_SUITE("Ellpack") {
  TEST_CASE("EllpackMat [OUT OF CLASS]" * doctest::description("constructor")) {
    EllpackMat sol(data.triplets, data.m, data.n);
    EllpackMat_TEST stud(data.triplets, data.m, data.n);

    for (std::size_t i = 0; i < data.m; ++i) {
      for (std::size_t j = 0; j < data.n; ++j) {
        REQUIRE(sol(i, j) == doctest::Approx(stud(i, j)).epsilon(1e-6));
      }
    }
  }

  TEST_CASE("index_t get_maxcols [OUT OF CLASS]" *
            doctest::description("get_maxcols")) {
    EllpackMat sol(data.triplets, data.m, data.n);
    EllpackMat_TEST stud(data.triplets, data.m, data.n);

    REQUIRE(sol.get_maxcols() == stud.get_maxcols());
  }

  TEST_CASE("void mvmult [OUT OF CLASS]" * doctest::description("mvmult")) {
    EllpackMat sol(data.triplets, data.m, data.n);
    EllpackMat_TEST stud(data.triplets, data.m, data.n);

    Eigen::VectorXd x = Eigen::VectorXd::Random(data.n);

    Eigen::VectorXd sol_vec = Eigen::VectorXd::Zero(data.m);
    Eigen::VectorXd stud_vec = Eigen::VectorXd::Zero(data.m);

    sol.mvmult(x, sol_vec);
    stud.mvmult(x, stud_vec);

    REQUIRE(sol_vec.size() == stud_vec.size());
    CHECK((stud_vec - sol_vec).norm() == doctest::Approx(0.).epsilon(1e-6));
  }

  TEST_CASE("void mtvmult [OUT OF CLASS]" * doctest::description("mtvmult")) {
    EllpackMat sol(data.triplets, data.m, data.n);
    EllpackMat_TEST stud(data.triplets, data.m, data.n);

    Eigen::VectorXd x = Eigen::VectorXd::Random(data.m);

    Eigen::VectorXd sol_vec = Eigen::VectorXd::Zero(data.n);
    Eigen::VectorXd stud_vec = Eigen::VectorXd::Zero(data.n);

    sol.mtvmult(x, sol_vec);
    stud.mtvmult(x, stud_vec);

    REQUIRE(sol_vec.size() == stud_vec.size());
    CHECK((stud_vec - sol_vec).norm() == doctest::Approx(0.).epsilon(1e-6));
  }
}

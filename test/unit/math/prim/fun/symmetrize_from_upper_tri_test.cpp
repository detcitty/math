#include <stan/math/prim.hpp>
#include <gtest/gtest.h>

TEST(Symmetrize_upper, Error) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using stan::math::symmetrize_from_upper_tri;

  Matrix<double, Dynamic, Dynamic> m(3, 4);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 4; ++j)
      m(i, j) = (i + 2) * (j + 1);
  EXPECT_THROW(symmetrize_from_upper_tri(m), std::invalid_argument);
}

TEST(Symmetrize_upper, Value) {
  using Eigen::Dynamic;
  using Eigen::Matrix;
  using stan::math::is_symmetric;
  using stan::math::symmetrize_from_upper_tri;

  Matrix<double, Dynamic, Dynamic> m(4, 4);
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      m(i, j) = (i + 2) * (j + 1);
  EXPECT_TRUE(is_symmetric(symmetrize_from_upper_tri(m)));
}

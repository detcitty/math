#ifdef STAN_OPENCL
#include <stan/math/opencl/rev.hpp>
#include <test/unit/math/opencl/util.hpp>
#include <test/unit/util.hpp>
#include <stan/math/prim/fun/typedefs.hpp>
#include <gtest/gtest.h>
#include <vector>

TEST(MathMatrixRevCL, row_exception) {
  stan::math::matrix_cl<double> m1_cl(3, 3);
  EXPECT_THROW(row(m1_cl, 0), std::invalid_argument);
  EXPECT_THROW(row(m1_cl, 4), std::invalid_argument);

  stan::math::var_value<stan::math::matrix_cl<double>> m2_cl = m1_cl;
  EXPECT_THROW(row(m2_cl, 0), std::invalid_argument);
  EXPECT_THROW(row(m2_cl, 4), std::invalid_argument);
}

auto row_functor = [](const auto& a) { return stan::math::row(a, 2); };

TEST(MathMatrixRevCL, row_value_check) {
  stan::math::matrix_d m1(3, 3);
  m1 << 1, 2, 3, 4, 5, 6, 7, 8, 9;

  stan::math::test::compare_cpu_opencl_prim(row_functor, m1);

  stan::math::matrix_v m2 = m1;
  stan::math::matrix_v m3 = m1;
  stan::math::var_value<stan::math::matrix_cl<double>> m3_cl
      = stan::math::to_matrix_cl(m3);
  stan::math::matrix_v m2_res = stan::math::row(m2, 3);
  stan::math::var_value<stan::math::matrix_cl<double>> m3_res_cl
      = stan::math::row(m3_cl, 3);
  stan::math::var s = sum(m2_res) + sum(m3_res_cl);
  s.grad();

  EXPECT_MATRIX_NEAR(m2_res.val(), stan::math::from_matrix_cl(m3_res_cl.val()),
                     1E-8);
  EXPECT_MATRIX_NEAR(m2.adj(), m3.adj(), 1E-8);
}

#endif

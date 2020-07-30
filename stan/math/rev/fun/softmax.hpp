#ifndef STAN_MATH_REV_FUN_SOFTMAX_HPP
#define STAN_MATH_REV_FUN_SOFTMAX_HPP

#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/functor/adj_jac_apply.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/typedefs.hpp>
#include <stan/math/prim/fun/softmax.hpp>
#include <tuple>
#include <vector>

namespace stan {
namespace math {

namespace internal {

template <typename T>
class softmax_op {
  adj_op<T> y_;
 public:
  softmax_op(const T& x) : y_(x.size()) {}

  /**
   * Compute the softmax of the unconstrained input vector
   *
   * @param alpha Unconstrained input vector.
   * @return Softmax of the input.
   */
  template <typename S>
  auto& operator()(S&& alpha) {
    y_.map() = softmax(std::forward<S>(alpha));
    return y_.map();
  }

  /**
   * Compute the result of multiply the transpose of the adjoint vector times
   * the Jacobian of the softmax operator. It is more efficient to do this
   * without actually computing the Jacobian and doing the vector-matrix
   * product.
   *
   * @param adj Eigen::VectorXd of adjoints at the output of the softmax
   * @return Eigen::VectorXd of adjoints propagated through softmax operation
   */
   auto multiply_adjoint_jacobian(const Eigen::VectorXd& adj) const {
     vector_d adj_times_jac = -y_.map() * adj.dot(y_.map()) + y_.map().cwiseProduct(adj);
     return std::make_tuple(adj_times_jac);
   }

};
}  // namespace internal

/**
 * Return the softmax of the specified Eigen vector.  Softmax is
 * guaranteed to return a simplex.
 *
 * @param alpha Unconstrained input vector.
 * @return Softmax of the input.
 * @throw std::domain_error If the input vector is size 0.
 */
template <typename T, require_eigen_vt<is_var, T>* = nullptr>
inline auto softmax(const T& alpha) {
  check_nonzero_size("softmax", "alpha", alpha);
  return adj_jac_apply<internal::softmax_op<T>>(alpha);
}

}  // namespace math
}  // namespace stan
#endif

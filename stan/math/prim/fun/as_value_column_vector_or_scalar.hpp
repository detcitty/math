#ifndef STAN_MATH_PRIM_FUN_AS_VALUE_COLUMN_VECTOR_OR_SCALAR
#define STAN_MATH_PRIM_FUN_AS_VALUE_COLUMN_VECTOR_OR_SCALAR

#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/fun/value_of.hpp>
#include <vector>

namespace stan {
namespace math {

/**
 * Extract values from input argument and transform to a column vector or a scalar.
 * For arithmetic types this is an identity function.
 *
 * @tparam T Type of scalar element.
 * @param a Specified scalar.
 * @return the scalar.
 */
template <typename T, require_stan_scalar_t<T>* = nullptr>
inline auto as_value_column_vector_or_scalar(const T& a) {
  return value_of(a);
}

/**
 * Extract values from input argument and transform to a column vector or a scalar.
 * For arithmetic column vector inputs this is an identity function.
 *
 * @tparam T Type of scalar element.
 * @param a Specified vector.
 * @return Same vector.
 */
template <typename T, require_col_vector_t<T>* = nullptr>
inline auto as_value_column_vector_or_scalar(T&& a) {
  return value_of(std::forward<T>(a));
}

/**
 * Extract values from input argument and transform to a column vector or a scalar.
 * For arithmetic row vector inputs this is transpose.
 *
 * @tparam T Type of scalar element.
 * @param a Specified vector.
 * @return Transposed vector.
 */
template <typename T, require_row_vector_t<T>* = nullptr,
          require_not_col_vector_t<T>* = nullptr>
inline auto as_value_column_vector_or_scalar(T&& a) {
  return value_of(a.transpose());
}

/**
 * Extract values from input argument and transform to a column vector or a scalar.
 * For `std::vector<double>` this will be converted to a column vector.
 *
 * @tparam T Type of scalar element.
 * @param a Specified vector.
 * @return input converted to a column vector.
 */
template <typename T, require_std_vector_t<T>* = nullptr>
inline auto as_value_column_vector_or_scalar(T&& a) {
  using plain_vector = Eigen::Matrix<value_type_t<T>, Eigen::Dynamic, 1>;
  using optionally_const_vector
      = std::conditional_t<std::is_const<std::remove_reference_t<T>>::value,
                           const plain_vector, plain_vector>;
  using T_map = Eigen::Map<optionally_const_vector>;
  return value_of(T_map(a.data(), a.size()));
}

}  // namespace math
}  // namespace stan

#endif

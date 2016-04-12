/* Created on: Apr 12, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# ifndef MEDIAN_PATH_REGULARIZATION_H_
# define MEDIAN_PATH_REGULARIZATION_H_
# include "median_skeleton.h"

BEGIN_MP_NAMESPACE
class regularizer {
public:

  struct parameters {
    parameters();
    typedef enum {
      SCALE_TRANSFORM
    } regularization_method;
    regularization_method m_regularization_method;
    real m_scale_factor;
  };

  regularizer(
      median_skeleton& skeleton,
      const parameters& params = parameters() );

  const real& get_execution_time() const noexcept;
private:
  real m_execution_time;
};
END_MP_NAMESPACE
# endif 

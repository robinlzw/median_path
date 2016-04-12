/* Created on: Apr 12, 2016
 *     Author: T.Delame (tdelame@gmail.com)
 */
# include "../median-path/regularization.h"
# include <graphics-origin/tools/log.h>
# include <omp.h>
BEGIN_MP_NAMESPACE

  void scale_regularizer(
    median_skeleton& skeleton,
    real scale );

  regularizer::parameters::parameters()
    : m_regularization_method{ SCALE_TRANSFORM },
      m_scale_factor{ 1.1 }
  {}

  regularizer::regularizer(
      median_skeleton& skeleton,
      const parameters& params )
    : m_execution_time{ omp_get_wtime() }
  {
    switch( params.m_regularization_method )
    {
      case parameters::SCALE_TRANSFORM:
        scale_regularizer( skeleton, params.m_scale_factor );
        break;
      default:
        LOG( error, "only scale transform regularization is available for the moment");
    }
    m_execution_time = omp_get_wtime() - m_execution_time;
  }

  const real&
  regularizer::get_execution_time() const noexcept
  {
    return m_execution_time;
  }

END_MP_NAMESPACE

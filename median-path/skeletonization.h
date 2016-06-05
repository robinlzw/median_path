/*  Created on: Mar 14, 2016
 *      Author: T. Delame (tdelame@gmail.com)
 */

# ifndef MEDIAN_PATH_SKELETONIZATION_H_
# define MEDIAN_PATH_SKELETONIZATION_H_

# include "structuration.h"
# include <graphics-origin/geometry/mesh.h>

BEGIN_MP_NAMESPACE

  /**@brief A class to skeletonize a shape.
   *
   * A skeleton is a shape representation model, just like boundary representation
   * models (e.g. meshes) or volume representation models (e.g. implicit surfaces).
   * However, shapes are rarely available in a skeleton representation.
   *
   * Skeletonization is the process that convert a shape known by a boundary or
   * volume representation model into a skeleton. This class performs a (small)
   * subset of the tremendous skeletonization available.
   *
   * In practice, a skeletonization is often decomposed into three steps:
   * - geometry step, to sample medial atoms in order to capture the geometry
   * of the shape
   * - topology step, to sample the skeletal structure by connecting center
   * of medial atoms with links and faces
   * - regularization step, to simplify/clean a skeleton.
   *
   * For each step, there are many existing methods. Not all combination of
   * methods are compatible. This class performs the geometry step, then
   * calls \ref structurer to perform the topology step. Depending on the
   * specificities of the geometry and topology steps, it calls the
   * \ref regularizer before or after the topology step.
   *
   * Currently, the following geometry methods are implemented:
   * - Voronoi balls, where a Voronoi diagram of a surface sampling of the
   * shape is computed. All inside Voronoi vertices are used to build
   * atoms (the radius is the distance between the vertex and the closest
   * surface sample)
   * - Polar balls, where only a subset of inside Voronoi vertices, the
   * inside poles, are kept. An inside pole is the inside Voronoi vertex
   * of the Voronoi cell of a sample s, that is the most distant of s.
   * - Shrinking balls, currently the fastest method. For each surface sample,
   * this method starts with a large tangent ball and shrink it iteratively
   * until it is maximally inscribed.
   *
   * \note The skeletonizer only deals with triangular mesh as input. Indeed,
   * it considers all mesh vertices as shape samples for the geometry methods.
   * If you have a shape represented by another shape representation model, it
   * is up to you to make the conversion. Keep in mind that the mesh vertices
   * need to be quite close to each other. To give you an idea, the sampling
   * should be made such that any point p of the input shape is at most at a
   * distance of 0.2 * local_thickness(p) of any sample.
   */
  class skeletonizer {
  public:

    /**@brief Parameters for the shrinking ball geometry step.
     *
     * Compared to Voronoi and Polar balls methods, the shrinking balls
     * method required additional parameters to iteratively shrink a ball.
     *
     */
    struct shrinking_ball_parameters {
      shrinking_ball_parameters();

      /**@brief How to determine the initial radius.
       *
       * Currently, there is two methods to determine the initial radius of
       * shrinking balls:
       * - cast a ray from a vertex in the opposite direction of the normal
       * and use the distance to the intersection as the initial value
       * - use a constant value expressed relatively to the smallest dimension
       * of the bounding box.
       */
      typedef enum {
        RAYTRACING, /*!< Cast a ray in the opposite direction of the normal */
        CONSTANT    /*!< Use always the same initial value.*/
      } initial_radius_method;

      initial_radius_method m_radius_method;

      /**@brief Ratio of the bounding box used to compute the constant initial radius.
       *
       * This value stores the ratio of the smallest dimension of the bounding box
       * to use as a constant initial radius. */
      real m_constant_radius_ratio;
      /**@brief Radius variation threshold.
       *
       * When a shrinking ball radius does not change more than this threshold,
       * the ball stop to shrinks. This value is used to avoid infinite loop due
       * to the numerical precision. */
      real m_min_radius_variation;
    };

    /**
     *
     * Not all combination of parameters are possible.
     *
     * Relation between geometry and topology method:
     * - any geometry method can be followed regular triangulation or Delaunay
     * reconstruction topology methods
     * - the Voronoi method requires the medial balls to be centered on Voronoi
     * vertices. As such, it can only happen after a Voronoi balls geometry method.
     * - the powershape method requires to know both the inside and the outside
     * medial balls. Thus it can only be applied after a Voronoi balls method or
     * a polar balls method since both methods compute inside and outside balls.
     */
    struct parameters {
      parameters();

      typedef enum {
        SHRINKING_BALLS,
        POLAR_BALLS,
        VORONOI_BALLS
      } geometry_method;

      geometry_method m_geometry_method;
      real m_cluster_volume_factor;
      median_skeleton::atom_index  m_neighbors_for_cluster_detection;
      bool m_merge_clusters;

      union {
        shrinking_ball_parameters m_shrinking_ball;
      };

      bool m_build_topology;
      structurer::parameters m_structurer_parameters;
    };

    skeletonizer(
        graphics_origin::geometry::mesh& input,
        median_skeleton& output,
        const parameters& params = parameters() );

    real get_execution_time() const noexcept;

  private:
    real m_execution_time;
  };

END_MP_NAMESPACE

# endif

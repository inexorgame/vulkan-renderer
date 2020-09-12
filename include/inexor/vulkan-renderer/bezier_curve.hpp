#pragma once

// TODO: Implement a curve manager.

/// INTRODUCTION
/// A bezier curve (named after french mathematician PIERRE ETIENNE BEZIER) is a parametric curve
/// whose only purpose is to look soft and smooth. Bezier curves are all about elegance!
/// Those curves can be used to represent the path of a everything (imagin a camera which is moving along a path for
/// example).
///
/// Bezier curves are fast, flexible, beautiful and easy to compute. You just pass a bunch of parameter points to
/// your code and the final curve will be computed. Because every complex curve can be represented with a
/// chain of smaller curves, it is recommended to create a chain of curves. Bezier curves are essential
/// in the field of computer graphics and image processing. They can also be used for approximation, interpolation and
/// more.
///
/// COMPUTING
/// There are two ways to generate a bezier curves from a group of [n] points.
/// You can either write a code that uses recursion to solve the problem or use Bernstein polynomials.
/// This engine uses Bernstein polynomial, because we want to avoid the recursion in de-casteljau algorithm.
///
/// Pierre Etienne BEZIER       (September 1, 1910 - November 25, 1999), French mathematician and engineer at RENAULT.
/// Paul de CASTELJAU           (November 19, 1930), French mathematician and physicist and engineer ar Citroen.
/// Sergei Natanovich BERNSTEIN (March 5, 1880 - October 26, 1968), Russian mathematician.
/// Charles HERMITE             (December 24, 1822 - January 14, 1901), French mathematician.

/// http://pomax.github.io/bezierinfo/
/// http://en.wikipedia.org/wiki/B%C3%A9zier_curve
/// http://mathworld.wolfram.com/BezierCurve.html
/// http://theagsc.com/community/tutorials/so-whats-the-big-deal-with-horizontal-vertical-bezier-handles-anyway#comment-1351842776
/// http://learn.scannerlicker.net/2014/04/16/bezier-curves-and-type-design-a-tutorial/
/// https://geom.ivd.kit.edu/downloads/pubs/pub-boehm-prautzsch_2002_preview.pdf
/// https://www.clear.rice.edu/comp360/lectures/BezSubd.pdf

#include <glm/glm.hpp>

#include <cmath>
#include <vector>

namespace inexor::vulkan_renderer {

/// @brief Those are the points that we pass into the bezier curve generator.
/// Every bezier curve will be generated from a list of BezierInputPoint.
/// Every input point can have a custom weight coefficient.
struct BezierInputPoint {
    glm::vec3 pos{0.0f, 0.0f, 0.0f};

    float weight{1.0f};
};

/// @brief Those are the points which will be generated from the bezier curve generator.
/// How many BezierOutputPoint points will be generated depends on the required precision.
/// The higher the requested precision, the more points will be
struct BezierOutputPoint : public BezierInputPoint {
    glm::vec3 normal{0.0f, 0.0f, 0.0f};

    glm::vec3 tangent{0.0f, 0.0f, 0.0f};
};

/// @brief This struct bundles describes everything about the bezier curve.
/// It contains both the input points and the generated output points.
class BezierCurve {
    bool m_curve_generated;
    float m_curve_precision;

    std::vector<BezierInputPoint> m_input_points;
    std::vector<BezierOutputPoint> m_output_points;

    /// @brief
    /// @param n
    /// @param k
    /// @return
    uint32_t binomial_coefficient(uint32_t n, const uint32_t k);

    /// @brief
    /// @param n
    /// @param k
    /// @param curve_precision
    /// @param coordinate_value
    /// @return
    float bernstein_polynomial(uint32_t n, uint32_t k, const float curve_precision, const float coordinate_value);

    /// @brief
    /// @param curve_precision
    /// @return
    BezierOutputPoint calculate_point_on_curve(const float curve_precision);

public:
    BezierCurve() = default;

    ~BezierCurve() = default;

    /// @brief
    /// @param input_point
    void add_input_point(const BezierInputPoint &input_point);

    /// @brief
    /// @param position
    /// @param weight
    void add_input_point(const glm::vec3 &position, const float weight = 1.0f);

    /// @brief
    /// @param curve_precision
    void calculate_bezier_curve(const float curve_precision);

    [[nodiscard]] std::vector<BezierOutputPoint> output_points();

    void clear_output();

    void clear_input();

    void clear();

    [[nodiscard]] bool is_curve_generated();
};

} // namespace inexor::vulkan_renderer

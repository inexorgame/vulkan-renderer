#include "inexor/vulkan-renderer/bezier_curve.hpp"

namespace inexor::vulkan_renderer {

std::uint32_t BezierCurve::binomial_coefficient(std::uint32_t n, const std::uint32_t k) {
    std::uint32_t r = 1;
    if (k > n)
        return 0;

    for (std::uint32_t d = 1; d <= k; d++) {
        r *= n--;
        r /= d;
    }

    return r;
}

float BezierCurve::bernstein_polynomial(std::uint32_t n, std::uint32_t k, const float curve_precision,
                                        const float coordinate_value) {
    return binomial_coefficient(n, k) * static_cast<float>(pow(curve_precision, k)) *
           static_cast<float>(pow(1 - curve_precision, n - k)) * coordinate_value;
}

void BezierCurve::clear_output() {
    std::vector<BezierOutputPoint> output_points;
}

void BezierCurve::clear_input() {
    std::vector<BezierInputPoint> input_points;
}

void BezierCurve::clear() {
    clear_input();
    clear_output();
}

bool BezierCurve::is_curve_generated() {
    return m_curve_generated;
}

std::vector<BezierOutputPoint> BezierCurve::output_points() {
    assert(m_curve_generated);
    assert(m_output_points.size() > 0);
    return m_output_points;
}

void BezierCurve::add_input_point(const BezierInputPoint &input_point) {
    assert(!m_curve_generated);
    m_input_points.push_back(input_point);
}

void BezierCurve::add_input_point(const glm::vec3 &position, const float weight) {
    assert(!m_curve_generated);
    assert(weight > 0.0f);

    BezierInputPoint input_point;
    input_point.pos = position;
    input_point.weight = weight;

    m_input_points.push_back(input_point);
}

BezierOutputPoint BezierCurve::calculate_point_on_curve(const float curve_precision) {
    BezierOutputPoint temp_output;

    const std::uint32_t n = static_cast<std::uint32_t>(m_input_points.size() - 1);

    // Calculate the coordinates of the output points of the bezier curve.
    for (std::size_t i = 0; i < m_input_points.size(); i++) {
        auto current_point = m_input_points[i];

        const std::uint32_t index = static_cast<std::uint32_t>(i);

        // Compute bezier curve coordinates using bernstein polynomials.
        temp_output.pos.x += bernstein_polynomial(n, index, curve_precision, current_point.pos.x);
        temp_output.pos.y += bernstein_polynomial(n, index, curve_precision, current_point.pos.y);
        temp_output.pos.z += bernstein_polynomial(n, index, curve_precision, current_point.pos.z);
    }

    // Calculate the derivatives of bezier curves.
    // https://www.rose-hulman.edu/~finn/CCLI/Notes/day13.pdf

    // An easier way would be to take the vector to the next point on the curve and calculate the difference.
    // This would lead to a direction vector with satisfying precision. But this would implicate that the
    // precision of the derivation depends on the curve precision!
    // With this technique, we can have precise derivations even if we have a precision of only 10.0f units!
    for (std::size_t i = 0; i < n; i++) {
        auto current_point = m_input_points[i];
        auto next_point = m_input_points[i + 1];

        const std::uint32_t index = static_cast<std::uint32_t>(i);

        // Compute bezier curve point's first derivative.
        temp_output.tangent.x += bernstein_polynomial(n - 1, index, curve_precision, current_point.pos.x);
        temp_output.tangent.y += bernstein_polynomial(n - 1, index, curve_precision, current_point.pos.y);
        temp_output.tangent.z += bernstein_polynomial(n - 1, index, curve_precision, current_point.pos.z);
    }

    // Subtract point position from tangent vector so the tangent vector is relative.
    temp_output.tangent -= temp_output.pos;

    // Calculate a relative normal vector.
    // Please note that there is an infinite amount of normal vectors from vector a to b.

    const float length = glm::length(temp_output.tangent);
    temp_output.normal = glm::vec3(-temp_output.tangent.y / length, temp_output.tangent.x / length, 0);

    // Do not normalize tangent vectors before you have copied the normal vector! They will be incorrect!

    // Normalize the vectors: divide them by length so their length is 1.
    // You could rescale the vectors to change the length to your wishes.
    temp_output.normal = glm::normalize(temp_output.normal);
    temp_output.tangent = glm::normalize(temp_output.tangent);

    // Store the output.
    return temp_output;
}

void BezierCurve::calculate_bezier_curve(const float curve_precision) {
    // We need at least 2 input points!
    assert(m_input_points.size() > 2);

    if (m_curve_generated) {
        clear();
        m_curve_generated = false;
    }

    const float curve_precision_interval = 1.0f / curve_precision;

    for (float position_on_curve = 0.0f; position_on_curve <= 1.0f; position_on_curve += curve_precision_interval) {
        const BezierOutputPoint temp_output = calculate_point_on_curve(position_on_curve);
        m_output_points.push_back(temp_output);
    }

    m_curve_generated = true;
}

} // namespace inexor::vulkan_renderer

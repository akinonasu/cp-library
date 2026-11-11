#include "fps/fps_light.hpp"
#include "fps/fps_light_algorithms.hpp"
#include "fps/compose_light.hpp"
#include "fps/compositional_inverse_light.hpp"
#include "fps/polynomial_gcd_light.hpp"
#include "fps/power_projection_light.hpp"
#include "fps/rational_fps_coefficients_light.hpp"
#include "fps/fps.hpp"
#include "fps/taylor_shift.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/polynomial_gcd.hpp"
#include "fps/power_projection.hpp"
#include "fps/rational_fps_coefficients.hpp"
#include "test/fixtures/fps_public_api.hpp"

static_assert(std::is_same_v<fps, light_fps>);
static_assert(std::is_same_v<FormalPowerSeries<mint>, LightFormalPowerSeries<mint>>);

int main() { fps_public_api_test::check(); }

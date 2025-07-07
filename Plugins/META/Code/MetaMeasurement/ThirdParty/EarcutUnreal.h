#pragma once

// https://github.com/mapbox/earcut
#include "earcut.hpp"

#include "Math/MathFwd.h"

namespace mapbox
{

        namespace util
        {

                template <>
                struct nth<0, FVector2f>
                {
                        static constexpr auto get(const FVector2f& t) noexcept { return t.X; }
                };

                //
                template <>
                struct nth<1, FVector2f>
                {
                        static constexpr auto get(const FVector2f& t) noexcept { return t.Y; }
                };

        } // namespace util

} // namespace mapbox

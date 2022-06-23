/*
 * Copyright (C) 2021 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2021 Vladimir Sadovnikov <sadko4u@gmail.com>
 *
 * This file is part of lsp-plugins-art-delay
 * Created on: 3 авг. 2021 г.
 *
 * lsp-plugins-art-delay is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * lsp-plugins-art-delay is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with lsp-plugins-art-delay. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PRIVATE_META_ART_DELAY_H_
#define PRIVATE_META_ART_DELAY_H_

#include <lsp-plug.in/plug-fw/meta/types.h>
#include <lsp-plug.in/plug-fw/const.h>


namespace lsp
{
    namespace meta
    {
        struct art_delay_metadata
        {
            static constexpr float  TIME_MIN            = 0.0f;
            static constexpr float  TIME_MAX            = 256.0f;
            static constexpr float  TIME_STEP           = 0.01f;
            static constexpr float  TIME_DFL            = 0.0f;

            static constexpr float  DSEL_MIN            = 0.0f;
            static constexpr float  DSEL_MAX            = 999.999f;
            static constexpr float  DSEL_STEP           = 0.01f;
            static constexpr float  DSEL_DFL            = 0.0f;

            static constexpr float  DENOMINATOR_MIN     = 1.0f;
            static constexpr float  DENOMINATOR_MAX     = 64.0f;
            static constexpr float  DENOMINATOR_STEP    = 1.0f;
            static constexpr float  DENOMINATOR_DFL     = 4.0f;

            static constexpr float  FRACTION_MIN        = 0.0f;
            static constexpr float  FRACTION_MAX        = 2.0f;
            static constexpr float  FRACTION_STEP       = 1.0f / 64.0f;
            static constexpr float  FRACTION_DFL        = 0.0f;

            static constexpr float  DFRACTION_MIN       = 0.0f;
            static constexpr float  DFRACTION_MAX       = 2.0f;
            static constexpr float  DFRACTION_STEP      = 1.0f / 64.0f;
            static constexpr float  DFRACTION_DFL       = 1.0f;

            static constexpr float  BAR_MULT_MIN        = 0.0f;
            static constexpr float  BAR_MULT_MAX        = 1000.0f;
            static constexpr float  BAR_MULT_STEP       = 0.5f;
            static constexpr float  BAR_MULT_DFL        = 1.0f;

            static constexpr float  DELAY_MULT_MIN      = 0.0f;
            static constexpr float  DELAY_MULT_MAX      = 1000.0f;
            static constexpr float  DELAY_MULT_STEP     = 0.1f;
            static constexpr float  DELAY_MULT_DFL      = 1.0f;

            static constexpr float  ATEMPO_MIN          = 0.0f;
            static constexpr float  ATEMPO_MAX          = 9000.0f;
            static constexpr float  ATEMPO_STEP         = 0.1f;
            static constexpr float  ATEMPO_DFL          = 120.0f;

            static constexpr float  TEMPO_MIN           = 20.0f;
            static constexpr float  TEMPO_MAX           = 360.0f;
            static constexpr float  TEMPO_STEP          = 0.05f;
            static constexpr float  TEMPO_DFL           = 120.0f;

            static constexpr float  BAND_GAIN_MIN       = GAIN_AMP_M_24_DB;
            static constexpr float  BAND_GAIN_MAX       = GAIN_AMP_P_24_DB;
            static constexpr float  BAND_GAIN_STEP      = 0.025f;
            static constexpr float  BAND_GAIN_DFL       = GAIN_AMP_0_DB;

            static constexpr float  LOW_CUT_MIN         = SPEC_FREQ_MIN;
            static constexpr float  LOW_CUT_MAX         = 1000.0f;
            static constexpr float  LOW_CUT_STEP        = 0.001f;
            static constexpr float  LOW_CUT_DFL         = 100.0f;

            static constexpr float  HIGH_CUT_MIN        = 1000.0f;
            static constexpr float  HIGH_CUT_MAX        = SPEC_FREQ_MAX;
            static constexpr float  HIGH_CUT_STEP       = 0.001f;
            static constexpr float  HIGH_CUT_DFL        = 8000.0f;

            static constexpr float  MEMORY_MIN          = 0.0f;
            static constexpr float  MEMORY_MAX          = 65536.0f;
            static constexpr float  MEMORY_DFL          = 0.0f;
            static constexpr float  MEMORY_STEP         = 0.01f;

            static const size_t EQ_BANDS            = 5;

            static const size_t MAX_PROCESSORS      = 16;
            static const size_t MAX_TEMPOS          = 8;

            enum op_modes_t
            {
                OP_MODE_NONE,
                OP_MODE_TIME,
                OP_MODE_NOTE,
                OP_MODE_REF
            };
        };

        extern const meta::plugin_t art_delay_mono;
        extern const meta::plugin_t art_delay_stereo;
    } // namespace meta
} // namespace lsp


#endif /* PRIVATE_META_ART_DELAY_H_ */

/*
 * Copyright (C) 2024 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2024 Vladimir Sadovnikov <sadko4u@gmail.com>
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

#include <lsp-plug.in/plug-fw/meta/ports.h>
#include <lsp-plug.in/shared/meta/developers.h>
#include <private/meta/art_delay.h>

#define LSP_PLUGINS_ART_DELAY_VERSION_MAJOR       1
#define LSP_PLUGINS_ART_DELAY_VERSION_MINOR       0
#define LSP_PLUGINS_ART_DELAY_VERSION_MICRO       20

#define LSP_PLUGINS_ART_DELAY_VERSION  \
    LSP_MODULE_VERSION( \
        LSP_PLUGINS_ART_DELAY_VERSION_MAJOR, \
        LSP_PLUGINS_ART_DELAY_VERSION_MINOR, \
        LSP_PLUGINS_ART_DELAY_VERSION_MICRO  \
    )

namespace lsp
{
    namespace meta
    {
        static const port_item_t art_delay_lines[] =
        {
            { "0",            "art_delay.line_0"   },
            { "1",            "art_delay.line_1"   },
            { "2",            "art_delay.line_2"   },
            { "3",            "art_delay.line_3"   },
            { "4",            "art_delay.line_4"   },
            { "5",            "art_delay.line_5"   },
            { "6",            "art_delay.line_6"   },
            { "7",            "art_delay.line_7"   },
            { "8",            "art_delay.line_8"   },
            { "9",            "art_delay.line_9"   },
            { "10",           "art_delay.line_10"  },
            { "11",           "art_delay.line_11"  },
            { "12",           "art_delay.line_12"  },
            { "13",           "art_delay.line_13"  },
            { "14",           "art_delay.line_14"  },
            { "15",           "art_delay.line_15"  },
            { NULL, NULL }
        };

        static port_item_t art_delay_maxlen[] =
        {
            { "1",              NULL    },
            { "2",              NULL    },
            { "4",              NULL    },
            { "8",              NULL    },
            { "16",             NULL    },
            { "24",             NULL    },
            { "32",             NULL    },
            { "40",             NULL    },
            { "48",             NULL    },
            { "56",             NULL    },
            { "64",             NULL    },
            { "96",             NULL    },
            { "128",            NULL    },
            { "160",            NULL    },
            { "192",            NULL    },
            { "224",            NULL    },
            { "256",            NULL    },

            { NULL, NULL }
        };

        static port_item_t art_delay_references[] =
        {
            { "None",           "art_delay.none"    },

            { "0",              "art_delay.line_0"  },
            { "1",              "art_delay.line_1"  },
            { "2",              "art_delay.line_2"  },
            { "3",              "art_delay.line_3"  },
            { "4",              "art_delay.line_4"  },
            { "5",              "art_delay.line_5"  },
            { "6",              "art_delay.line_6"  },
            { "7",              "art_delay.line_7"  },
            { "8",              "art_delay.line_8"  },
            { "9",              "art_delay.line_9"  },
            { "10",             "art_delay.line_10" },
            { "11",             "art_delay.line_11" },
            { "12",             "art_delay.line_12" },
            { "13",             "art_delay.line_13" },
            { "14",             "art_delay.line_14" },
            { "15",             "art_delay.line_15" },

            { NULL, NULL }
        };

        static port_item_t art_delay_tempo[] =
        {
            { "None",           "art_delay.none"    },

            { "0",              "art_delay.tempo_0" },
            { "1",              "art_delay.tempo_1" },
            { "2",              "art_delay.tempo_2" },
            { "3",              "art_delay.tempo_3" },
            { "4",              "art_delay.tempo_4" },
            { "5",              "art_delay.tempo_5" },
            { "6",              "art_delay.tempo_6" },
            { "7",              "art_delay.tempo_7" },

            { NULL, NULL }
        };

        static port_item_t art_delay_tempo_ratio[] =
        {
            { "1:1",            NULL    },
            { "1:2",            NULL    },
            { "1:3",            NULL    },
            { "2:1",            NULL    },
            { "2:3",            NULL    },
            { "3:1",            NULL    },
            { "3:2",            NULL    },

            { NULL, NULL }
        };

        #define ART_PAN_MONO(id, label) \
            PAN_CTL("p" id, label " panorama", 0.0f)

        #define ART_PAN_STEREO(id, label) \
            PAN_CTL("pl" id, label " left channel panorama", -100.0f), \
            PAN_CTL("pr" id, label " right channel panorama", 100.0f)

        #define ART_DELAY_COMMON(pan)  \
            BYPASS, \
            COMBO("lsel", "Delay line selector", 0, art_delay_lines), \
            COMBO("dmax", "Maximum possible delay selector", 3, art_delay_maxlen), \
            pan("_in", "Input"), \
            DRY_GAIN(GAIN_AMP_0_DB), \
            WET_GAIN(GAIN_AMP_0_DB), \
            DRYWET(100.0f), \
            SWITCH("dry_on", "Dry enable", 1.0f), \
            SWITCH("wet_on", "Wet enable", 1.0f), \
            SWITCH("mono", "Mono output", 0.0f), \
            SWITCH("fb", "Feedback", 1.0f), \
            AMP_GAIN1("fbg", "Feedback gain", GAIN_AMP_0_DB), \
            OUT_GAIN, \
            METER("dmaxv", "Actual delay maximum value", U_SEC, art_delay_metadata::DSEL), \
            METER("memuse", "Overall memory usage", U_BYTES, art_delay_metadata::MEMORY)

        #define ART_DELAY_TEMPO(id) \
            CONTROL("tempo" #id, "Tempo " #id, U_BPM, art_delay_metadata::TEMPO), \
            COMBO("ratio" #id, "Tempo " #id " ratio", 0, art_delay_tempo_ratio), \
            SWITCH("sync" #id, "Tempo" #id " sync", 0.0f), \
            METER("atempo" #id, "Delay " #id " actual tempo", U_BPM, art_delay_metadata::ATEMPO)

        #define ART_DELAY_PROCESSOR(id, pan) \
            SWITCH("on" #id, "Delay " #id " on", 0.0f), \
            SWITCH("s" #id, "Delay " #id " solo", 0.0f), \
            SWITCH("m" #id, "Delay " #id " mute", 0.0f), \
            COMBO("dref" #id, "Delay " #id " reference", 0, art_delay_references), \
            CONTROL("drefm" #id, "Delay " #id " reference multiplier", U_NONE, art_delay_metadata::DELAY_MULT), \
            COMBO("tref" #id, "Delay " #id " tempo reference", 0, art_delay_tempo), \
            CONTROL("treff" #id, "Delay " #id " bar fraction", U_BAR, art_delay_metadata::DFRACTION), \
            INT_CONTROL("trefd" #id, "Delay " #id " bar denominator", U_BEAT, art_delay_metadata::DENOMINATOR), \
            CONTROL("trefm" #id, "Delay " #id " bar multiplier", U_NONE, art_delay_metadata::BAR_MULT), \
            CONTROL("frac" #id, "Delay " #id " fraction", U_BAR, art_delay_metadata::FRACTION), \
            INT_CONTROL("den" #id, "Delay " #id " denominator", U_BEAT, art_delay_metadata::DENOMINATOR), \
            CONTROL("dadd" #id, "Delay " #id " time addition", U_SEC, art_delay_metadata::TIME), \
            SWITCH("eq" #id, "Equalizer " #id " on", 0.0f), \
            SWITCH("lc" #id, "Delay " #id " low-cut filter", 0.0f), \
            LOG_CONTROL("flc" #id, "Delay " #id " low-cut frequency", U_HZ, art_delay_metadata::LOW_CUT), \
            SWITCH("hc" #id, "Delay " #id " high-cut filter", 0.0f), \
            LOG_CONTROL("fhc" #id, "Delay " #id " high-cut frequency", U_HZ, art_delay_metadata::HIGH_CUT), \
            LOG_CONTROL("fbs" #id, "Delay " #id " sub-bass", U_GAIN_AMP, art_delay_metadata::BAND_GAIN), \
            LOG_CONTROL("fbb" #id, "Delay " #id " bass", U_GAIN_AMP, art_delay_metadata::BAND_GAIN), \
            LOG_CONTROL("fbm" #id, "Delay " #id " middle", U_GAIN_AMP, art_delay_metadata::BAND_GAIN), \
            LOG_CONTROL("fbp" #id, "Delay " #id " presence", U_GAIN_AMP, art_delay_metadata::BAND_GAIN), \
            LOG_CONTROL("fbt" #id, "Delay " #id " treble", U_GAIN_AMP, art_delay_metadata::BAND_GAIN), \
            pan(#id, "Delay " #id), \
            AMP_GAIN10("dg" #id, "Delay " #id " gain", GAIN_AMP_0_DB), \
            HUE_CTL("hue" #id, "Delay " #id " hue", float(id) / art_delay_metadata::MAX_PROCESSORS ), \
            SWITCH("fbe" #id, "Delay " #id " feedback enable", 0.0f), \
            AMP_GAIN1("fbg" #id, "Delay " #id " feedback gain", GAIN_AMP_M_INF_DB), \
            COMBO("fbtr" #id, "Delay " #id " feedback tempo reference", 0, art_delay_tempo), \
            CONTROL("fbbf" #id, "Delay " #id " feedback bar fraction", U_BAR, art_delay_metadata::DFRACTION), \
            INT_CONTROL("fbbd" #id, "Delay " #id " feedback bar denominator", U_BEAT, art_delay_metadata::DENOMINATOR), \
            CONTROL("fbbm" #id, "Delay " #id " feedback bar multiplier", U_NONE, art_delay_metadata::BAR_MULT), \
            CONTROL("fbf" #id, "Delay " #id " feedback fraction", U_BAR, art_delay_metadata::FRACTION), \
            INT_CONTROL("fbd" #id, "Delay " #id " feedback denominator", U_BEAT, art_delay_metadata::DENOMINATOR), \
            CONTROL("fbadd" #id, "Delay " #id " feedback time addition", U_SEC, art_delay_metadata::TIME), \
            METER("adt" #id, "Delay " #id " actual time", U_SEC, art_delay_metadata::DSEL), \
            METER("afbt" #id, "Delay " #id " actual feedback time", U_SEC, art_delay_metadata::DSEL), \
            BLINK("door" #id, "Delay " #id " out of range"), \
            BLINK("fbor" #id, "Delay " #id " feedback out of range"), \
            BLINK("loop" #id, "Delay " #id " dependency loop"), \
            METER("tval" #id, "Delay " #id " selected tempo", U_BPM, art_delay_metadata::ATEMPO), \
            METER("fbtv" #id, "Delay " #id " selected feedback tempo", U_BPM, art_delay_metadata::ATEMPO), \
            METER("dval" #id, "Delay " #id " reference selected delay", U_SEC, art_delay_metadata::DSEL)

        #define ART_DELAY_TEMPOS \
            ART_DELAY_TEMPO(0), \
            ART_DELAY_TEMPO(1), \
            ART_DELAY_TEMPO(2), \
            ART_DELAY_TEMPO(3), \
            ART_DELAY_TEMPO(4), \
            ART_DELAY_TEMPO(5), \
            ART_DELAY_TEMPO(6), \
            ART_DELAY_TEMPO(7)

        #define ART_DELAY_PROCESSORS(pan) \
            ART_DELAY_PROCESSOR(0, pan), \
            ART_DELAY_PROCESSOR(1, pan), \
            ART_DELAY_PROCESSOR(2, pan), \
            ART_DELAY_PROCESSOR(3, pan), \
            ART_DELAY_PROCESSOR(4, pan), \
            ART_DELAY_PROCESSOR(5, pan), \
            ART_DELAY_PROCESSOR(6, pan), \
            ART_DELAY_PROCESSOR(7, pan), \
            ART_DELAY_PROCESSOR(8, pan), \
            ART_DELAY_PROCESSOR(9, pan), \
            ART_DELAY_PROCESSOR(10, pan), \
            ART_DELAY_PROCESSOR(11, pan), \
            ART_DELAY_PROCESSOR(12, pan), \
            ART_DELAY_PROCESSOR(13, pan), \
            ART_DELAY_PROCESSOR(14, pan), \
            ART_DELAY_PROCESSOR(15, pan)

        static const port_t art_delay_mono_ports[] =
        {
            // Input audio ports
            AUDIO_INPUT_MONO,
            AUDIO_OUTPUT_LEFT,
            AUDIO_OUTPUT_RIGHT,
            ART_DELAY_COMMON(ART_PAN_MONO),
            ART_DELAY_TEMPOS,
            ART_DELAY_PROCESSORS(ART_PAN_MONO),

            PORTS_END
        };

        static const port_t art_delay_stereo_ports[] =
        {
            // Input audio ports
            PORTS_STEREO_PLUGIN,
            ART_DELAY_COMMON(ART_PAN_STEREO),
            ART_DELAY_TEMPOS,
            ART_DELAY_PROCESSORS(ART_PAN_STEREO),

            PORTS_END
        };

        static const int plugin_classes[]       = { C_DELAY, -1 };
        static const int clap_features_mono[]   = { CF_AUDIO_EFFECT, CF_DELAY, CF_MONO, -1 };
        static const int clap_features_stereo[] = { CF_AUDIO_EFFECT, CF_DELAY, CF_STEREO, -1 };

        const meta::bundle_t art_delay_bundle =
        {
            "art_delay",
            "Artistic Delay",
            B_DELAYS,
            "mEP1WyLFruY",
            "This plugin allows one to construct almost any desired delay using up to 16 delay lines and 8 tempo settings. Almost all parameters can be smootly automated."
        };

        const meta::plugin_t  art_delay_mono =
        {
            "Künstlerische Verzögerung",
            "Artistic Delay Mono",
            "Artistic Delay Mono",
            "KV16M",
            &developers::v_sadovnikov,
            "art_delay_mono",
            LSP_LV2_URI("art_delay_mono"),
            LSP_LV2UI_URI("art_delay_mono"),
            "vxll",
            LSP_VST3_UID("kv16m   vxll"),
            LSP_VST3UI_UID("kv16m   vxll"),
            LSP_LADSPA_ART_DELAY_BASE + 0,
            LSP_LADSPA_URI("art_delay_mono"),
            LSP_CLAP_URI("art_delay_mono"),
            LSP_PLUGINS_ART_DELAY_VERSION,
            plugin_classes,
            clap_features_mono,
            E_DUMP_STATE,
            art_delay_mono_ports,
            "delay/art_delay/mono.xml",
            NULL,
            mono_to_stereo_plugin_port_groups,
            &art_delay_bundle
        };

        const meta::plugin_t  art_delay_stereo =
        {
            "Künstlerische Verzögerung",
            "Artistic Delay Stereo",
            "Artistic Delay Stereo",
            "KV16S",
            &developers::v_sadovnikov,
            "art_delay_stereo",
            LSP_LV2_URI("art_delay_stereo"),
            LSP_LV2UI_URI("art_delay_stereo"),
            "kbbr",
            LSP_VST3_UID("kv16s   kbbr"),
            LSP_VST3UI_UID("kv16s   kbbr"),
            LSP_LADSPA_ART_DELAY_BASE + 1,
            LSP_LADSPA_URI("art_delay_stereo"),
            LSP_CLAP_URI("art_delay_stereo"),
            LSP_PLUGINS_ART_DELAY_VERSION,
            plugin_classes,
            clap_features_stereo,
            E_DUMP_STATE,
            art_delay_stereo_ports,
            "delay/art_delay/stereo.xml",
            NULL,
            stereo_plugin_port_groups,
            &art_delay_bundle
        };
    } /* namespace meta */
} /* namespace lsp */

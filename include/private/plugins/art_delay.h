/*
 * Copyright (C) 2025 Linux Studio Plugins Project <https://lsp-plug.in/>
 *           (C) 2025 Vladimir Sadovnikov <sadko4u@gmail.com>
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

#ifndef PRIVATE_PLUGINS_ART_DELAY_H_
#define PRIVATE_PLUGINS_ART_DELAY_H_

#include <lsp-plug.in/plug-fw/plug.h>
#include <lsp-plug.in/dsp-units/ctl/Blink.h>
#include <lsp-plug.in/dsp-units/ctl/Bypass.h>
#include <lsp-plug.in/dsp-units/filters/Equalizer.h>
#include <lsp-plug.in/dsp-units/util/DynamicDelay.h>

#include <private/meta/art_delay.h>

namespace lsp
{
    namespace plugins
    {
        /**
         * Artistic Delay Plugin Series
         */
        class art_delay: public plug::Module
        {
            protected:
                struct art_delay_t;

                class DelayAllocator: public ipc::ITask
                {
                    private:
                        art_delay      *pBase;          // Delay base
                        art_delay_t    *pDelay;         // Delay pointer
                        ssize_t         nSize;          // Requested delay size

                    public:
                        explicit DelayAllocator(art_delay *base, art_delay_t *delay);
                        virtual ~DelayAllocator();

                    public:
                        virtual status_t    run();

                    public:
                        void set_size(ssize_t size)     { nSize = size; }
                };

                typedef struct art_tempo_t
                {
                    float                   fTempo;         // The actual tempo
                    bool                    bSync;          // Sync flag

                    plug::IPort            *pTempo;         // Tempo port
                    plug::IPort            *pRatio;         // Ratio port
                    plug::IPort            *pSync;          // Sync flag
                    plug::IPort            *pOutTempo;      // Output tempo
                } art_tempo_t;

                typedef struct pan_t
                {
                    float                   l;              // Gain of left channel
                    float                   r;              // Gain of right channel
                } pan_t;

                typedef struct art_settings_t
                {
                    float                   fDelay;         // Delay value
                    float                   fFeedGain;      // Feedback gain
                    float                   fFeedLen;       // Feedback length
                    pan_t                   sPan[2];        // Pan value + gain for each channel
                    size_t                  nMaxDelay;      // Maximum possible delay
                } art_settings_t;

                typedef struct art_delay_t
                {
                    dspu::DynamicDelay     *pPDelay[2];     // Pending delay (waiting for replace)
                    dspu::DynamicDelay     *pCDelay[2];     // Currently used delay for each channel
                    dspu::DynamicDelay     *pGDelay[2];     // Garbage
                    dspu::Equalizer         sEq[2];         // Equalizers for each channel
                    dspu::Bypass            sBypass[2];     // Bypass
                    dspu::Blink             sOutOfRange;    // Blink
                    dspu::Blink             sFeedOutRange;  // Feedback is out of range
                    DelayAllocator         *pAllocator;     // Allocator

                    bool                    bStereo;        // Mode: Mono/stereo
                    bool                    bOn;            // Delay is enabled
                    bool                    bSolo;          // Soloing flag
                    bool                    bMute;          // Muting flag
                    bool                    bUpdated;       // Update flag
                    bool                    bValidRef;      // Valid reference flag
                    ssize_t                 nDelayRef;      // Reference to delay
                    float                   fOutDelay;      // Output delay
                    float                   fOutFeedback;   // Output feedback delay
                    float                   fOutTempo;      // Output tempo
                    float                   fOutFeedTempo;  // Output tempo
                    float                   fOutDelayRef;   // Output delay reference value

                    art_settings_t          sOld;           // Old settings
                    art_settings_t          sNew;           // New settings

                    plug::IPort            *pOn;            // On
                    plug::IPort            *pTempoRef;      // Tempo reference
                    plug::IPort            *pPan[2];        // Panning
                    plug::IPort            *pSolo;          // Solo flag
                    plug::IPort            *pMute;          // Mute flag
                    plug::IPort            *pDelayRef;      // Delay reference
                    plug::IPort            *pDelayMul;      // Delay reference multiplier
                    plug::IPort            *pBarFrac;       // Bar fraction
                    plug::IPort            *pBarDenom;      // Bar denominator
                    plug::IPort            *pBarMul;        // Bar multiplier
                    plug::IPort            *pFrac;          // Add fraction
                    plug::IPort            *pDenom;         // Add denominator
                    plug::IPort            *pDelay;         // Add delay
                    plug::IPort            *pEqOn;          // Equalizer on
                    plug::IPort            *pLcfOn;         // Low-cut filter on
                    plug::IPort            *pLcfFreq;       // Low-cut filter frequency
                    plug::IPort            *pHcfOn;         // High-cut filter on
                    plug::IPort            *pHcfFreq;       // High-cut filter frequency
                    plug::IPort            *pBandGain[meta::art_delay_metadata::EQ_BANDS];    // Band gain for each filter
                    plug::IPort            *pGain;          // Output gain

                    // Feedback control
                    plug::IPort            *pFeedOn;        // Feedback on
                    plug::IPort            *pFeedGain;      // Feedback gain
                    plug::IPort            *pFeedTempoRef;  // Tempo reference for feedback
                    plug::IPort            *pFeedBarFrac;   // Bar fraction
                    plug::IPort            *pFeedBarDenom;  // Bar denominator
                    plug::IPort            *pFeedBarMul;    // Bar multiplier
                    plug::IPort            *pFeedFrac;      // Add fraction
                    plug::IPort            *pFeedDenom;     // Add denominator
                    plug::IPort            *pFeedDelay;     // Add delay

                    // Outputs
                    plug::IPort            *pOutDelay;      // Output delay
                    plug::IPort            *pOutFeedback;   // Output feedback delay
                    plug::IPort            *pOutOfRange;    // Out of range status
                    plug::IPort            *pOutFeedRange;  // Feedbsck out of range status
                    plug::IPort            *pOutLoop;       // Dependency loop
                    plug::IPort            *pOutTempo;      // Actual tempo
                    plug::IPort            *pOutFeedTempo;  // Actual feedback tempo
                    plug::IPort            *pOutDelayRef;   // Actual delay reference value
                } art_delay_t;

            protected:
                bool                        bStereoIn;
                bool                        bMono;          // Mono switch
                size_t                      nMaxDelay;      // Maximum delay
                pan_t                       sOldDryPan[2];  // Old panning + gain
                pan_t                       sNewDryPan[2];  // New panning + gain
                art_tempo_t                *vTempo;         // Tempo settings
                art_delay_t                *vDelays;        // Delay lines
                float                      *vOutBuf[2];     // Output buffer
                float                      *vGainBuf;       // Gain control buffer
                float                      *vDelayBuf;      // Delay control buffer
                float                      *vFeedBuf;       // Feedback delay control buffer
                float                      *vTempBuf;       // Temporary buffer for delay processing
                volatile uint32_t           nMemUsed;       // Overall memory usage by delay lines

                dspu::Bypass                sBypass[2];     // Bypasses
                ipc::IExecutor             *pExecutor;

                plug::IPort                *pIn[2];         // Input ports
                plug::IPort                *pOut[2];        // Output ports
                plug::IPort                *pBypass;        // Bypass
                plug::IPort                *pMaxDelay;      // Maximum possible delay
                plug::IPort                *pPan[2];        // Panning
                plug::IPort                *pDryGain;       // Dry gain
                plug::IPort                *pWetGain;       // Wet gain
                plug::IPort                *pDryWet;        // Dry/Wet balance
                plug::IPort                *pDryOn;         // Dry enable
                plug::IPort                *pWetOn;         // Wet enable
                plug::IPort                *pMono;          // Mono/Stereo switch
                plug::IPort                *pFeedback;      // Enable feedback for all delays
                plug::IPort                *pFeedGain;      // Feedback gain control
                plug::IPort                *pOutGain;       // Overall output gain
                plug::IPort                *pOutDMax;       // Maximum delay output value
                plug::IPort                *pOutMemUse;     // Memory usage

                uint8_t                    *pData;

            protected:
                static inline float         decode_ratio(size_t v);
                inline size_t               decode_max_delay_value(size_t v);
                bool                        check_delay_ref(art_delay_t *ad);
                void                        sync_delay(art_delay_t *ad);
                void                        process_delay(art_delay_t *ad, float **out, const float * const *in, size_t samples, size_t i, size_t count);
                void                        do_destroy();

            protected:
                static void                 dump_pan(dspu::IStateDumper *v, const char *name, const pan_t *pan, size_t n);
                static void                 dump_art_settings(dspu::IStateDumper *v, const char *name, const art_settings_t *as);
                static void                 dump_art_delay(dspu::IStateDumper *v, const art_delay_t *ad);
                static void                 dump_art_tempo(dspu::IStateDumper *v, const art_tempo_t *at);

            public:
                explicit art_delay(const meta::plugin_t *metadata, bool stereo_in);
                art_delay(const art_delay &) = delete;
                art_delay(art_delay &&) = delete;
                virtual ~art_delay() override;

                art_delay & operator=(const art_delay &) = delete;
                art_delay & operator=(art_delay &&) = delete;

                virtual void        init(plug::IWrapper *wrapper, plug::IPort **ports) override;
                virtual void        destroy() override;

            public:
                virtual bool        set_position(const plug::position_t *pos) override;
                virtual void        update_settings() override;
                virtual void        update_sample_rate(long sr) override;
                virtual void        process(size_t samples) override;
                virtual void        dump(dspu::IStateDumper *v) const override;
        };
    } /* namespace plugins */
} /* namespace lsp */

#endif /* PRIVATE_PLUGINS_ART_DELAY_H_ */

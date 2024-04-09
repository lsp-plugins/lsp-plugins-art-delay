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

#include <private/plugins/art_delay.h>
#include <lsp-plug.in/common/atomic.h>
#include <lsp-plug.in/common/alloc.h>
#include <lsp-plug.in/common/debug.h>
#include <lsp-plug.in/dsp/dsp.h>
#include <lsp-plug.in/dsp-units/units.h>
#include <lsp-plug.in/shared/debug.h>

namespace lsp
{
    namespace plugins
    {
        static constexpr size_t BUFFER_SIZE             = 0x1000;
        static constexpr ssize_t DELAY_REF_NONE         = -1;

        static const float art_delay_ratio[] =
        {
            1.0f / 1.0f,
            1.0f / 2.0f,
            1.0f / 3.0f,
            2.0f / 1.0f,
            2.0f / 3.0f,
            3.0f / 1.0f,
            3.0f / 2.0f
        };

        static const float band_freqs[] =
        {
            60.0f,
            300.0f,
            1000.0f,
            6000.0f
        };

        static const uint16_t art_delay_max[] =
        {
            1, 2, 4, 8,
            16, 24, 32, 40,
            48, 56, 64, 96,
            128, 160, 192, 224,
            256
        };

        //-------------------------------------------------------------------------
        // Plugin factory
        static const meta::plugin_t *plugins[] =
        {
            &meta::art_delay_mono,
            &meta::art_delay_stereo
        };

        static plug::Module *plugin_factory(const meta::plugin_t *meta)
        {
            return new art_delay(meta, meta == &meta::art_delay_stereo);
        }

        static plug::Factory factory(plugin_factory, plugins, 2);

        //-------------------------------------------------------------------------
        art_delay::DelayAllocator::DelayAllocator(art_delay *base, art_delay_t *delay)
        {
            pBase       = base;
            pDelay      = delay;
            nSize       = 0;
        }

        art_delay::DelayAllocator::~DelayAllocator()
        {
        }

        status_t art_delay::DelayAllocator::run()
        {
            dspu::DynamicDelay *d;
            size_t channels = (pDelay->bStereo) ? 2 : 1;

            // Drop garbage
            for (size_t i=0; i<channels; ++i)
            {
                if ((d = pDelay->pGDelay[i]) != NULL)
                {
                    int32_t used        = d->capacity();
                    pDelay->pGDelay[i]  = NULL;
                    d->destroy();
                    delete d;
                    atomic_add(&pBase->nMemUsed, -used); // Decrement the overall memory usage
                }

                if ((d = pDelay->pPDelay[i]) != NULL)
                {
                    int32_t used        = d->capacity();
                    pDelay->pPDelay[i] = NULL;
                    d->destroy();
                    delete d;
                    atomic_add(&pBase->nMemUsed, -used); // Decrement the overall memory usage
                }
            }

            if (nSize < 0)
                return STATUS_OK;

            // Allocate delays
            for (size_t i=0; i<channels; ++i)
            {
                d = pDelay->pCDelay[i];
                if ((d != NULL) && (d->max_delay() == size_t(nSize)))
                    continue;

                // Allocate delay
                d = new dspu::DynamicDelay();
                if (d == NULL)
                    return STATUS_NO_MEM;

                // Reserve space for delay
                status_t res = d->init(nSize);
                if (res != STATUS_OK)
                {
                    d->destroy();
                    delete d;
                    return res;
                }

                // Add delay to list of pending
                pDelay->pPDelay[i]  = d;
                atomic_add(&pBase->nMemUsed, d->capacity()); // Increment the overall memory usage
            }

            return STATUS_OK;
        }

        //-------------------------------------------------------------------------
        art_delay::art_delay(const meta::plugin_t *metadata, bool stereo_in): plug::Module(metadata)
        {
            bStereoIn       = stereo_in;
            bMono           = false;
            nMaxDelay       = 0;
            sOldDryPan[0].l = 0.0f;
            sOldDryPan[0].r = 0.0f;
            sOldDryPan[1].l = 0.0f;
            sOldDryPan[1].r = 0.0f;
            sNewDryPan[0].l = 0.0f;
            sNewDryPan[0].r = 0.0f;
            sNewDryPan[1].l = 0.0f;
            sNewDryPan[1].r = 0.0f;
            vOutBuf[0]      = NULL;
            vOutBuf[1]      = NULL;
            vGainBuf        = NULL;
            vDelayBuf       = NULL;
            vFeedBuf        = NULL;
            vTempBuf        = NULL;
            vTempo          = NULL;
            vDelays         = NULL;
            pExecutor       = NULL;
            nMemUsed        = 0;

            pIn[0]          = NULL;
            pIn[1]          = NULL;
            pOut[0]         = NULL;
            pOut[1]         = NULL;
            pBypass         = NULL;
            pMaxDelay       = NULL;
            pPan[0]         = NULL;
            pPan[1]         = NULL;
            pDryGain        = NULL;
            pWetGain        = NULL;
            pDryWet         = NULL;
            pDryOn          = NULL;
            pWetOn          = NULL;
            pMono           = NULL;
            pFeedback       = NULL;
            pFeedGain       = NULL;
            pOutGain        = NULL;
            pOutDMax        = NULL;
            pOutMemUse      = NULL;

            pData           = NULL;
        }

        art_delay::~art_delay()
        {
            do_destroy();
        }

        void art_delay::init(plug::IWrapper *wrapper, plug::IPort **ports)
        {
            plug::Module::init(wrapper, ports);

            size_t sz_buf           = BUFFER_SIZE * sizeof(float);
            size_t sz_tempo         = align_size(sizeof(art_tempo_t) * meta::art_delay_metadata::MAX_TEMPOS, 64);
            size_t sz_proc          = align_size(sizeof(art_delay_t) * meta::art_delay_metadata::MAX_PROCESSORS, 64);
            size_t sz_alloc         = sz_tempo + sz_proc + sz_buf * 6;

            uint8_t *ptr            = alloc_aligned<uint8_t>(pData, sz_alloc, 64);
            if (ptr == NULL)
                return;

            // Allocate data buffers
            vTempo                  = advance_ptr_bytes<art_tempo_t>(ptr, sz_tempo);
            vDelays                 = advance_ptr_bytes<art_delay_t>(ptr, sz_proc);

            for (size_t i=0; i<2; ++i)
                vOutBuf[i]              = advance_ptr_bytes<float>(ptr, sz_buf);

            vGainBuf                = advance_ptr_bytes<float>(ptr, sz_buf);
            vDelayBuf               = advance_ptr_bytes<float>(ptr, sz_buf);
            vFeedBuf                = advance_ptr_bytes<float>(ptr, sz_buf);
            vTempBuf                = advance_ptr_bytes<float>(ptr, sz_buf);

            // Initialize tempos
            for (size_t i=0; i<meta::art_delay_metadata::MAX_TEMPOS; ++i)
            {
                art_tempo_t *at         = &vTempo[i];

                at->fTempo              = BPM_DEFAULT;
                at->bSync               = false;

                at->pTempo              = NULL;
                at->pRatio              = NULL;
                at->pSync               = NULL;
                at->pOutTempo           = NULL;
            }

            // Initialize delays
            for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
            {
                art_delay_t *ad         = &vDelays[i];

                ad->pPDelay[0]          = NULL;
                ad->pPDelay[1]          = NULL;
                ad->pCDelay[0]          = NULL;
                ad->pCDelay[1]          = NULL;
                ad->pGDelay[0]          = NULL;
                ad->pGDelay[1]          = NULL;

                ad->sEq[0].construct();
                ad->sEq[1].construct();
                ad->sBypass[0].construct();
                ad->sBypass[1].construct();
                ad->sOutOfRange.construct();
                ad->sFeedOutRange.construct();

                ad->sEq[0].init(meta::art_delay_metadata::EQ_BANDS + 2, 0);
                ad->sEq[1].init(meta::art_delay_metadata::EQ_BANDS + 2, 0);
                ad->sEq[0].set_mode(dspu::EQM_IIR);
                ad->sEq[1].set_mode(dspu::EQM_IIR);

                ad->pAllocator          = new DelayAllocator(this, ad);
                if (ad->pAllocator == NULL)
                    return;

                ad->bStereo             = bStereoIn;
                ad->bOn                 = false;
                ad->bSolo               = false;
                ad->bMute               = false;
                ad->bUpdated            = false;
                ad->bValidRef           = true;
                ad->nDelayRef           = DELAY_REF_NONE;
                ad->fOutDelay           = 0.0f;
                ad->fOutFeedback        = 0.0f;
                ad->fOutTempo           = 0.0f;
                ad->fOutDelayRef        = 0.0f;

                ad->sOld.fDelay         = 0.0f;
                ad->sOld.fFeedGain      = 0.0f;
                ad->sOld.fFeedLen       = 0.0f;
                if (bStereoIn)
                {
                    ad->sOld.sPan[0].l      = 1.0f;
                    ad->sOld.sPan[0].r      = 0.0f;
                    ad->sOld.sPan[1].l      = 0.0f;
                    ad->sOld.sPan[1].r      = 1.0f;
                }
                else
                {
                    ad->sOld.sPan[0].l      = 0.5f;
                    ad->sOld.sPan[0].r      = 0.5f;
                    ad->sOld.sPan[1].l      = 0.5f;
                    ad->sOld.sPan[1].r      = 0.5f;
                }
                ad->sOld.nMaxDelay      = 0;

                ad->sNew                = ad->sOld;

                ad->pOn                 = NULL;
                ad->pTempoRef           = NULL;
                ad->pPan[0]             = NULL;
                ad->pPan[1]             = NULL;
                ad->pSolo               = NULL;
                ad->pMute               = NULL;
                ad->pDelayRef           = NULL;
                ad->pDelayMul           = NULL;
                ad->pBarFrac            = NULL;
                ad->pBarDenom           = NULL;
                ad->pBarMul             = NULL;
                ad->pFrac               = NULL;
                ad->pDenom              = NULL;
                ad->pDelay              = NULL;
                ad->pEqOn               = NULL;
                ad->pLcfOn              = NULL;
                ad->pLcfFreq            = NULL;
                ad->pHcfOn              = NULL;
                ad->pHcfFreq            = NULL;
                for (size_t j=0; j<meta::art_delay_metadata::EQ_BANDS; ++j)
                    ad->pBandGain[j]        = NULL;
                ad->pGain               = NULL;

                ad->pFeedOn             = NULL;
                ad->pFeedGain           = NULL;
                ad->pFeedTempoRef       = NULL;
                ad->pFeedBarFrac        = NULL;
                ad->pFeedBarDenom       = NULL;
                ad->pFeedBarMul         = NULL;
                ad->pFeedFrac           = NULL;
                ad->pFeedDenom          = NULL;
                ad->pFeedDelay          = NULL;

                ad->pOutDelay           = NULL;
                ad->pOutFeedback        = NULL;
                ad->pOutOfRange         = NULL;
                ad->pOutFeedRange       = NULL;
                ad->pOutLoop            = NULL;
                ad->pOutTempo           = NULL;
                ad->pOutDelayRef        = NULL;
            }

            // Initialize bypasses
            sBypass[0].construct();
            sBypass[1].construct();

            // Get executor service
            pExecutor               = wrapper->executor();
            size_t port_id          = 0;

            // Bind in/out ports
            lsp_trace("Binding audio ports");

            BIND_PORT(pIn[0]);
            if (bStereoIn)
                BIND_PORT(pIn[1]);

            BIND_PORT(pOut[0]);
            BIND_PORT(pOut[1]);

            // Bind common ports
            lsp_trace("Binding common ports");
            BIND_PORT(pBypass);
            SKIP_PORT("Delay line selector"); // Skip delay line selector
            BIND_PORT(pMaxDelay);

            BIND_PORT(pPan[0]);
            if (bStereoIn)
                BIND_PORT(pPan[1]);

            BIND_PORT(pDryGain);
            BIND_PORT(pWetGain);
            BIND_PORT(pDryWet);
            BIND_PORT(pDryOn);
            BIND_PORT(pWetOn);
            BIND_PORT(pMono);
            BIND_PORT(pFeedback);
            BIND_PORT(pFeedGain);
            BIND_PORT(pOutGain);
            BIND_PORT(pOutDMax);
            BIND_PORT(pOutMemUse);

            // Bind delay ports
            lsp_trace("Binding tempo ports");
            for (size_t i=0; i<meta::art_delay_metadata::MAX_TEMPOS; ++i)
            {
                art_tempo_t *at         = &vTempo[i];
                BIND_PORT(at->pTempo);
                BIND_PORT(at->pRatio);
                BIND_PORT(at->pSync);
                BIND_PORT(at->pOutTempo);
            }

            // Bind delay ports
            lsp_trace("Binding delay ports");
            for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
            {
                art_delay_t *ad         = &vDelays[i];

                BIND_PORT(ad->pOn);
                BIND_PORT(ad->pSolo);
                BIND_PORT(ad->pMute);
                BIND_PORT(ad->pDelayRef);
                BIND_PORT(ad->pDelayMul);
                BIND_PORT(ad->pTempoRef);
                BIND_PORT(ad->pBarFrac);
                BIND_PORT(ad->pBarDenom);
                BIND_PORT(ad->pBarMul);
                BIND_PORT(ad->pFrac);
                BIND_PORT(ad->pDenom);
                BIND_PORT(ad->pDelay);
                BIND_PORT(ad->pEqOn);
                BIND_PORT(ad->pLcfOn);
                BIND_PORT(ad->pLcfFreq);
                BIND_PORT(ad->pHcfOn);
                BIND_PORT(ad->pHcfFreq);
                for (size_t j=0; j<meta::art_delay_metadata::EQ_BANDS; ++j)
                    BIND_PORT(ad->pBandGain[j]);

                BIND_PORT(ad->pPan[0]);
                if (ad->bStereo)
                    BIND_PORT(ad->pPan[1]);

                BIND_PORT(ad->pGain);
                SKIP_PORT("Hue"); // Skip hue settings

                // Feedback
                BIND_PORT(ad->pFeedOn);
                BIND_PORT(ad->pFeedGain);
                BIND_PORT(ad->pFeedTempoRef);
                BIND_PORT(ad->pFeedBarFrac);
                BIND_PORT(ad->pFeedBarDenom);
                BIND_PORT(ad->pFeedBarMul);
                BIND_PORT(ad->pFeedFrac);
                BIND_PORT(ad->pFeedDenom);
                BIND_PORT(ad->pFeedDelay);

                BIND_PORT(ad->pOutDelay);
                BIND_PORT(ad->pOutFeedback);
                BIND_PORT(ad->pOutOfRange);
                BIND_PORT(ad->pOutFeedRange);
                BIND_PORT(ad->pOutLoop);
                BIND_PORT(ad->pOutTempo);
                BIND_PORT(ad->pOutFeedTempo);
                BIND_PORT(ad->pOutDelayRef);
            }
        }

        void art_delay::destroy()
        {
            plug::Module::destroy();
            do_destroy();
        }

        void art_delay::do_destroy()
        {
            if (vDelays != NULL)
            {
                for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
                {
                    art_delay_t *ad         = &vDelays[i];

                    for (size_t j=0; j < 2; ++j)
                    {
                        if (ad->pPDelay[j] != NULL)
                            delete ad->pPDelay[j];
                        if (ad->pCDelay[j] != NULL)
                            delete ad->pCDelay[j];
                        if (ad->pGDelay[j] != NULL)
                            delete ad->pGDelay[j];

                        ad->sEq[j].destroy();
                    }

                    if (ad->pAllocator != NULL)
                    {
                        delete ad->pAllocator;
                        ad->pAllocator = NULL;
                    }
                }

                vDelays     = NULL;
            }

            if (pData != NULL)
            {
                free_aligned(pData);
                pData       = NULL;
            }
        }

        bool art_delay::set_position(const plug::position_t *pos)
        {
            for (size_t i=0; i<meta::art_delay_metadata::MAX_TEMPOS; ++i)
            {
                art_tempo_t *at         = &vTempo[i];
                if (at->bSync)
                    return pos->beatsPerMinute != pWrapper->position()->beatsPerMinute;
            }

            return false;
        }

        void art_delay::update_sample_rate(long sr)
        {
            sBypass[0].init(sr);
            sBypass[1].init(sr);

            for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
            {
                art_delay_t *ad         = &vDelays[i];

                // The length of each delay will be changed in offline mode
                ad->sOutOfRange.init(sr);
                ad->sFeedOutRange.init(sr);

                for (size_t j=0; j < 2; ++j)
                {
                    ad->sBypass[j].init(sr);
                    ad->sEq[j].set_sample_rate(sr);
                }
            }
        }

        float art_delay::decode_ratio(size_t v)
        {
            return (v < sizeof(art_delay_ratio)/sizeof(float)) ? art_delay_ratio[v] : 1.0f;
        }

        size_t art_delay::decode_max_delay_value(size_t v)
        {
            float seconds = (v < sizeof(art_delay_max)/sizeof(uint16_t)) ? art_delay_max[v] : 1.0f;
            return dspu::seconds_to_samples(fSampleRate, seconds);
        }

        bool art_delay::check_delay_ref(art_delay_t *ad)
        {
            art_delay_t *list[meta::art_delay_metadata::MAX_PROCESSORS];
            size_t n  = 0;
            list[n++] = ad;

            while (ad->nDelayRef >= 0)
            {
                // Get referenced delay
                ad  = &vDelays[ad->nDelayRef];

                // Check that there is no reference already
                for (size_t i=0; i<n; ++i)
                    if (list[i] == ad)
                        return false;

                // Append to list
                list[n++] = ad;
            }

            return true;
        }

        void art_delay::update_settings()
        {
            size_t channels     = (bStereoIn) ? 2 : 1;

            bool bypass         = pBypass->value() >= 0.5f;
            float g_out         = pOutGain->value();
            float dry_gain      = (pDryOn->value() >= 0.5f) ? pDryGain->value() * g_out : 0.0f;
            float wet_gain      = (pWetOn->value() >= 0.5f) ? pWetGain->value() * g_out : 0.0f;
            float drywet        = pDryWet->value() * 0.01f;
            float dry           = dry_gain * drywet + 1.0f - drywet;
            float wet           = wet_gain * drywet;
            float fback         = (pFeedback->value() >= 0.5f) ? pFeedGain->value() : 0.0f;

            bMono               = pMono->value() >= 0.5f;
            nMaxDelay           = decode_max_delay_value(pMaxDelay->value());

            for (size_t i=0; i<channels; ++i)
            {
                sNewDryPan[i].l     = (100.0f - pPan[i]->value()) * 0.005f * dry;
                sNewDryPan[i].r     = (pPan[i]->value() + 100.0f) * 0.005f * dry;
            }

            sBypass[0].set_bypass(bypass);
            sBypass[1].set_bypass(bypass);

            // Sync state of tempos
            for (size_t i=0; i<meta::art_delay_metadata::MAX_TEMPOS; ++i)
            {
                art_tempo_t *at         = &vTempo[i];

                bool sync               = at->pSync->value() >= 0.5f;
                float ratio             = decode_ratio(at->pRatio->value());
                float tempo             = (sync) ? pWrapper->position()->beatsPerMinute : at->pTempo->value();

                at->bSync               = sync;
                at->fTempo              = ratio * tempo;
            }

            // Sync state of each delay
            for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
            {
                art_delay_t *ad         = &vDelays[i];

                ad->bOn                 = ad->pOn->value() >= 0.5f;
                ad->bSolo               = ad->pSolo->value() >= 0.5f;
                ad->bMute               = ad->pMute->value() >= 0.5f;
                ad->bUpdated            = false;
                ad->nDelayRef           = ad->pDelayRef->value() - 1.0f;
            }

            // Validate state of delay
            bool has_solo           = false;
            for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
            {
                art_delay_t *ad         = &vDelays[i];
                ad->bValidRef           = check_delay_ref(ad);
                if ((ad->bOn) && (ad->bSolo))
                    has_solo                = true;
            }

            // Apply recursive settings to delays
            for (size_t i=0, nupd=0; nupd < meta::art_delay_metadata::MAX_PROCESSORS; i = ((i+1) % meta::art_delay_metadata::MAX_PROCESSORS))
            {
                // Get the delay
                art_delay_t *ad         = &vDelays[i];
                if (ad->bUpdated)
                    continue;

                // Get parent delay reference and check that it is updated
                art_delay_t *p_ad       = ((ad->bValidRef) && (ad->nDelayRef >= 0)) ? &vDelays[ad->nDelayRef] : NULL;
                if ((p_ad != NULL) && (!p_ad->bUpdated))
                    continue;

                float delay             = dspu::seconds_to_samples(fSampleRate, ad->pDelay->value());
                float fbdelay           = dspu::seconds_to_samples(fSampleRate, ad->pFeedDelay->value());

                // Tempo reference
                ssize_t tempo_ref       = ad->pTempoRef->value() - 1.0f;
                if (tempo_ref >= 0)
                {
                    // Compute bar * multiplier + fraction
                    art_tempo_t *at         = &vTempo[tempo_ref];
                    ad->fOutTempo           = at->fTempo;
                    float bfrac             = ad->pBarFrac->value() * ad->pBarMul->value() + ad->pFrac->value();
                    float bdelay            = (240.0f * bfrac) / ad->fOutTempo;
                    delay                  += dspu::seconds_to_samples(fSampleRate, bdelay);
                }
                else
                    ad->fOutTempo           = 0.0f;

                // Feedback tempo reference
                tempo_ref               = ad->pFeedTempoRef->value() - 1.0f;
                if (tempo_ref >= 0)
                {
                    // Compute bar * multiplier + fraction
                    art_tempo_t *at         = &vTempo[tempo_ref];
                    ad->fOutFeedTempo       = at->fTempo;
                    float bfrac             = ad->pFeedBarFrac->value() * ad->pFeedBarMul->value() + ad->pFeedFrac->value();
                    float bdelay            = (240.0f * bfrac) / ad->fOutFeedTempo;
                    fbdelay                += dspu::seconds_to_samples(fSampleRate, bdelay);
                }
                else
                    ad->fOutFeedTempo       = 0.0f;

                // Parent delay reference
                if (p_ad != NULL)
                {
                    ad->fOutDelayRef        = p_ad->sNew.fDelay;
                    delay                  += ad->fOutDelayRef * ad->pDelayMul->value();
                }
                else
                    ad->fOutDelayRef        = 0.0f;

                // Update delay settings
                float gain              = ad->pGain->value() * wet;
                ad->sNew.fDelay         = delay;
                ad->sNew.fFeedGain      = (ad->pFeedOn->value() >= 0.5f) ? fback * ad->pFeedGain->value() : 0.0f;
                ad->sNew.fFeedLen       = fbdelay;

                for (size_t j=0; j<channels; ++j)
                {
                    ad->sNew.sPan[j].l      = (100.0f - ad->pPan[j]->value()) * 0.005f * gain;
                    ad->sNew.sPan[j].r      = (ad->pPan[j]->value() + 100.0f) * 0.005f * gain;
                }

                ad->fOutDelay           = dspu::samples_to_seconds(fSampleRate, delay);

                // Determine mode
                bool eq_on          = ad->pEqOn->value() >= 0.5f;
                bool low_on         = ad->pLcfOn->value() >= 0.5f;
                bool high_on        = ad->pHcfOn->value() >= 0.5f;
                bool xbypass        = (bypass) || (ad->bMute) || ((has_solo) && (!ad->bSolo));
                dspu::equalizer_mode_t eq_mode = (eq_on || low_on || high_on) ? dspu::EQM_IIR : dspu::EQM_BYPASS;

                // Update processor settings
                for (size_t j=0; j<channels; ++j)
                {
                    // Update bypass
                    ad->sBypass[j].set_bypass(xbypass);

                    // Update equalizer
                    dspu::Equalizer *eq   = &ad->sEq[j];
                    eq->set_mode(eq_mode);

                    if (eq_mode == dspu::EQM_BYPASS)
                        continue;

                    dspu::filter_params_t fp;
                    size_t band     = 0;

                    // Set-up parametric equalizer
                    while (band < meta::art_delay_metadata::EQ_BANDS)
                    {
                        if (band == 0)
                        {
                            fp.fFreq        = band_freqs[band];
                            fp.fFreq2       = fp.fFreq;
                            fp.nType        = (eq_on) ? dspu::FLT_MT_LRX_LOSHELF : dspu::FLT_NONE;
                        }
                        else if (band == (meta::art_delay_metadata::EQ_BANDS - 1))
                        {
                            fp.fFreq        = band_freqs[band-1];
                            fp.fFreq2       = fp.fFreq;
                            fp.nType        = (eq_on) ? dspu::FLT_MT_LRX_HISHELF : dspu::FLT_NONE;
                        }
                        else
                        {
                            fp.fFreq        = band_freqs[band-1];
                            fp.fFreq2       = band_freqs[band];
                            fp.nType        = (eq_on) ? dspu::FLT_MT_LRX_LADDERPASS : dspu::FLT_NONE;
                        }

                        fp.fGain        = ad->pBandGain[band]->value();
                        fp.nSlope       = 2;
                        fp.fQuality     = 0.0f;

                        // Update filter parameters
                        eq->set_params(band++, &fp);
                    }

                    // Setup hi-pass filter
                    fp.nType        = (low_on) ? dspu::FLT_BT_BWC_HIPASS : dspu::FLT_NONE;
                    fp.fFreq        = ad->pLcfFreq->value();
                    fp.fFreq2       = fp.fFreq;
                    fp.fGain        = 1.0f;
                    fp.nSlope       = 4;
                    fp.fQuality     = 0.0f;
                    eq->set_params(band++, &fp);

                    // Setup low-pass filter
                    fp.nType        = (high_on) ? dspu::FLT_BT_BWC_LOPASS : dspu::FLT_NONE;
                    fp.fFreq        = ad->pHcfFreq->value();
                    fp.fFreq2       = fp.fFreq;
                    fp.fGain        = 1.0f;
                    fp.nSlope       = 4;
                    fp.fQuality     = 0.0f;
                    eq->set_params(band++, &fp);
                }

                // Mark delay as updated
                ad->bUpdated    = true;
                ++nupd;
            }
        }

        void art_delay::sync_delay(art_delay_t *ad)
        {
            DelayAllocator *da = ad->pAllocator;
            size_t channels    = (ad->bStereo) ? 2 : 1;

            if (da->idle())
            {
                if (ad->bOn)
                {
                    bool resize     = false;

                    // Check that delay has to be resized
                    for (size_t i=0; i < channels; ++i)
                    {
                        if ((ad->pCDelay[i] == NULL) || (ad->pCDelay[i]->max_delay() != nMaxDelay))
                            resize      = true;
                    }

                    // Need to issue resize?
                    if (resize)
                    {
                        da->set_size(nMaxDelay);
                        pExecutor->submit(da);
                    }
                }
                else
                {
                    // Estimate the garbage cleanup flag
                    bool gc = false;
                    for (size_t i=0; i < channels; ++i)
                    {
                        if ((ad->pGDelay[i] == NULL) && (ad->pCDelay[i] != NULL))
                        {
                            ad->pGDelay[i] = ad->pCDelay[i];
                            ad->pCDelay[i] = NULL;
                        }

                        gc = gc || (ad->pGDelay[i] != NULL) || (ad->pPDelay[i] != NULL);
                    }

                    // Need to clean the whole garbage ?
                    if (gc)
                    {
                        da->set_size(-1);
                        pExecutor->submit(da);
                    }
                }
            }
            else if (da->completed())
            {
                // Update delay
                if (ad->bOn)
                {
                    bool gc = false;

                    for (size_t i=0; i < channels; ++i)
                    {
                        // There is data to commit?
                        if (ad->pPDelay[i] == NULL)
                            continue;

                        // Copy delay data if it is present
                        if (ad->pCDelay[i] != NULL)
                            ad->pPDelay[i]->copy(ad->pCDelay[i]);

                        // Swap pointers
                        ad->pGDelay[i] = ad->pCDelay[i];
                        ad->pCDelay[i] = ad->pPDelay[i];
                        ad->pPDelay[i] = NULL;

                        // Update garbage flag
                        gc = gc || (ad->pGDelay[i] != NULL);
                    }

                    // Reset task state
                    da->reset();

                    // Need to clean garbage?
                    if (gc)
                    {
                        da->set_size(nMaxDelay);
                        pExecutor->submit(da);
                    }
                }
            }
        }

        void art_delay::process_delay(art_delay_t *ad, float **out, const float * const *in, size_t samples, size_t off, size_t count)
        {
            float dmax, fbmax;

            // Create delay control signal if it is changing slowly
            if ((ad->sOld.fDelay != ad->sNew.fDelay) &&
                (fabsf(ad->sOld.fDelay - ad->sNew.fDelay)*0.25f <= samples))
            {
                dsp::lin_inter_set(vDelayBuf, 0, ad->sOld.fDelay, samples, ad->sNew.fDelay, off, count);
                dmax = lsp_max(vDelayBuf[0], vDelayBuf[count-1]);
            }
            else
            {
                dsp::fill(vDelayBuf, ad->sNew.fDelay, count);
                dmax = ad->sNew.fDelay;
            }

            // Create feedback delay control signal if it is changing slowly
            if ((ad->sOld.fFeedLen != ad->sNew.fFeedLen) &&
                (fabsf(ad->sOld.fFeedLen - ad->sNew.fFeedLen)*0.25f <= samples))
            {
                dsp::lin_inter_set(vFeedBuf, 0, ad->sOld.fFeedLen, samples, ad->sNew.fFeedLen, off, count);
                fbmax = lsp_max(vFeedBuf[0], vFeedBuf[count-1]);
            }
            else
            {
                dsp::fill(vFeedBuf, ad->sNew.fFeedLen, count);
                fbmax = ad->sNew.fFeedLen;
            }

            // Process the feedback signal and check that it is not out of range
            ad->fOutFeedback    = dspu::samples_to_seconds(fSampleRate, fbmax);
            if ((fbmax > nMaxDelay) || (fbmax > dmax))
                ad->sFeedOutRange.blink(); // Indicate out of range

            // Check if there is nothing to do
            if (!ad->bOn)
                return;
            size_t channels = (ad->bStereo) ? 2 : 1;
            for (size_t i=0; i<channels; ++i)
                if ((ad->pCDelay[i] == NULL) || (ad->pCDelay[i]->max_delay() < nMaxDelay))
                    return;

            // Create feedback gain control signal
            if (ad->sOld.fFeedGain != ad->sNew.fFeedGain)
                dsp::lin_inter_set(vGainBuf, 0, ad->sOld.fFeedGain, samples, ad->sNew.fFeedGain, off, count);
            else
                dsp::fill(vGainBuf, ad->sOld.fFeedGain, count);

            for (size_t i=0; i<channels; ++i)
            {
                // Process the delay -> eq -> bypass chain
                ad->pCDelay[i]->process(vTempBuf, in[i], vDelayBuf, vGainBuf, vFeedBuf, count);
                ad->sEq[i].process(vTempBuf, vTempBuf, count);
                ad->sBypass[i].process(vTempBuf, NULL, vTempBuf, count);

                // Pan the output
                if (ad->sOld.sPan[i].l != ad->sNew.sPan[i].l)
                {
                    dsp::lin_inter_fmadd2(out[0], vTempBuf, 0, ad->sOld.sPan[i].l, samples, ad->sNew.sPan[i].l, off, count);
                    dsp::lin_inter_fmadd2(out[1], vTempBuf, 0, ad->sOld.sPan[i].r, samples, ad->sNew.sPan[i].r, off, count);
                }
                else
                {
                    dsp::fmadd_k3(out[0], vTempBuf, ad->sOld.sPan[i].l, count);
                    dsp::fmadd_k3(out[1], vTempBuf, ad->sOld.sPan[i].r, count);
                }
            }
        }

        void art_delay::process(size_t samples)
        {
            // Estimate number of channels
            size_t channels = (bStereoIn) ? 2 : 1;

            // Sync delay lines
            for (size_t j=0; j<meta::art_delay_metadata::MAX_PROCESSORS; ++j)
            {
                art_delay_t *ad     = &vDelays[j];
                sync_delay(ad);
            }

            // Bind ports
            float *in[2], *out[2];

            in[0]   = pIn[0]->buffer<float>();
            in[1]   = (bStereoIn) ? pIn[1]->buffer<float>() : in[0];

            out[0]  = pOut[0]->buffer<float>();
            out[1]  = pOut[1]->buffer<float>();

            // Process audio signal
            for (size_t i=0; i<samples; )
            {
                // How many samples we can process at one time?
                size_t count        = lsp_min(samples - i, BUFFER_SIZE);

                // Process the dry sound (gain + pan)
                dsp::fill_zero(vOutBuf[0], count);
                dsp::fill_zero(vOutBuf[1], count);

                for (size_t j=0; j<channels; ++j)
                {
                    if (sOldDryPan[j].l != sNewDryPan[j].l)
                    {
                        dsp::lin_inter_fmadd2(vOutBuf[0], in[j], 0, sOldDryPan[j].l, samples, sNewDryPan[j].l, i, count);
                        dsp::lin_inter_fmadd2(vOutBuf[1], in[j], 0, sOldDryPan[j].r, samples, sNewDryPan[j].r, i, count);
                    }
                    else
                    {
                        dsp::fmadd_k3(vOutBuf[0], in[j], sOldDryPan[j].l, count);
                        dsp::fmadd_k3(vOutBuf[1], in[j], sOldDryPan[j].r, count);
                    }
                }

                // Process all delay channels and store result to vDataBuf
                for (size_t j=0; j<meta::art_delay_metadata::MAX_PROCESSORS; ++j)
                    process_delay(&vDelays[j], vOutBuf, in, samples, i, count);

                // Output internal buffer data to external outputs via applied bypass

                if (bMono)
                {
                    dsp::lr_to_mid(vOutBuf[0], vOutBuf[0], vOutBuf[1], count);
                    sBypass[0].process(out[0], in[0], vOutBuf[0], count);
                    sBypass[1].process(out[1], in[1], vOutBuf[0], count);
                }
                else
                {
                    sBypass[0].process(out[0], in[0], vOutBuf[0], count);
                    sBypass[1].process(out[1], in[1], vOutBuf[1], count);
                }

                // Update positions
                in[0]              += count;
                in[1]              += count;
                out[0]             += count;
                out[1]             += count;
                i                  += count;
            }

            // Commit dynamic settings and output values
            sOldDryPan[0]       = sNewDryPan[0];
            sOldDryPan[1]       = sNewDryPan[1];

            for (size_t j=0; j<meta::art_delay_metadata::MAX_TEMPOS; ++j)
            {
                art_tempo_t *at     = &vTempo[j];
                at->pOutTempo->set_value(at->fTempo);
            }

            for (size_t j=0; j<meta::art_delay_metadata::MAX_PROCESSORS; ++j)
            {
                art_delay_t *ad     = &vDelays[j];
                ad->sOld            = ad->sNew;

                // Update blink state
                if (ad->sNew.fDelay > nMaxDelay)
                    ad->sOutOfRange.blink();

                // Output values
                ad->pOutDelay->set_value(ad->fOutDelay);
                ad->pOutFeedback->set_value(ad->fOutFeedback);
                ad->pOutDelayRef->set_value(dspu::samples_to_seconds(fSampleRate, ad->fOutDelayRef));
                ad->pOutTempo->set_value(ad->fOutTempo);
                ad->pOutFeedTempo->set_value(ad->fOutFeedTempo);
                ad->pOutOfRange->set_value(ad->sOutOfRange.value());
                ad->pOutFeedRange->set_value(ad->sFeedOutRange.value());
                ad->pOutLoop->set_value((ad->bValidRef) ? 0.0f : 1.0f);

                // Post-process blink
                ad->sOutOfRange.process(samples);
                ad->sFeedOutRange.process(samples);
            }

            float used = nMemUsed;
            pOutDMax->set_value(dspu::samples_to_seconds(fSampleRate, nMaxDelay));
            pOutMemUse->set_value((used / (1024.0f * 1024.0f)) * sizeof(float)); // Translate floats into megabytes
        }

        void art_delay::dump_pan(dspu::IStateDumper *v, const char *name, const pan_t *pan, size_t n)
        {
            v->begin_array(name, pan, n);
            {
                for (size_t i=0; i<n; ++i)
                {
                    const pan_t *p = &pan[i];
                    v->begin_object(p, sizeof(pan_t));
                    {
                        v->write("l", p->l);
                        v->write("r", p->r);
                    }
                    v->end_object();
                }
            }
            v->end_array();
        }

        void art_delay::dump_art_settings(dspu::IStateDumper *v, const char *name, const art_settings_t *as)
        {
            v->begin_object(name, as, sizeof(art_settings_t));
            {
                v->write("fDelay", as->fDelay);
                v->write("fFeedGain", as->fFeedGain);
                v->write("fFeedLen", as->fFeedLen);
                dump_pan(v, "sPan", as->sPan, 2);
                v->write("nMaxDelay", as->nMaxDelay);
            }
            v->end_object();
        }

        void art_delay::dump_art_tempo(dspu::IStateDumper *v, const art_tempo_t *at)
        {
            v->begin_object(at, sizeof(art_tempo_t));
            {
                v->write("fTempo", at->fTempo);
                v->write("bSync", at->bSync);
                v->write("pTempo", at->pTempo);
                v->write("pRatio", at->pRatio);
                v->write("pSync", at->pSync);
                v->write("pOutTempo", at->pOutTempo);
            }
            v->end_object();
        }

        void art_delay::dump_art_delay(dspu::IStateDumper *v, const art_delay_t *ad)
        {
            v->begin_object(ad, sizeof(art_delay_t));
            {
                v->begin_array("pPDelay", ad->pPDelay, 2);
                {
                    v->write_object(ad->pPDelay[0]);
                    v->write_object(ad->pPDelay[1]);
                }
                v->end_array();

                v->begin_array("pCDelay", ad->pCDelay, 2);
                {
                    v->write_object(ad->pCDelay[0]);
                    v->write_object(ad->pCDelay[1]);
                }
                v->end_array();

                v->begin_array("pGDelay", ad->pGDelay, 2);
                {
                    v->write_object(ad->pGDelay[0]);
                    v->write_object(ad->pGDelay[1]);
                }
                v->end_array();

                v->begin_array("sEq", ad->sEq, 2);
                {
                    v->write_object(&ad->sEq[0]);
                    v->write_object(&ad->sEq[1]);
                }
                v->end_array();

                v->begin_array("sBypass", ad->sBypass, 2);
                {
                    v->write_object(&ad->sBypass[0]);
                    v->write_object(&ad->sBypass[1]);
                }
                v->end_array();

                v->write_object("sOutOfRange", &ad->sOutOfRange);
                v->write_object("sFeedOutRange", &ad->sFeedOutRange);
                v->write("pAllocator", &ad->pAllocator);

                v->write("bStereo", ad->bStereo);
                v->write("bOn", ad->bOn);
                v->write("bSolo", ad->bSolo);
                v->write("bMute", ad->bMute);
                v->write("bUpdated", ad->bUpdated);
                v->write("bValidRef", ad->bValidRef);
                v->write("nDelayRef", ad->nDelayRef);
                v->write("fOutDelay", ad->fOutDelay);
                v->write("fOutFeedback", ad->fOutFeedback);
                v->write("fOutTempo", ad->fOutTempo);
                v->write("fOutFeedTempo", ad->fOutFeedTempo);
                v->write("fOutDelayRef", ad->fOutDelayRef);

                dump_art_settings(v, "sOld", &ad->sOld);
                dump_art_settings(v, "sNew", &ad->sNew);

                v->write("pOn", ad->pOn);
                v->write("pTempoRef", ad->pTempoRef);
                v->writev("pPan", ad->pPan, 2);
                v->write("pSolo", ad->pSolo);
                v->write("pMute", ad->pMute);
                v->write("pDelayRef", ad->pDelayRef);
                v->write("pDelayMul", ad->pDelayMul);
                v->write("pBarFrac", ad->pBarFrac);
                v->write("pBarMul", ad->pBarMul);
                v->write("pFrac", ad->pFrac);
                v->write("pDenom", ad->pDenom);
                v->write("pDelay", ad->pDelay);
                v->write("pEqOn", ad->pEqOn);
                v->write("pLcfOn", ad->pLcfOn);
                v->write("pLcfFreq", ad->pLcfFreq);
                v->write("pHcfOn", ad->pHcfOn);
                v->write("pHcfFreq", ad->pHcfFreq);
                v->writev("pBandGain", ad->pBandGain, meta::art_delay_metadata::EQ_BANDS);
                v->write("pGain", ad->pGain);

                v->write("pFeedOn", ad->pFeedOn);
                v->write("pFeedGain", ad->pFeedGain);
                v->write("pFeedTempoRef", ad->pFeedTempoRef);
                v->write("pFeedBarFrac", ad->pFeedBarFrac);
                v->write("pFeedBarDenom", ad->pFeedBarDenom);
                v->write("pFeedBarMul", ad->pFeedBarMul);
                v->write("pFeedFrac", ad->pFeedFrac);
                v->write("pFeedDenom", ad->pFeedDenom);
                v->write("pFeedDelay", ad->pFeedDelay);

                v->write("pOutDelay", ad->pOutDelay);
                v->write("pOutFeedback", ad->pOutFeedback);
                v->write("pOutOfRange", ad->pOutOfRange);
                v->write("pOutFeedRange", ad->pOutFeedRange);
                v->write("pOutLoop", ad->pOutLoop);
                v->write("pOutTempo", ad->pOutTempo);
                v->write("pOutFeedTempo", ad->pOutFeedTempo);
                v->write("pOutDelayRef", ad->pOutDelayRef);
            }
            v->end_object();
        }

        void art_delay::dump(dspu::IStateDumper *v) const
        {
            plug::Module::dump(v);

            v->write("bStereoIn", bStereoIn);
            v->write("bMono", bMono);
            v->write("nMaxDelay", nMaxDelay);
            dump_pan(v, "sOldDryPan", sOldDryPan, 2);
            dump_pan(v, "sNewDryPan", sNewDryPan, 2);

            v->begin_array("vTempo", vTempo, meta::art_delay_metadata::MAX_TEMPOS);
            {
                for (size_t i=0; i<meta::art_delay_metadata::MAX_TEMPOS; ++i)
                    dump_art_tempo(v, &vTempo[i]);
            }
            v->end_array();
            v->begin_array("vDelays", vDelays, meta::art_delay_metadata::MAX_PROCESSORS);
            {
                for (size_t i=0; i<meta::art_delay_metadata::MAX_PROCESSORS; ++i)
                    dump_art_delay(v, &vDelays[i]);
            }

            v->end_array();
            v->writev("vOutBuf", vOutBuf, 2);
            v->write("vGainBuf", vGainBuf);
            v->write("vDelayBuf", vDelayBuf);
            v->write("vFeedBuf", vFeedBuf);
            v->write("vTempBuf", vTempBuf);
            v->write("nMemUsed", nMemUsed);

            v->begin_array("sBypass", sBypass, 2);
            {
                v->write_object(&sBypass[0]);
                v->write_object(&sBypass[1]);
            }
            v->end_array();
            v->write("pExecutor", pExecutor);


            v->writev("pIn", pIn, 2);
            v->writev("pOut", pOut, 2);
            v->write("pBypass", pBypass);
            v->write("pMaxDelay", pMaxDelay);
            v->writev("pPan", pPan, 2);
            v->write("pDryGain", pDryGain);
            v->write("pWetGain", pWetGain);
            v->write("pDryWet", pDryWet);
            v->write("pDryOn", pDryOn);
            v->write("pWetOn", pWetOn);
            v->write("pMono", pMono);
            v->write("pFeedback", pFeedback);
            v->write("pFeedGain", pFeedGain);
            v->write("pOutGain", pOutGain);
            v->write("pOutDMax", pOutDMax);
            v->write("pOutMemUse", pOutMemUse);

            v->write("pData", pData);
        }
    } /* namespace plugins */
} /* namespace lsp */



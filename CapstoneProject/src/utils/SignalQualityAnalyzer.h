#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>

class SignalQualityAnalyzer_C {
    public:
        static constexpr int NUM_CHANNELS = 8;

        void update(const float* samples, size_t scans) {
            std::vector<float> ch[NUM_CHANNELS];

            for (size_t s = 0; s<scans; s++){
                for (int c=0; c<NUM_CHANNELS; c++){
                    ch[c].push_back(samples[s*NUM_CHANNELS + c]);
                }
            }

            for (int c=0; c<NUM_CHANNELS; c++){
                float ch_rms = rms(ch[c]);
                bool ch_flat = isFlat(ch[c]);
                bool ch_spike = isSpiky(ch[c]);
                quality[c] = (ch_rms > 2.0f && ch_rms < 80.0f) && !ch_flat && !ch_spike;
            }
        }
        const std::array<bool, NUM_CHANNELS>& getQuality() const {
            return quality;
        }

    private:
    std::array<bool, NUM_CHANNELS> quality;

    float rms(const std::vector<float>& x){
        float s = 0;
        for (float v : x) s += v*v;
        return std::sqrt(s/x.size());
    }

    bool isFlat(const std::vector<float>& x){
        float min_v = *std::min_element(x.begin(), x.end());
        float max_v = *std::max_element(x.begin(), x.end());
        return (max_v-min_v) < 1.0f;
    }

    bool isSpiky(const std::vector<float>& x){
        float prev = x[0];
        for (float v : x){
            if (fabs(v-prev)>100.0f)
                return true;
            prev = v;
        }
        return false;
    }
};

#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

class AlbedoRearField
{
public:
    void prepare(double sampleRate)
    {
        const double times[] = {0.0137, 0.0211};
        for (unsigned ch=0; ch<2; ++ch)
            lines[ch].assign(std::max(1, int(std::round(sampleRate * times[ch]))), 0.0);
        head = {};
        low = {};
        bassPole = std::exp(-2.0 * 3.141592653589793 * 155.0 / sampleRate);
    }

    template<typename T>
    void render(const T* left, const T* right, T* rearL, T* rearR, int samples) noexcept
    {
        for (int i=0; i<samples; ++i)
        {
            const double signals[] = {0.76*left[i]+0.24*right[i], 0.76*right[i]+0.24*left[i]};
            T* outputs[] = {rearL, rearR};
            for (unsigned ch=0; ch<2; ++ch)
            {
                low[ch] += (1.0-bassPole)*(signals[ch]-low[ch]);
                const auto input = signals[ch]-low[ch];
                auto& tap = lines[ch][head[ch]];
                const auto diffuse = tap - 0.38*input;
                tap = input + 0.38*diffuse;
                outputs[ch][i] = static_cast<T>(0.28*diffuse);
                if (++head[ch] == lines[ch].size()) head[ch]=0;
            }
        }
    }
private:
    std::array<std::vector<double>,2> lines;
    std::array<size_t,2> head{};
    std::array<double,2> low{};
    double bassPole = 0.0;
};

#pragma once

#include <cassert>
#include <vector>
#include <cmath>

struct RMSMeter
{
    using millisec = int;
    
private:
    size_t     sampling_rate_;
    double     square_sum_;
    millisec   duration_;
    
    std::vector<double> history_;
    int hist_index_;
    
public:
    RMSMeter(size_t sampling_rate, millisec duration = 300)
    :   sampling_rate_(sampling_rate)
    ,   duration_(duration)
    ,   square_sum_(0)
    {
        ResetImpl();
        assert(history_.size() > 0);
    }
    
    size_t    GetSamplingRate    () const { return sampling_rate_; }
    
    //! 現在のRMS値を取得する
    double    GetRMS            () const
    {
        assert(history_.size() > 0);
        return sqrt(square_sum_ / (double)history_.size());
    }
    
    //! RMS値を計測するための時間幅
    millisec   GetDuration () const { return duration_; }
    
public:
    void    SetSamplingRate    (size_t sampling_rate)
    {
        sampling_rate_ = sampling_rate;
        ResetImpl();
    }
    
    void SetDuration(millisec duration)
    {
        duration_ = duration;
        ResetImpl();
    }
    
public:
    void    PushSquaredSample(double squared_sample)
    {
        assert(history_.size() > 0);
        
        auto &x = history_[hist_index_];
        square_sum_ -= x;
        x = squared_sample;
        square_sum_ += x;
        square_sum_ = std::max<double>(square_sum_, 0);
        hist_index_ = (hist_index_ + 1) % history_.size();
    }
    
    void    PushSample    (double sample)
    {
        PushSquaredSample(sample * sample);
    }
    
    void    Consume(size_t n)
    {
        assert(history_.size() > 0);
        
        for(int i = 0; i < n; ++i) {
            auto &x = history_[hist_index_];
            square_sum_ -= x;
            x = 0;
            hist_index_ = (hist_index_ + 1) % history_.size();
        }
    }
    
private:
    void ResetImpl()
    {
        history_.resize(std::round(sampling_rate_ * duration_ / 1000.0)); // 400ms
        std::fill(history_.begin(), history_.end(), 0.0);
        square_sum_ = 0;
        hist_index_ = 0;
    }
};


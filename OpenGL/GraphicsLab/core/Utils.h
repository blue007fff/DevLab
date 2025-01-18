#pragma once

#include "Common.h"

namespace utils
{
    std::filesystem::path GetExecutablePath();

    class UpdateTimer
    {
    public:
        UpdateTimer(double update_time) : m_update_time{ update_time } {}
        bool CadUpdate() {
            auto tp = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> elaseped_time = tp - m_tp;
            m_elasped_time = elaseped_time.count();
            double time = m_prev_elasped_time + m_elasped_time;
            if (time >= m_update_time)
            {
                m_prev_elasped_time = time - m_update_time;
                m_tp = tp;
                return true;
            }
            return false;
        }
        double ElaspedTime() const { return m_elasped_time; }

    private:
        double m_elasped_time = 0.0;
        double m_update_time = 1.0;
        double m_prev_elasped_time = 0.0;
        std::chrono::high_resolution_clock::time_point m_tp{
            std::chrono::high_resolution_clock::now() };
    };
    class FPSCounter
    {
    public:
        bool update() {
            m_frame_count++;
            auto tp = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elasepd_time = tp - m_tp;
            if (elasepd_time.count() > m_update_time)
            {
                m_fps = m_frame_count / elasepd_time.count();
                m_frame_count = 0;
                m_tp = tp;
                return true;
            }
            return false;
        }
        double fps() const { return m_fps; }
    private:
        double m_update_time = 1.0;
        double m_fps = 0.0;
        int m_frame_count{ 0 };
        std::chrono::high_resolution_clock::time_point m_tp{
            std::chrono::high_resolution_clock::now() };
    };

    template <class Rep, class Period>
    void delay(const std::chrono::duration<Rep, Period>& delay_duration)
    {
        using namespace std::chrono_literals;

        bool use_sleep = true;
        const auto start = std::chrono::high_resolution_clock::now();
        for (;;)
        {
            const auto time_elapsed = std::chrono::high_resolution_clock::now() - start;
            if (delay_duration <= time_elapsed)
            {
                return;
            }
            if (use_sleep)
            {
                const auto time_remaining = delay_duration - time_elapsed;
                if (time_remaining < 100ms)
                {
                    use_sleep = false;
                }
                else
                {
                    std::this_thread::sleep_for(time_remaining / 2);
                }
            }
        }

    }
}

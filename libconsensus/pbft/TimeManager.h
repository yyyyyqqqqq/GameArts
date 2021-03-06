
#pragma once
#include <libdevcore/Common.h>
namespace dev
{
namespace consensus
{
struct TimeManager
{
    /// the max block generation time
    unsigned m_emptyBlockGenTime = 1000;

    /// last execution finish time, only one will be used at last
    /// the finish time of executing tx by leader
    uint64_t m_viewTimeout = 3 * m_emptyBlockGenTime;
    unsigned m_changeCycle = 0;
    uint64_t m_lastSignTime = 0;
    uint64_t m_lastConsensusTime;
    /// the minimum block generation time(default is 500ms)
    unsigned m_minBlockGenTime = 500;

    /// time point of last signature collection
    std::chrono::steady_clock::time_point m_lastGarbageCollection;
    const unsigned kMaxChangeCycle = 20;
    const unsigned CollectInterval = 60;

    inline void initTimerManager(unsigned view_timeout)
    {
        m_lastConsensusTime = utcSteadyTime();
        m_lastSignTime = 0;
        m_viewTimeout = view_timeout;
        m_changeCycle = 0;
        m_lastGarbageCollection = std::chrono::steady_clock::now();
    }

    inline void changeView()
    {
        m_lastConsensusTime = 0;
        m_lastSignTime = 0;
    }

    inline void updateChangeCycle()
    {
        m_changeCycle = std::min(m_changeCycle + 1, (unsigned)kMaxChangeCycle);
    }

    inline bool isTimeout()
    {
        auto now = utcSteadyTime();
        auto last = std::max(m_lastConsensusTime, m_lastSignTime);
        auto interval = (uint64_t)(m_viewTimeout * std::pow(1.5, m_changeCycle));
        return (now - last >= interval);
    }
};
}  // namespace consensus
}  // namespace dev

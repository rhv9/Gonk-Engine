//
// Basic instrumentation profiler by Cherno

// Usage: include this header file somewhere in your code (eg. precompiled header), and then use like:
//
// Instrumentor::Get().BeginSession("Session Name");        // Begin session 
// {
//     InstrumentationTimer timer("Profiled Scope Name");   // Place code like this in scopes you'd like to include in profiling
//     // Code
// }
// Instrumentor::Get().EndSession();                        // End Session
//
// You will probably want to macro-fy this, to switch on/off easily and use things like __FUNCSIG__ for the profile name.
//
#pragma once

#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <mutex>

#include <thread>

struct ProfileResult
{
    std::string Name;
    long long Start, End;
    size_t ThreadID;
};

struct InstrumentationSession
{
    std::string Name;
};

namespace Gonk {

    class Instrumentor
    {

    public:

        ~Instrumentor()
        {
            EndSession();
        }

        void BeginSession(const std::string& name, const std::string& filepath = "results.json")
        {
            // check directory and create
            std::filesystem::path directoryPath = std::filesystem::path(filepath).parent_path();

            if (!std::filesystem::exists(directoryPath)) {
                if (!std::filesystem::create_directories(directoryPath)) {
                    GK_CORE_ERROR("Failed to create directory for profiling path!");
                }
            }

            m_OutputStream.open(filepath);
            GK_CORE_ASSERT(m_OutputStream.is_open(), "Failed to open profiling filepath");
            WriteHeader();
            m_CurrentSession = new InstrumentationSession{ name };
        }

        void EndSession()
        {
            WriteFooter();
            m_OutputStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }

        void WriteProfile(const ProfileResult& result)
        {
            std::lock_guard<std::mutex> lock(m_lock);

            if (m_ProfileCount++ > 0)
                m_OutputStream << ",";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":" << result.ThreadID << ",";
            m_OutputStream << "\"ts\":" << result.Start;
            m_OutputStream << "}";

            m_OutputStream.flush();
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
            m_OutputStream.flush();
        }

        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        static Instrumentor& Get()
        {
            static Instrumentor instance;
            return instance;
        }

    private:
        Instrumentor()
            : m_CurrentSession(nullptr), m_ProfileCount(0) {}

    private: 
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;

        std::mutex m_lock;
    };

    class InstrumentationTimer
    {
    public:
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_Result.Name = name;
            m_StartTimepoint = std::chrono::high_resolution_clock::now();
        }

        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            m_Result.Start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
            m_Result.End = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

            m_Result.ThreadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
            Instrumentor::Get().WriteProfile(m_Result);

            m_Stopped = true;
        }
    private:
        ProfileResult m_Result;
        const char* m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
        bool m_Stopped;
    };
}

//#define GK_PROFILE
#ifdef GK_PROFILE

    // Resolve which function signature macro will be used. Note that this only
    // is resolved when the (pre)compiler starts, so the syntax highlighting
    // could mark the wrong one in your editor!
#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
#define GK_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
#define GK_FUNC_SIG __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#define GK_FUNC_SIG __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
#define GK_FUNC_SIG __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
#define GK_FUNC_SIG __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
#define GK_FUNC_SIG __func__
#elif defined(__cplusplus) && (__cplusplus >= 201103)
#define GK_FUNC_SIG __func__
#else
#define GK_FUNC_SIG "GK_FUNC_SIG unknown!"
#endif

#define GK_PROFILE_BEGIN_SESSION(name, filepath) ::Gonk::Instrumentor::Get().BeginSession(name, filepath)
#define GK_PROFILE_END_SESSION() ::Gonk::Instrumentor::Get().EndSession()
#define GK_PROFILE_SCOPE(name) ::Gonk::InstrumentationTimer timer##__LINE__(name);
#define GK_PROFILE_FUNCTION() GK_PROFILE_SCOPE(GK_FUNC_SIG)

#else

#define GK_PROFILE_BEGIN_SESSION(name, filepath)
#define GK_PROFILE_END_SESSION()
#define GK_PROFILE_SCOPE(name)
#define GK_PROFILE_FUNCTION()

#endif
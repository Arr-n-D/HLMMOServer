#include <Core.hpp>
#include <filesystem>

#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

/// Checks if the current process is running inside a Docker container
bool IsInsideDocker() {
#ifdef defined( _WIN32 ) || defined( WIN32 )
    return false;
#else
    return std::filesystem::exists( "/.dockerenv" );
#endif
}

/// Sets up spdlog
/// spdlog will then log to STDOUT and to a daily rotating file inside the logs directory.
void SetupLogging() {
    spdlog::init_thread_pool( 8192, 1 );

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>( "logs/daily.txt", 0, 0 );

    if ( IsInsideDocker() ) {
        // Enforce colors for spdlog inside Docker
        stdout_sink->set_color_mode( spdlog::color_mode::always );
    }

    // Create the multi-sink which contains the STDOUT + daily logger
    std::vector<spdlog::sink_ptr> sinks{ stdout_sink, daily_sink };

    auto logger = std::make_shared<spdlog::async_logger>( "default", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::block );

    spdlog::set_default_logger( logger );

    // Write every log message immediately
    spdlog::flush_on( spdlog::level::trace );

#ifdef DEBUG
    // Set the logging level to "trace" when doing debug builds
    spdlog::set_level( spdlog::level::trace );
#endif
}

int main() {
    SetupLogging();

    Core core;
    core.Initialize();

    return 0;
}

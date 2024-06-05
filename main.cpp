#include <Core.hpp>

#include "spdlog/async.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main() {
    spdlog::init_thread_pool( 8192, 1 );

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto daily_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>( "logs/daily.txt", 0, 0 );

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

    Core core;
    core.Initialize();

    return 0;
}

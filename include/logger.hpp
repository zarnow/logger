#include <format>
#include <atomic>
#include <iostream>
#include <fstream>
#include <string_view>
#include <chrono>
#include <filesystem>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace logger
{
    inline bool log_to_console{true};
    inline std::string log_directory{"logs"};
    inline std::string log_file_prefix{"LOG"};
    inline std::string log_file_mask{"{:%Y%m%d}"};
}

template <typename... Args>
void log(std::format_string<Args...> fmt, Args&&... args) 
{
    using namespace std::chrono;

    struct LogEvent
    {
        zoned_time<microseconds> zt;
        std::string message;
    };
    
    static std::queue<LogEvent> s_queue;
    static std::mutex s_mutex;
    static std::condition_variable_any s_cv;
    
    
    static std::jthread s_thread([](std::stop_token st)
    {
        std::ofstream log_file;
        std::string active_date;
        std::error_code directory_error;
        std::filesystem::create_directories(logger::log_directory, directory_error);
        if (directory_error)
            std::cerr << "Failed to create logs directory: " << directory_error.message() << '\n';
        
        for(;;) // released when all logs written and stop_requested
		{
            std::queue<LogEvent> local_queue;
            {
                std::unique_lock lock{s_mutex};
                
                s_cv.wait(lock, st, [&] { return !s_queue.empty(); });
                
                if (s_queue.empty() && st.stop_requested()) 
                    break;
                
                std::swap(s_queue, local_queue);
            }
            
            
            while (!local_queue.empty()) 
			{
                const auto& [event_zt, event_message] = local_queue.front();
                const std::string event_date = std::vformat(logger::log_file_mask, std::make_format_args(event_zt));
                if (event_date != active_date)
                {
                    if (log_file.is_open())
                        log_file.close();

                    const auto log_path = std::filesystem::path(logger::log_directory) / std::format("{}{}.txt", logger::log_file_prefix, event_date);
                    log_file.open(log_path, std::ios::app);
                    if (!log_file.is_open())
                        std::cerr << "Failed to open    log file: " << log_path.string() << '\n';
                    active_date = event_date;
                }

                std::string log_line = std::format("[{}] - {}\n", std::format("{:%Y-%m-%d %H:%M:%S}", event_zt), event_message);
                
                if (logger::log_to_console)
                    std::cout << log_line;
                
                if (log_file.is_open())
                    log_file << log_line;
                
                local_queue.pop();
            }
            
            
            if (log_file.is_open())
               log_file.flush();
        }
    });
    
    
    zoned_time<microseconds> zt{current_zone(), time_point_cast<microseconds>(system_clock::now())};
    std::string message = std::vformat(fmt.get(), std::make_format_args(args...));
    
    {
        std::scoped_lock lock{s_mutex};
        s_queue.emplace(LogEvent{std::move(zt), std::move(message)});
    }
    
    s_cv.notify_one(); 
}
#include <cstdio>
#include <cstdarg>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <windows.h>

void LOG(const char* fmt, ...) {
    struct LOGEntry {
        SYSTEMTIME st;
        std::string message;
    };

    static std::queue<LOGEntry> log_queue;
    static std::mutex log_mutex;
    static std::condition_variable_any log_cv;
    static std::jthread log_thread([](std::stop_token stop_token) {
        FILE* file = nullptr;
        if (fopen_s(&file, "log.txt", "a") != 0)
            std::fprintf(stderr, "Failed to open log.txt for writing\n");

        for (;;) {
            std::queue<LOGEntry> pending_logs;
            {
                std::unique_lock lock(log_mutex);
                log_cv.wait(lock, stop_token, [] { return !log_queue.empty(); });

                if (log_queue.empty() && stop_token.stop_requested())
                    break;

                std::swap(log_queue, pending_logs);
            }

            while (!pending_logs.empty()) {
                const LOGEntry& entry = pending_logs.front();
                char timestamp[64];
                int timestamp_length = std::snprintf(
                    timestamp,
                    sizeof(timestamp),
                    "[%04d-%02d-%02d %02d:%02d:%02d.%03d] - ",
                    entry.st.wYear,
                    entry.st.wMonth,
                    entry.st.wDay,
                    entry.st.wHour,
                    entry.st.wMinute,
                    entry.st.wSecond,
                    entry.st.wMilliseconds);
                std::string log_line(timestamp, static_cast<std::size_t>(timestamp_length));
                log_line += entry.message;
                log_line += "\r\n";
                // std::printf("%s", log_line.c_str());

                if (file)
                    std::fwrite(log_line.data(), 1, log_line.size(), file);

                pending_logs.pop();
            }

            if (file)
                std::fflush(file);
        }

        if (file)
            std::fclose(file);
    });

    SYSTEMTIME st;
    GetLocalTime(&st);

    char buffer[4096];
    std::va_list args;
    va_start(args, fmt);
    int msg_len = std::vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (msg_len < 0) return;

    {
        std::scoped_lock lock(log_mutex);
        log_queue.emplace(LOGEntry{std::move(st), std::move(buffer)});//, static_cast<std::size_t>(msg_len) < sizeof(buffer) ? static_cast<std::size_t>(msg_len) : sizeof(buffer) - 1)});
    }

    log_cv.notify_one();
}

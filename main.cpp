#include "include/logger.hpp"
#include "include/log.hpp"

int main() 
{
    log("running application...");

    logger::log_to_console = false;
    constexpr std::size_t attempts = 100'000;
    const auto start = std::chrono::steady_clock::now();

    for (std::size_t i = 0; i < attempts; ++i)
        log("12345678901234");
        //log("#{} Value of variable X: {}, and Y: {:.2f}",i, 42, 3.14159);

    const auto elapsed = std::chrono::steady_clock::now() - start;
    const double total_us = std::chrono::duration<double, std::micro>(elapsed).count();
    std::cout << std::format("Time for {} log() calls: {:.2f} us; average: {:.4f} us/call\n", attempts, total_us, total_us / attempts);

    const auto start2 = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < attempts; ++i)
        LOG("12345678901234");
       
    const auto elapsed2 = std::chrono::steady_clock::now() - start2;
    const double total_us2 = std::chrono::duration<double, std::micro>(elapsed2).count();
    std::cout << std::format("Time for {} LOG() calls: {:.2f} us; average: {:.4f} us/call\n", attempts, total_us2, total_us2 / attempts);

    return 0; 
}


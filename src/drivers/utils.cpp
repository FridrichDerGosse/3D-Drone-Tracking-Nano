#include "utils.hpp"


void utils::restart() {
    wdt_enable(WDTO_15MS);  // Enable watchdog timer with a 15ms timeout
    for (;;);              // Wait for the WDT to trigger a reset
}

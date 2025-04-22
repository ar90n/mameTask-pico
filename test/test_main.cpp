// Include the test framework
#include "utest.h"

// Include the platform abstraction layer
#include "platform.h"

// Include the library header
#include "../src/mameTaskPico.hpp"

// Define the global utest state
UTEST_STATE();

// Define the main function
int main(int argc, const char* const argv[]) {
    // Initialize platform-specific resources
    test_platform::init();
    
    // Print platform information
    #ifdef PLATFORM_DEVICE
    printf("\n=== Running mameTask-pico tests on DEVICE ===\n\n");
    #else
    printf("\n=== Running mameTask-pico tests on HOST ===\n\n");
    #endif
    
    // Run the tests
    int result = utest_main(argc, argv);
    
    // Clean up platform-specific resources
    test_platform::cleanup();
    
    return result;
}

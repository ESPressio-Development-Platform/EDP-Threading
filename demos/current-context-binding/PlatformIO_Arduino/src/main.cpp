#include <Arduino.h>
#include "Example.hpp"

/// Runs the complete statically owned example after framework startup.
void setup() { Example::Run(); }

/// Leaves the demonstration idle after its one-shot result.
void loop() { delay(1000); }

#include "d_internal.h"
#include "DgnEngine/DgnEngine.h"

#include <MemLeaker/malloc.h>

void dgnEngineTerminate()
{
    glfwTerminate();
    printMemUsage();
}

double dgnEngineGetSeconds()
{
    return glfwGetTime();
}

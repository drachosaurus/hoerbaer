#include <Arduino.h>
#include <cstdarg>
#include "log.h"

SemaphoreHandle_t logSema;

void Log::init()
{
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    
#if ARDUINO_USB_CDC_ON_BOOT
    usleep(300 * 1000); // give usb serial some time to connect (switch to CDC)
#endif
    logSema = xSemaphoreCreateBinary();
    xSemaphoreGive(logSema);
}

void Log::println(const char * module, const char * fmt, ...) 
{
    va_list va;
    va_start (va, fmt);
    char buf[255];
    vsprintf(buf, fmt, va);
    va_end (va);
    xSemaphoreTake(logSema, portMAX_DELAY);
    Serial.printf("%s\t%s\n", module, buf);
    xSemaphoreGive(logSema);
}

void Log::logCurrentHeap(const char * text) 
{
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();

    xSemaphoreTake(logSema, portMAX_DELAY);
    Serial.printf("MEMORY\t%s\tHeap - %u bytes / %u bytes (%.1f%% free)\n", 
        text, freeHeap, totalHeap, (float)freeHeap * 100.0 / totalHeap);
    xSemaphoreGive(logSema);
}

void Log::printMemoryInfo() 
{
    xSemaphoreTake(logSema, portMAX_DELAY);

    // Heap information
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t minFreeHeap = ESP.getMinFreeHeap();
    uint32_t maxHeapBlock = ESP.getMaxAllocHeap();
    
    Serial.println("------ Memory Information ------");
    Serial.printf("Heap - Free: %u bytes, Total: %u bytes (%.1f%% free)\n", 
                    freeHeap, totalHeap, (float)freeHeap * 100.0 / totalHeap);
    Serial.printf("Heap - Min Free Ever: %u bytes, Max Block: %u bytes\n", 
                    minFreeHeap, maxHeapBlock);
    
    // PSRAM information if available
    if (psramFound()) {
        uint32_t freePSRAM = ESP.getFreePsram();
        uint32_t totalPSRAM = ESP.getPsramSize();
        uint32_t minFreePSRAM = ESP.getMinFreePsram();
        uint32_t maxPSRAMBlock = ESP.getMaxAllocPsram();
        
        Serial.printf("PSRAM - Free: %u bytes, Total: %u bytes (%.1f%% free)\n", 
                    freePSRAM, totalPSRAM, (float)freePSRAM * 100.0 / totalPSRAM);
        Serial.printf("PSRAM - Min Free Ever: %u bytes, Max Block: %u bytes\n", 
                    minFreePSRAM, maxPSRAMBlock);
    } 
    else
        Serial.println("PSRAM not found or not enabled");
        
    Serial.println("--------------------------------");

    xSemaphoreGive(logSema);
}

void Log::printTaskInfo()
{
    // FreeRTOS Task Information
    Serial.println("\n------ FreeRTOS Task Information ------");
    Serial.println("Task Name\tState\tPrio\tStack\tNum\tCore");

    // Iterate through tasks
    UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();
    TaskStatus_t *pxTaskStatusArray = (TaskStatus_t*)malloc(uxArraySize * sizeof(TaskStatus_t));
    
    if (pxTaskStatusArray != NULL) {
        // Generate raw status information about each task
        uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);
        
        // Print task information
        for (UBaseType_t i = 0; i < uxArraySize; i++) {
            TaskStatus_t task = pxTaskStatusArray[i];
            Serial.printf("%-16s%u\t%u\t%u\t%u\t%d\n",
                        task.pcTaskName,
                        task.eCurrentState,
                        task.uxCurrentPriority,
                        task.usStackHighWaterMark * 4,  // High water mark in bytes (4 bytes per word on ESP32)
                        task.xTaskNumber,
                        task.xCoreID);
        }
        
        free(pxTaskStatusArray);
    } 
    else
        Serial.println("Memory allocation failed for task status array");
    
    Serial.println("--------------------------------");
}

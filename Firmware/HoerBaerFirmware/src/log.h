#pragma once

class Log {
    public:
        static void init();
        // static void println(const char * fmt);
        static void println(const char * module, const char * fmt, ...);
        static void logCurrentHeap(const char * text);
        static void printMemoryInfo();
        static void printTaskInfo();
};
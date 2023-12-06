#pragma once

class Log {
    public:
        static void init();
        // static void println(const char * fmt);
        static void println(const char * fmt, ...);
};
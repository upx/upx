// Copyright (C) Markus Franz Xaver Johannes Oberhumer

#include <mutex>

static std::mutex lock_mutex;

int main() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return 0;
}

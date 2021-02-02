#ifndef PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP
#define PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP

#include <string>
#include <exception>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>

#ifdef __unix__
#   define OS_UNIX
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#   define OS_WINDOWS
#endif

#ifdef OS_WINDOWS

#   include <windows.h>

#elif defined(OS_UNIX)

#   include <semaphore.h>
#   include <fcntl.h>

#   define PERM 0600

#else

#   error "shared_mutex is only supported on windows and unix systems"

#endif

class shared_mutex_exception : public std::exception {
public:
    explicit shared_mutex_exception(std::string error) : std::exception(), error(std::move(error)) {}

    [[nodiscard]] const char *what() const noexcept override {
        return error.c_str();
    }

private:
    const std::string error;
};

class shared_mutex {
public:
    explicit shared_mutex(std::string mtx_name) : prog_name(std::move(mtx_name)) {
        if (std::find(owned_mutexes.begin(), owned_mutexes.end(), prog_name) != owned_mutexes.end()) {
            throw shared_mutex_exception("A mutex with the name '" + prog_name + "' is already owned by this program");
        }

#ifdef OS_WINDOWS
        std::string name = "Local\\";
        name.append(prog_name);

        HANDLE mutex = CreateMutex(nullptr, false, name.c_str());

        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            throw shared_mutex_exception(
                    "A mutex with the name '" + prog_name + "' is already owned by another program");
        } else {
            this->handle = mutex;
            owned_mutexes.push_back(prog_name);
        }
#else
        std::string name = "/";
        name.append(prog_name);

        sem_t *semaphore = sem_open(name.c_str(), O_CREAT | O_EXCL, PERM, 0);
        if (semaphore == SEM_FAILED) {
            throw shared_mutex_exception("A mutex with the name '" + prog_name + "' is already owned by another program");
        } else {
            this->semaphore = semaphore;
            owned_mutexes.push_back(prog_name);
        }
#endif //OS_WINDOWS
    }

#ifdef OS_WINDOWS

    shared_mutex(shared_mutex &&rhs) noexcept: handle(rhs.handle), prog_name(std::move(rhs.prog_name)) {
        rhs.handle = nullptr;
    }

#else

    shared_mutex(shared_mutex &&rhs) noexcept: semaphore(rhs.semaphore), prog_name(std::move(rhs.prog_name)) {
        rhs.semaphore = nullptr;
    }

#endif //OS_WINDOWS

    ~shared_mutex() {
#ifdef OS_WINDOWS
        if (handle == nullptr) return;

        if (!CloseHandle(handle)) {
            std::cerr << "Could not close the handle to the mutex" << std::endl;
        }
#else
        if (semaphore == nullptr) return;

        std::string name = "/";
        name.append(prog_name);

        if (sem_close(semaphore) == -1) {
            std::cerr << "Could not close the semaphore" << std::endl;
        } else if (sem_unlink(name.c_str()) == -1) {
            std::cerr << "Could not delete the semaphore" << std::endl;
        }
#endif //OS_WINDOWS

        try {
            if (std::find(owned_mutexes.begin(), owned_mutexes.end(), prog_name) != owned_mutexes.end()) {
                owned_mutexes.erase(std::find(owned_mutexes.begin(), owned_mutexes.end(), prog_name));
            }
        } catch (...) {}
    }

private:
    static inline std::vector<std::string> owned_mutexes = std::vector<std::string>();

    std::string prog_name;
#ifdef OS_WINDOWS
    HANDLE handle;
#else
    sem_t *semaphore;
#endif //OS_WINDOWS
};

#endif //PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP

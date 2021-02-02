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

/**
 * A shared mutex
 */
class shared_mutex {
public:
    /**
     * Create a shared_mutex instance.
     * Throws an exception if another shared_mutex with the same name already exists.
     *
     * @param mutex_name the mutex name
     */
    explicit shared_mutex(std::string mutex_name) : mtx_name(std::move(mutex_name)) {
        // If mtx_name already exists in owned_mutexes, throw an exception
        if (std::find(owned_mutexes.begin(), owned_mutexes.end(), mtx_name) != owned_mutexes.end()) {
            throw shared_mutex_exception("A mutex with the name '" + mtx_name + "' is already owned by this program");
        }

#ifdef OS_WINDOWS
        // Create the name for the mutex
        std::string name = "Local\\";
        name.append(mtx_name);

        // Try to create the mutex
        HANDLE mutex = CreateMutex(nullptr, false, name.c_str());

        // If the mutex already exists, throw an exception
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
            throw shared_mutex_exception(
                    "A mutex with the name '" + mtx_name + "' is already owned by another program");
        } else {
            // Set the handle to mutex and add the mutex name to owned_mutexes
            this->handle = mutex;
            owned_mutexes.push_back(mtx_name);
        }
#else
        // Create the name for the mutex
        std::string name = "/";
        name.append(mtx_name);

        // Try to create the semaphore
        sem_t *semaphore = sem_open(name.c_str(), O_CREAT | O_EXCL, PERM, 0);

        // If the creation failed, thrown an exception
        if (semaphore == SEM_FAILED) {
            throw shared_mutex_exception("A mutex with the name '" + mtx_name + "' is already owned by another program");
        } else {
            // Set the semaphore and add the mutex name to owned_mutexes
            this->semaphore = semaphore;
            owned_mutexes.push_back(mtx_name);
        }
#endif //OS_WINDOWS
    }

    /**
     * No copy constructor
     */
    shared_mutex(const shared_mutex &) = delete;

    /**
     * Move constructor
     *
     * @param rhs the shared_mutex to move
     */
    shared_mutex(shared_mutex &&rhs) noexcept: mtx_name(std::move(rhs.mtx_name)) {
        // Set the handle or semaphore of rhs to nullptr
#ifdef OS_WINDOWS
        this->handle = rhs.handle;
        rhs.handle = nullptr;
#else
        this->semaphore = rhs.semaphore;
        rhs.semaphore = nullptr;
#endif //OS_WINDOWS
    }

    /**
     * No copy assignment operator
     */
    shared_mutex &operator=(const shared_mutex &) = delete;

    /**
     * Move assignment operator
     *
     * @param rhs the instance to assign
     * @return this
     */
    shared_mutex &operator=(shared_mutex &&rhs) noexcept {
        // Swap all class members
        std::swap(rhs.mtx_name, this->mtx_name);
#ifdef OS_WINDOWS
        std::swap(rhs.handle, this->handle);
#else
        std::swap(rhs.semaphore, this->semaphore);
#endif //OS_WINDOWS

        return *this;
    }

    /**
     * Delete this shared_mutex.
     * Destroys the semaphore or mutex.
     */
    ~shared_mutex() {
#ifdef OS_WINDOWS
        // If the handle is null, return
        if (handle == nullptr) return;

        // Close the handle to the mutex
        if (!CloseHandle(handle)) {
            std::cerr << "Could not close the handle to the mutex" << std::endl;
        }
#else
        // If the semaphore is null, return
        if (semaphore == nullptr) return;

        // Re-create the semaphore name
        std::string name = "/";
        name.append(mtx_name);

        // Close and delete the semaphore
        if (sem_close(semaphore) == -1) {
            std::cerr << "Could not close the semaphore" << std::endl;
        } else if (sem_unlink(name.c_str()) == -1) {
            std::cerr << "Could not delete the semaphore" << std::endl;
        }
#endif //OS_WINDOWS

        // Try to delete the mutex name from the owned_mutexes vector, if it exists in it
        try {
            if (std::find(owned_mutexes.begin(), owned_mutexes.end(), mtx_name) != owned_mutexes.end()) {
                owned_mutexes.erase(std::find(owned_mutexes.begin(), owned_mutexes.end(), mtx_name));
            }
        } catch (...) {}
    }

private:
    static inline std::vector<std::string> owned_mutexes = std::vector<std::string>();

    // The mutex name
    std::string mtx_name;
#ifdef OS_WINDOWS
    // The handle to the mutex
    HANDLE handle;
#else
    // A pointer to the semaphore
    sem_t *semaphore;
#endif //OS_WINDOWS
};

#endif //PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP

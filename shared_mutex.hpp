#ifndef PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP
#define PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP

#include <string>
#include <exception>
#include <utility>
#include <vector>
#include <algorithm>
#include <iostream>

#if defined(__unix__) || defined(__linux__) || defined(__APPLE__)
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

/**
 * A shared mutex exception
 */
class shared_mutex_exception : public std::exception {
public:
    /**
     * Create a shared mutex exception
     *
     * @param error the error description
     */
    explicit shared_mutex_exception(std::string error) : std::exception(), error(std::move(error)) {}

    /**
     * Get the error message
     *
     * @return the error message
     */
    [[nodiscard]] const char *what() const noexcept override {
        return error.c_str();
    }

private:
    // The error
    const std::string error;
};

/**
 * A shared mutex
 */
class shared_mutex {
public:
    /**
     * Create a shared_mutex instance.
     *
     * @param mutex_name the mutex name
     * @param openIfExists whether to open if the mutex already exists or throw an exception
     * @return a pointer to the mutex
     */
    static std::unique_ptr<shared_mutex> createShared_mutex(const std::string &mtx_name, bool openIfExists);

    /**
     * Create a shared_mutex instance.
     *
     * @param mutex_name the mutex name
     * @param openIfExists whether to open the mutex if it already exists or throw an exception
     */
    explicit shared_mutex(std::string mutex_name, bool openIfExists) : _mtx_name(std::move(mutex_name)), _locked(false),
                                                                       _unique(!openIfExists) {
        // If mtx_name already exists in owned_mutexes, throw an exception
        if (_unique && std::find(_owned_mutexes.begin(), _owned_mutexes.end(), _mtx_name) != _owned_mutexes.end()) {
            throw shared_mutex_exception("A mutex with the name '" + _mtx_name + "' is already owned by this program");
        }
    }

    shared_mutex(shared_mutex &&) = delete;

    /**
     * No copy constructor
     */
    shared_mutex(const shared_mutex &) = delete;

    /**
     * No copy assignment operator
     */
    shared_mutex &operator=(const shared_mutex &) = delete;

    /**
     * No move assignment operator
     */
    shared_mutex &operator=(shared_mutex &&) noexcept = delete;

    /**
     * Lock the mutex
     */
    virtual void lock() = 0;

    /**
     * Unlock the mutex
     */
    virtual void unlock() = 0;

    /**
     * Try locking the mutex
     *
     * @return true, if the ownership could be acquired
     */
    [[nodiscard]] virtual bool try_lock() = 0;

    /**
     * Delete the shared_mutex instance
     */
    virtual ~shared_mutex() = default;

protected:
    // A vector containing all mutexes owned by this program
    static inline std::vector<std::string> _owned_mutexes = std::vector<std::string>();

    /**
     * Try to remove the mutex name from the owned_mutexes vector
     */
    void try_remove_mutex() {
        // Try to delete the mutex name from the owned_mutexes vector, if it exists in it
        try {
            if (_unique && std::find(_owned_mutexes.begin(), _owned_mutexes.end(), _mtx_name) != _owned_mutexes.end()) {
                _owned_mutexes.erase(std::find(_owned_mutexes.begin(), _owned_mutexes.end(), _mtx_name));
            }
        } catch (...) {}
    }

    // The mutex name
    std::string _mtx_name;
    // Whether this instance owns the mutex
    bool _locked;
    // Whether this mutex should be unique
    bool _unique;
};

#ifdef OS_WINDOWS

/**
 * A shared mutex for windows
 */
class win_shared_mutex : public shared_mutex {
public:
    /**
     * Create a shared_mutex instance.
     *
     * @param mutex_name the mutex name
     * @param openIfExists whether to open the mutex if it already exists or throw an exception
     */
    win_shared_mutex(const std::string &mutex_name, bool openIfExists) : shared_mutex(mutex_name, openIfExists) {
        // Create the name for the mutex
        std::string name = "Local\\";
        name.append(_mtx_name);

        // Set the last error to zero
        SetLastError(0);

        // Try to create the mutex
        HANDLE sem = CreateSemaphoreA(nullptr, 1, 1, name.c_str());

        // If the mutex creation failed, throw an exception.
        // If the mutex already exists and the mutex should not be opened
        // if it already exists, throw an exception.
        if (sem == nullptr) {
            throw shared_mutex_exception("Could not create the mutex");
        } else if (GetLastError() == ERROR_ALREADY_EXISTS && !openIfExists) {
            // Close the handle to the mutex
            CloseHandle(sem);
            throw shared_mutex_exception(
                    "A mutex with the name '" + _mtx_name + "' is already owned by another program");
        }

        // Set the handle to mutex and add the mutex name to owned_mutexes
        this->_semaphore = sem;
        if (_unique) _owned_mutexes.push_back(_mtx_name);
    }

    /**
     * Move constructor
     *
     * @param rhs the shared_mutex to move
     */
    win_shared_mutex(win_shared_mutex &&rhs) noexcept: shared_mutex(std::move(rhs._mtx_name), !rhs._unique),
                                                       _semaphore(rhs._semaphore) {
        // Set the semaphore of rhs to nullptr
        rhs._semaphore = nullptr;
        this->_locked = rhs._locked;
    }

    /**
     * No copy assignment operator
     */
    win_shared_mutex &operator=(const win_shared_mutex &) = delete;

    /**
     * Move assignment operator
     *
     * @param rhs the instance to assign
     * @return this
     */
    win_shared_mutex &operator=(win_shared_mutex &&rhs) noexcept {
        // Swap all class members
        std::swap(rhs._mtx_name, this->_mtx_name);
        std::swap(rhs._semaphore, this->_semaphore);
        std::swap(rhs._locked, this->_locked);
        std::swap(rhs._unique, this->_unique);

        return *this;
    }

    void lock() override {
        DWORD res = WaitForSingleObject(_semaphore, INFINITE);
        switch (res) {
            case WAIT_ABANDONED: // The mutex the handle is pointing to is abandoned
                throw shared_mutex_exception("The mutex was abandoned");
            case WAIT_FAILED: // The wait failed
                throw shared_mutex_exception("The wait failed");
            case WAIT_OBJECT_0: // The ownership could be acquired
                _locked = true;
                return;
            default: // Unknown error
                throw shared_mutex_exception("WaitForSingleObject() failed with an unknown error");
        }
    }

    void unlock() override {
        // Try releasing the mutex
        if (!ReleaseSemaphore(_semaphore, 1, nullptr)) {
            throw shared_mutex_exception("Could not release the mutex. Error: " + getLastErrorAsString());
        } else {
            _locked = false;
        }
    }

    [[nodiscard]] bool try_lock() override {
        // Wait for max 1 millisecond
        DWORD res = WaitForSingleObject(_semaphore, 1);
        switch (res) {
            case WAIT_ABANDONED: // The mutex the handle is pointing to is abandoned
                throw shared_mutex_exception("The mutex was abandoned");
            case WAIT_FAILED: // The wait failed
                throw shared_mutex_exception("The wait failed");
            case WAIT_OBJECT_0: // The ownership could be acquired
                _locked = true;
                return true;
            case WAIT_TIMEOUT: // The wait timed out
                return false;
            default: // Unknown error
                throw shared_mutex_exception("WaitForSingleObject failed with an unknown error");
        }
    }

    /**
     * Delete this shared_mutex.
     * Destroys the semaphore or mutex.
     */
    ~win_shared_mutex() override {
        // If the semaphore is null, return
        if (_semaphore == nullptr) return;

        // Unlock the mutex if locked
        if (_locked) {
            if (!ReleaseSemaphore(_semaphore, 1, nullptr)) {
                std::cerr << "Could not release the mutex. Error: " << getLastErrorAsString() << std::endl;
            }
        }

        // Close the handle to the mutex
        if (!CloseHandle(_semaphore)) {
            std::cerr << "Could not close the handle to the mutex. Error: " << getLastErrorAsString() << std::endl;
        }

        // Try to delete the mutex name from the owned_mutexes vector
        try_remove_mutex();
    }

private:
    // The semaphore pointer
    HANDLE _semaphore;

    /**
     * Get the last error as a string.
     * Source: https://stackoverflow.com/a/17387176
     *
     * @return the las win32 error as a string
     */
    static std::string getLastErrorAsString() {
        // Get the error message ID, if any.
        DWORD errorMessageID = ::GetLastError();
        if (errorMessageID == 0) {
            return std::string(); // No error message has been recorded
        }

        LPSTR messageBuffer = nullptr;

        // Ask Win32 to give us the string version of that message ID.
        // The parameters we pass in, tell Win32 to create the buffer that
        // holds the message for us (because we don't yet know how long the message string will be).
        size_t size = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
                errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR) &messageBuffer, 0, nullptr);

        // Copy the error message into a std::string.
        std::string message(messageBuffer, size);

        // Free the Win32's string's buffer.
        LocalFree(messageBuffer);

        return message;
    }
};

#endif //OS_WINDOWS

#ifdef OS_UNIX

/**
 * A shared mutex for unix
 */
class unix_shared_mutex : public shared_mutex {
public:
    /**
     * Create a shared_mutex instance.
     *
     * @param mutex_name the mutex name
     * @param openIfExists whether to open if the mutex already exists or throw an exception
     */
    unix_shared_mutex(const std::string &mutex_name, bool openIfExists) : shared_mutex(mutex_name, openIfExists) {
        // Create the name for the mutex
        std::string name = "/";
        name.append(_mtx_name);

        // The semaphore pointer
        sem_t *sem;

        // Try to create the semaphore
        if (openIfExists) {
            sem = sem_open(name.c_str(), O_CREAT, PERM, 1);
        } else {
            sem = sem_open(name.c_str(), O_CREAT | O_EXCL, PERM, 1);
        }

        // If the creation failed, thrown an exception
        if (sem == SEM_FAILED) {
            throw shared_mutex_exception(
                    "A mutex with the name '" + _mtx_name + "' is already owned by another program");
        } else {
            // Set the semaphore and add the mutex name to owned_mutexes
            this->_semaphore = sem;
            if (_unique) _owned_mutexes.push_back(_mtx_name);
        }
    }

    /**
     * Move constructor
     *
     * @param rhs the shared_mutex to move
     */
    unix_shared_mutex(unix_shared_mutex &&rhs) noexcept: shared_mutex(std::move(rhs._mtx_name), !rhs._unique),
                                                         _semaphore(rhs._semaphore) {
        // Set the semaphore of rhs to nullptr
        rhs._semaphore = nullptr;
        this->_locked = rhs._locked;
    }

    /**
     * No copy assignment operator
     */
    unix_shared_mutex &operator=(const unix_shared_mutex &) = delete;

    /**
     * Move assignment operator
     *
     * @param rhs the instance to assign
     * @return this
     */
    unix_shared_mutex &operator=(unix_shared_mutex &&rhs) noexcept {
        // Swap all class members
        std::swap(rhs._mtx_name, this->_mtx_name);
        std::swap(rhs._semaphore, this->_semaphore);
        std::swap(rhs._locked, this->_locked);
        std::swap(rhs._unique, this->_unique);

        return *this;
    }

    void lock() override {
        // Try acquire the ownership of the semaphore
        if (sem_wait(_semaphore) != 0) {
            throw shared_mutex_exception("The wait failed");
        } else {
            // The operation was successful, we now own the semaphore
            _locked = true;
        }
    }

    void unlock() override {
        // Try to unlock the semaphore
        if (sem_post(_semaphore) != 0) {
            throw shared_mutex_exception("sem_post() failed");
        } else {
            // The operation was successful, we don't own the semaphore anymore
            _locked = false;
        }
    }

    [[nodiscard]] bool try_lock() override {
        // Try waiting for the semaphore to lock
        if (sem_trywait(_semaphore) != 0) {
            // sem_trywait failed, could not acquire ownership
            return false;
        } else {
            // The operation was successful, we now own the semaphore
            _locked = true;
            return true;
        }
    }

    /**
     * Delete this shared_mutex.
     * Destroys the semaphore or mutex.
     */
    ~unix_shared_mutex() override {
        // If the semaphore is null, return
        if (_semaphore == nullptr) return;

        // If locked, unlock the mutex
        if (_locked) {
            sem_post(_semaphore);
        }

        // Re-create the semaphore name
        std::string name = "/";
        name.append(_mtx_name);

        // Close the semaphore
        if (sem_close(_semaphore) == -1) {
            std::cerr << "Could not close the semaphore" << std::endl;
        }

        // Delete the semaphore
        if (sem_unlink(name.c_str()) == -1) {
            std::cerr << "Could not delete the semaphore" << std::endl;
        }

        // Try to delete the mutex name from the owned_mutexes vector
        try_remove_mutex();
    }

private:
    // The semaphore pointer
    sem_t *_semaphore;
};

#endif //OS_UNIX

std::unique_ptr<shared_mutex> shared_mutex::createShared_mutex(const std::string &mtx_name, bool openIfExists) {
#ifdef OS_WINDOWS
    return std::make_unique<win_shared_mutex>(mtx_name, openIfExists);
#elif defined(OS_UNIX)
    return std::make_unique<unix_shared_mutex>(mtx_name, openIfExists);
#endif //OS_WINDOWS
}

#endif //PROGRAM_IS_ALREADY_RUNNING_SHARED_MUTEX_HPP

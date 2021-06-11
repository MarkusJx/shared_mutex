/**
 * A class for detecting if this program is already running.
 * Uses named mutexes/semaphores to detect if the program is already running.
 * Can also check if other programs using @markusjx/shared_semaphore are running.
 */
export class process_mutex {
    /**
     * Create a new processMutex instance.
     * Throws an error if the program is already running.
     *
     * @param name the name of the program
     */
    constructor(name: string);

    /**
     * Delete this instance's named mutex/semaphore
     * and mark this program therefore as 'not running'.
     */
    destroy(): void;

    /**
     * Try creating a processMutex instance.
     * Returns null if the program is already running.
     *
     * @param name the name of the program
     * @return a processMutex instance or null if the program is already running
     */
    static try_create(name: string): process_mutex | null;
}

/**
 * A shared mutex
 */
export class shared_mutex {
    /**
     * Create a new shared_mutex instance
     *
     * @param name the name of the mutex
     */
    constructor(name: string);

    /**
     * Lock the mutex. Blocking call.
     * May freeze your node.js instance.
     */
    lock_blocking(): void;

    /**
     * Lock the mutex
     *
     * @return the promise to be resolved when the ownership of the mutex is acquired
     */
    lock(): Promise<void>;

    /**
     * Try locking the mutex.
     * Returns true if the mutex ownership could be acquired.
     * If so, the mutex will now be owned by this shared_mutex
     * instance and must be unlocked using unlock().
     * Returns false if the mutex ownership could not be acquired.
     * In that case, the mutex will not be owned by this shared_mutex
     * instance and **must not be unlocked** using unlock().
     *
     * @return true if the mutex ownership could be acquired
     */
    try_lock(): boolean;

    /**
     * Unlock the mutex
     */
    unlock(): void;

    /**
     * Delete the shared_mutex
     */
    destroy(): void;
}

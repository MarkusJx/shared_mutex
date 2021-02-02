/**
 * A class for detecting if this program is already running.
 * Uses named mutexes/semaphores to detect if the program is already running.
 * Can also check if other programs using program-is-already-running are running.
 */
export class program_mutex {
    /**
     * The name of the program
     */
    name: string;

    /**
     * Create a new program_mutex instance.
     * Throws an error if the program is already running.
     *
     * @param name the name of the program
     */
    constructor(name: string);

    /**
     * Delete this instance's named mutex/semaphore
     * and mark this program therefore as 'not running'.
     */
    delete(): void;
}

/**
 * Try creating a program_mutex instance.
 * Returns null if the program is already running.
 *
 * @param name the name of the program
 * @return a program_mutex instance or null if the program is already running
 */
export function try_create(name: string): program_mutex;

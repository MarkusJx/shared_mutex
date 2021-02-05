const native_addon = require('./bin/shared_mutex');

module.exports = {
    processMutex: class {
        // Whether this was already deleted
        #deleted = false;

        constructor(name) {
            // the name must be a string and at least one character long
            if (typeof name !== "string") {
                throw new Error("The name must be of type string");
            } else if (name.length === 0) {
                throw new Error("The name must not be empty");
            }

            // Define 'id' read-only
            Object.defineProperty(this, 'id', {
                value: native_addon.lib_createProcessMutex(name),
                configurable: true,
                enumerable: true,
                writable: false
            });
        }

        delete() {
            // If this was already deleted, throw an exception
            if (!this.#deleted) {
                native_addon.lib_deleteMutex(this.id);
                this.#deleted = true;
            } else {
                throw new Error("delete() was already called on this instance");
            }
        }

        static try_create(name) {
            // Try creating a programMutex.
            // if this fails, return null.
            try {
                return new this(name);
            } catch (ignored) {
                return null;
            }
        }
    },
    sharedMutex: class {
        // Whether this was already deleted
        #deleted = false;

        constructor(name) {
            // The name must be a string and at least one character long
            if (typeof name !== "string") {
                throw new Error("The name must be of type string");
            } else if (name.length === 0) {
                throw new Error("The name must not be empty");
            }

            // Define 'id' read-only
            Object.defineProperty(this, 'id', {
                value: native_addon.lib_createMutex(name),
                configurable: true,
                enumerable: true,
                writable: false
            });
        }

        lock_blocking() {
            native_addon.lib_blockLock(this.id);
        }

        lock() {
            return native_addon.lib_lock(this.id);
        }

        try_lock() {
            return native_addon.lib_try_lock(this.id);
        }

        unlock() {
            native_addon.lib_unlock(this.id);
        }

        delete() {
            // If this was already deleted, throw an exception
            if (!this.#deleted) {
                native_addon.lib_deleteMutex(this.id);
                this.#deleted = true;
            } else {
                throw new Error("delete() was already called on this instance");
            }
        }
    }
}
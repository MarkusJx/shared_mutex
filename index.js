const native_addon = require('./build/Release/is_program_already_running');

module.exports = {
    program_mutex: class {
        constructor(name, fromExisting = false, id = "") {
            if (typeof name !== "string") {
                throw new Error("The name must be of type string");
            } else if (name.length === 0) {
                throw new Error("The name must not be empty");
            }

            this.deleted = false;
            if (!fromExisting) {
                this.id = native_addon.lib_createProgramMutex(name);
            } else {
                this.id = id;
                if (id.length === 0) {
                    throw new Error("The id must not be empty");
                }
            }
        }

        delete() {
            if (!this.deleted) {
                native_addon.lib_deleteMutex(this.id);
                this.deleted = true;
                this.id = null;
            } else {
                throw new Error("delete() was already called on this instance");
            }
        }
    },
    shared_mutex: class {
        constructor(name) {
            if (typeof name !== "string") {
                throw new Error("The name must be of type string");
            } else if (name.length === 0) {
                throw new Error("The name must not be empty");
            }

            this.deleted = false;
            this.id = native_addon.lib_createMutex(name);
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
            if (!this.deleted) {
                native_addon.lib_deleteMutex(this.id);
                this.deleted = true;
                this.id = null;
            } else {
                throw new Error("delete() was already called on this instance");
            }
        }
    },
    try_create_program_mutex: function (name) {
        try {
            let id = native_addon.lib_createProgramMutex(name);
            return new this.program_mutex(name, true, id);
        } catch (e) {
            return null;
        }
    }
}
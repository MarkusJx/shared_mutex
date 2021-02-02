const native_addon = require('./build/Release/is_program_already_running');

module.exports = {
    program_mutex: class {
        constructor(name, fromExisting = false) {
            this.name = name;
            this.deleted = false;
            if (!fromExisting) {
                native_addon.lib_createMutex(name);
            }
        }

        delete() {
            if (!this.deleted) {
                native_addon.lib_deleteMutex(this.name);
                this.deleted = true;
            } else {
                throw new Error("delete() was already called on this instance");
            }
        }
    },
    try_create: function (name) {
        try {
            native_addon.lib_createMutex(name);
            return new this.program_mutex(name, true);
        } catch (e) {
            return null;
        }
    }
}
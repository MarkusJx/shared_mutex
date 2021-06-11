const native_addon = require('./bin/shared_mutex');

module.exports = {
    process_mutex: native_addon.process_mutex,
    shared_mutex: native_addon.shared_mutex
};
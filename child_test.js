const {process_mutex} = require('./index');
const assert = require("assert");

if (process.argv[2] === "expectFail") {
    assert.throws(() => {
        new process_mutex("test");
    }, Error, "A mutex with the name 'test' is already owned by another program");
} else {
    const mtx = new process_mutex("test");
    mtx.destroy();
}

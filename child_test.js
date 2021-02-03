const checker = require('./index');
const assert = require("assert");

if (process.argv[2] === "expectFail") {
    assert.throws(() => {
        new checker.program_mutex("test");
    }, Error, "A mutex with the name 'test' is already owned by another program");
} else {
    const mtx = new checker.program_mutex("test");
    mtx.delete();
}

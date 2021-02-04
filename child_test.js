const {processMutex} = require('./index');
const assert = require("assert");

if (process.argv[2] === "expectFail") {
    assert.throws(() => {
        new processMutex("test");
    }, Error, "A mutex with the name 'test' is already owned by another program");
} else {
    const mtx = new processMutex("test");
    mtx.delete();
}

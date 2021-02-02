const {fork} = require('child_process');
const checker = require('./index');
const assert = require("assert");

console.log("Testing is-program-already-running functionality");
console.log("========================================\n");

console.log("Creating a new program_mutex instance (mtx1)");
const mtx1 = new checker.program_mutex("test");

console.log("Deleting the created program_mutex instance (mtx1)");
mtx1.delete();

console.log("========================================\n");

console.log("Creating a new program_mutex instance (mtx2)")
const mtx2 = new checker.program_mutex("test");

function error_test() {
    console.log("Trying to create a new program_mutex instance when another one is already owning the mutex");
    try {
        new checker.program_mutex("test");
    } catch (e) {
        console.log("The program threw an exception, as expected");
        return;
    }
    throw new Error("The program should have thrown, but it didn't");
}

error_test();
console.log("========================================\n");

console.log("Trying to create a new program_mutex instance in a child process");
fork("child_test.js");

console.log("Deleting the existing program_mutex instance (mtx2)");
mtx2.delete();

console.log("========================================\n");

console.log("Using try_create to create a program_mutex instance");
console.log("This call should return a non-null value");
assert(checker.try_create("test") !== null, "This call should return a non-null value");

console.log("This call should return null, as another program_mutex instance already owns this mutex");
assert(checker.try_create("test") === null,
    "This call should return null, as another program_mutex instance already owns this mutex");

console.log("========================================\n");

console.log("Done.");
const checker = require('./index');

function child_test() {
    try {
        new checker.program_mutex("test");
    } catch (e) {
        return;
    }
    throw new Error("The program should have thrown, but it didn't");
}

child_test();

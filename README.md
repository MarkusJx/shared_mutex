# Is-program-already-running

Check whether your program is already running.
This script uses named mutexes/semaphores to check whether a program is already running
to make sure only one instance of a program is running at once.

## Usage
```js
const run_checker = require('is-program-already-running');

// Create a new instance of program_mutex.
// This will throw an exception if another instance
// already owns a mutex with the same name
const lock = new run_checker.program_mutex("YOUR_PROGRAM_NAME");

// Delete the lock instance to release it
// and allow another instance to gain ownership
// of the mutex.
lock.delete();

// It is also possible to try creating a program_mutex instance:
const res = run_checker.try_create("YOUR_PROGRAM_NAME");

// If the result is null, another instance is already owning the
// mutex. Otherwise, a new instance of program_mutex is returned.
```
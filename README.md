# shared_mutex

Shared mutexes/semaphores in javascript.
## Usage
### Import the module
```js
const shared_mutex = require('shared_mutex');
```

### Check if your program is already running
Check whether your program is already running.
This script uses named semaphores to check whether a program is already running
to make sure only one instance of a program is running at once. 

You could use the ``delete`` method on an instance of the ``program_mutex`` class
to allow another instance of your program to start. You don't have to do this,
the semaphores will be automatically destroyed once the module is unloaded.

#### ``new program_mutex``
Create a new instance of program_mutex.This will throw an exception if another
instance already owns a mutex with the same name.
```js
const lock = new shared_mutex.program_mutex("YOUR_PROGRAM_NAME");
```

#### ``program_mutex.delete``
Delete the lock instance to release it and allow another
instance to gain ownership of the mutex.
```js
lock.delete();
```

#### ``try_create``
It is also possible to try creating a program_mutex instance using
``try_create()``. The function will either return ``null`` if a
semaphore with the same name already exists or a new instance of
``program_mutex`` if no semaphore with the same name already exists.
```js
const try_lock = shared_mutex.try_create("YOUR_PROGRAM_NAME");
```

#### ``program_mutex.delete``
You can call ``delete()`` on the object to allow other instances to
gain ownership of the mutex, if you want to. If not, this will
automatically executed on module unload, so the semaphore will be
destroyed.
```js
try_lock.delete();
```

### Shared mutexes
#### ``new shared_mutex``
Create a shared mutex
```js
const shared_mutex = require('shared_mutex');

const mutex = new shared_mutex.shared_mutex("A_MUTEX_NAME");
```

#### ``shared_mutex.lock``
Lock the mutex
```js
mutex.lock().then(() => {
  // Mutex locked
}, () => {
  // The ownership could not be acquired
});

// In an async context
await mutex.lock();
```

#### ``shared_mutex.lock_blocking``
Blocking call (not recommended as it will freeze your node.js instance):
```js
mutex.lock_blocking();
```

#### ``shared_mutex.try_lock``
Try locking the mutex: ``try_lock()`` will return ``true``, if the mutex ownership could
be acquired. If so, the mutex will now be owned by your shared_mutex instance and must be
unlocked using ``unlock()``. If another instance already owns the mutex, ``try_lock()``
will return ``false`` and ``unlock()`` **must not be called**. Although you can call it
to remove the ownership over the mutex from the other instance, which is not really recommended.
```js
let could_lock = mutex.try_lock();

if (could_lock) {
  // We now own the mutex and should call unlock
  // when leaving the critical area 
} else {
  // We could not acquire ownership over the mutex,
  // guess we'll have to accept that...
}
```

#### ``shared_mutex.unlock``
Unlocking the mutex so another instance can now acquire the ownership over the mutex:
```js
mutex.unlock();
```

#### ``shared_mutex.delete``
Delete the mutex: ``delete()`` will also be called when the module is unloaded.
``delete()`` will also call ``unlock()`` if this instance of shared_mutex is
currently the owner of the mutex, so any other instance can acquire ownership
over the mutex as the current instance obviously does not need it anymore.
NOTE: ``delete()`` can only be called once on a single ``shared_mutex`` instance.
```js
mutex.delete();
```

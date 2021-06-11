const {fork} = require('child_process');
const assert = require("assert");

const mutex = require('./index');

describe('processMutex', () => {
    describe('#new processMutex', () => {
        const TEST = "test";
        let mtx;
        it('create: should not throw', () => {
            mtx = new mutex.process_mutex(TEST);
        });

        it('create: should throw an exception', () => {
            assert.throws(() => {
                new mutex.process_mutex(TEST);
            }, Error, "A mutex with the name 'test' is already owned by this program");
        });

        it('delete: should not throw', () => {
            mtx.destroy();
        });

        it('delete: should throw', () => {
            assert.throws(mtx.destroy, Error, "delete() was already called on this instance");
        });

        it('re-create: should not throw', () => {
            mtx = new mutex.process_mutex(TEST);
        });

        it('create in different process: should throw', (done) => {
            fork("child_test.js", ["expectFail"]).on('close', (e) => {
                if (e === 0) {
                    done();
                } else {
                    done("The child process exited with non-zero error code");
                }
            });
        });

        it('re-delete: should not throw', () => {
            mtx.destroy();
        });

        it('create in different process: should not throw', (done) => {
            fork("child_test.js").on('close', (e) => {
                if (e === 0) {
                    done();
                } else {
                    done("The child process exited with non-zero error code");
                }
            });
        });
    });

    describe('#processMutex.try_create', () => {
        let mtx;
        it('create: should return processMutex', () => {
            mtx = mutex.process_mutex.try_create("test");
            assert(mtx !== null, "mtx should not be null");
        });

        it('create: should return null', () => {
            const val = mutex.process_mutex.try_create("test");
            assert(val === null, "processMutex.try_create should return null");
        });

        it('delete: should not throw', () => {
            mtx.destroy();
        });

        it('re-create: should return processMutex', () => {
            mtx = mutex.process_mutex.try_create("test");
            assert(mtx !== null, "mtx should not be null");
            mtx.destroy();
        });
    });
});

describe('sharedMutex', () => {
    describe('#basic tests', () => {
        let mtx;
        it('create: should not throw', () => {
            mtx = new mutex.shared_mutex("test");
        });

        it('lock: should not throw', (done) => {
            mtx.lock().then(() => {
                done();
            }, () => {
                done("lock() threw an error");
            });
        });

        it('unlock: should not throw', () => {
            mtx.unlock();
        })

        it('lock blocking: should not throw', (done) => {
            mtx.lock().then(() => {
                done();
            }, () => {
                done("lock() threw an error");
            });
        }).timeout(1000);

        it('try_lock: should return false', () => {
            let res = mtx.try_lock();
            assert(res === false, "res should be false");
        });

        it('unlock: should not throw', () => {
            mtx.unlock();
        });

        it('try_lock: should return true', () => {
            let res = mtx.try_lock();
            assert(res === true);
        });

        it('delete: should delete the mutex', () => {
            mtx.destroy();
            assert.throws(mtx.destroy, Error, "delete() was already called on this instance");
        });
    });

    describe('#shared tests', () => {
        let mtx1, mtx2;
        it('create: should not throw', () => {
            mtx1 = new mutex.shared_mutex("test");
            mtx2 = new mutex.shared_mutex("test");
        })

        it('lock first mutex: should lock the mutex', (done) => {
            mtx1.lock().then(() => done(), done);
        }).timeout(1000);

        it('try lock the second mutex: should return false', () => {
            let res = mtx2.try_lock();
            assert(res === false, "mtx2.try_lock() should return false");
        });

        it('unlock the first mutex: should not throw', () => {
            mtx1.unlock();
        });

        it('try lock the second mutex: should return true', () => {
            let res = mtx2.try_lock();
            assert(res === true, "mtx2.try_lock() should return true");
        });

        it('try lock the first mutex: should return false', () => {
            let res = mtx1.try_lock();
            assert(res === false, "mtx1.try_lock() should return false");
        });

        it('try lock the second mutex again: should return false', () => {
            let res = mtx2.try_lock();
            assert(res === false, "mtx2.try_lock() should return false");
        });

        it('delete the first mutex: should not throw', () => {
            mtx1.destroy();
        });

        it('try lock the second mutex again: should return false', () => {
            let res = mtx2.try_lock();
            assert(res === false, "mtx2.try_lock() should return false");
        });

        it('unlock the second mutex: should not throw', () => {
            mtx2.unlock();
        });

        it('lock the second mutex: should lock the mutex', (done) => {
            mtx2.lock().then(() => done(), done);
        });

        it('try lock the second mutex again: should return false', () => {
            let res = mtx2.try_lock();
            assert(res === false, "mtx2.try_lock() should return false");
        });

        it('delete the second mutex: should not throw', () => {
            mtx2.destroy();
        });
    });
});

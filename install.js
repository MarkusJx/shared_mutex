const path = require('path');
const fs = require('fs');

const BINARY_NAME = "shared_mutex.node";
const OUT_DIR = path.join(__dirname, 'bin');
const BUILD_DIR = path.join(__dirname, 'build');

// Source: https://stackoverflow.com/a/32197381
function deleteFolderRecursive(p) {
    if (fs.existsSync(p)) {
        fs.readdirSync(p).forEach((file) => {
            const curPath = path.join(p, file);
            if (fs.lstatSync(curPath).isDirectory()) { // recurse
                deleteFolderRecursive(curPath);
            } else { // delete file
                fs.unlinkSync(curPath);
            }
        });
        fs.rmdirSync(p);
    }
}

function deleteIfExists(p) {
    if (fs.existsSync(p)) {
        if (fs.lstatSync(p).isDirectory()) {
            console.log(`Directory ${p} exists, deleting it`);
            deleteFolderRecursive(p);
        } else {
            console.log(`${p} exists, deleting it`);
            fs.unlinkSync(p);
        }
    }
}

if (process.argv.length === 2) {
    deleteIfExists(BUILD_DIR);
} else if (process.argv.length === 3) {
    switch (process.argv[2]) {
        case "--post_build":
            deleteIfExists(OUT_DIR);
            fs.mkdirSync(OUT_DIR);
            fs.copyFileSync(path.join(BUILD_DIR, 'Release', BINARY_NAME), path.join(OUT_DIR, BINARY_NAME));
            break;
        default:
            throw new Error(`Unknown argument: ${process.argv[2]}`);
    }
} else {
    throw new Error("Invalid number of arguments supplied");
}

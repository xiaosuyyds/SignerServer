const path = require('node:path');
const fs = require('node:fs');
const os = require('node:os');

const exePath = path.dirname(process.execPath);

if (os.platform() === "win32") {
    let configVersionInfoPath = path.join(exePath, "resources/app/versions/config.json");
    const qqVersionConfigInfo = JSON.parse(fs.readFileSync(configVersionInfoPath).toString());
    require(path.join(exePath, "resources/app/versions", qqVersionConfigInfo.curVersion, "wrapper.node"));
} else {
    require(path.join(exePath, "resources/app/wrapper.node"));
}
const path = require('node:path');
const os = require('node:os');

const exePath = path.dirname(process.execPath);

let QQWrapper, appid, qua;
const qqPkgInfo = require(path.join(exePath, "resources/app/package.json"));

if (os.platform() === "win32") {
    QQWrapper = require(path.join(exePath, "resources/app/versions", qqPkgInfo.version, "wrapper.node"));
    appid = "537213803";
    qua = `V1_WIN_NQ_${qqVersionConfigInfo.curVersion.replace("-", "_")}_GW_B`;
} else {
    QQWrapper = require(path.join(exePath, "resources/app/wrapper.node"));
    appid = "537213827";
    qua = qqPkgInfo.qua;
}

class GlobalAdapter {
    onLog(...args) {
    }
    onGetSrvCalTime(...args) {
    }
    onShowErrUITips(...args) {
    }
    fixPicImgType(...args) {
    }
    getAppSetting(...args) {
    }
    onInstallFinished(...args) {
    }
    onUpdateGeneralFlag(...args) {
    }
    onGetOfflineMsg(...args) {
    }
}

const dataPathGlobal = "/root"

const engine = new QQWrapper.NodeIQQNTWrapperEngine();
engine.initWithDeskTopConfig({
    base_path_prefix: "",
    platform_type: 3,
    app_type: 4,
    app_version: qqPkgInfo.version,
    os_version: os.release(),
    use_xlog: true,
    qua: qua,
    global_path_config: {
        desktopGlobalPath: dataPathGlobal
    },
    thumb_config: { maxSide: 324, minSide: 48, longLimit: 6, density: 2 }
}, new QQWrapper.NodeIGlobalAdapter(new GlobalAdapter()));

const loginService = new QQWrapper.NodeIKernelLoginService();
loginService.initConfig({
    machineId: "",
    appid: appid,
    platVer: os.release(),
    commonPath: dataPathGlobal,
    clientVer: qqPkgInfo.version,
    hostName: os.hostname()
});
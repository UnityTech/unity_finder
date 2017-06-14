"use strict";

exports.find_unity_installs = function (callback) {
    var exec = require ('child_process').exec;
    var result = '';
    var isWin = /^win/.test(process.platform);
    var cmd;
    cmd = __dirname + '/../bin/unity_finder';
    if (isWin) {
        cmd += '.exe';
    }
    else {
        cmd += '_osx';
    }
    var child = exec (cmd, function(err, stdout, stderr) {
    	if (err) return callback(err);
        result += stdout;
    });
    child.on('close', function (err) {
    	if (err) return callback(err);
        return callback (null, result);
    });
    child.on('error', function (err) {
        return callback (err);
    });
}

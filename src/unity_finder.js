"use strict";

let debug = require('debug')('unity-finder');

exports.find_unity_installs = function (callback) {
    var exec = require ('child_process').execFile;
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
    	if (err) {
    	    debug(`Error while executing ${cmd}`);
    	    debug(`Error message: ${err}`);
    	    return callback(err);
      }
        result += stdout;
    });
    child.on('close', function (err) {
    	if (err) return callback(err);
        return callback (null, result);
    });
    child.on('error', function (err) {
        debug(error);
        return callback (err);
    });
}

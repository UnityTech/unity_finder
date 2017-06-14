'use strict';

var unity_finder = require('./unity_finder');

unity_finder.find_unity_installs (function (err, result) {
    if (err) return console.log (err);
    console.log ("Data: " + result);

    var json = JSON.parse (result);
    console.dir (json);
    process.exit ();
})

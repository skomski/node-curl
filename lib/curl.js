var Events = require('events');
var addon;

function inherits(target, source) {
  for (var key in source.prototype) {
    target.prototype[key] = source.prototype[key];
  }
}

try {
  addon = require('../build/default/addon');
} catch(e) {
  addon = require('../build/Release/addon');
}

inherits(addon.CurlMultiWrapper, Events.EventEmitter);
inherits(addon.CurlEasyWrapper, Events.EventEmitter);

exports.createPool = function() {
  var pool = new addon.CurlMultiWrapper();
  return pool;
};

exports.createHandle = function() {
  var wrapper = new addon.CurlEasyWrapper();
  return wrapper;
};

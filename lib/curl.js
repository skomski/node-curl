var Events  = require('events');
var options = require('./curl_options');
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
  wrapper.setOption = function(key, value) {
    var enumKey = options.stringOptions[key.toUpperCase()]
    wrapper._setOption(enumKey, value);
  }
  return wrapper;
};

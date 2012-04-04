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

inherits(addon.CurlWrapper, Events.EventEmitter);

exports.createRequest = function() {
  var curl = new addon.CurlWrapper();
  return curl;
};

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

exports.multiPool = new addon.CurlMultiWrapper();

addon.CurlEasyWrapper.prototype.execute = function() {
  exports.multiPool.execute(this);
}

addon.CurlEasyWrapper.prototype.setOption = function(key, value) {
  switch(typeof value) {
    case 'string':
      var enumNumber = options.stringOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      this._setStringOption(enumNumber, value);
      break;
    case 'number':
      var enumNumber = options.numberOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      this._setNumberOption(enumNumber, value);
      break;
    case 'boolean':
      var enumNumber = options.numberOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      this._setNumberOption(enumNumber, value ? 1 : 0);
      break;
    default:
      throw Error('Unsupported value type');
  }
}


exports.createRequest = function() {
  return new addon.CurlEasyWrapper();
};

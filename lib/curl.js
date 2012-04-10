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

exports.setOption = function(key, value) {
  switch(typeof value) {
    case 'number':
      var enumNumber = options.numberMultiOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      exports.multiPool._setNumberOption(enumNumber, value);
      break;
    case 'boolean':
      var enumNumber = options.numberMultiOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      exports.multiPool._setNumberOption(enumNumber, value ? 1 : 0);
      break;
    default:
      throw Error('Unsupported value type');
  }
}

addon.CurlEasyWrapper.prototype.execute = function() {
  exports.multiPool.execute(this);
}

addon.CurlEasyWrapper.prototype.getNumberInfo = function(key) {
  var enumNumber = options.doubleInfos[key.toUpperCase()];
  if (enumNumber) return this._getDoubleInfo(enumNumber);

  enumNumber = options.integerInfos[key.toUpperCase()];
  if (enumNumber) return this._getIntegerInfo(enumNumber);

  throw new Error('Unknown number info');
}

addon.CurlEasyWrapper.prototype.getStringInfo = function(key) {
  var enumNumber = options.stringInfos[key.toUpperCase()];
  if (!enumNumber) throw new Error('Unknown string info');
  return this._getStringInfo(enumNumber);
}

addon.CurlEasyWrapper.prototype.setOption = function(key, value) {
  switch(typeof value) {
    case 'object':
      var enumNumber = options.listOptions[key.toUpperCase()];
      if (enumNumber === undefined) throw new Error('Unknown option');
      this._setListOption(enumNumber, value);
      break;
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

addon.CurlEasyWrapper.prototype.setFormData = function(entries) {
  for (var i = 0; i < entries.length; i++) {
    var entry = entries[i];
    if (!entry.name) throw new Error('Need name');
    if (!entry.content) throw new Error('Need content');
    if (!entry.type) throw new Error('Need type');

    var enumNumber = options.formOptions[entry.type.toUpperCase()];
    if (enumNumber === undefined) throw new Error('Unknown type');
    entry._type = enumNumber;
  }

  this._setFormData(entries);
}


exports.createRequest = function() {
  return new addon.CurlEasyWrapper();
};

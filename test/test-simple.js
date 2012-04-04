var curl   = require('..');
var assert = require('assert');

var request = curl.createRequest();

request.on('data', function(buffer) {
  console.log(buffer.toString());
});

request.on('error', function(err) {
  assert.ifError(err);
});

request.on('end', function() {
  console.error('end')
});

request.execute();
console.log('executed');

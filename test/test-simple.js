var curl   = require('..');
var assert = require('assert');

var pool    = curl.createPool();
var request = curl.createHandle();

request.on('data', function(buffer) {
  console.log(buffer.toString());
});

pool.on('error', function(err) {
  assert.ifError(err);
});

request.on('end', function() {
  console.error('end')
});

request.setOption();

pool.execute(request);

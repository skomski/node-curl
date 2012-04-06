var curl   = require('..');
var assert = require('assert');

var pool    = curl.createPool();
var request = curl.createHandle();

var testFinished = false;

request.on('error', function(err) {
  assert.equal(err.message, "Couldn't resolve host name");
  testFinished = true;
});

request.setOption('url', 'fail');
pool.execute(request);

process.on('exit', function() {
  assert.ok(testFinished, 'testFinished != true');
});

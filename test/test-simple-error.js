var curl   = require('..');
var assert = require('assert');

var request = curl.createRequest();

var testFinished = false;

request.on('error', function(err) {
  assert.equal(err.message, "Couldn't resolve host name");
  testFinished = true;
});

request.setOption('url', 'fail');
request.setOption('verbose', true);
request.execute();

process.on('exit', function() {
  assert.ok(testFinished, 'testFinished != true');
});

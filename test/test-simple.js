var curl   = require('..');
var assert = require('assert');

var pool    = curl.createPool();
var request = curl.createHandle();

var testFinished = false;

var jsonData = '';

request.on('data', function(buffer) {
  jsonData += buffer;
});

request.on('end', function() {
  var testData = JSON.parse(jsonData);
  assert.equal(testData.status, 200);
  assert.equal(testData.testData, 1234567890);
  testFinished = true;
});

request.setOption('url', 'http://jsfiddle.net/echo/jsonp/?status=200&testData=1234567890');
pool.execute(request);

process.on('exit', function() {
  assert.ok(testFinished, 'testFinished != true');
});

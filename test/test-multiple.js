var curl   = require('..');
var assert = require('assert');

var pool    = curl.createPool();
var request200 = curl.createHandle();
request200.setOption('url', 'http://jsfiddle.net/echo/jsonp/?status=200');

request200Data = '';
request200Finished = false;

request200.on('data', function(buffer) {
  request200Data += buffer;
});

request200.on('end', function() {
  var json = JSON.parse(request200Data);
  assert.equal(json.status, 200);
  request200Finished = true;
});

var request400 = curl.createHandle();
request400.setOption('url', 'http://jsfiddle.net/echo/jsonp/?status=400');

request400Data = '';
request400Finished = false;

request400.on('data', function(buffer) {
  request400Data += buffer;
});

request400.on('end', function(buffer) {
  var json = JSON.parse(request400Data);
  assert.equal(json.status, 400);
  request400Finished = true;
});

pool.execute(request200);
pool.execute(request400);

process.on('exit', function() {
  assert.ok(request200Finished, 'request200Finished != true');
  assert.ok(request400Finished, 'request400Finished != true');
});

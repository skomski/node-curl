var curl   = require('..');
var assert = require('assert');

var request = curl.createRequest();

var requestResumed = false;
var requestPaused  = false;
var requestEnded   = false;

request.once('data', function(buffer) {
  setTimeout(function() {
    request.pause();
    console.log('paused')
    requestPaused = true;

    setTimeout(function() {
      request.resume();
      requestResumed = true;
      console.log('resumed')
    }, 500);
  }, 500);
});

request.on('data', function(buffer) {
});

request.on('end', function() {
  requestEnded = true;
});

request.setOption('url', 'http://tmp.transloadit.com.s3.amazonaws.com/2eb7af87f3885c6b63f97dfd2169083a');
request.setOption('verbose', true);
request.execute();

process.on('exit', function() {
  assert.ok(requestPaused,  'requestPaused != true');
  assert.ok(requestResumed, 'requestResumed != true');
  assert.ok(requestEnded,   'requestEnded != true');
});

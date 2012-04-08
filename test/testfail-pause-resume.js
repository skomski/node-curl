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
    }, 3000);
  }, 3000);
});

request.on('data', function(buffer) {
  console.log(buffer);
});

request.on('end', function() {
  requestEnded = true;
});

request.setOption('url', 'http://media.die-drei-vogonen.de/podcast/die_drei_vogonen__folge057__dummtueten_frispo.mp3');
request.setOption('verbose', true);
request.execute();

process.on('exit', function() {
  assert.ok(requestPaused,  'requestPaused != true');
  assert.ok(requestResumed, 'requestResumed != true');
  assert.ok(requestEnded,   'requestEnded != true');
});

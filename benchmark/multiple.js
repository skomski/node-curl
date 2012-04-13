var curl      = require('..')
var measured   = require('measured');
var collection = new measured.Collection('benchmark');

var http      = require('http')

//http.globalAgent.maxSockets = 100;

var processArguments = process.argv.splice(2);

var rps = collection.meter('requestsPerSecond');

var queuedRequests = 0;

var curlFunction = function() {
  queuedRequests++;
  var request = curl.createRequest();
  request.setOption('url', 'http://' + processArguments[0]);
  request.on('error', function(err) {
    console.error(err);
  });
  var data = '';
  request.on('data', function(buffer) {
    data += buffer;
  });
  request.on('end', function() {
    rps.mark();
    queuedRequests--;
  });
  request.execute();
}

var httpFunction = function() {
  queuedRequests++;
  var options = {
    host: processArguments[0],
    port: 80
  };

  var data = '';

  var req = http.request(options, function(res) {
    res.on('data', function(buffer) {
      data += buffer;
    });

    res.on('end', function () {
      rps.mark();
      queuedRequests--;
    });
  });
  req.on('error', function(err) {
    console.error(err);
  });
  req.end();
}

var interval = undefined;

if (processArguments[1] === 'curl') {
  interval = setInterval(curlFunction, 5);
}
if (processArguments[1] === 'http') {
  interval = setInterval(httpFunction, 5);
}

setInterval(function() {
  process.stdout.write(
    '\rmemory=' + Math.ceil(process.memoryUsage().rss/1000/1000) +
    ' requests=' + rps._count +
    ' queued=' + queuedRequests +
    ' mean=' + rps.meanRate());
  if (queuedRequests === 0) {
    process.nextTick(function() {
      process.exit(0);
    });
  }
}, 1000);

setTimeout(function() {
  clearInterval(interval);
}, 120 * 1000);

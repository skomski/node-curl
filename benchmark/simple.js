var Benchmark = require('benchmark');
var fs        = require('fs')
var curl      = require('..')
var http      = require('http')

http.Agent.maxSockets = 20;

var processArguments = process.argv.splice(2);

var suite = new Benchmark.Suite();

suite.add('http', function(deferred) {
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
      deferred.resolve();
    });
  });
  req.end();
}, { defer: true, minSamples: 50, maxTime: 10 });

suite.add('curl', function(deferred) {
  var request = curl.createRequest();
  request.setOption('url', 'http://' + processArguments[0]);
  var data = '';
  request.on('data', function(buffer) {
    data += buffer;
  });
  request.on('end', function() {
    deferred.resolve();
  });
  request.execute();
},{ defer: true, minSamples: 50, maxTime: 10 });

suite.on('cycle', function(event, bench) {
  console.log(String(bench));
})
.on('complete', function() {
  console.log('Fastest is ' + this.filter('fastest').pluck('name'));
})
.run({ async: true });

var Benchmark = require('benchmark');
var fs        = require('fs')
var curl      = require('..')
var http      = require('http')

var suite = new Benchmark.Suite();

suite.add('http', function(deferred) {
  var options = {
    host: 'twitter.com',
    port: 80
  };

  var data = '';

  var req = http.request(options, function(res) {
    res.on('data', function(buffer) {
      data += buffer;
    });
    res.on('end', function (chunk) {
      deferred.resolve();
    });
  });
  req.end();
}, { defer: true, minSamples: 50, maxTime: 30 });

suite.add('curl', function(deferred) {
  var request = curl.createRequest();
  request.setOption('url', 'http://twitter.com');
  var data = '';
  request.on('data', function(buffer) {
    data += buffer;
  });
  request.on('end', function() {
    deferred.resolve();
  });
  request.execute();
},{ defer: true, minSamples: 50, maxTime: 30 });

suite.on('cycle', function(event, bench) {
  console.log(String(bench));
})
.on('complete', function() {
  console.log('Fastest is ' + this.filter('fastest').pluck('name'));
})
.run({ async: true });

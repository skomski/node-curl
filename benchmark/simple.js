var Benchmark = require('benchmark');
var fs        = require('fs')
var curl      = require('..')
var http      = require('http')

var suite = new Benchmark.Suite;

suite.add('http', function(deferred) {
  var options = {
    host: 'www.google.com',
    port: 80
  };

  var req = http.request(options, function(res) {
    res.on('end', function (chunk) {
      deferred.resolve();
    });
  });
  req.end();
}, { defer: true });

suite.add('curl', function(deferred) {
  var request = curl.createRequest();
  request.setOption('url', 'http://google.com');
  request.on('end', function() {
    deferred.resolve();
  });
  request.execute();
},{ defer: true});

suite.on('cycle', function(event, bench) {
  console.log(String(bench));
})
.on('complete', function() {
  console.log('Fastest is ' + this.filter('fastest').pluck('name'));
})
.run({ async: false });

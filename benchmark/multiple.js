var curl      = require('..')

var processArguments = process.argv.splice(2);

var requests = 0;

setInterval(function() {
  var request = curl.createRequest();
  request.setOption('url', 'http://' + processArguments[0]);
  var data = '';
  request.on('data', function(buffer) {
    data += buffer;
  });
  request.on('end', function() {
    requests++;
  });
  request.execute();
}, 100);

setInterval(function() {
  process.stdout.write(
    '\rmemory=' + Math.ceil(process.memoryUsage().rss/1000/1000) +
    ' requests=' + requests);
}, 1000);

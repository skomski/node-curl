var curl   = require('..');
var assert = require('assert');

var request = curl.createRequest();

var testFinished = false;

var jsonData = '';

request.on('data', function(buffer) {
  jsonData += buffer;
});

request.on('end', function() {
  testFinished = true;
});

request.setOption('url', 'http://hastebin.com/documents');
request.setOption('verbose', true);
var formData = [{
    type: 'file',
    content: 'test/test-form.js',
    name: 'formtest'
  },{
    type: 'content',
    content: 'petermann',
    name: 'username'
}];
request.setFormData(formData);
request.setFormData(formData);
request.execute();

process.on('exit', function() {
  assert.ok(testFinished, 'testFinished != true');
});

/**
 * @constructor
 */
function Test() {
  this.value = "OK";
}

Test.prototype.getValue = function() {
  return this.value;
}

/**
 * @type {Test}
 */
var test;

test.getValue();

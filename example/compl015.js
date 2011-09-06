/**
 * @constructor
 */
function Test() { this.value = "OK"; }

Test.prototype.getValue = function() {
  this.value;
}

/**
 * @constructor
 * @extends {Test}
 */
function SubTest() { }

/**
 * @type {SubTest}
 */
var sub;

// sub.<COMPLETION>
// "getValue" should be shown
sub

// issue #18
function Test() {
}

Test.prototype.getValue = function() {
  return "STRING";
}

/**
 * @this {Test}
 */
function main() {
  // this.<COMPLETION>
  this.

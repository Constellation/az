var container = {
  value: function() {
    return 10;
  }
};

container.obj = (function() {
  var i = {
    x: 20,
    y: 30
  };
  var manager = {
    publicMethod: function() {
      return i;
    }
  };
  return manager;
})();

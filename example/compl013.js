// issue #19
(function() {
  var obj = {
    test: "OK",
    testing: "OKOK"
  };
  var res = {};
  for (var i in obj) {
    res[i] = obj[i];
  }
  return res;
})()

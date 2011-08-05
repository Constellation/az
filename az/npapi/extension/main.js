(function (win, doc) {

function ready() {

var last = 0;
var area = doc.getElementById('main');
var lines = doc.getElementById('lines');
var line = doc.createElement('div');
line.setAttribute('class', 'line');
var lineElements = [];
var analyzer = doc.getElementById('az');
var onInput = _.debounce(function onInput() {
  var res = analyzer.analyze(area.value);
  console.log(res);
  var obj = JSON.parse(res);
  var set = { };
  for (var k in obj) {
    var l = (obj[k].line - 1);
    set[l] = true;
  }
  for (var i = 0, len = lineElements.length; i < len; ++i) {
    if (set[i]) {
      lineElements[i].classList.add("error");
    } else {
      lineElements[i].classList.remove("error");
    }
  }
}, 0);

function addLines(height, top) {
  var n = Math.ceil(height / 16);
  if (n > last) {
    var df = doc.createDocumentFragment();
    for (var i = last; last < n; ++last) {
      var l = lineElements[last] = line.cloneNode(false);
      l.appendChild(doc.createTextNode(last + 1));
      df.appendChild(l);
    }
    lines.appendChild(df);
    last = n;
  }
  lines.scrollHeight = height;
  lines.scrollTop = top;
}

function onScroll(ev) {
  addLines(area.scrollHeight, area.scrollTop);
}

addLines(300, 0);

area.addEventListener('input', onInput, false);
area.addEventListener('scroll', onScroll, false);

}

doc.addEventListener('DOMContentLoaded', ready, false);
})(window, document);

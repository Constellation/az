(function (win, doc) {
  doc.addEventListener('DOMContentLoaded', function ready() {
    var area = doc.getElementById('main');
    var analyzer = doc.getElementById('az');
    var listener = _.debounce(function onInput() {
      console.log(analyzer.analyze(area.value));
    }, 600);
    area.addEventListener('input', listener, false);
  }, false);
})(window, document);

var it = null;
window.addEventListener('DOMContentLoaded', function(ev){
  it = document.getElementById('az');
}, false);

var requestsTable = {
  'test': function(req, sender, func){
    var track = currentTrack();
    if(track){
      func({
        success: true,
        content: track
      });
    } else {
      // not selected
      func({
        success: false
      });
    }
    return true;
  }
};


chrome.extension.onRequestExternal.addListener(function(req, sender, func){
  if(!(req.action &&
       req.action in requestsTable &&
       requestsTable[req.action](req, sender, func))){
    func({
      success: false,
      response: 'invalid request'
    });
  }
});


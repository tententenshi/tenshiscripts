var request = null;
var cache = {};

function setStyle(text) {
  var style = document.getElementsByTagName('style').item(0);
  if (style.styleSheet) {
    style.styleSheet.cssText = text;
  }
  else {
    while (style.hasChildNodes()) {
      style.removeChild(style.firstChild);
    }
    style.appendChild(document.createTextNode(text));
  }
}

function loadStyle() {
  var select = document.getElementById('themeSelect');
  var value = select.options[select.selectedIndex].innerHTML;

  var themeCode = document.getElementById('themeCode');
  themeCode.innerHTML = value;

  var cachedText = cache[value];
  if (cachedText) {
    setStyle(cachedText);
    return;
  }

  select.disabled = true;

  var caption = document.getElementById('caption');
  caption.style.display = 'none';

//  var throbber = document.getElementById('throbberImg');
//  throbber.style.display = 'block';

	if(document.all) {
		document.all.tags('link').item(1).href = 'styles/shTheme' + value + '.css';
	} else if(document.getElementsByTagName) {
		document.getElementsByTagName('link').item(1).href = 'styles/shTheme' + value + '.css';
	}
      var select = document.getElementById('themeSelect');
      select.disabled = false;
}

function body_load() {
  if (/Konqueror/.test(navigator.userAgent)) {
    var previous = document.getElementById('previous');
    previous.innerHTML = '&lt;';
    var next = document.getElementById('next');
    next.innerHTML = '&gt;';
  }

  loadStyle();
//  sh_highlightDocument();

/*
  // Opera needs this, or else it truncates the pre
  var pre = document.getElementById('codePre');
  var width = pre.scrollWidth + 'px';
  pre.style.width = width;
*/

  // freeze the width of the caption
  var caption = document.getElementById('caption');
  caption.style.width = width;
}

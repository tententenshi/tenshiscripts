var xml, xslt, xslProc;
function load(xmlFile, xslFile, destination) {
    // 読み込む前に空のドキュメントを生成
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xml = document.implementation.createDocument("", "", null);
        xslt = document.implementation.createDocument("", "", null);
    } else {
        // Internet Explorer
        xml = new ActiveXObject("Msxml2.DOMDocument");
        xslt = new ActiveXObject("Msxml2.FreeThreadedDOMDocument");
    }

    xml.async = false;
    xslt.async = false;

    // 読み込み(*.xml,*.xsl)
    xml.load(xmlFile);
    xslt.load(xslFile);
    
    // XSLTプロセッサにスタイルシート(*.xsl)をセット
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xslProc = new XSLTProcessor();
        xslProc.importStylesheet(xslt);
    } else {
        // Internet Explorer
        var xslTemp = new ActiveXObject("Msxml2.XSLTemplate");
        xslTemp.stylesheet = xslt;
        xslProc = xslTemp.createProcessor();
        xslProc.input = xml;
    }

    var today = new Date();
    var year = today.getFullYear() - 20;
    var month = preZero(today.getMonth() + 1);
    var date = preZero(today.getDate());
    var dateStr = "" + year  + month + date;	// 文字列として連結
    setExpire(dateStr);

    transform(destination);
}

function preZero(value) {
    return (parseInt(value) < 10) ? "0" + value : value
};

function setExpire(date) {
    // XSLTプロセッサにパラメータを与えて変換、結果はresult要素に
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xslProc.setParameter(null, "Expire", date);
    } else {
        // Internet Explorer
        xslProc.addParameter("Expire", date);
    }
}

function setCategory(category, destination) {
    // XSLTプロセッサにパラメータを与えて変換、結果はresult要素に
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xslProc.setParameter(null, "Category", category);
        xslProc.setParameter(null, "SearchQuery", "");
    } else {
        // Internet Explorer
        xslProc.addParameter("Category", category);
        xslProc.addParameter("SearchQuery", "");
    }
//    document.getElementById('query').value = "";
    transform(destination);
}

function setQuery(query, destination) {
    // XSLTプロセッサにパラメータを与えて変換、結果はresult要素に
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xslProc.setParameter(null, "Category", "");
        xslProc.setParameter(null, "SearchQuery", query);
    } else {
        // Internet Explorer
        xslProc.addParameter("Category", "");
        xslProc.addParameter("SearchQuery", query);
    }
//    document.getElementById('category').value = "";
    transform(destination);
}

function transform(destination) {
    // XSLTプロセッサにパラメータを与えて変換、結果はresult要素に
    if(!document.all) { // ブラウザ判別
        // Mozilla
        var fragment = xslProc.transformToFragment(xml, document);
        document.getElementById(destination).innerHTML = "";
        document.getElementById(destination).appendChild(fragment);
    } else {
        // Internet Explorer
        try { 
            xslProc.transform;
            document.getElementById(destination).innerHTML = xslProc.output;
        } catch(e) {
            document.getElementById(destination).innerHTML = e.description;
        }
    }
}

function sort(key) {
    var nodSrt=objXsl.selectSingleNode("//xsl:sort");
    nodSrt.setAttribute("select", key);
}
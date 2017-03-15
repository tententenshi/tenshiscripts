
function loadXMLDoc(fname)
{
	var xmlDoc;
	// code for IE
	if (window.ActiveXObject) {
//		xmlDoc = new ActiveXObject( "MSXML2.DOMDocument" );
		xmlDoc = new ActiveXObject( "Msxml2.FreeThreadedDOMDocument" );
		xmlDoc.async = false;
		xmlDoc.load(fname);
	}
	// code for Mozilla, Firefox, Opera, etc.
	else if (window.XMLHttpRequest) {
		var oHttpReq = new XMLHttpRequest();
		oHttpReq.open( "GET", fname, false );
		oHttpReq.send( null );
		xmlDoc = oHttpReq.responseXML;
	} else {
		alert('Your browser cannot handle this script');
	}

	return xmlDoc;
}

function createXslObject(xmlDoc, xslDoc)
{
	// code for Mozilla, Firefox, Opera, etc.
    if(!document.all) {
        // Mozilla
		xslObj = new XSLTProcessor();
		xslObj.importStylesheet(xslDoc);
    } else {
        // Internet Explorer
        var xslTemp = new ActiveXObject("Msxml2.XSLTemplate");
        xslTemp.stylesheet = xslDoc;
        xslObj = xslTemp.createProcessor();
        xslObj.input = xmlDoc;
    }
	return xslObj;
}

function applyXmlToXsl(xmlDoc, xslObj, destination)
{
	// code for Mozilla, Firefox, Opera, etc.
    if(!document.all) {
        // Mozilla
		resultDocument = xslObj.transformToFragment(xmlDoc, document);
        document.getElementById(destination).innerHTML = "";
		document.getElementById(destination).appendChild(resultDocument);
    } else {
        // Internet Explorer
        try { 
            xslObj.transform;
            document.getElementById(destination).innerHTML = xslObj.output;
        } catch(e) {
            document.getElementById(destination).innerHTML = e.description;
        }
    }
}

function loadXML(xmlFile, xslFile, destination)
{
	var xmlDoc=loadXMLDoc(xmlFile);
	var xslDoc=loadXMLDoc(xslFile);

	var xslObj = createXslObject(xmlDoc, xslDoc);
	applyXmlToXsl(xmlDoc, xslObj, destination);
}

function setXsltParameter(xslObj, paramName, value)
{
    // XSLTプロセッサにパラメータを与えて変換、結果はresult要素に
    if(!document.all) { // ブラウザ判別
        // Mozilla
        xslObj.setParameter(null, paramName, value);
    } else {
        // Internet Explorer
        xslObj.addParameter(paramName, value);
    }
}


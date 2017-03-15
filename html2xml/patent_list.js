// global variables
var xmlDoc, xslObj;

function load(xmlFile, xslFile, destination)
{
	xmlDoc=loadXMLDoc(xmlFile);
	var xslDoc=loadXMLDoc(xslFile);

	xslObj = createXslObject(xmlDoc, xslDoc);

	var today = new Date();
	var year = today.getFullYear() - 20;
	var month = preZero(today.getMonth() + 1);
	var date = preZero(today.getDate());
	var dateStr = "" + year  + month + date;	// •¶Žš—ñ‚Æ‚µ‚Ä˜AŒ‹
	setExpire(dateStr);

	applyXmlToXsl(xmlDoc, xslObj, destination);
}

function preZero(value) {
	return (parseInt(value) < 10) ? "0" + value : value
};

function setExpire(date)
{
	setXsltParameter(xslObj, "Expire", date);
}

function setCategory(category)
{
	setXsltParameter(xslObj, "Category", category);
	setXsltParameter(xslObj, "SearchQuery", "");
//	document.getElementById('query').value = "";
	applyXmlToXsl(xmlDoc, xslObj, 'result');
}

function setQuery(query)
{
	setXsltParameter(xslObj, "Category", "");
	setXsltParameter(xslObj, "SearchQuery", query);
//	document.getElementById('category').value = "";
	applyXmlToXsl(xmlDoc, xslObj, 'result');
}

function sort(key)
{
	var nodSrt=objXsl.selectSingleNode("//xsl:sort");
	nodSrt.setAttribute("select", key);
}

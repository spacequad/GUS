// This javascript came from:
// The JavaScript Source!! http://javascript.internet.com
// Original:  Joar Vatnaland (joar@accelrys.com )

// Modified 7 Sep 2005 by Andy Maloney [imol00@gmail.com]
//	- allow arbitrary ids
//	- writeTabstrip() takes an id to make active [defaults to the first tab if not set]
//	- general cleanup of code

var	currentPaneStyle = 0;
var	currentTab = 0;

function	tabstrip()
{
	this.tabs = new Array();
	this.add = addTab;
	this.write = writeTabstrip;
}

function	tab( strip, id, caption, content )
{
	this.caption = caption;
	this.content = content;
	this.write = writeTab;
	this.writeContent = writePane;
	
	strip.add( this, id );
}

function	addTab( tab, id )
{
	tab.id = id;
	this.tabs[this.tabs.length] = tab;
}

function showPane( div )
{
	if ( currentTab != 0 )
		currentTab.style.backgroundColor = "#CCCCFF";		// IF you change this, you'll have to edit the css to match it

	div.style.backgroundColor = "rgb(231, 231, 231)";
	currentTab = div;
	
	if ( currentPaneStyle != 0 )
		currentPaneStyle.display = "none";
		
	var paneId = "pn_" + div.id;
	var objPaneStyle = document.getElementById( paneId ).style;
	
	objPaneStyle.display = "block";
	currentPaneStyle = objPaneStyle;
}

function writePane()
{
	document.write( "<div class='tabpane' id='pn_" + this.id + "'>" + this.content + "</div>" );
}

function writeTab()
{
	document.write( "<td class='tabs'><div class='tabs' id='" + this.id + "' onclick='showPane(this)'>" + this.caption + "</div></td>" );
}

function writeTabstrip( id )
{
	document.write( "<table cellpadding=0 cellspacing=5 class='tabs'><tr>" );
	
	for ( var i = 0; i < this.tabs.length; i++ )
		this.tabs[i].write();
	
	document.write( "</tr></table>" );
	
	for ( var k = 0; k < this.tabs.length; k++ )
		this.tabs[k].writeContent();
	
	if ( !id )
		id = this.tabs[0].id;
		
	showPane( document.getElementById( id ) );
}
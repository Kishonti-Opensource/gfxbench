// Walk recursively on all properties of an object (e.g. JSON)
function walkTree( obj, fun, path)
{
	for ( name in obj )
	{
		var pathNext = path==null ? name : path+'/'+name;
		if (typeof obj[name] == "object")
			walkTree( obj[name], fun, pathNext);
		else
			fun( obj, name, pathNext);
	}
}

// Print structured keys
function printkeys(req)
{
	walkTree(req, function( obj, name, path){ WScript.Echo( path)});
}

// Print everyting (keys, types and values)
// Tip: .js files can be run from command line with cscript.exe (WSH)
function print(req)
{
	walkTree( req, function( obj, name, path){
		WScript.StdOut.Write( path + '\t' + typeof obj[name]);
		if (typeof obj[name] == "string")
			WScript.Echo( '\t"' + obj[name] + '"');
		else
			WScript.Echo( '\t' + obj[name]);
	});
}

// Tests
if (false)
{
	printkeys({"hello":{"world":1}})
	print(jsonRequest)
}

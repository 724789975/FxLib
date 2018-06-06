mergeInto(LibraryManager.library,
{
	GetUrl: function ()
	{
		var url = window.document.location.href;
		var bufferSize = lengthBytesUTF8(url) + 1;
		var buffer = _malloc(bufferSize);
		stringToUTF8(url, buffer, bufferSize);
		return buffer;
	},
	LogStr: function(szStyle, szContent)
	{
		console.log("%c%s", Pointer_stringify(szStyle), Pointer_stringify(szContent));
	},
	AlertString: function(str)
	{
		window.alert(Pointer_stringify(str));
	},
});

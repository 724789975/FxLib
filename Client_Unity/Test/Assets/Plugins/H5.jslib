mergeInto(LibraryManager.library,
{
	GetUrl: function ()
	{
		window.alert(window.document.location.href);
		return window.document.location.href;
		//var gameInstance = UnityLoader.instantiate("gameContainer", "Build/view.json", {onProgress: UnityProgress});
		//window.alert(window.document.location.href);
		//gameInstance.SendMessage("GameManager", "Init", window.document.location.href);
	},
	H5LogStr: function(str)
	{
		console.log(str);
	},
});

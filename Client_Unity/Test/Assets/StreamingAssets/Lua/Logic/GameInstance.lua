xlua.hotfix(CS.GameInstance, 'Update', function(self)
	
end)

xlua.hotfix(CS.GameInstance, 'OnDestroy', function(self)
	CS.SampleDebuger.LogColorAquamarine("GameInstance OnDestroy")
	xlua.hotfix(CS.GameInstance, 'Update', nil)
	xlua.hotfix(CS.GameInstance, 'OnDestroy', nil)
	xlua.hotfix(CS.GameInstance, 'OnApplicationQuit', nil)
end)

xlua.hotfix(CS.GameInstance, 'OnApplicationQuit', function(self)
	CS.SampleDebuger.LogColorAquamarine("GameInstance OnApplicationQuit")
	xlua.hotfix(CS.GameInstance, 'Update', nil)
	xlua.hotfix(CS.GameInstance, 'OnDestroy', nil)
	xlua.hotfix(CS.GameInstance, 'OnApplicationQuit', nil)
end)

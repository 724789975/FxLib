using UnityEngine;
using System.Collections;
using System.Diagnostics;

public class IPABuilder {

	public static string ProjectPath = Application.dataPath + "/../proj_ios/Unity-iPhone.xcodeproj";

	public static string appPath = Application.dataPath + "/../proj_ios/build/Release-iphoneos/heroesland.app ";

	public static string ipaPath =  Application.dataPath + "/../game.ipa";

	public static string CodeSing = "\"iPhone Distribution: shanghai Giant Tongping Network Technology Co., Ltd.\"";

	public static string provision = Application.dataPath + "/../IOSLicense/hero_distribution.mobileprovision";

	// Use this for initialization

	public static void buildIPA(){
	

		UnityEngine.Debug.Log ("Start !!!!!");

		if (!clearBuild ()) {
			UnityEngine.Debug.LogError (" clear build error");
			return;
		}

		if (!buildAPP ()) {
			UnityEngine.Debug.LogError (" build app error");
			return;
		}

		if (!generateIPA ()) {
			UnityEngine.Debug.LogError (" generate ipa error");
			return;
		}
	}

	public static bool clearBuild(){

		string args = " -verbose " + " -project " + ProjectPath + " -target " + " Unity-iPhone " + " -configuration " + " Release " + " clean";

		UnityEngine.Debug.Log (args); 

		return startProcess ("xcodebuild",args);
	}

	public static bool buildAPP(){

		string args =  " -verbose " + " -project "+ProjectPath + " -target " + " Unity-iPhone " + " -configuration " + " Release " + " -jobs 8";

		UnityEngine.Debug.Log (args);



		return startProcess ("xcodebuild",args);
	}

	public static bool generateIPA(){
	
		string args =  " -sdk iphoneos PackageApplication -v " + appPath + " -o " + ipaPath + " --sign " + CodeSing + " --embed " + provision;
		
		UnityEngine.Debug.Log (args);

		return startProcess ("xcrun",args);

	}

	public static bool startProcess(string cmd, string args){

		try  
		{  
			Process process = new Process();  
			 
			ProcessStartInfo startInfo = new ProcessStartInfo(cmd, args);  

			process.StartInfo = startInfo;  


			process.StartInfo.UseShellExecute = false;

			process.StartInfo.RedirectStandardError = true;
			process.StartInfo.RedirectStandardOutput = true;

			process.StartInfo.WindowStyle = ProcessWindowStyle.Normal; 

			process.Start();  

			System.IO.StreamReader output = process.StandardOutput;
			
			System.IO.StreamReader err = process.StandardError;

			string outputString = output.ReadToEnd();
			
			string errString = err.ReadToEnd();

			process.WaitForExit();  

			UnityEngine.Debug.Log(outputString);

			if(errString != null && errString.Length != 0)
				UnityEngine.Debug.LogError(errString);

			output.Close();

			err.Close();

			process.Close();  
		}  
		catch (System.Exception e)  
		{  
			UnityEngine.Debug.Log(e);  

			return false;
		}  

		return true;

	}


}
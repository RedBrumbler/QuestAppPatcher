#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <array>
#include <filesystem>

using namespace std;

#define LIBPATH "lib/arm64-v8a/"

std::string exec(const char* cmd) {
    cout << "running cmd:[" << cmd << "]" << endl;
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::vector<std::string> linesToVector(std::string text)
{
    // turns a piece of text seperated by newlines into a vector of strings
    vector<string> result = {};
    string current = "";    
    for (auto c : text)
    {
        if (c == '\n' || c == '\0')
        {
            if (c == '\0') break;
            result.push_back(current);
            current = "";
            continue;
        }
        current += c;
    }
     return result;
}

bool isInstalled(vector<string>& list, string target)
{
    for (string p : list)
    {
        p.erase(0, p.find_first_of(':') + 1);
        if (p.find('\n') != string::npos) p.erase(p.find_first_of('\n'));
        if (p == target) return true;
    }
    return false;
}

void checkError(string text)
{
    if (text.find("error") != string::npos) 
    {
        cout << text << endl;
        exit(0);
    }
}

string decompile(string apkName)
{
    string cmd = "java -Xmx1024m -jar apktool_2.5.0.jar d -f -o \"./workingDir/" + apkName + "\" \"workingDir/" + apkName + ".apk\"";
    return exec(cmd.c_str());
}

string compile(string apkName)
{
    string cmd = "java -Xmx1024m -jar apktool_2.5.0.jar b -f -o \"workingDir/" + apkName + ".apk\" \"./workingDir/" + apkName + "\"";
    return exec(cmd.c_str());
}

string sign(string apkName)
{
    string cmd = "java -Xmx1024m -jar uber-apk-signer-1.2.1.jar --apks \"workingDir/" + apkName + ".apk\"";
    return exec(cmd.c_str());
}

string install(string apkName)
{
    string cmd = "adb install workingDir/" + apkName + ".apk";
    return exec(cmd.c_str());
}

string uninstall(string apkName)
{
    string cmd = "adb uninstall " + apkName;
    return exec(cmd.c_str());
}

string modManifest(string manifest)
{
    // adds the file perm requirements into the manifest
    string writePerms = "<uses-permission android:name=\"android.permission.WRITE_EXTERNAL_STORAGE\"/>";
    string readPerms = "<uses-permission android:name=\"android.permission.READ_EXTERNAL_STORAGE\"/>";
    string newManifest = manifest.substr(0, manifest.find_first_of('\n')) + "\n";
    if (manifest.find(writePerms) == string::npos)
    {
        newManifest += "    " + writePerms + "\n";
    }

    if (manifest.find(readPerms) == string::npos)
    {
        newManifest += "    " + readPerms + "\n";
    }

    newManifest += manifest.substr(manifest.find_first_of('\n') + 1);
    return newManifest;
}

void copyFile(string src, string dst)
{
    cout << "copying file from \n\t[" << src << "]\nto\n\t[" << dst << "]" << endl;
    if (filesystem::exists(dst.c_str())) remove(dst.c_str());
    std::filesystem::copy(src.c_str(), dst.c_str());
}

int main()
{
    string target = "invalid";
    cout << "Please give the package name you wish to patch: ";
    cin >> target;
    // checking if the target app for modding is installed
    std::string text = exec("adb shell pm list packages");
    checkError(text);

    vector<string> vector = linesToVector(text);

    if (!isInstalled(vector, target))
    {
        cout << "ERROR: " + target + " was not installed" << endl;
        exit(0);
    }

    // get the app path so we can get it
    std::string cmd = "adb shell pm path " + target;
    std::string appPath = exec(cmd.c_str());
    appPath.erase(0, appPath.find_first_of(':') + 1);
    appPath.erase(appPath.find_first_of('\n'));

    // make a temp directory
    cmd = "mkdir workingDir";
    string output = exec(cmd.c_str());

    // pull the apk onto the current pc
    cmd = "adb pull " + appPath + " ./workingDir/" + target + ".apk";
    output = exec(cmd.c_str());

    // decompile the apk so we can change files in it
    output = decompile(target);

    // copy the .so files into the libs dir
    copyFile("extraFiles/libmain.so", "workingDir/" + target + "/" + LIBPATH + "libmain.so");
    copyFile("extraFiles/libmodloader.so", "workingDir/" + target + "/" + LIBPATH + "libmodloader.so");

    // read the manifest and make it ask for storage perms
    string manifestPath = ".\\workingDir\\" + target + "\\AndroidManifest.xml";
    cmd = "type " + manifestPath;

    output = exec(cmd.c_str());
    
    // actually mod it
    string newManifest = modManifest(output);

    // write to file
    FILE *fp;
    fp = fopen(manifestPath.c_str(),"w");
    fprintf(fp, "%s", newManifest.c_str());
    fclose(fp);

    // compile the apk
    output = compile(target);

    // sign the apk
    output = sign(target);

    // uninstall the apk
    output = uninstall(target);
    
    // move the old app into a different dir so we can rename the other one
    cmd = "move \".\\workingDir\\" + target + ".apk\" \".\\workingDir\\" + target + "\\" + target + ".apk\""; 
    output = exec(cmd.c_str());
    
    // rename signed apk
    cmd = "move \".\\workingDir\\" + target + "-aligned-debugSigned.apk\" \".\\workingDir\\" + target + ".apk\""; 
    output = exec(cmd.c_str());

    // install app
    output = install(target);

    // remove working Dir
    cmd = "RD /S /Q \"workingDir\""; 
    output = exec(cmd.c_str());
    
    return 0;
}
# Manifest Files
Simply a json object defining a few important properties to allow `KZP` Modules to be fetched.
An example might be all you need, and then reference the complete set of properties below. 
```json
{
    "name":"My Module Name",
    "folder":"ModuleName",
    "type":"qml",
    "version":1,
    "vtext":"v0.01",
    "icon":"module_icon_name.png",
    "settings":"sub/path_if_needed/ModuleSettingsFileName.qml",
    "entry":"the_file_to_be_loaded_as_root.qml",
    "files":["some_other_file.qml","settings_helper.qml","my_image1.png", "my_gif.gif", "settings.json"],
    "dependencies":["some url/manifest.json", "another url/manifest.json"],
    "optional":["optional url/manifest.json", "optional2 url/manifest.json"]
}
```

## **name** `(required)`
Defines the modules' user visible name. Can be up to 80 characters, anything longer will be truncated.

## **folder** `(required)`
Defines the modules' folder name (in the file structure). Can be up to 64 characters, anything longer will be truncated.

## **url** `(required)`
Where the package will come from (what is the root path). Example
```
https://raw.githubusercontent.com/Tpimp/KZP_Carousel/main/
```

## **type** `(required)`
Needs to be of valid types that KZP can handle. 
- `qml` is for a qml application
- `gif` is for an animation
- `image` is for static image content
- `repo` is for a repository of manifest files (see optional)

## **version** `(required)`
A 32bit **integer** version number, max **2,147,483,647**.
 - Use as you wish, but always increment. 
    - `-18 -> 0 -> 2 -> 3`  or  `1 -> 22 -> 33 -> 55 -> 1999` 
    - **doesnt' matter but each version needs to be larger**
  ```c++
  // Update version logic
  if (currentVersion <  repoVersion)
  ```

## **entry** `(required)`
The file path (could be relative or absolute) will point to a valid Qml file. The qml file will be loaded as the root element

## **dependencies** `(required, if you have dependencies 😉)`
A is a list of absolute paths to other manifest files that are required by this module. The manifest files linked will be checked against the installed set of modules and if the dependencies do not exist, an attempt to install them will be made.

## **files**
Files list any file not explicitly named before `(entry, settings, icon)`. If there are extra files related to images or internal qml files, list them here. Relative paths will have respect the directory structute but absolute paths will be placed in the root directory.

## **optional**
A list of optionally installed manifests. This can be used to create a "root repository" for Qml Apps. Point KZP to a manifest with optional manifests files, and it will present
them as "can be optionally" installed.

## **vtext**
Version text is displayed to the user . If it does not exist **version** integer will be converted to text.

## **icon**
Icon is the path to the icon that should be used. This path can be relative (path from current directory) or absolute (full http path).

## **settings**
The file path (could be relative or absolute) will point to a valid Qml file or be empty (app does not have a settings).



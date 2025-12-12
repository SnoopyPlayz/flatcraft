
workspace "HelloWorld"
   configurations { "Debug", "Release" }
   platforms { "Linux64", "Windows64" }
   defaultplatform "Windows64"
    
    filter "platforms:Windows64"
        architecture "x86_64"
        system "windows"
        toolset "mingw"
    
    filter "platforms:Linux64"
        architecture "x86_64" 
        system "linux"
        toolset "clang"

project "HelloWorld"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   location "build"

   includedirs {"lib/raylib/include"}
   includedirs {"include"}
   -- libdirs { "lib/raylib/lib" }
   --links { "raylib" }

   files { "include/**.hpp", "src/**.cpp", }

   filter "platforms:Windows64"
        libdirs { "lib/raylib_mingw/lib" }
        links { "raylib", "winmm", "gdi32", "opengl32" }
    
    filter "platforms:Linux64"
        libdirs { "lib/raylib/lib" }
        links { "raylib", "pthread", "dl", "m" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

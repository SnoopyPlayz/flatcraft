workspace "flatcraft"
   configurations { "Debug", "Release" }
   platforms { "Linux64", "Windows64" }
   defaultplatform "Linux64"
    
    filter "platforms:Windows64"
        architecture "x86_64"
        system "windows"
        toolset "clang"
    
    filter "platforms:Linux64"
        architecture "x86_64" 
        system "linux"
        toolset "clang"

project "flatcraft"
   kind "WindowedApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   location "build"

   includedirs {"lib/raylib/include"}
   includedirs {"lib/enet"}
   includedirs {"include"}

   files { "include/**.hpp", "src/**.cpp", }

   filter "platforms:Windows64"
   	--gccprefix "x86_64-w64-mingw32-"
	--linkoptions {"-static"}
	
	syslibdirs { "/usr/x86_64-w64-mingw32/lib" }
        libdirs { "lib/raylib_mingw/lib" }
        libdirs { "lib/enet_mingw/lib" }
	buildoptions { "--target=x86_64-w64-mingw32" }
        linkoptions  { "--target=x86_64-w64-mingw32", "-fuse-ld=lld", "-static"}
        links { "raylib", "enet", "ws2_32", "winmm", "pthread", "gdi32", "opengl32" }
    
    filter "platforms:Linux64"
        libdirs { "lib/raylib/lib" }
        libdirs { "lib/enet/lib" }
        links { "raylib", "enet", "pthread", "dl", "m" }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "Speed"
      symbols "Off"
      linktimeoptimization "On"
      linkoptions { "-s" }

workspace "flatcraft"
   configurations { "Debug", "Release" }
   platforms { "Linux64", "Windows64" }
    
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
   cppdialect "C++20"
   targetdir "bin/%{cfg.buildcfg}"
   location "build"

   externalincludedirs {"lib/raylib/include"}
   externalincludedirs {"lib/lz4/include"}
   externalincludedirs {"lib/enet"}
   externalincludedirs {"lib"}
   includedirs {"include"}

   files { "include/**.hpp", "src/**.cpp", }

   filter "platforms:Windows64"
   	--gccprefix "x86_64-w64-mingw32-"
	--linkoptions {"-static"}
	
	syslibdirs { "/usr/x86_64-w64-mingw32/lib" }
        libdirs { "lib/raylib_mingw/lib" }
        libdirs { "lib/enet_mingw/lib" }
        libdirs { "lib/lz4_mingw/lib" }
	buildoptions { "--target=x86_64-w64-mingw32", "-std=c++20"}
        linkoptions  { "--target=x86_64-w64-mingw32", "-fuse-ld=lld", "-static"}
        links { "raylib", "enet", "lz4", "ws2_32", "winmm", "pthread", "gdi32", "opengl32" }
    
    filter "platforms:Linux64"
        libdirs { "lib/raylib/lib" }
        libdirs { "lib/enet/lib" }
        libdirs { "lib/lz4/lib" }
        links { "raylib", "enet", "lz4", "pthread", "dl", "m" }

    filter "configurations:Debug"
      defines { "DEBUG" }
      optimize "Debug"
      symbols "On"
      warnings "Extra"
      filter { "system:linux" , "configurations:Debug" }
          buildoptions { "-fsanitize=address", "-fno-omit-frame-pointer" }
          linkoptions { "-fsanitize=address" }
      filter{}

    filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "Speed"
      symbols "Off"
      linktimeoptimization "On"
      linkoptions { "-s" }

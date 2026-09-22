$root = $PSScriptRoot
if (-not $root) { $root = Split-Path -Parent $MyInvocation.MyCommand.Path }

$mingw = "$root\tools\mingw\mingw64\bin"
$gxx   = "$mingw\g++.exe"
$gcc   = "$mingw\gcc.exe"
$ar    = "$mingw\ar.exe"
$src   = "$root\src"
$out   = "$root\build\microlight.exe"
$tp    = "$root\third_party"
$obj   = "$root\build\obj"
$luaSrc = "$tp\lua"
$ufbxSrc = "$tp\ufbx"
$ufbxObj = "$obj\ufbx.o"
$luaLib = "$root\build\liblua.a"

$env:Path = "$mingw;$env:Path"

if (-not (Test-Path $obj)) {
    New-Item -ItemType Directory -Force -Path $obj | Out-Null
}

if (-not (Test-Path $luaLib)) {
    Write-Host "Lua build..." -ForegroundColor Cyan
    $luaFiles = Get-ChildItem "$luaSrc\*.c" | Where-Object {
        $_.Name -notin @("lua.c", "luac.c", "onelua.c")
    } | ForEach-Object { $_.FullName }
    $luaObjs = @()
    foreach ($f in $luaFiles) {
        $base = [System.IO.Path]::GetFileNameWithoutExtension($f)
        $o = "$obj\lua_$base.o"
        $luaObjs += $o
        if (-not (Test-Path $o)) {
            & $gcc -O2 -c -I"$luaSrc" -DLUA_USE_WINDOWS $f -o $o
            if ($LASTEXITCODE -ne 0) { exit 1 }
        }
    }
    & $ar rcs $luaLib @luaObjs
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "Lua built" -ForegroundColor Green
}

# === ufbx (C-библиотека) ===
if (-not (Test-Path $ufbxObj)) {
    Write-Host "ufbx build..." -ForegroundColor Cyan
    & $gcc -O2 -c -I"$ufbxSrc" -w "$ufbxSrc\ufbx.c" -o $ufbxObj
    if ($LASTEXITCODE -ne 0) { exit 1 }
    Write-Host "ufbx built" -ForegroundColor Green
}

Write-Host "Building engine..." -ForegroundColor Cyan

$files = @()
$files += (Get-ChildItem "$src\*.cpp" -Recurse | ForEach-Object { $_.FullName })
$files += "$tp\glad\src\glad.c"

$includes = @(
    "-I$src", "-I$src\core", "-I$src\ui", "-I$src\scripting",
    "-I$tp", "-I$tp\glad\include", "-I$tp\glm", "-I$tp\stb",
    "-I$tp\miniaudio", "-I$tp\tinyobj", "-I$tp\lua", "-I$tp\ufbx"
)

& $gxx -std=c++17 -O2 -Wall @includes $files $luaLib $ufbxObj -o $out -static -static-libgcc -static-libstdc++ -lopengl32 -lgdi32 -luser32 -lwinmm -lole32

if ($LASTEXITCODE -eq 0) {
    Write-Host "OK -> $out" -ForegroundColor Green
    Remove-Item "$root\build\shaders" -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item "$root\build\assets"  -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item "$root\build\game"    -Recurse -Force -ErrorAction SilentlyContinue
    Copy-Item "$root\shaders" "$root\build\shaders" -Recurse -Force
    Copy-Item "$root\assets"  "$root\build\assets"  -Recurse -Force
    Copy-Item "$root\game"    "$root\build\game"    -Recurse -Force
    Write-Host "Running..." -ForegroundColor Cyan
    Set-Location "$root\build"
    & $out
    Set-Location $root
} else {
    Write-Host "Build failed" -ForegroundColor Red
}

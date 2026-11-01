# ENGINEX Workbench - Standalone Turnkey Packaging Script
# PASS Science Works R&D Internship 2026-2027 Evaluation

$ErrorActionPreference = 'Stop'

Write-Host '================================================================='
Write-Host '  ENGINEX Workbench - Packaging Release ZIP'
Write-Host '================================================================='

$ProjectRoot = (Get-Item $PSScriptRoot).Parent.Parent.FullName
Set-Location $ProjectRoot

$BuildDir = Join-Path $ProjectRoot 'build'
$DistDir = Join-Path $ProjectRoot 'dist'
$BundleDir = Join-Path $DistDir 'CandidateID_Workbench'
$ZipFile = Join-Path $DistDir 'CandidateID_Workbench.zip'
$CompilerCacheFile = Join-Path $BuildDir 'CMakeCache.txt'

$compilerEntry = Select-String -Path $CompilerCacheFile -Pattern '^CMAKE_CXX_COMPILER(?::[^=]+)?=(.+)$' | Select-Object -First 1
if (-not $compilerEntry) {
    throw "Could not determine CMAKE_CXX_COMPILER from $CompilerCacheFile"
}
if ($compilerEntry.Line -match '^CMAKE_CXX_COMPILER(?::[^=]+)?=(.+)$') {
    $CompilerPath = $Matches[1].Trim().Replace('/', '\')
} else {
    throw "Could not parse CMAKE_CXX_COMPILER from $CompilerCacheFile"
}
if (-not (Test-Path -LiteralPath $CompilerPath -PathType Leaf)) {
    throw "Configured C++ compiler not found: $CompilerPath"
}
$CompilerBinDir = Split-Path -Parent $CompilerPath
$ObjdumpPath = Join-Path $CompilerBinDir 'objdump.exe'
if (-not (Test-Path -LiteralPath $ObjdumpPath -PathType Leaf)) {
    throw "objdump.exe was not found beside the configured C++ compiler: $CompilerBinDir"
}
$WindowsSystemDir = Join-Path $env:WINDIR 'System32'

function Get-PeImports {
    param([Parameter(Mandatory = $true)][string]$FilePath)

    $output = & $ObjdumpPath -p $FilePath
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to inspect executable dependencies: $FilePath"
    }

    return @($output | ForEach-Object {
        if ($_ -match '^\s*DLL Name:\s*(\S+)\s*$') {
            $Matches[1]
        }
    } | Sort-Object -Unique)
}

function Test-WindowsSystemImport {
    param([Parameter(Mandatory = $true)][string]$ModuleName)

    if ($ModuleName -match '^(api-ms-win-|ext-ms-win-)') {
        return $true
    }
    return Test-Path -LiteralPath (Join-Path $WindowsSystemDir $ModuleName) -PathType Leaf
}

function Get-CompilerRuntimeDependencies {
    param([Parameter(Mandatory = $true)][string]$ExecutablePath)

    $pending = New-Object 'System.Collections.Generic.Queue[string]'
    $pending.Enqueue((Get-Item -LiteralPath $ExecutablePath).FullName)
    $dependencies = @{}

    while ($pending.Count -gt 0) {
        $binaryPath = $pending.Dequeue()
        foreach ($moduleName in (Get-PeImports -FilePath $binaryPath)) {
            if (Test-WindowsSystemImport -ModuleName $moduleName) {
                continue
            }

            $runtimePath = Join-Path $CompilerBinDir $moduleName
            if (-not (Test-Path -LiteralPath $runtimePath -PathType Leaf)) {
                throw "Dependency '$moduleName' required by '$binaryPath' is neither a Windows system DLL nor present beside the configured compiler: $runtimePath"
            }

            $fullRuntimePath = (Get-Item -LiteralPath $runtimePath).FullName
            $key = $moduleName.ToLowerInvariant()
            if (-not $dependencies.ContainsKey($key)) {
                $dependencies[$key] = $fullRuntimePath
                $pending.Enqueue($fullRuntimePath)
            }
        }
    }

    return @($dependencies.Values | Sort-Object)
}

function Assert-PackagedDependencies {
    param([Parameter(Mandatory = $true)][string]$ExecutablePath)

    $pending = New-Object 'System.Collections.Generic.Queue[string]'
    $pending.Enqueue((Get-Item -LiteralPath $ExecutablePath).FullName)
    $visited = @{}

    while ($pending.Count -gt 0) {
        $binaryPath = $pending.Dequeue()
        if ($visited.ContainsKey($binaryPath)) {
            continue
        }
        $visited[$binaryPath] = $true

        foreach ($moduleName in (Get-PeImports -FilePath $binaryPath)) {
            if (Test-WindowsSystemImport -ModuleName $moduleName) {
                continue
            }

            $localDependency = Join-Path (Split-Path -Parent $binaryPath) $moduleName
            if (-not (Test-Path -LiteralPath $localDependency -PathType Leaf)) {
                throw "Packaged executable dependency '$moduleName' was not found beside '$binaryPath'"
            }
            $pending.Enqueue((Get-Item -LiteralPath $localDependency).FullName)
        }
    }
}

$LaunchSource = Join-Path $BuildDir 'bin\Launch.exe'
$BackendSource = Join-Path $BuildDir 'bin\backend_server.exe'
$WebSource = Join-Path $ProjectRoot 'frontend\out'
$RequiredSources = @(
    $LaunchSource,
    $BackendSource,
    (Join-Path $ProjectRoot 'data\geometry\sample_cube.stl'),
    (Join-Path $ProjectRoot 'data\temperature\synthetic_ansys_sample.csv'),
    (Join-Path $PSScriptRoot 'README_DIST.txt')
)
foreach ($source in $RequiredSources) {
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
        throw "Required packaging input was not found: $source"
    }
}
if (-not (Test-Path -LiteralPath $WebSource -PathType Container)) {
    throw "Static frontend export was not found: $WebSource"
}

$LaunchRuntimeDependencies = Get-CompilerRuntimeDependencies -ExecutablePath $LaunchSource
$BackendRuntimeDependencies = Get-CompilerRuntimeDependencies -ExecutablePath $BackendSource

Write-Host "[INFO] Using configured compiler: $CompilerPath"
Write-Host "[INFO] Using compiler dependency inspector: $ObjdumpPath"

if (-not (Test-Path -LiteralPath $DistDir -PathType Container)) {
    New-Item -ItemType Directory -Force -Path $DistDir | Out-Null
}
if (Test-Path -LiteralPath $BundleDir) {
    Write-Host '[INFO] Replacing previous ENGINEX distribution folder...'
    Remove-Item -LiteralPath $BundleDir -Recurse -Force
}
if (Test-Path -LiteralPath $ZipFile) {
    Remove-Item -LiteralPath $ZipFile -Force
}

New-Item -ItemType Directory -Force -Path $BundleDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'bin') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'data\geometry') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'data\temperature') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'results\csv') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $BundleDir 'results\benchmarks') | Out-Null

Write-Host '[INFO] Copying Launch.exe...'
Copy-Item -LiteralPath $LaunchSource -Destination (Join-Path $BundleDir 'Launch.exe')
foreach ($dependency in $LaunchRuntimeDependencies) {
    Copy-Item -LiteralPath $dependency -Destination $BundleDir
}

Write-Host '[INFO] Copying backend_server.exe...'
Copy-Item -LiteralPath $BackendSource -Destination (Join-Path $BundleDir 'bin\backend_server.exe')
foreach ($dependency in $BackendRuntimeDependencies) {
    Copy-Item -LiteralPath $dependency -Destination (Join-Path $BundleDir 'bin')
}

Write-Host '[INFO] Copying static web export to web/...'
Copy-Item -Recurse -LiteralPath $WebSource -Destination (Join-Path $BundleDir 'web')

Write-Host '[INFO] Copying sample fixtures...'
Copy-Item -LiteralPath (Join-Path $ProjectRoot 'data\geometry\sample_cube.stl') -Destination (Join-Path $BundleDir 'data\geometry\sample_cube.stl')
Copy-Item -LiteralPath (Join-Path $ProjectRoot 'data\temperature\synthetic_ansys_sample.csv') -Destination (Join-Path $BundleDir 'data\temperature\synthetic_ansys_sample.csv')

Write-Host '[INFO] Copying reference CSV outputs...'
if (Test-Path (Join-Path $ProjectRoot 'results\csv\thermal_convergence.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\csv\thermal_convergence.csv') (Join-Path $BundleDir 'results\csv\thermal_convergence.csv')
}
if (Test-Path (Join-Path $ProjectRoot 'results\benchmarks\benchmark_results.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\benchmarks\benchmark_results.csv') (Join-Path $BundleDir 'results\benchmarks\benchmark_results.csv')
}
if (Test-Path (Join-Path $ProjectRoot 'results\csv\backpropagation_training.csv')) {
    Copy-Item (Join-Path $ProjectRoot 'results\csv\backpropagation_training.csv') (Join-Path $BundleDir 'results\csv\backpropagation_training.csv')
}

Write-Host '[INFO] Copying README.txt...'
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'README_DIST.txt') -Destination (Join-Path $BundleDir 'README.txt')

Write-Host '[INFO] Verifying packaged executable dependencies...'
Assert-PackagedDependencies -ExecutablePath (Join-Path $BundleDir 'Launch.exe')
Assert-PackagedDependencies -ExecutablePath (Join-Path $BundleDir 'bin\backend_server.exe')

Write-Host ('[INFO] Creating release archive: ' + $ZipFile + '...')
Compress-Archive -Path ($BundleDir + '\*') -DestinationPath $ZipFile -Force

$ZipSizeMB = [math]::Round((Get-Item $ZipFile).Length / 1MB, 2)
Write-Host '================================================================='
Write-Host ('  SUCCESS! Release package generated: ' + $ZipFile)
Write-Host ('  Package Size: ' + $ZipSizeMB + ' MB')
Write-Host '================================================================='
